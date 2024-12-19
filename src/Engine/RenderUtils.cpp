#include "stdafx.h"
#include "RenderUtils.h"
#include "RenderSystem.h"
#include "Log.h"

const std::string utils::effects::BasicEffect::VertexShaderCode = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;
layout(location = TEXCOORD_LOCATION) in vec2 aTexCoord;
layout(location = NORMAL_LOCATION) in vec3 aNormal;
layout(location = TANGENT_LOCATION) in vec3 aTangent;

layout(binding = SETTINGS_UNIFORM_BINDING) uniform _settings
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec3 eye_position;
	float mipmap_bias;
	vec4 color;
	uint has_normal_texture;
} settings;

layout(location = 0) out struct
{
	vec3 world_position;
	vec4 color;
	vec2 tex_coord;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} Out;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.world_position = vec3(settings.model * vec4(aPosition, 1.0));
	Out.normal = transpose(inverse(mat3(settings.model))) * aNormal;
	Out.tangent = transpose(inverse(mat3(settings.model))) * aTangent;
	Out.bitangent = cross(Out.normal, Out.tangent);
	Out.color = aColor;
	Out.tex_coord = aTexCoord;
#ifdef FLIP_TEXCOORD_Y
	Out.tex_coord.y = 1.0 - Out.tex_coord.y;
#endif
	gl_Position = settings.projection * settings.view * settings.model * vec4(aPosition, 1.0);
})";

const std::string utils::effects::BasicEffect::FragmentShaderCode = R"(
#version 450 core

layout(binding = SETTINGS_UNIFORM_BINDING) uniform _settings
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec3 eye_position;
	float mipmap_bias;
	vec4 color;
	uint has_normal_texture;
} settings;

layout(location = 0) in struct
{
	vec3 world_position;
	vec4 color;
	vec2 tex_coord;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} In;

layout(binding = COLOR_TEXTURE_BINDING) uniform sampler2D sColorTexture;

#ifdef EFFECT_FUNC
void EFFECT_FUNC(inout vec4);
#endif

vec4 GetBasicResult()
{	
	vec4 result = In.color;
	result *= settings.color;
	result *= texture(sColorTexture, In.tex_coord, settings.mipmap_bias);
	return result;
}

layout(location = 0) out vec4 result;

void main()
{
#ifdef EFFECT_FUNC
	EFFECT_FUNC(result);
#else
	result = GetBasicResult();
#endif
})";

static std::vector<std::string> ConcatDefines(std::vector<std::vector<std::string>> defines)
{
	auto result = std::vector<std::string>();
	for (const auto& strs : defines)
	{
		result.insert(result.end(), strs.begin(), strs.end());
	}
	return result;
}

static std::vector<std::string> MakeBasicEffectDefines()
{
	return ConcatDefines({ Mesh::Vertex::Defines, {
		"COLOR_TEXTURE_BINDING 0",
		"SETTINGS_UNIFORM_BINDING 2"
	} });
}

static std::vector<std::string> MakeEffectDefines()
{
	return ConcatDefines({ utils::effects::BasicEffect::Defines, {
		"NORMAL_TEXTURE_BINDING 1",
		"EFFECT_UNIFORM_BINDING 3",
		"EFFECT_FUNC effect"
	} });
}

const std::vector<std::string> utils::effects::BasicEffect::Defines = MakeBasicEffectDefines();

const std::vector<std::string> utils::effects::forward_shading::DirectionalLight::Defines = MakeEffectDefines();

const std::string utils::effects::forward_shading::DirectionalLight::VertexShaderCode =
utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::forward_shading::DirectionalLight::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
} light;

layout(binding = NORMAL_TEXTURE_BINDING) uniform sampler2D sNormalTexture;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	vec3 normal;

	if (settings.has_normal_texture != 0)
	{
		normal = vec3(texture(sNormalTexture, In.tex_coord, settings.mipmap_bias));
		normal = normal * 2.0 - 1.0;
		normal = mat3(In.tangent, In.bitangent, In.normal) * normal;
		normal = normalize(normal);
	}
	else
	{
		normal = normalize(In.normal);
	}

	vec3 view_dir = normalize(settings.eye_position - In.world_position);
	vec3 light_dir = normalize(light.direction);

	float diff = max(dot(normal, -light_dir), 0.0);
	vec3 reflect_dir = reflect(light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);

	vec3 intensity = light.ambient + (light.diffuse * diff) + (light.specular * spec);

	result *= vec4(intensity, 1.0);
})";

const std::vector<std::string> utils::effects::forward_shading::PointLight::Defines = MakeEffectDefines();

const std::string utils::effects::forward_shading::PointLight::VertexShaderCode =
utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::forward_shading::PointLight::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float shininess;
} light;

layout(binding = NORMAL_TEXTURE_BINDING) uniform sampler2D sNormalTexture;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	vec3 normal;

	if (settings.has_normal_texture != 0)
	{
		normal = vec3(texture(sNormalTexture, In.tex_coord, settings.mipmap_bias));
		normal = normal * 2.0 - 1.0;
		normal = mat3(In.tangent, In.bitangent, In.normal) * normal;
		normal = normalize(normal);
	}
	else
	{
		normal = normalize(In.normal);
	}

	vec3 light_offset = light.position - In.world_position;

	float distance = length(light_offset);
	float linear_attn = light.linear_attenuation * distance;
	float quadratic_attn = light.quadratic_attenuation * (distance * distance);
	float attenuation = 1.0 / (light.constant_attenuation + linear_attn + quadratic_attn);

	vec3 light_dir = normalize(light_offset);
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	vec3 view_dir = normalize(settings.eye_position - In.world_position);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);

	vec3 intensity = light.ambient + (light.diffuse * diff) + (light.specular * spec);

	intensity *= attenuation;

	result *= vec4(intensity, 1.0);
})";

