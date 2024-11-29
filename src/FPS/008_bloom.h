#pragma once

class StageViewer : public utils::StageViewer
{
private:
	struct Stage
	{
		std::string name;
		RenderTarget* target;
	};

	std::vector<Stage> mStages;

public:
	void stage(const std::string& name, Texture* texture) override;
	void show();
};

void StageViewer::stage(const std::string& name, Texture* texture)
{
	if (texture == nullptr)
		return;

	Stage stage;
	stage.name = name;
	stage.target = gRenderSystem->AcquireTransientRenderTarget(texture->GetWidth(), texture->GetHeight());
	utils::passes::Blit(texture, stage.target, {
		.clear = true
		});
	mStages.push_back(stage);
}

void StageViewer::show()
{
	mStages.clear();
}

void E008()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		const auto WhiteColor = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		const utils::Mesh::Vertices vertices = {
			/* front */
			/* 0  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 1  */ { {  1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 2  */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 3  */ { {  1.0f, -1.0f,  1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			/* top */
			/* 4  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 5  */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 6  */ { {  1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 7  */ { {  1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			/* left */
			/* 8  */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 9  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 10 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 11 */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },

			/* back */
			/* 12 */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 13 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 14 */ { {  1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 15 */ { {  1.0f, -1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

			/* bottom */
			/* 16 */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 17 */ { {  1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 18 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 19 */ { {  1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

			/* right */
			/* 20 */ { { 1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 21 */ { { 1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 22 */ { { 1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 23 */ { { 1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		};

		const utils::Mesh::Indices indices = {
			0, 1, 2, 1, 3, 2, // front
			4, 5, 6, 5, 7, 6, // top
			8, 9, 10, 9, 11, 10, // left
			12, 13, 14, 13, 15, 14, // back
			16, 17, 18, 17, 19, 18, // bottom
			20, 21, 22, 21, 23, 22, // right
		};

		auto [tex_width, tex_height, tex_memory] = LoadTexture("assets/bricks.jpg");

		auto texture = Texture(tex_width, tex_height, PixelFormat::RGBA8UNorm, tex_memory, true);

		const auto camera = utils::PerspectiveCamera{
			.yaw = 0.0f,
			.pitch = glm::radians(-25.0f),
			.position = { -5.0f, 2.0f, 0.0f }
		};

		const auto light = utils::DirectionalLight{
			.direction = { 1.0f, 0.5f, 0.5f },
			.ambient = { 0.25f, 0.25f, 0.25f },
			.diffuse = { 1.0f, 1.0f, 1.0f },
			.specular = { 1.0f, 1.0f, 1.0f },
			.shininess = 32.0f
		};

		utils::Mesh cube_mesh;
		cube_mesh.setVertices(vertices);
		cube_mesh.setIndices(indices);

		float angle = 1.0f;
		float speed = 1.0f;
		bool animated = true;

		float threshold = 1.0f;
		float intensity = 2.0f;
		bool gaussian = false;

		auto model = utils::Model{
			.mesh = &cube_mesh,
			.color_texture = &texture,
			.cull_mode = CullMode::Back
		};

		StageViewer stage_viewer;
		utils::SetStageViewer(&stage_viewer);

		auto& rhi = engine.GetRenderSystem();

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();
			rhi.Resize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());

			static float time = 0.0f;
			time += 0.01f;
			
			auto src_target = rhi.AcquireTransientRenderTarget();
			if (animated) angle = glm::wrapAngle(time * speed);

			model.matrix = glm::rotate(glm::mat4(1.0f), angle, { 0.0f, 1.0f, 0.0f });

			utils::DrawScene(src_target, camera, { model }, { light });

			if (gaussian)
				utils::passes::BloomGaussian(src_target, nullptr, threshold, intensity);
			else
				utils::passes::Bloom(src_target, nullptr, threshold, intensity);

			stage_viewer.show();

			rhi.Present();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}