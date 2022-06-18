#include <algorithm>


#include <runtime/function/rhi/dx12/DX12Buffer.h>
#include <runtime/function/rhi/dx12/DX12CommandList.h>

namespace Horizon {
	namespace RHI {

		DX12CommandList::DX12CommandList(CommandQueueType type, ID3D12GraphicsCommandList6* command_list) noexcept : CommandList(type), m_command_list(command_list)
		{

		}

		DX12CommandList::~DX12CommandList() noexcept
		{
		}

		void DX12CommandList::BeginRecording() noexcept {
			is_recoring = true;

			//m_command_list->Reset();
			
		}

		void DX12CommandList::EndRecording() noexcept {
			is_recoring = false;
			m_command_list->Close();
		}

		// graphics commands
		void DX12CommandList::BeginRenderPass() noexcept {

			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}

			if (m_type != CommandQueueType::GRAPHICS) {
				LOG_ERROR("invalid commands for current commandlist, expect graphics commandlist");
				return;
			}

			//m_command_list->BeginRenderPass();

		}

		void DX12CommandList::EndRenderPass() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::GRAPHICS) {
				LOG_ERROR("invalid commands for current commandlist, expect graphics commandlist");
				return;
			}
			
			m_command_list->EndRenderPass();
		}
		void DX12CommandList::Draw() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::GRAPHICS) {
				LOG_ERROR("invalid commands for current commandlist, expect graphics commandlist");
				return;
			}
		}
		void DX12CommandList::DrawIndirect() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::GRAPHICS) {
				LOG_ERROR("invalid commands for current commandlist, expect graphics commandlist");
				return;
			}
		}

		// compute commands
		void DX12CommandList::Dispatch() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::COMPUTE) {
				LOG_ERROR("invalid commands for current commandlist, expect compute commandlist");
				return;
			}

			//m_command_list->Dispatch();
		}
		void DX12CommandList::DispatchIndirect() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::COMPUTE) {
				LOG_ERROR("invalid commands for current commandlist, expect compute commandlist");
				return;
			}
			//m_command_list->Dispatch();
		}

		// transfer commands
		void DX12CommandList::UpdateBuffer(Buffer* buffer, void* data, u64 size) noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::TRANSFER) {
				LOG_ERROR("invalid commands for current commandlist, expect transfer commandlist");
				return;
			}

			assert(buffer->GetBufferSize() == size);

			// cannot update static buffer more than once
			bool& initialized = buffer->Initialized();
			if (!(buffer->GetBufferUsage() & BufferUsage::BUFFER_USAGE_DYNAMIC_UPDATE) && !initialized) {
				LOG_ERROR("buffer {} is a static buffer and is initialized", (void*)buffer);
				return;
			}

			// frequently updated buffer
			{
				initialized = true;
				auto dx12_buffer = dynamic_cast<DX12Buffer*>(buffer);

				DX12Buffer* stage_buffer = GetStageBuffer(dx12_buffer->m_allocator, BufferCreateInfo{ BufferUsage::BUFFER_USAGE_TRANSFER_SRC , size });
				//stage_buffer->m_allocation->SetResource();
				//if (memcmp(stage_buffer->m_allocation_info.pMappedData, data, size) == 0) {
				//	LOG_DEBUG("prev buffer and current buffer are same");
				//	return;
				//}
				
				memcpy(stage_buffer, data, size);

				//barrier 1
				{
					BufferMemoryBarrierDesc bmb{};
					bmb.src_access_mask = MemoryAccessFlags::ACCESS_HOST_WRITE_BIT;
					bmb.dst_access_mask = MemoryAccessFlags::ACCESS_TRANSFER_READ_BIT;
					bmb.buffer = stage_buffer->GetBufferPointer();
					bmb.offset = 0;
					bmb.size = stage_buffer->GetBufferSize();

					BarrierDesc desc{};
					desc.src_stage = PipelineStageFlags::PIPELINE_STAGE_HOST_BIT;
					desc.dst_stage = PipelineStageFlags::PIPELINE_STAGE_TRANSFER_BIT;
					desc.buffer_memory_barriers.emplace_back(bmb);

					InsertBarrier(desc);
				}

				// copy to gpu buffer
				CopyBuffer(stage_buffer, dx12_buffer);
				
			}
		}

		void DX12CommandList::CopyBuffer(Buffer* src_buffer, Buffer* dst_buffer) noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::TRANSFER) {
				LOG_ERROR("invalid commands for current commandlist, expect transfer commandlist");
				return;
			}
			auto dx12_src_buffer = dynamic_cast<DX12Buffer*>(src_buffer);
			auto dx12_dst_buffer = dynamic_cast<DX12Buffer*>(dst_buffer);
			CopyBuffer(dx12_src_buffer, dx12_dst_buffer);
		}

		void DX12CommandList::CopyBuffer(DX12Buffer* src_buffer, DX12Buffer* dst_buffer) noexcept {
			assert(dst_buffer->GetBufferSize() == src_buffer->GetBufferSize());
			// copy buffer
		}

		void DX12CommandList::UpdateTexture() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::TRANSFER) {
				LOG_ERROR("invalid commands for current commandlist, expect transfer commandlist");
			}
		}

		void DX12CommandList::CopyTexture() noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}
			if (m_type != CommandQueueType::TRANSFER) {
				LOG_ERROR("invalid commands for current commandlist, expect transfer commandlist");
			}
		}

		void DX12CommandList::InsertBarrier(const BarrierDesc& desc) noexcept {
			if (!is_recoring) {
				LOG_ERROR("command buffer isn't recording");
				return;
			}

			VkPipelineStageFlags src_stage = ToVkPipelineStage(desc.src_stage);
			VkPipelineStageFlags dst_stage = ToVkPipelineStage(desc.dst_stage);

			std::vector<VkBufferMemoryBarrier> buffer_memory_barriers(desc.buffer_memory_barriers.size());
			std::vector<VkImageMemoryBarrier> image_memory_barriers(desc.image_memory_barriers.size());

			for (u32 i = 0; i < desc.buffer_memory_barriers.size(); i++) {
				buffer_memory_barriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				buffer_memory_barriers[i].srcAccessMask = ToVkMemoryAccessFlags(desc.buffer_memory_barriers[i].src_access_mask);
				buffer_memory_barriers[i].dstAccessMask = ToVkMemoryAccessFlags(desc.buffer_memory_barriers[i].dst_access_mask);
				buffer_memory_barriers[i].buffer = static_cast<VkBuffer>(desc.buffer_memory_barriers[i].buffer);
				buffer_memory_barriers[i].offset = desc.buffer_memory_barriers[i].offset;
				buffer_memory_barriers[i].size = desc.buffer_memory_barriers[i].size;
			}

			//for (u32 i = 0; i < desc.image_memory_barriers.size(); i++) {
			//	image_memory_barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			//	image_memory_barriers[i].srcAccessMask = ToVkMemoryAccessFlags(desc.image_memory_barriers[i].src_access_mask);
			//	image_memory_barriers[i].dstAccessMask = ToVkMemoryAccessFlags(desc.image_memory_barriers[i].dst_access_mask);
			//	image_memory_barriers[i].oldLayout = ToVkImageLayout(desc.image_memory_barriers[i].src_usage);
			//	image_memory_barriers[i].newLayout = ToVkImageLayout(desc.image_memory_barriers[i].dst_usage);
			//	image_memory_barriers[i].image = desc.image_memory_barriers[i].texture->GetImage();
			//	image_memory_barriers[i].subresourceRange = desc.image_memory_barriers[i].texture->GetSubresourceRange();

			//}

			//m_command_list->ResourceBarrier();
		}

		DX12Buffer* DX12CommandList::GetStageBuffer(D3D12MA::Allocator* allocator, const BufferCreateInfo& buffer_create_info) noexcept
		{
			if (m_stage_buffer) {
				return m_stage_buffer;
			}
			else {
				m_stage_buffer = new DX12Buffer(allocator, buffer_create_info, MemoryFlag::CPU_VISABLE_MEMORY);
				return m_stage_buffer;
			}
		}

	}
}