const std::vector<std::string> utils::effects::deferred_shading::DirectionalLight::Defines = MakeEffectDefines();

const std::string utils::effects::deferred_shading::DirectionalLight::VertexShaderCode =
utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::deferred_shading::DirectionalLight::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
} light;

layout(binding = 5) uniform sampler2D sColorBufferTexture;
layout(binding = 6) uniform sampler2D sNormalBufferTexture;
layout(binding = 7) uniform sampler2D sPositionsBufferTexture;

void effect(inout vec4 result)
{
	result = In.color;
	result *= settings.color;
	result *= texture(sColorBufferTexture, In.tex_coord);

	vec3 pixel_normal = vec3(texture(sNormalBufferTexture, In.tex_coord)) * 2.0 - 1.0;
	vec3 pixel_position = vec3(texture(sPositionsBufferTexture, In.tex_coord));

	vec3 view_dir = normalize(settings.eye_position - pixel_position);
	vec3 light_dir = normalize(light.direction);

	float diff = max(dot(pixel_normal, -light_dir), 0.0);
	vec3 reflect_dir = reflect(light_dir, pixel_normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);

	vec3 intensity = light.ambient + (light.diffuse * diff) + (light.specular * spec);

	result *= vec4(intensity, 1.0);
})";

const std::vector<std::string> utils::effects::deferred_shading::PointLight::Defines = MakeEffectDefines();

const std::string utils::effects::deferred_shading::PointLight::VertexShaderCode =
utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::deferred_shading::PointLight::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float shininess;
} light;

layout(binding = 5) uniform sampler2D sColorBufferTexture;
layout(binding = 6) uniform sampler2D sNormalBufferTexture;
layout(binding = 7) uniform sampler2D sPositionsBufferTexture;

void effect(inout vec4 result)
{
	result = In.color;
	result *= settings.color;
	result *= texture(sColorBufferTexture, In.tex_coord);

	vec3 pixel_normal = vec3(texture(sNormalBufferTexture, In.tex_coord)) * 2.0 - 1.0;
	vec3 pixel_position = vec3(texture(sPositionsBufferTexture, In.tex_coord));

	vec3 light_offset = light.position - pixel_position;

	float distance = length(light_offset);
	float linear_attn = light.linear_attenuation * distance;
	float quadratic_attn = light.quadratic_attenuation * (distance * distance);
	float attenuation = 1.0 / (light.constant_attenuation + linear_attn + quadratic_attn);

	vec3 light_dir = normalize(light_offset);
	float diff = max(dot(pixel_normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, pixel_normal);
	vec3 view_dir = normalize(settings.eye_position - pixel_position);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);

	vec3 intensity = light.ambient + (light.diffuse * diff) + (light.specular * spec);

	intensity *= attenuation;

	result *= vec4(intensity, 1.0);
})";

const std::vector<std::string> utils::effects::deferred_shading::ExtractGeometryBuffer::Defines = MakeEffectDefines();

const std::string utils::effects::deferred_shading::ExtractGeometryBuffer::VertexShaderCode =
utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::deferred_shading::ExtractGeometryBuffer::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
//layout(location = 0) out vec4 result; // color_buffer
layout(location = 1) out vec4 normal_buffer;
layout(location = 2) out vec4 positions_buffer;

layout(binding = NORMAL_TEXTURE_BINDING) uniform sampler2D sNormalTexture;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	if (settings.has_normal_texture != 0)
	{
		vec3 normal = vec3(texture(sNormalTexture, In.tex_coord, settings.mipmap_bias));
		normal = normal * 2.0 - 1.0;
		normal = mat3(In.tangent, In.bitangent, In.normal) * normal;
		normal = normalize(normal);
		normal_buffer = vec4(normal * 0.5 + 0.5, 1.0);
	}
	else
	{
		vec3 normal = normalize(In.normal);
		normal_buffer = vec4(normal * 0.5 + 0.5, 1.0);
	}

	positions_buffer = vec4(In.world_position, 1.0);
})";

const std::vector<std::string> utils::effects::GaussianBlur::Defines = MakeEffectDefines();

const std::string utils::effects::GaussianBlur::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::GaussianBlur::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _blur
{
	vec2 direction;
} blur;

void effect(inout vec4 result)
{
	result = vec4(0.0);

	vec2 tex_size = textureSize(sColorTexture, 0);
	vec2 off1 = vec2(1.3846153846) * blur.direction / tex_size;
	vec2 off2 = vec2(3.2307692308) * blur.direction / tex_size;

	result += texture(sColorTexture, In.tex_coord) * 0.2270270270;

	result += texture(sColorTexture, In.tex_coord + off1) * 0.3162162162;
	result += texture(sColorTexture, In.tex_coord - off1) * 0.3162162162;

	result += texture(sColorTexture, In.tex_coord + off2) * 0.0702702703;
	result += texture(sColorTexture, In.tex_coord - off2) * 0.0702702703;
})";

const std::vector<std::string> utils::effects::BloomDownsample::Defines = MakeEffectDefines();

const std::string utils::effects::BloomDownsample::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::BloomDownsample::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _downsample
{
	uint step_number;
} downsample;

vec3 getSample(sampler2D srcSampler, const vec2 uv)
{
	return texture(srcSampler, uv, 0).rgb;
}

float getLuminance(vec3 c)
{
	return 0.2125 * c.r + 0.7154 * c.g + 0.0721 * c.b;
}

float getKarisWeight(const vec3 box4x4)
{
	return 1.0 / (1.0 + getLuminance(box4x4));
}

