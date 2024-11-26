#include "stdafx.h"
#if RENDER_OPENGL
#include "ShaderOpenGL.h"
//=============================================================================
ShaderGL::ShaderGL(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines)
{
	defines.push_back("FLIP_TEXCOORD_Y");

	auto vertex_shader_spirv = CompileGlslToSpirv(ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = CompileGlslToSpirv(ShaderStage::Fragment, fragment_code, defines);

#if defined(PLATFORM_WINDOWS)
	options.es = false;
	options.version = 450;
	options.enable_420pack_extension = true;
	options.force_flattened_io_blocks = true;
#elif defined(PLATFORM_EMSCRIPTEN)
	options.es = true;
	options.version = 300;
	options.enable_420pack_extension = false;
	options.force_flattened_io_blocks = false;
#endif

	auto glsl_vert = CompileSpirvToGlsl(vertex_shader_spirv, options.es, options.version, options.enable_420pack_extension, options.force_flattened_io_blocks);

	auto glsl_frag = CompileSpirvToGlsl(fragment_shader_spirv, options.es, options.version, options.enable_420pack_extension, options.force_flattened_io_blocks);

	auto throw_error = [](auto shader, auto get_length_func, auto get_info_log_func) {
		GLint length = 0;
		get_length_func(shader, GL_INFO_LOG_LENGTH, &length);
		std::string str;
		str.resize(length);
		get_info_log_func(shader, length, &length, &str[0]);
		throw std::runtime_error(str);
		};

	auto compile_shader = [&](auto type, const std::string& glsl) {
		auto shader = glCreateShader(type);
		auto v = glsl.c_str();
		glShaderSource(shader, 1, &v, NULL);
		glCompileShader(shader);

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE)
			throw_error(shader, glGetShaderiv, glGetShaderInfoLog);

		return shader;
		};

	auto vertexShader = compile_shader(GL_VERTEX_SHADER, glsl_vert);
	auto fragmentShader = compile_shader(GL_FRAGMENT_SHADER, glsl_frag);

	m_program = glCreateProgram();
	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);

	GLint link_status = 0;
	glGetProgramiv(m_program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE)
		throw_error(m_program, glGetProgramiv, glGetProgramInfoLog);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_vertRefl = MakeSpirvReflection(vertex_shader_spirv);
	m_fragRefl = MakeSpirvReflection(fragment_shader_spirv);

	bool need_fix_bindings =
		(options.es && options.version <= 300) ||
		(!options.es && options.version < 420 && !options.enable_420pack_extension);

	if (need_fix_bindings)
	{
		auto for_each_descriptor_binding = [&](auto type, std::function<void(uint32_t binding, const ShaderReflection::Descriptor& descriptor)> callback) {
			for (const auto& reflection : { m_vertRefl, m_fragRefl })
			{
				if (!reflection.typed_descriptor_bindings.contains(type))
					continue;

				for (const auto& [binding, descriptor] : reflection.typed_descriptor_bindings.at(type))
				{
					callback(binding, descriptor);
				}
			};
			};
		for_each_descriptor_binding(ShaderReflection::DescriptorType::UniformBuffer, [&](auto binding, const auto& descriptor) {
			auto block_index = glGetUniformBlockIndex(m_program, descriptor.type_name.c_str());
			glUniformBlockBinding(m_program, block_index, binding);
			});
		GLint prevProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
		glUseProgram(m_program);
		for_each_descriptor_binding(ShaderReflection::DescriptorType::CombinedImageSampler, [&](auto binding, const auto& descriptor) {
			auto location = glGetUniformLocation(m_program, descriptor.name.c_str());
			glUniform1i(location, binding);
			});
		glUseProgram(prevProgram);
	}
}
//=============================================================================
ShaderGL::~ShaderGL()
{
	glDeleteProgram(m_program);
}
//=============================================================================
#endif // RENDER_OPENGL