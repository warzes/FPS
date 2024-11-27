#include "stdafx.h"
#if RENDER_VULKAN
#include "AccelerationStructureVK.h"
#include "ContextVK.h"
//=============================================================================
BottomLevelAccelerationStructureVK::BottomLevelAccelerationStructureVK(const void* vertex_memory, uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count, uint32_t index_stride, const glm::mat4& _transform)
{
	auto transform = glm::transpose(_transform);

	auto [vertex_buffer, vertex_buffer_memory] = CreateBuffer(vertex_count * vertex_stride,
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto [index_buffer, index_buffer_memory] = CreateBuffer(index_count * index_stride,
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto [transform_buffer, transform_buffer_memory] = CreateBuffer(sizeof(transform),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	WriteToBuffer(vertex_buffer_memory, vertex_memory, vertex_count * vertex_stride);
	WriteToBuffer(index_buffer_memory, index_memory, index_count * index_stride);
	WriteToBuffer(transform_buffer_memory, &transform, sizeof(transform));

	auto vertex_buffer_device_address = GetBufferDeviceAddress(*vertex_buffer);
	auto index_buffer_device_address = GetBufferDeviceAddress(*index_buffer);
	auto transform_buffer_device_address = GetBufferDeviceAddress(*transform_buffer);

	auto triangles = vk::AccelerationStructureGeometryTrianglesDataKHR()
		.setVertexFormat(vk::Format::eR32G32B32Sfloat)
		.setVertexData(vertex_buffer_device_address)
		.setMaxVertex(vertex_count)
		.setVertexStride(vertex_stride)
		.setIndexType(GetIndexTypeFromStride(index_stride))
		.setIndexData(index_buffer_device_address)
		.setTransformData(transform_buffer_device_address);

	auto geometry_data = vk::AccelerationStructureGeometryDataKHR()
		.setTriangles(triangles);

	auto geometry = vk::AccelerationStructureGeometryKHR()
		.setGeometryType(vk::GeometryTypeKHR::eTriangles)
		.setGeometry(geometry_data)
		.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

	auto build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR()
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
		.setGeometries(geometry);

	auto build_sizes = gContext.device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, build_geometry_info, { 1 });

	std::tie(mBlasBuffer, mBlasMemory) = CreateBuffer(build_sizes.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);

	auto create_info = vk::AccelerationStructureCreateInfoKHR()
		.setBuffer(*mBlasBuffer)
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
		.setSize(build_sizes.accelerationStructureSize);

	mBlas = gContext.device.createAccelerationStructureKHR(create_info);

	auto [scratch_buffer, scratch_memory] = CreateBuffer(build_sizes.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto scratch_buffer_addr = GetBufferDeviceAddress(*scratch_buffer);

	build_geometry_info
		.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
		.setDstAccelerationStructure(*mBlas)
		.setScratchData(scratch_buffer_addr);

	auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR()
		.setPrimitiveCount(static_cast<uint32_t>(index_count / 3));

	auto build_geometry_infos = { build_geometry_info };
	std::vector build_range_infos = { &build_range_info };

	OneTimeSubmit([&](auto& cmdbuf) {
		cmdbuf.buildAccelerationStructuresKHR(build_geometry_infos, build_range_infos);
		});
}
//=============================================================================
BottomLevelAccelerationStructureVK::~BottomLevelAccelerationStructureVK()
{
	DestroyStaging(std::move(mBlas));
	DestroyStaging(std::move(mBlasBuffer));
	DestroyStaging(std::move(mBlasMemory));
}
//=============================================================================
TopLevelAccelerationStructureVK::TopLevelAccelerationStructureVK(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures)
{
	auto transform = glm::mat4(1.0f);

	std::vector<vk::AccelerationStructureInstanceKHR> instances;

	for (auto [custom_index, handle] : bottom_level_acceleration_structures)
	{
		const auto& blas = *(BottomLevelAccelerationStructureVK*)handle;

		auto blas_device_address_info = vk::AccelerationStructureDeviceAddressInfoKHR()
			.setAccelerationStructure(*blas.GetBlas());

		auto blas_device_address = gContext.device.getAccelerationStructureAddressKHR(blas_device_address_info);

		auto instance = vk::AccelerationStructureInstanceKHR()
			.setTransform(*(vk::TransformMatrixKHR*)&transform)
			.setMask(0xFF)
			.setInstanceShaderBindingTableRecordOffset(0)
			.setInstanceCustomIndex(custom_index) // gl_InstanceCustomIndexEXT
			.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
			.setAccelerationStructureReference(blas_device_address);

		instances.push_back(instance);
	}

	auto instance_buffer_size = sizeof(vk::AccelerationStructureInstanceKHR) * instances.size();

	auto [instance_buffer, instance_buffer_memory] = CreateBuffer(instance_buffer_size,
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	WriteToBuffer(instance_buffer_memory, instances.data(), instance_buffer_size);

	auto instance_buffer_addr = GetBufferDeviceAddress(*instance_buffer);

	auto geometry_instances = vk::AccelerationStructureGeometryInstancesDataKHR()
		.setData(instance_buffer_addr);

	auto geometry_data = vk::AccelerationStructureGeometryDataKHR()
		.setInstances(geometry_instances);

	auto geometry = vk::AccelerationStructureGeometryKHR()
		.setGeometryType(vk::GeometryTypeKHR::eInstances)
		.setGeometry(geometry_data)
		.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

	auto build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR()
		.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
		.setGeometries(geometry);

	auto build_sizes = gContext.device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, build_geometry_info, { (uint32_t)instances.size() });

	std::tie(mTlasBuffer, mTlasMemory) = CreateBuffer(build_sizes.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);

	auto create_info = vk::AccelerationStructureCreateInfoKHR()
		.setBuffer(*mTlasBuffer)
		.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
		.setSize(build_sizes.accelerationStructureSize);

	mTlas = gContext.device.createAccelerationStructureKHR(create_info);

	auto [scratch_buffer, scratch_memory] = CreateBuffer(build_sizes.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto scratch_buffer_addr = GetBufferDeviceAddress(*scratch_buffer);

	build_geometry_info
		.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
		.setDstAccelerationStructure(*mTlas)
		.setScratchData(scratch_buffer_addr);

	auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR()
		.setPrimitiveCount((uint32_t)instances.size());

	auto build_geometry_infos = { build_geometry_info };
	std::vector build_range_infos = { &build_range_info };

	OneTimeSubmit([&](auto& cmdbuf) {
		cmdbuf.buildAccelerationStructuresKHR(build_geometry_infos, build_range_infos);
		});
}
//=============================================================================
TopLevelAccelerationStructureVK::~TopLevelAccelerationStructureVK()
{
	DestroyStaging(std::move(mTlas));
	DestroyStaging(std::move(mTlasBuffer));
	DestroyStaging(std::move(mTlasMemory));
}
//=============================================================================
#endif // RENDER_VULKAN