vec3 downsample13tap(sampler2D srcSampler, const vec2 centerUV)
{
	const vec2 pixelSize = vec2(1.0) / textureSize(srcSampler, 0);
	vec3 taps[13];
	taps[0] = getSample(srcSampler, centerUV + vec2(-2,-2) * pixelSize);
	taps[1] = getSample(srcSampler, centerUV + vec2( 0,-2) * pixelSize);
	taps[2] = getSample(srcSampler, centerUV + vec2( 2,-2) * pixelSize);
	taps[3] = getSample(srcSampler, centerUV + vec2(-1,-1) * pixelSize);
	taps[4] = getSample(srcSampler, centerUV + vec2( 1,-1) * pixelSize);
	taps[5] = getSample(srcSampler, centerUV + vec2(-2, 0) * pixelSize);
	taps[6] = getSample(srcSampler, centerUV + vec2( 0, 0) * pixelSize);
	taps[7] = getSample(srcSampler, centerUV + vec2( 2, 0) * pixelSize);
	taps[8] = getSample(srcSampler, centerUV + vec2(-1, 1) * pixelSize);
	taps[9] = getSample(srcSampler, centerUV + vec2( 1, 1) * pixelSize);
	taps[10] = getSample(srcSampler, centerUV + vec2(-2, 2) * pixelSize);
	taps[11] = getSample(srcSampler, centerUV + vec2( 0, 2) * pixelSize);
	taps[12] = getSample(srcSampler, centerUV + vec2( 2, 2) * pixelSize);

	// on the first downsample use Karis average

	if (downsample.step_number == 0)
	{
		vec3 box[5];
		box[0] = 0.25 * (taps[3] + taps[4] + taps[8]  + taps[9]);
		box[1] = 0.25 * (taps[0] + taps[1] + taps[5]  + taps[6]);
		box[2] = 0.25 * (taps[1] + taps[2] + taps[6]  + taps[7]);
		box[3] = 0.25 * (taps[5] + taps[6] + taps[10] + taps[11]);
		box[4] = 0.25 * (taps[6] + taps[7] + taps[11] + taps[12]);

		// weight by partial Karis average to reduce fireflies
		return 
			0.5   * getKarisWeight(box[0]) * box[0] + 
			0.125 * getKarisWeight(box[1]) * box[1] + 
			0.125 * getKarisWeight(box[2]) * box[2] + 
			0.125 * getKarisWeight(box[3]) * box[3] + 
			0.125 * getKarisWeight(box[4]) * box[4];
	}
	else
	{
		return 
			0.5   * (0.25 * (taps[3] + taps[4] + taps[8]  + taps[9]))  + 
			0.125 * (0.25 * (taps[0] + taps[1] + taps[5]  + taps[6]))  + 
			0.125 * (0.25 * (taps[1] + taps[2] + taps[6]  + taps[7]))  + 
			0.125 * (0.25 * (taps[5] + taps[6] + taps[10] + taps[11])) + 
			0.125 * (0.25 * (taps[6] + taps[7] + taps[11] + taps[12]));
	}
}

void effect(inout vec4 result)
{
	result = vec4(downsample13tap(sColorTexture, In.tex_coord), 1.0);
})";

const std::vector<std::string> utils::effects::BloomUpsample::Defines = MakeEffectDefines();

const std::string utils::effects::BloomUpsample::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::BloomUpsample::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
void effect(inout vec4 result)
{
	const vec2 pixelSize = vec2(1.0) / textureSize(sColorTexture, 0);

	const vec2 offsets[] = 
	{
		vec2(-1,-1), vec2(0,-1), vec2(1,-1),
		vec2(-1, 0), vec2(0, 0), vec2(1, 0),
		vec2(-1, 1), vec2(0, 1), vec2(1, 1),
	};

	const float weights[] = 
	{
		1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
		2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
		1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
	};

	vec3 r = vec3(0.0);

	for (int i = 0; i < 9; i++)
	{
		r += weights[i] * texture(sColorTexture, In.tex_coord + offsets[i] * pixelSize).rgb;
	}

	result = vec4(r, 1.0) * settings.color;
})";

const std::vector<std::string> utils::effects::BrightFilter::Defines = MakeEffectDefines();

const std::string utils::effects::BrightFilter::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::BrightFilter::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _bright
{
	float threshold;
} bright;

void effect(inout vec4 result)
{
	result = GetBasicResult();
	float luminance = dot(vec3(0.2125, 0.7154, 0.0721), result.xyz);
	luminance = max(0.0, luminance - bright.threshold);
	result *= sign(luminance);
})";

const std::vector<std::string> utils::effects::Grayscale::Defines = MakeEffectDefines();

const std::string utils::effects::Grayscale::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::Grayscale::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _grayscale
{
	float intensity;
} grayscale;

void effect(inout vec4 result)
{
	result = GetBasicResult();
	float gray = dot(result.rgb, vec3(0.299, 0.587, 0.114));
	result.rgb = mix(result.rgb, vec3(gray), grayscale.intensity);
})";

const std::vector<std::string> utils::effects::AlphaTest::Defines = MakeEffectDefines();

const std::string utils::effects::AlphaTest::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::AlphaTest::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _alphatest
{
	float threshold;
} alphatest;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	if (result.a <= alphatest.threshold)
		discard;
})";

const std::vector<std::string> utils::effects::GammaCorrection::Defines = MakeEffectDefines();

const std::string utils::effects::GammaCorrection::VertexShaderCode = utils::effects::BasicEffect::VertexShaderCode;

const std::string utils::effects::GammaCorrection::FragmentShaderCode =
utils::effects::BasicEffect::FragmentShaderCode + R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _gamma
{
	float value;
} gamma;

void effect(inout vec4 result)
{
	result = GetBasicResult();
    result.rgb = pow(result.rgb, vec3(1.0 / gamma.value));
})";

static std::optional<utils::Context> gContext;

utils::effects::BaseDirectionalLightEffect::BaseDirectionalLightEffect(const utils::DirectionalLight& light) :
	direction(light.direction),
	ambient(light.ambient),
	diffuse(light.diffuse),
	specular(light.specular),
	shininess(light.shininess)
{
}

