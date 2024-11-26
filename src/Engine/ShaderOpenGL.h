#pragma once

#if RENDER_OPENGL

#include "ShaderCompiler.h"

class ShaderGL final
{
public:
	ShaderGL(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines);
	~ShaderGL();

	auto GetProgram() const { return m_program; }

private:
	GLuint m_program;
	ShaderReflection m_vertRefl;
	ShaderReflection m_fragRefl;

	struct {
		bool es;
		uint32_t version;
		bool enable_420pack_extension;
		bool force_flattened_io_blocks;
	} options;
};

#endif // RENDER_OPENGL