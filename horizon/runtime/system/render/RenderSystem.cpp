#include "RenderSystem.h"

#include <iostream>
#include <runtime/core/math/Math.h>
#include <runtime/core/path/Path.h>
#include <runtime/function/rhi/vulkan/VulkanEnums.h>
#include <runtime/function/rhi/vulkan/ResourceBarrier.h>
#include <runtime/function/rhi/RHIInterface.h>
#include <runtime/function/rhi/vulkan2/RHIVulkan.h>
#include <runtime/function/rhi/dx12/RHIDX12.h>
#include <runtime/core/jobsystem/ThreadPool.h>

namespace Horizon
{

	class Window;

	RenderSystem::RenderSystem(u32 width, u32 height, std::shared_ptr<Window> window) noexcept : m_window(window)
	{

		RenderBackend backend = RenderBackend::RENDER_BACKEND_VULKAN;

		//RenderBackend backend = RenderBackend::RENDER_BACKEND_DX12;

		InitializeRenderAPI(backend);

		{
			// BUFFER TEST

			auto buffer = m_render_api->CreateBuffer(BufferCreateInfo{ BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER, 32 });
			m_render_api->DestroyBuffer(buffer);

			// TEXTURE TEST

			auto texture = m_render_api->CreateTexture(TextureCreateInfo{ TextureType::TEXTURE_TYPE_2D, TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM, TextureUsage::TEXTURE_USAGE_R, 4, 4, 1 });
			m_render_api->DestroyTexture(texture);
		}

		{
			std::string file_name = "C:/Users/hylu/OneDrive/mycode/horizon/horizon/tools/shaders/hlsl/shader.hlsl";
			m_render_api->CreateShaderProgram(ShaderTargetStage::vs, "vs_main", 0, file_name);
		}

		{

			auto graphics = m_render_api->GetCommandList(RHI::CommandQueueType::GRAPHICS);
			graphics->Dispatch();
			graphics->BeginRecording();
			graphics->Dispatch();
			graphics->Draw();
			graphics->EndRecording();

			auto compute = m_render_api->GetCommandList(RHI::CommandQueueType::COMPUTE);
			compute->BeginRecording();
			compute->Dispatch();
			compute->EndRecording();

			auto transfer = m_render_api->GetCommandList(RHI::CommandQueueType::TRANSFER);

			transfer->BeginRecording();
			auto buffer = m_render_api->CreateBuffer(BufferCreateInfo{ BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER, sizeof(Math::vec3) });
			Math::vec3 data(1.0);
			transfer->UpdateBuffer(buffer, &data, sizeof(data));

			// barrier for queue family ownership transfer

			BufferMemoryBarrierDesc bmb{
				buffer->GetBufferPointer(),
				0,
				buffer->GetBufferSize(),
				MemoryAccessFlags::ACCESS_TRANSFER_READ_BIT,
				0,
				RHI::CommandQueueType::TRANSFER,
				RHI::CommandQueueType::COMPUTE,
			};
			
			BarrierDesc desc{};
			desc.src_stage = PipelineStageFlags::PIPELINE_STAGE_TRANSFER_BIT;
			desc.dst_stage = PipelineStageFlags::PIPELINE_STAGE_ALL_COMMANDS_BIT;
			desc.buffer_memory_barriers.emplace_back(bmb);
			transfer->InsertBarrier(desc);

			transfer->EndRecording();

			m_render_api->ResetCommandResources();

			auto compute2 = m_render_api->GetCommandList(RHI::CommandQueueType::COMPUTE);
			compute2->BeginRecording();
			compute2->Dispatch();
			compute2->EndRecording();

			auto compute3 = m_render_api->GetCommandList(RHI::CommandQueueType::COMPUTE);
			compute3->BeginRecording();
			compute3->Dispatch();
			compute3->EndRecording();
		}
		





		m_device = std::make_shared<Device>(window);

		m_render_context.width = width;
		m_render_context.height = height;

		m_swap_chain = std::make_shared<SwapChain>(m_render_context, m_device, m_device->GetSurface());
		m_command_buffer = std::make_shared<CommandBuffer>(m_render_context, m_device);
		m_scene = std::make_shared<Scene>(m_render_context, m_device, m_command_buffer);
		m_fullscreen_triangle = std::make_shared<FullscreenTriangle>(m_device, m_command_buffer);
		m_pipeline_manager = std::make_shared<PipelineManager>(m_device);
		PrepareAssests();
		CreatePipelines();
	}

	RenderSystem::~RenderSystem() noexcept
	{
	}