utils::effects::BasePointLightEffect::BasePointLightEffect(const utils::PointLight& light) :
	position(light.position),
	ambient(light.ambient),
	diffuse(light.diffuse),
	specular(light.specular),
	constant_attenuation(light.constant_attenuation),
	linear_attenuation(light.linear_attenuation),
	quadratic_attenuation(light.quadratic_attenuation),
	shininess(light.shininess)
{
}

utils::effects::GaussianBlur::GaussianBlur(glm::vec2 _direction) :
	direction(std::move(_direction))
{
}

utils::effects::BloomDownsample::BloomDownsample(uint32_t _step_number) :
	step_number(_step_number)
{
}

utils::effects::BrightFilter::BrightFilter(float _threshold) :
	threshold(_threshold)
{
}

std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> utils::MakeCameraMatrices(const OrthogonalCamera& camera)
{
	auto width = (float)camera.width.value_or(gRenderSystem->GetBackBufferWidth());
	auto height = (float)camera.height.value_or(gRenderSystem->GetBackBufferHeight());
	auto proj = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	auto view = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	return { proj, view };
}

std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> utils::MakeCameraMatrices(const PerspectiveCamera& camera)
{
	auto sin_yaw = glm::sin(camera.yaw);
	auto sin_pitch = glm::sin(camera.pitch);

	auto cos_yaw = glm::cos(camera.yaw);
	auto cos_pitch = glm::cos(camera.pitch);

	auto front = glm::normalize(glm::vec3(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch));
	auto right = glm::normalize(glm::cross(front, camera.world_up));
	auto up = glm::normalize(glm::cross(right, front));

	auto width = (float)camera.width.value_or(gRenderSystem->GetBackBufferWidth());
	auto height = (float)camera.height.value_or(gRenderSystem->GetBackBufferHeight());

	auto proj = glm::perspectiveFov(camera.fov, width, height, camera.near_plane, camera.far_plane);
	auto view = glm::lookAtRH(camera.position, camera.position + front, up);

	return { proj, view };
}

std::tuple<glm::mat4/*view*/, glm::mat4/*projection*/> utils::CalculatePerspectiveViewProjection(float yaw, float pitch, const glm::vec3& position, uint32_t width, uint32_t height, float fov, float near_plane, float far_plane, const glm::vec3& world_up)
{
	auto [proj, view] = utils::MakeCameraMatrices(utils::PerspectiveCamera{
		.width = width,
		.height = height,
		.yaw = yaw,
		.pitch = pitch,
		.position = position,
		.world_up = world_up,
		.far_plane = far_plane,
		.near_plane = near_plane,
		.fov = fov
		});
	return { view, proj };
}

void utils::ClearContext()
{
	gContext.reset();
}

utils::Context& utils::GetContext()
{
	if (!gContext.has_value())
		gContext.emplace();

	return gContext.value();
}

static const uint32_t white_pixel = 0xFFFFFFFF;

