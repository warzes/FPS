#pragma once

namespace game
{

	static auto gTechnique = utils::DrawSceneOptions::Technique::DeferredShading;
	static auto gNormalMapping = true;

	struct Material
	{
		std::shared_ptr<Texture> color_texture;
		std::shared_ptr<Texture> normal_texture;
		std::shared_ptr<Texture> metallic_roughness_texture;
		glm::vec4 color;
	};

	struct RenderBuffer
	{
		struct DrawData
		{
			Mesh::Vertices vertices; // for normals debug
			Mesh::Indices indices; // for normals debug
			Topology topology;
			Mesh mesh;
			utils::commands::DrawMesh::DrawCommand draw_command;
		};

		std::unordered_map<std::shared_ptr<Material>, std::vector<DrawData>> meshes;
	};

	RenderBuffer BuildRenderBuffer(const tinygltf::Model& model)
	{
		// https://github.com/syoyo/tinygltf/blob/master/examples/glview/glview.cc
		// https://github.com/syoyo/tinygltf/blob/master/examples/basic/main.cpp

		RenderBuffer result;

		const auto& scene = model.scenes.at(0);

		std::unordered_map<int, std::shared_ptr<Texture>> textures_cache;

		auto get_or_create_texture = [&](int index) -> std::shared_ptr<Texture> {
			if (index == -1)
				return nullptr;

			if (!textures_cache.contains(index))
			{
				const auto& texture = model.textures.at(index);
				const auto& image = model.images.at(texture.source);
				textures_cache[index] = std::make_shared<Texture>((uint32_t)image.width,
					(uint32_t)image.height, PixelFormat::RGBA8UNorm, (void*)image.image.data(), true);
			}

			return textures_cache.at(index);
			};

		for (auto node_index : scene.nodes)
		{
			const auto& node = model.nodes.at(node_index);

			auto mesh_index = node.mesh;

			const auto& mesh = model.meshes.at(mesh_index);

			for (const auto& primitive : mesh.primitives)
			{
				static const std::unordered_map<int, Topology> ModesMap = {
					{ TINYGLTF_MODE_POINTS, Topology::PointList },
					{ TINYGLTF_MODE_LINE, Topology::LineList },
					//	{ TINYGLTF_MODE_LINE_LOOP, Topology:: },
						{ TINYGLTF_MODE_LINE_STRIP, Topology::LineStrip },
						{ TINYGLTF_MODE_TRIANGLES, Topology::TriangleList },
						{ TINYGLTF_MODE_TRIANGLE_STRIP, Topology::TriangleStrip },
						//	{ TINYGLTF_MODE_TRIANGLE_FAN, Topology:: } 
				};

				auto topology = ModesMap.at(primitive.mode);

				const static std::unordered_map<int, int> IndexStride = {
					{ TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, 2 },
					{ TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, 4 },
				};

				/* buffer_view.target is:
					TINYGLTF_TARGET_ARRAY_BUFFER,
					TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER
				*/

				const auto& index_buffer_accessor = model.accessors.at(primitive.indices);
				const auto& index_buffer_view = model.bufferViews.at(index_buffer_accessor.bufferView);
				const auto& index_buffer = model.buffers.at(index_buffer_view.buffer);

				auto index_buf_size = index_buffer_view.byteLength;
				auto index_buf_stride = IndexStride.at(index_buffer_accessor.componentType);
				auto index_buf_data = (void*)((size_t)index_buffer.data.data() + index_buffer_view.byteOffset);

				auto index_count = index_buffer_accessor.count;
				auto index_offset = index_buffer_accessor.byteOffset / 2;

				const auto& positions_buffer_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
				const auto& positions_buffer_view = model.bufferViews.at(positions_buffer_accessor.bufferView);
				const auto& positions_buffer = model.buffers.at(positions_buffer_view.buffer);

				const auto& normal_buffer_accessor = model.accessors.at(primitive.attributes.at("NORMAL"));
				const auto& normal_buffer_view = model.bufferViews.at(normal_buffer_accessor.bufferView);
				const auto& normal_buffer = model.buffers.at(normal_buffer_view.buffer);

				const auto& texcoord_buffer_accessor = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
				const auto& texcoord_buffer_view = model.bufferViews.at(texcoord_buffer_accessor.bufferView);
				const auto& texcoord_buffer = model.buffers.at(texcoord_buffer_view.buffer);

				if (!primitive.attributes.contains("TANGENT"))
					continue;

				const auto& tangents_buffer_accessor = model.accessors.at(primitive.attributes.at("TANGENT"));
				const auto& tangents_buffer_view = model.bufferViews.at(tangents_buffer_accessor.bufferView);
				const auto& tangents_buffer = model.buffers.at(tangents_buffer_view.buffer);

				//const auto& bitangents_buffer_accessor = model.accessors.at(primitive.attributes.at("BITANGENT"));
				//const auto& bitangents_buffer_view = model.bufferViews.at(bitangents_buffer_accessor.bufferView);
				//const auto& bitangents_buffer = model.buffers.at(bitangents_buffer_view.buffer);

				auto positions_ptr = (glm::vec3*)(((size_t)positions_buffer.data.data()) + positions_buffer_view.byteOffset);
				auto texcoord_ptr = (glm::vec2*)(((size_t)texcoord_buffer.data.data()) + texcoord_buffer_view.byteOffset);
				auto normal_ptr = (glm::vec3*)(((size_t)normal_buffer.data.data()) + normal_buffer_view.byteOffset);
				auto tangents_ptr = (glm::vec3*)(((size_t)tangents_buffer.data.data()) + tangents_buffer_view.byteOffset);
				//auto bitangents_ptr = (glm::vec3*)(((size_t)bitangents_buffer.data.data()) + bitangents_buffer_view.byteOffset);

				auto indices = Mesh::Indices();

				for (int i = 0; i < index_buffer_accessor.count; i++)
				{
					uint32_t index;

					if (index_buf_stride == 2)
						index = static_cast<uint32_t>(((uint16_t*)index_buf_data)[i]);
					else
						index = ((uint32_t*)index_buf_data)[i];

					indices.push_back(index);
				}

				Mesh::Vertices vertices;

				for (int i = 0; i < positions_buffer_accessor.count; i++)
				{
					Mesh::Vertex vertex;

					vertex.pos = positions_ptr[i];
					vertex.normal = normal_ptr[i];
					vertex.texCoord = texcoord_ptr[i];
					vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // TODO: colors_ptr[i]
					vertex.tangent = tangents_ptr[i];
					//vertex.bitangent = bitangents_ptr[i];

					vertices.push_back(vertex);
				}

				auto mesh = Mesh();
				mesh.SetIndices(indices);
				mesh.SetVertices(vertices);

				auto draw_command = utils::commands::DrawMesh::DrawIndexedVerticesCommand{
					.index_count = (uint32_t)index_count,
					.index_offset = (uint32_t)index_offset
				};

				const auto& material = model.materials.at(primitive.material);
				const auto& baseColorTexture = material.pbrMetallicRoughness.baseColorTexture;
				const auto& metallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture;
				const auto& baseColorFactor = material.pbrMetallicRoughness.baseColorFactor;
				const auto& occlusionTexture = material.occlusionTexture;

				auto _material = std::make_shared<Material>();
				_material->color_texture = get_or_create_texture(baseColorTexture.index);
				_material->normal_texture = get_or_create_texture(material.normalTexture.index);
				_material->metallic_roughness_texture = get_or_create_texture(metallicRoughnessTexture.index);
				_material->color = {
					baseColorFactor.at(0),
					baseColorFactor.at(1),
					baseColorFactor.at(2),
					baseColorFactor.at(3)
				};

				auto draw_data = RenderBuffer::DrawData{
					.vertices = std::move(vertices),
					.indices = std::move(indices),
					.topology = topology,
					.mesh = std::move(mesh),
					.draw_command = draw_command
				};

				result.meshes[_material].push_back(std::move(draw_data));
			}
			// TODO: dont forget to draw childrens of node
		}

		return result;
	}

