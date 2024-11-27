#pragma once

#if RENDER_OPENGL

#include "RenderCoreOpenGL.h"
#include "TextureOpenGL.h"
#include "RenderTargetOpenGL.h"
#include "ShaderOpenGL.h"
#include "BufferOpenGL.h"
#include "PipelineStateOpenGL.h"

class RenderContext final
{
public:
	bool Create();
	void Destroy();

	int max_vertex_attribs;

	std::vector<uint32_t> draw_buffers;

	uint32_t width = 0;
	uint32_t height = 0;

	ExecuteList execute_after_present;

	std::unordered_map<uint32_t, TextureGL*> textures;
	std::unordered_set<uint32_t> dirty_textures;

	enum class SamplerType
	{
		Mipmap,
		NoMipmap
	};

	std::unordered_map<SamplerStateGL, std::unordered_map<SamplerType, GLuint>> sampler_states;
	SamplerStateGL sampler_state;

	std::vector<RenderTargetGL*> render_targets;

	GLuint pixel_buffer;
	GLuint vao;

	GLenum topology;
	ShaderGL* shader = nullptr;
	std::vector<VertexBufferGL*> vertex_buffers; // TODO: store pointer and count, not std::vector
	IndexBufferGL* index_buffer = nullptr;
	std::optional<Viewport> viewport;
	std::optional<Scissor> scissor;
	FrontFace front_face = FrontFace::Clockwise;
	std::vector<InputLayout> input_layouts;

	bool shader_dirty = false;
	bool vertex_array_dirty = false;
	bool index_buffer_dirty = false;
	bool viewport_dirty = true;
	bool scissor_dirty = true;
	bool sampler_state_dirty = true;
	bool front_face_dirty = true;

	uint32_t GetBackbufferWidth();
	uint32_t GetBackbufferHeight();
	PixelFormat GetBackbufferFormat();
};

void EnsureGraphicsState(bool draw_indexed);

#endif // RENDER_OPENGL