utils::Context::Context() :
	default_mesh({
		{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		}, { 0, 1, 2, 0, 2, 3 }),
		white_pixel_texture(1, 1, PixelFormat::RGBA8UNorm, (void*)&white_pixel)
{
}

// commands

utils::commands::SetEffect::SetEffect(std::nullopt_t) : SetEffect(effects::BasicEffect{})
{
}

utils::commands::SetEffect::SetEffect(Shader* _shader, uint32_t uniform_binding, void* uniform_data,
	size_t uniform_size) :
	shader(_shader)
{
	uniform.emplace();
	uniform->binding = uniform_binding;
	uniform->data.resize(uniform_size);
	std::memcpy(uniform->data.data(), uniform_data, uniform_size);
}

utils::commands::SetTopology::SetTopology(Topology _topology) :
	topology(_topology)
{
}

utils::commands::SetViewport::SetViewport(std::optional<Viewport> _viewport) :
	viewport(std::move(_viewport))
{
}

utils::commands::SetScissor::SetScissor(std::optional<Scissor> _scissor) :
	scissor(_scissor)
{
}

utils::commands::SetBlendMode::SetBlendMode(std::optional<BlendMode> _blend_mode) :
	blendMode(std::move(_blend_mode))
{
}

utils::commands::SetSamplerFilter::SetSamplerFilter(Filter _sampler) :
	sampler(_sampler)
{
}

utils::commands::SetCullMode::SetCullMode(CullMode _cull_mode) :
	cull_mode(_cull_mode)
{
}

utils::commands::SetTextureAddress::SetTextureAddress(TextureAddress _texture_address) :
	texture_address(_texture_address)
{
}

utils::commands::SetFrontFace::SetFrontFace(FrontFace _front_face) :
	front_face(_front_face)
{
}

utils::commands::SetDepthBias::SetDepthBias(std::optional<DepthBias> _depth_bias) :
	depth_bias(_depth_bias)
{
}

utils::commands::SetDepthMode::SetDepthMode(std::optional<DepthMode> _depth_mode) :
	depth_mode(_depth_mode)
{
}

utils::commands::SetStencilMode::SetStencilMode(std::optional<StencilMode> _stencil_mode) :
	stencil_mode(_stencil_mode)
{
}

utils::commands::SetShader::SetShader(const Shader* _shader) :
	shader(_shader)
{
}

utils::commands::SetVertexBuffer::SetVertexBuffer(const VertexBuffer* _buffer) :
	buffer(_buffer)
{
}

utils::commands::SetIndexBuffer::SetIndexBuffer(const IndexBuffer* _buffer) :
	buffer(_buffer)
{
}

utils::commands::SetUniformBuffer::SetUniformBuffer(uint32_t _binding, const void* _memory, size_t _size) :
	binding(_binding), memory(_memory), size(_size)
{
}

utils::commands::SetMesh::SetMesh(const Mesh* _mesh) :
	mesh(_mesh)
{
}

utils::commands::SetTexture::SetTexture(uint32_t _binding, const Texture2D* _texture) :
	binding(_binding),
	texture(_texture)
{
}

utils::commands::Draw::Draw(uint32_t _vertex_count, uint32_t _vertex_offset, uint32_t _instance_count) :
	vertex_count(_vertex_count),
	vertex_offset(_vertex_offset),
	instance_count(_instance_count)
{
}

utils::commands::DrawIndexed::DrawIndexed(uint32_t _index_count, uint32_t _index_offset, uint32_t _instance_count) :
	index_count(_index_count),
	index_offset(_index_offset),
	instance_count(_instance_count)
{
}

utils::commands::SetColorTexture::SetColorTexture(const Texture2D* _color_texture) :
	color_texture(_color_texture)
{
}

utils::commands::SetNormalTexture::SetNormalTexture(const Texture2D* _normal_texture) :
	normal_texture(_normal_texture)
{
}

utils::commands::SetColor::SetColor(glm::vec4 _color) :
	color(std::move(_color))
{
}

utils::commands::SetProjectionMatrix::SetProjectionMatrix(glm::mat4 _projection_matrix) :
	projection_matrix(std::move(_projection_matrix))
{
}

utils::commands::SetViewMatrix::SetViewMatrix(glm::mat4 _view_matrix) :
	view_matrix(std::move(_view_matrix))
{
}

utils::commands::SetModelMatrix::SetModelMatrix(glm::mat4 _model_matrix) :
	model_matrix(std::move(_model_matrix))
{
}

utils::commands::SetCamera::SetCamera(Camera _camera) :
	camera(std::move(_camera))
{
}

utils::commands::SetEyePosition::SetEyePosition(glm::vec3 _eye_position) :
	eye_position(std::move(_eye_position))
{
}

utils::commands::SetMipmapBias::SetMipmapBias(float _mipmap_bias) :
	mipmap_bias(_mipmap_bias)
{
}

utils::commands::Subcommands::Subcommands(const std::vector<Command>* _subcommands) :
	subcommands(_subcommands)
{
}

utils::commands::DrawMesh::DrawMesh(std::optional<DrawCommand> _draw_command) :
	draw_command(std::move(_draw_command))
{
}

void utils::ExecuteCommands(const std::vector<Command>& cmds)
{
	gRenderSystem->SetTopology(Topology::TriangleList);
	gRenderSystem->SetViewport(std::nullopt);
	gRenderSystem->SetScissor(std::nullopt);
	gRenderSystem->SetBlendMode(std::nullopt);
	gRenderSystem->SetSamplerFilter(Filter::Linear);
	gRenderSystem->SetCullMode(CullMode::None);
	gRenderSystem->SetTextureAddress(TextureAddress::Clamp);
	gRenderSystem->SetFrontFace(FrontFace::Clockwise);
	gRenderSystem->SetDepthBias(std::nullopt);
	gRenderSystem->SetDepthMode(std::nullopt);
	gRenderSystem->SetStencilMode(std::nullopt);
	gRenderSystem->SetInputLayout(Mesh::Vertex::Layout);

	constexpr uint32_t ColorTextureBinding = 0;
	constexpr uint32_t NormalTextureBinding = 1;
	constexpr uint32_t SettingsUniformBinding = 2;

	auto& context = GetContext();

	gRenderSystem->SetTexture(ColorTextureBinding, context.white_pixel_texture);

	const Mesh* mesh = &context.default_mesh;
	bool mesh_dirty = true;

	struct alignas(16) Settings
	{
		glm::mat4 projection = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		alignas(16) glm::vec3 eye_position = { 0.0f, 0.0f, 0.0f };
		float mipmap_bias = 0.0f;
		alignas(16) glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint32_t has_normal_texture = 0;
	} settings;

	bool settings_dirty = true;
	bool effect_setted_up = false;

	std::function<void(const Command&)> execute_command;

	auto execute_commands = [&](const std::vector<Command>& _cmds) {
		for (const auto& cmd : _cmds)
		{
			execute_command(cmd);
		}
		};

	execute_command = [&](const Command& _cmd) {
		std::visit(cases{
			[&](const commands::SetTopology& cmd) { gRenderSystem->SetTopology(cmd.topology); },
			[&](const commands::SetViewport& cmd) { gRenderSystem->SetViewport(cmd.viewport); },
			[&](const commands::SetScissor& cmd) { gRenderSystem->SetScissor(cmd.scissor); },
			[&](const commands::SetBlendMode& cmd) { gRenderSystem->SetBlendMode(cmd.blendMode); },
			[&](const commands::SetSamplerFilter& cmd) { gRenderSystem->SetSamplerFilter(cmd.sampler); },
			[&](const commands::SetCullMode& cmd) { gRenderSystem->SetCullMode(cmd.cull_mode); },
			[&](const commands::SetTextureAddress& cmd) { gRenderSystem->SetTextureAddress(cmd.texture_address); },
			[&](const commands::SetFrontFace& cmd) { gRenderSystem->SetFrontFace(cmd.front_face); },
			[&](const commands::SetDepthBias& cmd) { gRenderSystem->SetDepthBias(cmd.depth_bias); },
			[&](const commands::SetDepthMode& cmd) { gRenderSystem->SetDepthMode(cmd.depth_mode); },
			[&](const commands::SetStencilMode& cmd) { gRenderSystem->SetStencilMode(cmd.stencil_mode); },
			[&](const commands::SetShader& cmd) { gRenderSystem->SetShader(*cmd.shader); },
			[&](const commands::SetVertexBuffer& cmd) { gRenderSystem->SetVertexBuffer(*cmd.buffer); },
			[&](const commands::SetIndexBuffer& cmd) { gRenderSystem->SetIndexBuffer(*cmd.buffer); },
			[&](const commands::SetUniformBuffer& cmd) { gRenderSystem->SetUniformBuffer(cmd.binding, cmd.memory, cmd.size); },
			[&](const commands::SetTexture& cmd) { gRenderSystem->SetTexture(cmd.binding, *cmd.texture); },
			[&](const commands::Draw& cmd) { gRenderSystem->Draw(cmd.vertex_count, cmd.vertex_offset, cmd.instance_count); },
			[&](const commands::DrawIndexed& cmd) { gRenderSystem->DrawIndexed(cmd.index_count, cmd.index_offset, cmd.instance_count); },
			[&](const commands::SetMesh& cmd) {
				mesh = cmd.mesh ? cmd.mesh : &context.default_mesh;
				mesh_dirty = true;
			},
			[&](const commands::SetEffect& cmd) {
				if (cmd.shader == nullptr)
					Fatal("shader must be not null");

				effect_setted_up = true;
				execute_command(commands::SetShader(cmd.shader));

				if (cmd.uniform.has_value())
				{
					execute_command(commands::SetUniformBuffer(cmd.uniform->binding, (void*)cmd.uniform->data.data(),
						cmd.uniform->data.size()));
				}
			},
			[&](const commands::SetColorTexture& cmd) {
				auto texture = cmd.color_texture ? cmd.color_texture : &context.white_pixel_texture;
				execute_command(commands::SetTexture(ColorTextureBinding, texture));
			},
			[&](const commands::SetNormalTexture& cmd) {
				auto texture = cmd.normal_texture ? cmd.normal_texture : &context.white_pixel_texture;
				execute_command(commands::SetTexture(NormalTextureBinding, texture));
				settings.has_normal_texture = cmd.normal_texture != nullptr;
				settings_dirty = true;
			},
			[&](const commands::SetColor& cmd) {
				settings.color = cmd.color;
				settings_dirty = true;
			},
			[&](const commands::SetProjectionMatrix& cmd) {
				settings.projection = cmd.projection_matrix;
				settings_dirty = true;
			},
			[&](const commands::SetViewMatrix& cmd) {
				settings.view = cmd.view_matrix;
				settings_dirty = true;
			},
			[&](const commands::SetModelMatrix& cmd) {
				settings.model = cmd.model_matrix;
				settings_dirty = true;
			},
			[&](const commands::SetCamera& cmd) {
				std::tie(settings.projection, settings.view, settings.eye_position) = std::visit(cases{
					[&](const OrthogonalCamera& camera) {
						return std::tuple_cat(MakeCameraMatrices(camera), std::make_tuple(glm::vec3{ 0.0f, 0.0f, 0.0f }));
					},
					[&](const PerspectiveCamera& camera) {
						return std::tuple_cat(MakeCameraMatrices(camera), std::make_tuple(camera.position));
					},
				}, cmd.camera);
				settings_dirty = true;
			},
			[&](const commands::SetEyePosition& cmd) {
				settings.eye_position = cmd.eye_position;
				settings_dirty = true;
			},
			[&](const commands::SetMipmapBias& cmd) {
				settings.mipmap_bias = cmd.mipmap_bias;
				settings_dirty = true;
			},
			[&](const commands::Subcommands& cmd) {
				execute_commands(*cmd.subcommands);
			},
			[&](const commands::DrawMesh& cmd) {
				if (!effect_setted_up)
					execute_command(commands::SetEffect(std::nullopt));

				if (mesh_dirty)
				{
					const auto& vertex_buffer = mesh->GetVertexBuffer();
					const auto& index_buffer = mesh->GetIndexBuffer();

					if (vertex_buffer.has_value())
						execute_command(commands::SetVertexBuffer(&vertex_buffer.value()));

					if (index_buffer.has_value())
						execute_command(commands::SetIndexBuffer(&index_buffer.value()));

					mesh_dirty = false;
				}

				if (settings_dirty)
				{
					execute_command(commands::SetUniformBuffer(SettingsUniformBinding, &settings, sizeof(settings)));
					settings_dirty = false;
				}

				auto draw_command = cmd.draw_command;

				if (!draw_command.has_value())
				{
					if (mesh->GetIndexCount() == 0)
						draw_command = commands::DrawMesh::DrawVerticesCommand{};
					else
						draw_command = commands::DrawMesh::DrawIndexedVerticesCommand{};
				}

				std::visit(cases{
					[&](const commands::DrawMesh::DrawVerticesCommand& draw) {
						auto vertex_count = draw.vertex_count.value_or(mesh->GetVertexCount());
						auto vertex_offset = draw.vertex_offset;
						execute_command(commands::Draw(vertex_count, vertex_offset));
					},
					[&](const commands::DrawMesh::DrawIndexedVerticesCommand& draw) {
						auto index_count = draw.index_count.value_or(mesh->GetIndexCount());
						auto index_offset = draw.index_offset;
						execute_command(commands::DrawIndexed(index_count, index_offset));
					}
				}, draw_command.value());
			}
			}, _cmd);
		};

	execute_commands(cmds);
}

void utils::ExecuteRenderPass(const RenderPass& render_pass)
{
	if (render_pass.targets.empty() || render_pass.targets == std::vector<const RenderTarget*>{ nullptr })
		gRenderSystem->SetRenderTarget(std::nullopt);
	else
		gRenderSystem->SetRenderTarget(render_pass.targets);

	if (render_pass.clear)
		gRenderSystem->Clear(render_pass.clear_value.color, render_pass.clear_value.depth, render_pass.clear_value.stencil);

	ExecuteCommands(render_pass.commands);
}

void utils::passes::Blit(Texture2D* src, RenderTarget* dst, const BlitOptions& options)
{
	auto render_pass = RenderPass{
		.targets = { dst },
		.clear = options.clear
	};

	if (options.effect.has_value())
		render_pass.commands.push_back(options.effect.value());

	render_pass.commands.insert(render_pass.commands.end(), {
		commands::SetSamplerFilter(options.sampler),
		commands::SetColor(options.color),
		commands::SetBlendMode(options.blendMode),
		commands::SetColorTexture(src),
		commands::DrawMesh()
		});

	ExecuteRenderPass(render_pass);
}

void utils::passes::GaussianBlur(RenderTarget* src, RenderTarget* dst)
{
	auto blur_target = gRenderSystem->AcquireTransientRenderTarget(src->GetWidth(), src->GetHeight());
	Blit(src, blur_target, {
		.clear = true,
		.effect = effects::GaussianBlur({ 1.0f, 0.0f })
		});
	ViewStage("gaussian horizontal", blur_target);
	Blit(blur_target, dst, {
		.effect = effects::GaussianBlur({ 0.0f, 1.0f })
		});
	ViewStage("gaussian vertical", dst);
	gRenderSystem->ReleaseTransientRenderTarget(blur_target);
}

void utils::passes::Grayscale(RenderTarget* src, RenderTarget* dst, float intensity)
{
	Blit(src, dst, {
		.effect = effects::Grayscale{ intensity }
		});
	ViewStage("grayscale", dst);
}

void utils::passes::Bloom(RenderTarget* src, RenderTarget* dst, float bright_threshold, float intensity)
{
	Blit(src, dst);

	if (intensity <= 0.0f)
		return;

	constexpr int ChainSize = 8;

	// acquire targets

	auto bright = gRenderSystem->AcquireTransientRenderTarget(src->GetWidth(), src->GetHeight());

	std::array<RenderTarget*, ChainSize> tex_chain;

	for (int i = 0; i < ChainSize; i++)
	{
		auto w = src->GetWidth() >> (i + 1);
		auto h = src->GetHeight() >> (i + 1);

		w = glm::max(w, 1u);
		h = glm::max(h, 1u);

		tex_chain[i] = gRenderSystem->AcquireTransientRenderTarget(w, h);
	}

	// extract bright

	auto downsample_src = src;

	if (bright_threshold > 0.0f)
	{
		Blit(src, bright, {
			.clear = true,
			.effect = effects::BrightFilter(bright_threshold)
			});
		ViewStage("bright", bright);
		downsample_src = bright;
	}

	// downsample

	uint32_t step_number = 0;

	for (auto target : tex_chain)
	{
		Blit(downsample_src, target, {
			.effect = effects::BloomDownsample(step_number)
			});
		ViewStage("downsample", target);
		downsample_src = target;
		step_number += 1;
	}

	// upsample

	for (auto it = std::next(tex_chain.rbegin()); it != tex_chain.rend(); ++it)
	{
		Blit(*std::prev(it), *it, {
			.blendMode = BlendStates::Additive,
			.effect = effects::BloomUpsample()
			});
		ViewStage("upsample", *it);
	}

	// combine

	Blit(*tex_chain.begin(), dst, {
		.color = glm::vec4(intensity),
		.blendMode = BlendStates::Additive,
		.effect = effects::BloomUpsample()
		});

	// release targets

	gRenderSystem->ReleaseTransientRenderTarget(bright);

	for (auto target : tex_chain)
	{
		gRenderSystem->ReleaseTransientRenderTarget(target);
	}
}

void utils::passes::BloomGaussian(RenderTarget* src, RenderTarget* dst, float bright_threshold,
	float intensity)
{
	Blit(src, dst);

	if (intensity <= 0.0f)
		return;

	constexpr auto DownsampleCount = 8;

	auto width = static_cast<uint32_t>(glm::floor(static_cast<float>(src->GetWidth()) / static_cast<float>(DownsampleCount)));
	auto height = static_cast<uint32_t>(glm::floor(static_cast<float>(src->GetHeight()) / static_cast<float>(DownsampleCount)));

	auto bright = gRenderSystem->AcquireTransientRenderTarget(width, height);
	auto blur_dst = gRenderSystem->AcquireTransientRenderTarget(width, height);
	auto blur_src = src;

	if (bright_threshold > 0.0f)
	{
		Blit(src, bright, {
			.clear = true,
			.effect = effects::BrightFilter(bright_threshold)
			});
		ViewStage("bright", bright);
		blur_src = bright;
	}

	GaussianBlur(blur_src, blur_dst);

	Blit(blur_dst, dst, {
		.color = glm::vec4(intensity),
		.blendMode = BlendStates::Additive
		});

	gRenderSystem->ReleaseTransientRenderTarget(bright);
	gRenderSystem->ReleaseTransientRenderTarget(blur_dst);
}

std::vector<utils::Command> utils::Model::Draw(const Model& model, bool use_color_texture, bool use_normal_texture)
{
	return {
		commands::SetColorTexture(use_color_texture ? model.color_texture : nullptr),
		commands::SetNormalTexture(use_normal_texture ? model.normal_texture : nullptr),
		commands::SetMesh(model.mesh),
		commands::SetModelMatrix(model.matrix),
		commands::SetCullMode(model.cull_mode),
		commands::SetTextureAddress(model.texture_address),
		commands::SetDepthMode(model.depth_mode),
		commands::SetColor(model.color),
		commands::SetSamplerFilter(model.sampler),
		commands::DrawMesh(model.draw_command)
	};
}

static void DrawSceneForwardShading(RenderTarget* target, const utils::PerspectiveCamera& camera,
	const std::vector<utils::Model>& models, const std::vector<utils::Light>& lights,
	const utils::DrawSceneOptions& options)
{
	using namespace utils;

	if (models.empty())
		return;

	if (lights.empty())
		return;

	auto render_pass = RenderPass{
		.targets = { target },
		.clear = options.clear_target,
		.commands = {
			commands::SetCamera(camera),
			commands::SetMipmapBias(options.mipmap_bias)
		}
	};

	std::vector<Command> draw_models;

	for (const auto& model : models)
	{
		auto cmds = Model::Draw(model, options.use_color_textures, options.use_normal_textures);
		std::ranges::move(cmds, std::back_inserter(draw_models));
	}

	bool first_light_done = false;

	for (const auto& light : lights)
	{
		render_pass.commands.insert(render_pass.commands.end(), {
			std::visit(cases{
				[](const DirectionalLight& light) {
					return commands::SetEffect(effects::forward_shading::DirectionalLight(light));
				},
				[](const PointLight& light) {
					return commands::SetEffect(effects::forward_shading::PointLight(light));
				}
			}, light),
			commands::Subcommands(&draw_models)
			});

		if (!first_light_done)
		{
			first_light_done = true;
			render_pass.commands.push_back(commands::SetBlendMode(BlendStates::Additive));
		}
	}

	ExecuteRenderPass(render_pass);
}

static void DrawSceneDeferredShading(RenderTarget* target, const utils::PerspectiveCamera& camera,
	const std::vector<utils::Model>& models, const std::vector<utils::Light>& lights,
	const utils::DrawSceneOptions& options)
{
	using namespace utils;

	if (models.empty())
		return;

	if (lights.empty())
		return;

	// g-buffer pass

	auto color_buffer = gRenderSystem->AcquireTransientRenderTarget();
	auto normal_buffer = gRenderSystem->AcquireTransientRenderTarget();
	auto positions_buffer = gRenderSystem->AcquireTransientRenderTarget();

	auto gbuffer_pass = RenderPass{
		.targets = { color_buffer, normal_buffer, positions_buffer },
		.clear = true,
		.commands = {
			commands::SetMipmapBias(options.mipmap_bias),
			commands::SetCamera(camera),
			commands::SetEffect(effects::deferred_shading::ExtractGeometryBuffer{}),
		}
	};

	for (const auto& model : models)
	{
		auto cmds = Model::Draw(model, options.use_color_textures, options.use_normal_textures);
		std::ranges::move(cmds, std::back_inserter(gbuffer_pass.commands));
	}

	// light pass

	auto light_pass = RenderPass{
		.targets = { target },
		.clear = options.clear_target,
		.commands = {
			commands::SetEyePosition(camera.position),
			commands::SetBlendMode(BlendStates::Additive),
			commands::SetTexture(5, color_buffer),
			commands::SetTexture(6, normal_buffer),
			commands::SetTexture(7, positions_buffer)
		}
	};

	for (const auto& light : lights)
	{
		light_pass.commands.insert(light_pass.commands.end(), {
			std::visit(cases{
				[](const DirectionalLight& light) {
					return commands::SetEffect(effects::deferred_shading::DirectionalLight(light));
				},
				[](const PointLight& light) {
					return commands::SetEffect(effects::deferred_shading::PointLight(light));
				}
			}, light),
			commands::DrawMesh()
			});
	}

	ExecuteRenderPass(gbuffer_pass);
	ExecuteRenderPass(light_pass);

	ViewStage("color_buffer", color_buffer);
	ViewStage("normal_buffer", normal_buffer);
	ViewStage("positions_buffer", positions_buffer);

	gRenderSystem->ReleaseTransientRenderTarget(color_buffer);
	gRenderSystem->ReleaseTransientRenderTarget(normal_buffer);
	gRenderSystem->ReleaseTransientRenderTarget(positions_buffer);
}

void utils::DrawScene(RenderTarget* target, const PerspectiveCamera& camera, const std::vector<Model>& models, const std::vector<Light>& lights, const DrawSceneOptions& options)
{
	std::optional<RenderTarget*> scene_target;

	auto get_same_transient_target = [](const RenderTarget* target) {
		if (target != nullptr)
			return gRenderSystem->AcquireTransientRenderTarget(target->GetWidth(), target->GetHeight());
		else
			return gRenderSystem->AcquireTransientRenderTarget();
		};

	if (!options.posteffects.empty())
	{
		scene_target = get_same_transient_target(target);
	}

	using DrawSceneFunc = std::function<void(RenderTarget* target, const PerspectiveCamera& camera,
		const std::vector<Model>& models, const std::vector<Light>& lights,
		const DrawSceneOptions& options)>;

	static const std::unordered_map<DrawSceneOptions::Technique, DrawSceneFunc> DrawSceneFuncs = {
		{ DrawSceneOptions::Technique::ForwardShading, DrawSceneForwardShading },
		{ DrawSceneOptions::Technique::DeferredShading, DrawSceneDeferredShading },
	};

	DrawSceneFuncs.at(options.technique)(scene_target.value_or((RenderTarget*)target), camera, models, lights, options);

	if (options.posteffects.empty())
		return;

	auto src = scene_target.value();

	for (size_t i = 0; i < options.posteffects.size(); i++)
	{
		const auto& posteffect = options.posteffects.at(i);
		auto dst = i == options.posteffects.size() - 1 ? (RenderTarget*)target : get_same_transient_target(target);

		std::visit(cases{
			[&](const DrawSceneOptions::BloomPosteffect& bloom) {
				passes::Bloom(src, dst, bloom.threshold, bloom.intensity);
			},
			[&](const DrawSceneOptions::GrayscalePosteffect& grayscale) {
				passes::Grayscale(src, dst, grayscale.intensity);
			},
			[&](const DrawSceneOptions::GaussianBlurPosteffect& blur) {
				passes::GaussianBlur(src, dst);
			}
			}, posteffect);

		gRenderSystem->ReleaseTransientRenderTarget(src);
		src = dst;
	}
}

utils::StageViewer* gStageViewer = nullptr;

void utils::SetStageViewer(StageViewer* value)
{
	gStageViewer = value;
}

void utils::ViewStage(const std::string& name, Texture2D* texture)
{
	if (gStageViewer == nullptr)
		return;

	gStageViewer->stage(name, texture);
}