	void RenderSystem::Tick() noexcept
	{
		Update();
		Render();
	}



	void RenderSystem::Update() noexcept
	{
		m_scene->Prepare();


		m_light_pass->BindResource(0, m_scene->m_light_count_ub);
		m_light_pass->BindResource(1, m_scene->m_light_ub);
		m_light_pass->BindResource(2, m_scene->m_camera_ub);

		m_light_pass->BindResource(3, m_geometry_pass->GetFrameBufferAttachment(0));
		m_light_pass->BindResource(4, m_geometry_pass->GetFrameBufferAttachment(1));
		m_light_pass->BindResource(5, m_geometry_pass->GetFrameBufferAttachment(2));


		m_light_pass->UpdateDescriptorSets();

		m_atmosphere_pass->SetCameraParams(m_scene->GetMainCamera()->GetInvViewProjectionMatrix(), m_scene->GetMainCamera()->GetPosition());


		m_atmosphere_pass->BindResource(0, m_scene->getCameraUbo());
		m_atmosphere_pass->BindResource(3, m_light_pass->GetFrameBufferAttachment(0));
		m_atmosphere_pass->BindResource(4, m_geometry_pass->GetFrameBufferAttachment(3));
		m_atmosphere_pass->UpdateDescriptorSets();

		m_post_process_pass->BindResource(0, m_atmosphere_pass->GetFrameBufferAttachment(0));
		m_post_process_pass->UpdateDescriptorSets();

		m_present_descriptorSet->AllocateDescriptorSet();
		DescriptorSetUpdateDesc desc;
		desc.BindResource(0, m_post_process_pass->GetFrameBufferAttachment(0));
		m_present_descriptorSet->UpdateDescriptorSet(desc);
	}

	void RenderSystem::Render() noexcept
	{
		DrawFrame();
		m_command_buffer->submit(m_swap_chain);
	}

	void RenderSystem::Wait() noexcept
	{
		vkDeviceWaitIdle(m_device->Get());
	}

	std::shared_ptr<Camera> RenderSystem::GetMainCamera() const noexcept
	{
		return m_scene->GetMainCamera();
	}

