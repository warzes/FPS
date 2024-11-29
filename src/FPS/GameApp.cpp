#include "stdafx.h"
#include "Temp.h"
//=============================================================================
static double cursor_saved_pos_x = 0.0;
static double cursor_saved_pos_y = 0.0;
static bool cursor_is_interacting = false;
//=============================================================================
void InputUpdate(InputSystem& input)
{
	if (input.IsPress(MouseButton::Right))
	{
		if (!cursor_is_interacting)
		{
			cursor_is_interacting = true;
			input.SetMouseVisible(false);
		}
	}
	else if (cursor_is_interacting)
	{
		cursor_is_interacting = false;
		input.SetMouseVisible(true);
	}

	if (input.IsPress(Key::D1))
	{
		game::gTechnique = game::gTechnique == utils::DrawSceneOptions::Technique::ForwardShading ?
			utils::DrawSceneOptions::Technique::DeferredShading :
			utils::DrawSceneOptions::Technique::ForwardShading;
	}
	if (input.IsPress(Key::D2))
	{
		game::gNormalMapping = !game::gNormalMapping;
	}
}
//=============================================================================
void UpdateCamera(InputSystem& input, float deltaTime, utils::PerspectiveCamera& camera)
{
	if (cursor_is_interacting)
	{
		auto dPos = input.GetDeltaMouse();

		const auto sensitivity = 0.25f;

		dPos.x *= sensitivity;
		dPos.y *= sensitivity;

		camera.yaw += glm::radians(static_cast<float>(dPos.x));
		camera.pitch -= glm::radians(static_cast<float>(dPos.y));
	}

	auto speed = (float)deltaTime * 500.0f;

	if (input.IsPress(Key::LeftShift))
		speed *= 3.0f;
	if (input.IsPress(Key::LeftControl))
		speed /= 6.0f;

	glm::vec2 direction = { 0.0f, 0.0f };

	if (input.IsPress(Key::W))
		direction.y = 1.0f;

	if (input.IsPress(Key::S))
		direction.y = -1.0f;

	if (input.IsPress(Key::A))
		direction.x = -1.0f;

	if (input.IsPress(Key::D))
		direction.x = 1.0f;

	if (glm::length(direction) > 0.0f)
	{
		direction = glm::normalize(direction);
		direction *= speed;
	}

	auto angles_speed = deltaTime * 100.0f;

	if (input.IsPress(Key::Right))
		camera.yaw += glm::radians(static_cast<float>(angles_speed));

	if (input.IsPress(Key::Left))
		camera.yaw -= glm::radians(static_cast<float>(angles_speed));

	if (input.IsPress(Key::Up))
		camera.pitch += glm::radians(static_cast<float>(angles_speed));

	if (input.IsPress(Key::Down))
		camera.pitch -= glm::radians(static_cast<float>(angles_speed));

	constexpr auto limit = glm::pi<float>() / 2.0f - 0.01f;

	camera.pitch = fmaxf(-limit, camera.pitch);
	camera.pitch = fminf(+limit, camera.pitch);

	auto pi = glm::pi<float>();

	while (camera.yaw > pi)
		camera.yaw -= pi * 2.0f;

	while (camera.yaw < -pi)
		camera.yaw += pi * 2.0f;

	auto sin_yaw = glm::sin(camera.yaw);
	auto sin_pitch = glm::sin(camera.pitch);

	auto cos_yaw = glm::cos(camera.yaw);
	auto cos_pitch = glm::cos(camera.pitch);

	const float fov = 70.0f;
	const float near_plane = 1.0f;
	const float far_plane = 8192.0f;
	const glm::vec3 world_up = { 0.0f, 1.0f, 0.0f };

	auto front = glm::normalize(glm::vec3(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch));
	auto right = glm::normalize(glm::cross(front, world_up));
	//auto up = glm::normalize(glm::cross(right, front));

	if (glm::length(direction) > 0.0f)
	{
		camera.position += front * direction.y;
		camera.position += right * direction.x;
	}
}
//=============================================================================
void GameStart()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		auto& input = engine.GetInputSystem();
		auto& rhi = engine.GetRenderSystem();