	template<typename T>
	std::string GetPosteffectName()
	{
		static_assert(sizeof(T) == -1, "GetPosteffectName<T> must be specialized for T");
		return "";
	}

	template<>
	std::string GetPosteffectName<utils::DrawSceneOptions::GrayscalePosteffect>() { return "Grayscale"; }

	template<>
	std::string GetPosteffectName<utils::DrawSceneOptions::BloomPosteffect>() { return "Bloom"; }

	template<>
	std::string GetPosteffectName<utils::DrawSceneOptions::GaussianBlurPosteffect>() { return "Gaussian Blur"; }

	static int gDrawcalls = 0;

	Mesh CreateNormalsDebugMesh(const RenderBuffer& render_buffer)
	{
		utils::MeshBuilder mesh_builder;

		for (const auto& [material, draw_datas] : render_buffer.meshes)
		{
			for (const auto& draw_data : draw_datas)
			{
				auto draw_vertex = [&](const Mesh::Vertex& vertex) {
					mesh_builder.begin(utils::MeshBuilder::Mode::Lines);
					mesh_builder.vertex({ .pos = vertex.pos, .color = { 0.0f, 1.0f, 0.0f, 1.0f } });
					mesh_builder.vertex({ .pos = vertex.pos + (vertex.normal * 25.0f), .color = { 0.0f, 1.0f, 0.0f, 1.0f } });
					mesh_builder.end();
					};

				std::visit(cases{
					[&](const utils::commands::DrawMesh::DrawVerticesCommand& draw) {
						auto vertex_count = draw.vertex_count.value_or((uint32_t)draw_data.vertices.size());
						auto vertex_offset = draw.vertex_offset;

						for (uint32_t i = vertex_offset; i < vertex_count; i++)
						{
							const auto& vertex = draw_data.vertices.at(i);
							draw_vertex(vertex);
						}
					},
					[&](const utils::commands::DrawMesh::DrawIndexedVerticesCommand& draw) {
						auto index_count = draw.index_count.value_or((uint32_t)draw_data.indices.size());
						auto index_offset = draw.index_offset;

						for (uint32_t i = index_offset; i < index_count; i++)
						{
							auto index = draw_data.indices.at(i);
							const auto& vertex = draw_data.vertices.at(index);
							draw_vertex(vertex);
						}
					}
					}, draw_data.draw_command);
			}
		}

		Mesh mesh;
		mesh_builder.setToMesh(mesh);

		return mesh;
	}

	void DrawNormals(const utils::PerspectiveCamera& camera, const RenderBuffer& render_buffer)
	{
		static auto mesh = CreateNormalsDebugMesh(render_buffer);;

		utils::ExecuteCommands({
			utils::commands::SetCamera(camera),
			utils::commands::SetMesh(&mesh),
			utils::commands::DrawMesh{}
			});
	}

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

}