	void RenderSystem::DrawFrame() noexcept
	{
		for (u32 i = 0; i < m_render_context.swap_chain_image_count; i++)
		{
			m_command_buffer->beginCommandRecording(i);

			// geometry pass
			m_scene->Draw(i, m_command_buffer, m_geometry_pass->GetPipeline());

			m_fullscreen_triangle->Draw(i, m_command_buffer, m_light_pass->GetPipeline(), { m_light_pass->m_descriptorset });

			// scattering pass

			if (!m_atmosphere_pass->precomputed) {
				m_command_buffer->Dispatch(i, m_atmosphere_pass->m_transmittance_lut_pass, { m_atmosphere_pass->m_transmittance_lut_descriptor_set });

				// barrier
				{
					BarrierDesc desc1;
					ImageMemoryBarrierDesc transmittance_lut_barrier;
					transmittance_lut_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
					transmittance_lut_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					transmittance_lut_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
					transmittance_lut_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;
					transmittance_lut_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					transmittance_lut_barrier.texture = m_atmosphere_pass->transmittance_lut;
					desc1.image_memory_barriers.push_back(transmittance_lut_barrier);
					desc1.src_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					desc1.dst_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					InsertBarrier(i, m_command_buffer, desc1);
				}

				m_command_buffer->Dispatch(i, m_atmosphere_pass->m_direct_irradiance_lut_pass, { m_atmosphere_pass->m_direct_irradiance_lut_descriptor_set });

				m_command_buffer->Dispatch(i, m_atmosphere_pass->m_single_scattering_lut_pass, { m_atmosphere_pass->m_single_scattering_lut_descriptor_set });

				// barrier
				{
					BarrierDesc desc2;

					ImageMemoryBarrierDesc delta_r_barrier;
					delta_r_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
					delta_r_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					delta_r_barrier.texture = m_atmosphere_pass->single_rayleigh_scattering_lut;
					delta_r_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
					delta_r_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

					ImageMemoryBarrierDesc delta_mie_barrier;
					delta_mie_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
					delta_mie_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					delta_mie_barrier.texture = m_atmosphere_pass->single_mie_scattering_lut;
					delta_mie_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
					delta_mie_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

					ImageMemoryBarrierDesc irradiance_barrier;
					irradiance_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
					irradiance_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					irradiance_barrier.texture = m_atmosphere_pass->direct_irradiance_lut;
					irradiance_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
					irradiance_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

					ImageMemoryBarrierDesc multi_scattering_barrier;
					multi_scattering_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
					multi_scattering_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
					multi_scattering_barrier.texture = m_atmosphere_pass->multi_scattering_lut;
					multi_scattering_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
					multi_scattering_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

					desc2.image_memory_barriers.push_back(delta_r_barrier);
					desc2.image_memory_barriers.push_back(delta_mie_barrier);
					desc2.image_memory_barriers.push_back(irradiance_barrier);
					desc2.image_memory_barriers.push_back(multi_scattering_barrier);

					desc2.src_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					desc2.dst_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;

					InsertBarrier(i, m_command_buffer, desc2);

				}

				for (u32 j = 0; j < m_atmosphere_pass->m_multi_scattering_order; j++) {
					m_atmosphere_pass->scattering_order_push_constants->ranges[0].value = &m_atmosphere_pass->layers[j + 1];
					m_command_buffer->Dispatch(i, m_atmosphere_pass->m_scattering_density_lut, { m_atmosphere_pass->m_scattering_density_lut_descriptor_set });
					// barrier
					{
						BarrierDesc desc;
						desc.src_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						desc.dst_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						InsertBarrier(i, m_command_buffer, desc);
					}
					m_atmosphere_pass->scattering_order_push_constants->ranges[0].value = &m_atmosphere_pass->layers[j];
					m_command_buffer->Dispatch(i, m_atmosphere_pass->m_indirect_irradiance_lut, { m_atmosphere_pass->m_indirect_irradiance_lut_descriptor_set });
					// barrier
					{
						BarrierDesc desc2;

						ImageMemoryBarrierDesc density_barrier;
						density_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						density_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
						density_barrier.texture = m_atmosphere_pass->scattering_density_lut;
						density_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
						density_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

						ImageMemoryBarrierDesc multi_scattering_barrier;
						multi_scattering_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						multi_scattering_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						multi_scattering_barrier.texture = m_atmosphere_pass->single_rayleigh_scattering_lut;
						multi_scattering_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
						multi_scattering_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

						desc2.image_memory_barriers.push_back(density_barrier);
						desc2.image_memory_barriers.push_back(multi_scattering_barrier);

						desc2.src_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						desc2.dst_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT | PipelineStageFlags::PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

						InsertBarrier(i, m_command_buffer, desc2);

					}
					m_atmosphere_pass->scattering_order_push_constants->ranges[0].value = &m_atmosphere_pass->layers[j + 1];
					m_command_buffer->Dispatch(i, m_atmosphere_pass->m_multi_scattering_lut, { m_atmosphere_pass->m_multi_scattering_lut_descriptor_set });
					// barrier
					{
						BarrierDesc desc2;

						ImageMemoryBarrierDesc _scattering_barrier;
						_scattering_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						_scattering_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						_scattering_barrier.texture = m_atmosphere_pass->_scattering_tex;
						_scattering_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
						_scattering_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

						ImageMemoryBarrierDesc multi_scattering_barrier;
						multi_scattering_barrier.src_access_mask = MemoryAccessFlags::ACCESS_SHADER_WRITE_BIT;
						multi_scattering_barrier.dst_access_mask = MemoryAccessFlags::ACCESS_SHADER_READ_BIT;
						multi_scattering_barrier.texture = m_atmosphere_pass->single_rayleigh_scattering_lut;
						multi_scattering_barrier.src_usage = TextureUsage::TEXTURE_USAGE_RW;
						multi_scattering_barrier.dst_usage = TextureUsage::TEXTURE_USAGE_RW;

						desc2.image_memory_barriers.push_back(_scattering_barrier);
						desc2.image_memory_barriers.push_back(multi_scattering_barrier);

						desc2.src_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						desc2.dst_stage = PipelineStageFlags::PIPELINE_STAGE_COMPUTE_SHADER_BIT | PipelineStageFlags::PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

						InsertBarrier(i, m_command_buffer, desc2);
					}
				}
				m_atmosphere_pass->precomputed = true;
			}

			//TODO: barrier

			m_fullscreen_triangle->Draw(i, m_command_buffer, m_atmosphere_pass->m_sky_pass, { m_atmosphere_pass->m_sky_descriptor_set });

			// post process pass
			m_fullscreen_triangle->Draw(i, m_command_buffer, m_post_process_pass->GetPipeline(), { m_post_process_pass->GetDescriptorSet() });
			// final present pass
			m_fullscreen_triangle->Draw(i, m_command_buffer, m_pipeline_manager->Get("present"), { m_present_descriptorSet }, true);

			m_command_buffer->endCommandRecording(i);
		}
	}