#pragma region content load

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		auto path = "assets/sponza/sponza.glb";

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

		auto camera = utils::PerspectiveCamera();

		camera.position = glm::vec3(0, 200, 0);

		auto render_buffer = game::BuildRenderBuffer(model);

		auto directional_light = utils::DirectionalLight();
		directional_light.ambient = { 0.125f, 0.125f, 0.125f };
		directional_light.diffuse = { 0.125f, 0.125f, 0.125f };
		directional_light.specular = { 1.0f, 1.0f, 1.0f };
		directional_light.shininess = 16.0f;
		directional_light.direction = { 0.5f, -1.0f, 0.5f };

		auto base_light = utils::PointLight();
		base_light.shininess = 32.0f;
		base_light.constant_attenuation = 0.0f;
		base_light.linear_attenuation = 0.00128f;
		base_light.quadratic_attenuation = 0.0f;

		auto red_light = base_light;
		red_light.ambient = { 0.0625f, 0.0f, 0.0f };
		red_light.diffuse = { 0.5f, 0.0f, 0.0f };
		red_light.specular = { 1.0f, 0.0f, 0.0f };

		auto green_light = base_light;
		green_light.ambient = { 0.0f, 0.0625f, 0.0f };
		green_light.diffuse = { 0.0f, 0.5f, 0.0f };
		green_light.specular = { 0.0f, 1.0f, 0.0f };

		auto blue_light = base_light;
		blue_light.ambient = { 0.0f, 0.0f, 0.0625f };
		blue_light.diffuse = { 0.0f, 0.0f, 0.5f };
		blue_light.specular = { 0.0f, 0.0f, 1.0f };

		auto lightblue_light = base_light;
		lightblue_light.ambient = { 0.0f, 0.0625f, 0.0625f };
		lightblue_light.diffuse = { 0.0f, 0.5f, 0.5f };
		lightblue_light.specular = { 0.0f, 1.0f, 1.0f };

		struct MovingLight
		{
			utils::PointLight light;
			glm::vec3 begin;
			glm::vec3 end;
			float multiplier = 1.0f;
		};

		std::vector<MovingLight> moving_lights = {
			// first floor
			{ red_light, { 1200.0f, 256.0f, -36.0f }, { -1200.0f, 256.0f, -36.0f }, 4.0f },
			{ green_light, { 1200.0f, 256.0f, -36.0f }, { -1200.0f, 256.0f, -36.0f }, 3.0f },
			{ blue_light, { 1200.0f, 256.0f, -36.0f }, { -1200.0f, 256.0f, -36.0f }, 2.0f },

			// second floor
			{ green_light, { 1100.0f, 550.0f, 400.0f }, { 1100.0f, 550.0f, -400.0f }, 1.0f },
			{ red_light, { -1200.0f, 550.0f, -400.0f }, { -1200.0f, 550.0f, 400.0f }, 2.0f },
			{ blue_light, { 1100.0f, 550.0f, 400.0f }, { -1200.0f, 550.0f, 400.0f }, 3.0f },
			{ lightblue_light, { 1100.0f, 550.0f, -400.0f }, { -1200.0f, 550.0f, -400.0f }, 4.0f }
		};

		std::vector<utils::Model> models;

		for (const auto& [material, draw_datas] : render_buffer.meshes)
		{
			for (const auto& draw_data : draw_datas)
			{
				utils::Model model;
				model.mesh = (utils::Mesh*)&draw_data.mesh;
				model.draw_command = draw_data.draw_command;
				model.color = material->color;
				model.color_texture = material->color_texture.get();
				model.normal_texture = material->normal_texture.get();
				model.cull_mode = CullMode::Front;
				model.texture_address = TextureAddress::Wrap;
				model.depth_mode = ComparisonFunc::LessEqual;
				models.push_back(model);
			}
		}

		utils::DrawSceneOptions options = {
			.posteffects = {
				utils::DrawSceneOptions::BloomPosteffect{}
			}
		};

		bool animate_lights = true;
		bool show_normals = false;
		float time = 0.0f;

		game::StageViewer stage_viewer;
		utils::SetStageViewer(&stage_viewer);

#pragma endregion

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();
			InputUpdate(input);
			UpdateCamera(input, 0.01f, camera);

			rhi.Resize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());
			
			time += 0.01f;

			std::vector<utils::Light> lights = { directional_light };

			for (auto& moving_light : moving_lights)
			{
				moving_light.light.position = glm::lerp(moving_light.begin, moving_light.end, (glm::sin(time / moving_light.multiplier) + 1.0f) * 0.5f);
				lights.push_back(moving_light.light);
			}

			options.technique = game::gTechnique;
			options.use_normal_textures = game::gNormalMapping;
			utils::DrawScene(nullptr, camera, models, lights, options);

			if (show_normals)
				DrawNormals(camera, render_buffer);

			stage_viewer.show();

			rhi.Present();
			engine.EndFrame();
		}
	}
	engine.Destroy();
}
//=============================================================================