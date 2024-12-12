#include "stdafx.h"
#if RENDER_OPENGL
#include "ContextOpenGL.h"
//=============================================================================
bool RenderContext::Create()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &pixel_buffer);

	int max_draw_buffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	for (int i = 0; i < max_draw_buffers; i++)
	{
		draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

	return true;
}
//=============================================================================
void RenderContext::Destroy()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &pixel_buffer);

	for (const auto& [state, objects_map] : sampler_states)
	{
		for (const auto& [type, object] : objects_map)
		{
			glDeleteSamplers(1, &object);
		}
	}
}
//=============================================================================
uint32_t RenderContext::GetBackBufferWidth()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetWidth() : width;
}
//=============================================================================
uint32_t RenderContext::GetBackBufferHeight()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetHeight() : height;
}
//=============================================================================
PixelFormat RenderContext::GetBackbufferFormat()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetFormat() : PixelFormat::RGBA8UNorm;
}
//=============================================================================
void EnsureScissor()
{
	if (!gContext.scissor_dirty)
		return;

	gContext.scissor_dirty = false;

	const auto& scissor = gContext.scissor;

	if (!scissor.has_value())
	{
		glDisable(GL_SCISSOR_TEST);
		return;
	}

	glEnable(GL_SCISSOR_TEST);

	auto x = (GLint)glm::round(scissor->position.x);
	auto y = (GLint)glm::round(gContext.height - scissor->position.y - scissor->size.y); // TODO: need different calculations when render target
	auto width = (GLint)glm::round(scissor->size.x);
	auto height = (GLint)glm::round(scissor->size.y);
	glScissor(x, y, width, height);
}
//=============================================================================
void EnsureDepthMode()
{
	if (!gContext.depth_mode_dirty)
		return;

	gContext.depth_mode_dirty = false;

	const auto& depth_mode = gContext.depth_mode;

	if (!depth_mode.has_value())
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(ComparisonFuncMap.at(depth_mode->func));
	glDepthMask(depth_mode->writeMask);
}
//=============================================================================
void EnsureGraphicsState(bool draw_indexed)
{
	if (gContext.shaderDirty)
	{
		glUseProgram(gContext.shader->GetProgram());
		gContext.vertex_array_dirty = true;
		gContext.index_buffer_dirty = draw_indexed;
		gContext.shaderDirty = false;
	}

	if (gContext.index_buffer_dirty && draw_indexed)
	{
		gContext.index_buffer_dirty = false;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gContext.index_buffer->GetGLBuffer());
	}

	if (gContext.vertex_array_dirty)
	{
		gContext.vertex_array_dirty = false;

		std::unordered_set<uint32_t> active_locations;

		for (size_t i = 0; i < gContext.vertex_buffers.size(); i++)
		{
			auto vertex_buffer = gContext.vertex_buffers.at(i);
			auto stride = (GLsizei)vertex_buffer->GetStride();

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->GetGLBuffer());

			const auto& input_layout = gContext.inputLayouts.at(i);

			for (const auto& [location, attribute] : input_layout.attributes)
			{
				active_locations.insert(location);

				auto index = (GLuint)location;
				auto size = (GLint)VertexFormatSizeMap.at(attribute.format);
				auto type = (GLenum)VertexFormatTypeMap.at(attribute.format);
				auto normalized = (GLboolean)VertexFormatNormalizeMap.at(attribute.format);
				auto pointer = (void*)attribute.offset;
				glVertexAttribPointer(index, size, type, normalized, stride, pointer);
				glVertexAttribDivisor(index, input_layout.rate == InputLayout::Rate::Vertex ? 0 : 1);
			}
		}

		for (int i = 0; i < gContext.max_vertex_attribs; i++)
		{
			if (active_locations.contains(i))
				glEnableVertexAttribArray(i);
			else
				glDisableVertexAttribArray(i);
		}
	}

	for (auto binding : gContext.dirty_textures)
	{
		auto texture = gContext.textures.at(binding);

		glActiveTexture(GL_TEXTURE0 + binding);
		glBindTexture(GL_TEXTURE_2D, texture->GetGLTexture());
	}

	gContext.dirty_textures.clear();

	if (gContext.samplerStateDirty)
	{
		gContext.samplerStateDirty = false;

		const auto& value = gContext.samplerState;

		if (!gContext.samplerStates.contains(value))
		{
			const static std::unordered_map<Sampler, std::unordered_map<RenderContext::SamplerType, GLint>> SamplerMap = {
				{ Sampler::Nearest, {
					{ RenderContext::SamplerType::Mipmap, GL_NEAREST_MIPMAP_NEAREST },
					{ RenderContext::SamplerType::NoMipmap, GL_NEAREST },
				} },
				{ Sampler::Linear, {
					{ RenderContext::SamplerType::Mipmap, GL_LINEAR_MIPMAP_LINEAR },
					{ RenderContext::SamplerType::NoMipmap, GL_LINEAR },
				} },
			};

			const static std::unordered_map<TextureAddress, GLint> TextureAddressMap = {
				{ TextureAddress::Clamp, GL_CLAMP_TO_EDGE },
				{ TextureAddress::Wrap, GL_REPEAT },
				{ TextureAddress::MirrorWrap, GL_MIRRORED_REPEAT }
			};

			std::unordered_map<RenderContext::SamplerType, GLuint> sampler_state_map;

			for (auto sampler_type : { RenderContext::SamplerType::Mipmap, RenderContext::SamplerType::NoMipmap })
			{
				GLuint sampler_object;
				glGenSamplers(1, &sampler_object);

				glSamplerParameteri(sampler_object, GL_TEXTURE_MIN_FILTER, SamplerMap.at(value.sampler).at(sampler_type));
				glSamplerParameteri(sampler_object, GL_TEXTURE_MAG_FILTER, SamplerMap.at(value.sampler).at(RenderContext::SamplerType::NoMipmap));
				glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_S, TextureAddressMap.at(value.textureAddress));
				glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_T, TextureAddressMap.at(value.textureAddress));

				sampler_state_map.insert({ sampler_type, sampler_object });
			}

			gContext.samplerStates.insert({ value, sampler_state_map });
		}

		for (auto [binding, texture_handle] : gContext.textures)
		{
			bool texture_has_mips = ((TextureGL*)texture_handle)->GetMipCount() > 1;
			auto sampler_type = texture_has_mips ? RenderContext::SamplerType::Mipmap : RenderContext::SamplerType::NoMipmap;
			auto sampler = gContext.samplerStates.at(value).at(sampler_type);
			glBindSampler(binding, sampler);
		}
	}

	if (gContext.front_face_dirty)
	{
		gContext.front_face_dirty = false;

		static const std::unordered_map<FrontFace, GLenum> FrontFaceMap = {
			{ FrontFace::Clockwise, GL_CW },
			{ FrontFace::CounterClockwise, GL_CCW }
		};

		glFrontFace(FrontFaceMap.at(gContext.front_face));
	}

	if (gContext.viewportDirty)
	{
		gContext.viewportDirty = false;

		auto width = static_cast<float>(gContext.GetBackBufferWidth());
		auto height = static_cast<float>(gContext.GetBackBufferHeight());

		auto viewport = gContext.viewport.value_or(Viewport{ { 0.0f, 0.0f }, { width, height } });

		glViewport(
			(GLint)viewport.position.x,
			(GLint)viewport.position.y,
			(GLint)viewport.size.x,
			(GLint)viewport.size.y);

#if PLATFORM_WINDOWS
		glDepthRange((GLclampd)viewport.minDepth, (GLclampd)viewport.maxDepth);
#elif PLATFORM_EMSCRIPTEN
		glDepthRangef((GLfloat)viewport.minDepth, (GLfloat)viewport.maxDepth);
#endif
	}

	EnsureScissor();
	EnsureDepthMode();
}
//=============================================================================
#endif // RENDER_OPENGL