	void RenderSystem::PrepareAssests() noexcept
	{
		//m_scene->LoadModel(Path::GetInstance().GetModelPath("earth6378/earth.gltf"), "earth");
		//
		//auto earth = m_scene->GetModel("earth");
		//Math::mat4 scaleMatrix = Math::scale(Math::mat4(1.0f), Math::vec3(6400)); // a hack value due to mesh precision
		//earth->SetModelMatrix(scaleMatrix);
		//earth->UpdateModelMatrix();


		m_scene->LoadModel("C:/Users/hylu/OneDrive/Program/Computer Graphics/models/vulkan_asset_pack_gltf/data/models/FlightHelmet/glTF/FlightHelmet.gltf", "flighthelmet");

		auto flighthelmet = m_scene->GetModel("flighthelmet");
		Math::mat4 scale_mat = Math::scale(Math::mat4(1.0f), Math::vec3(20.0)); // a hack value due to mesh precision
		Math::mat4 traslate_mat = Math::translate(Math::mat4(1.0f), Math::vec3(0.0, 6400.0, 0));
		flighthelmet->SetModelMatrix(traslate_mat * scale_mat);
		flighthelmet->UpdateModelMatrix();

		m_scene->AddDirectLight(Math::vec3(1.0), 1.0, Math::normalize(Math::vec3(0.0, -1.0, -1.0)));
	}

	void RenderSystem::CreatePipelines() noexcept
	{

		m_geometry_pass = std::make_shared<Geometry>(m_scene, m_pipeline_manager, m_device, m_render_context);

		m_light_pass = std::make_shared<LightPass>(m_scene, m_pipeline_manager, m_device, m_render_context);

		m_atmosphere_pass = std::make_shared<Atmosphere>(m_pipeline_manager, m_device, m_command_buffer, m_render_context);

		m_post_process_pass = std::make_shared<PostProcess>(m_pipeline_manager, m_device, m_render_context);

		CreatePresentPipeline();
	}
	void RenderSystem::CreatePresentPipeline() noexcept
	{
		// present pass

		std::shared_ptr<DescriptorSetInfo> present_descriptor_set_create_info = std::make_shared<DescriptorSetInfo>();
		present_descriptor_set_create_info->AddBinding(DescriptorType::DESCRIPTOR_TYPE_TEXTURE, SHADER_STAGE_PIXEL_SHADER);
		m_present_descriptorSet = std::make_shared<DescriptorSet>(m_device, present_descriptor_set_create_info);
		std::shared_ptr<DescriptorSetLayouts> presentDescriptorSetLayout = std::make_shared<DescriptorSetLayouts>();
		presentDescriptorSetLayout->layouts.push_back(m_present_descriptorSet->GetLayout());

		std::shared_ptr<Shader> presentVs = std::make_shared<Shader>(m_device->Get(), Path::GetInstance().GetShaderPath("simplevs.vert.spv"));
		std::shared_ptr<Shader> presentPs = std::make_shared<Shader>(m_device->Get(), Path::GetInstance().GetShaderPath("present.frag.spv"));

		GraphicsPipelineCreateInfo presentPipelineCreateInfo;
		presentPipelineCreateInfo.name = "present";
		presentPipelineCreateInfo.vs = presentVs;
		presentPipelineCreateInfo.ps = presentPs;
		presentPipelineCreateInfo.descriptor_layouts = presentDescriptorSetLayout;
		std::vector<AttachmentCreateInfo> presentAttachmentsCreateInfo{
			{TextureFormat::TEXTURE_FORMAT_RGBA16_UNORM, COLOR_ATTACHMENT | PRESENT_SRC, TextureType::TEXTURE_TYPE_2D, m_render_context.width, m_render_context.height} };
		m_pipeline_manager->createPresentPipeline(presentPipelineCreateInfo, presentAttachmentsCreateInfo, m_render_context, m_swap_chain);
	}

	void RenderSystem::InitializeRenderAPI(RenderBackend backend) noexcept
	{
		switch (backend)
		{
		case Horizon::RenderBackend::RENDER_BACKEND_NONE:
			break;
		case Horizon::RenderBackend::RENDER_BACKEND_VULKAN:
			m_render_api = std::make_shared<RHI::RHIVulkan>();
			break;
		case Horizon::RenderBackend::RENDER_BACKEND_DX12:
			m_render_api = std::make_shared<RHI::RHIDX12>();
			break;
		}
		m_render_api->InitializeRenderer();
		m_render_api->CreateSwapChain(m_window);
	}
}