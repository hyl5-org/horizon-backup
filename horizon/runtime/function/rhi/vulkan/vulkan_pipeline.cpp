#include "vulkan_pipeline.h"

#include <algorithm>

#include "runtime/function/rhi/vulkan/vulkan_shader.h"

namespace Horizon::Backend {

VulkanPipeline::VulkanPipeline(const VulkanRendererContext &context, const GraphicsPipelineCreateInfo &create_info) noexcept
    : m_context(context){
    m_create_info.type = PipelineType::GRAPHICS;
    m_create_info.gpci = const_cast<GraphicsPipelineCreateInfo *>(&create_info);
}

VulkanPipeline::VulkanPipeline(const VulkanRendererContext &context, const ComputePipelineCreateInfo &create_info) noexcept
    : m_context(context){
    m_create_info.type = PipelineType::COMPUTE;
    m_create_info.cpci = const_cast<ComputePipelineCreateInfo *>(&create_info);
}

VulkanPipeline::~VulkanPipeline() noexcept {
    vkDestroyPipeline(m_context.device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_context.device, m_pipeline_layout, nullptr);
}
void VulkanPipeline::SetShader(Shader *shader) {
    // check pipelinetype
    shaders[static_cast<u32>(shader->GetType())] = shader;
}

//DescriptorSet *VulkanPipeline::GetDescriptorSet(ResourceUpdateFrequency frequency) {
//
//    //if (frequency == ResourceUpdateFrequency::BINDLESS) {
//
//    //    if (!m_descriptor_set_allocator.m_bindless_descriptor_pool) {
//    //        m_descriptor_set_allocator.CreateBindlessDescriptorPool();
//    //    }
//
//    //    VkDescriptorSetAllocateInfo alloc_info{};
//    //    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    //    alloc_info.descriptorPool = m_descriptor_set_allocator.m_bindless_descriptor_pool;
//
//    //    alloc_info.descriptorSetCount = 1;
//    //    VkDescriptorSetLayout layout = m_descriptor_set_allocator.GetVkDescriptorSetLayout(
//    //        this->m_pipeline_layout_desc.descriptor_set_hash_key[static_cast<u32>(frequency)]);
//    //    alloc_info.pSetLayouts = &layout;
//
//    //    VkDescriptorSetVariableDescriptorCountAllocateInfo count_info{};
//    //    count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
//    //    u32 max_binding = m_descriptor_set_allocator.k_max_bindless_resources;
//    //    count_info.descriptorSetCount = 1;
//    //    // This number is the max allocatable count
//    //    count_info.pDescriptorCounts = &max_binding;
//    //    alloc_info.pNext = &count_info;
//    //    VkDescriptorSet vk_ds;
//    //    CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device, &alloc_info, &vk_ds));
//    //    DescriptorSet *set = Memory::Alloc<VulkanDescriptorSet>(m_context, frequency,
//    //                                                            rsd.descriptors[static_cast<u32>(frequency)], vk_ds);
//    //    m_descriptor_set_allocator.allocated_sets.push_back(set);
//    //    return set;
//    //} else {
//
//    //    if (!m_descriptor_set_allocator.m_temp_descriptor_pool) {
//    //        m_descriptor_set_allocator.CreateDescriptorPool();
//    //    }
//
//    //    VkDescriptorSetAllocateInfo alloc_info{};
//    //    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//
//    //    VkDescriptorSetLayout layout = m_descriptor_set_allocator.GetVkDescriptorSetLayout(
//    //        this->m_pipeline_layout_desc.descriptor_set_hash_key[static_cast<u32>(frequency)]);
//
//    //    alloc_info.descriptorPool = m_descriptor_set_allocator.m_temp_descriptor_pool;
//    //    alloc_info.descriptorSetCount = 1;
//    //    alloc_info.pSetLayouts = &layout;
//    //    VkDescriptorSet vk_ds;
//    //    CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device, &alloc_info, &vk_ds));
//    //    DescriptorSet *set = Memory::Alloc<VulkanDescriptorSet>(m_context, frequency,
//    //                                                            rsd.descriptors[static_cast<u32>(frequency)], vk_ds);
//    //    m_descriptor_set_allocator.allocated_sets.push_back(set);
//    //    return set;
//    //}
//    ////new VulkanDescriptorSet();
//
//    //// m_descriptor_set_allocator.UpdateDescriptorPoolInfo(this, m_pipeline_layout_desc.descriptor_set_hash_key);
//
//    ////if (m_descriptor_set_allocator.pool_created == false) {
//    ////    m_descriptor_set_allocator.CreateDescriptorPool();
//    ////    m_descriptor_set_allocator.pool_created = true;
//    ////}
//
//    // auto &resource = m_descriptor_set_allocator.pipeline_descriptor_set_resources[this];
//    // u32 freq = static_cast<u32>(frequency);
//
//    // if (resource.layout_hash_key[freq] == m_descriptor_set_allocator.m_empty_descriptor_set_layout_hash_key) {
//    //     LOG_ERROR("return an empty descriptor set");
//    //     return {};
//    // }
//
//    // assert(("descriptor pool not allocated",
//    //         m_descriptor_set_allocator.m_descriptor_pools[freq].back() != VK_NULL_HANDLE));
//
//    // u32 counter = resource.m_used_set_counter[static_cast<u32>(frequency)]++;
//
//    // VkDescriptorSet set =
//    //     m_descriptor_set_allocator.pipeline_descriptor_set_resources[this].allocated_sets[freq][counter];
//
//    //    auto &resource = m_descriptor_set_allocator.pipeline_descriptor_set_resources.at(this);
//    // u32 freq = static_cast<u32>(frequency);
//    //// allocate sets to be used
//    // VkDescriptorSetAllocateInfo alloc_info{};
//    // alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//
//    // VkDescriptorSetLayout layout = m_descriptor_set_allocator.FindLayout(resource.layout_hash_key[freq]);
//
//    // Container::Array<VkDescriptorSetLayout> layouts(m_reserved_max_sets[freq], layout);
//    // resource.allocated_sets[freq].resize(m_reserved_max_sets[freq]);
//
//    // alloc_info.descriptorPool = m_descriptor_set_allocator.m_descriptor_pools[static_cast<u32>(freq)].back();
//    // alloc_info.descriptorSetCount = m_reserved_max_sets[freq];
//    // alloc_info.pSetLayouts = layouts.data();
//
//    // CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device, &alloc_info, resource.allocated_sets[freq].data()));
//
//    // Container::Array<DescriptorSet *> sets(count);
//    // for (u32 i = 0; i < count; i++) {
//    //     sets[i] = new DescriptorSet()
//    // }
//    // return sets;
//    return {};
//}
//
//void ParseRootSignatureFromShader(VulkanShader *shader, ResourceUpdateFrequency frequency &dst_binding) {
//    dst_binding.merge(shader->descriptor_bindings[static_cast<u32>(frequency)]);
//    shader->descriptor_bindings[static_cast<u32>(frequency)].clear();
//}

//Container::HashMap<Container::String, VkDescriptorSetLayoutBinding>
//VulkanPipeline::GetDescriptorSetLayoutBinding(ResourceUpdateFrequency frequency) noexcept {
//    Container::HashMap<Container::String, VkDescriptorSetLayoutBinding> pipeline_descriptor_set_layout_binding{};
//    Container::Array<Container::String>
//    if (m_create_info.type == PipelineType::GRAPHICS) {
//        ParseRootSignatureFromShader(reinterpret_cast<VulkanShader *>(m_vs), frequency,
//                                     pipeline_descriptor_set_layout_binding);
//        ParseRootSignatureFromShader(reinterpret_cast<VulkanShader *>(m_ps), frequency,
//                                     pipeline_descriptor_set_layout_binding);
//    } else if (m_create_info.type == PipelineType::COMPUTE) {
//        ParseRootSignatureFromShader(reinterpret_cast<VulkanShader *>(m_cs), frequency,
//                                     pipeline_descriptor_set_layout_binding);
//    }
//    //return pipeline_descriptor_set_layout_binding;
//    return pipeline_descriptor_set_layout_binding;
//}

void VulkanPipeline::Create() {
    // crete descriptorset layout
    // crete pipeline layout
    //create pipeline
}

void VulkanPipeline::CreateGraphicsPipeline() {

    auto ci = m_create_info.gpci;
    {
        auto stack_memory = Memory::GetStackMemoryResource(4096);
        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.flags = 0;
        graphics_pipeline_create_info.pNext = nullptr;

        uint32_t input_binding_count = 0;
        Container::FixedArray<VkVertexInputBindingDescription, MAX_BINDING_COUNT> input_bindings = {};
        uint32_t input_attribute_count = 0;
        Container::FixedArray<VkVertexInputAttributeDescription, MAX_ATTRIBUTE_COUNT> input_attributes = {};
        Container::Array<VkPipelineShaderStageCreateInfo> shader_stage_create_infos(&stack_memory);
        LOG_INFO("{}", sizeof(VkPipelineShaderStageCreateInfo));
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        VkPipelineMultisampleStateCreateInfo multi_sample_state_create_info{};
        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
        VkPipelineViewportStateCreateInfo view_port_state_create_info{};
        VkPipelineRenderingCreateInfo rendering_create_info{};
        Container::Array<VkPipelineColorBlendAttachmentState> color_blend_attachment_state(&stack_memory);
        // shader stage
        {

            shader_stage_create_infos.reserve(2);
            {
                auto vs = reinterpret_cast<VulkanShader *>(m_vs);

                shader_stage_create_infos.emplace_back(VkPipelineShaderStageCreateInfo{
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, ToVkShaderStageBit(vs->GetType()),
                    vs->m_shader_module, "main", nullptr});

                auto ps = reinterpret_cast<VulkanShader *>(m_ps);

                shader_stage_create_infos.emplace_back(VkPipelineShaderStageCreateInfo{
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, ToVkShaderStageBit(ps->GetType()),
                    ps->m_shader_module, "main", nullptr});
            }

            graphics_pipeline_create_info.stageCount = static_cast<u32>(shader_stage_create_infos.size());
            graphics_pipeline_create_info.pStages = shader_stage_create_infos.data();
        }

        // vertex input state
        {

            uint32_t binding_value = UINT32_MAX;

            // Initial values
            for (u32 i = 0; i < ci->vertex_input_state.attribute_count; ++i) {
                auto *attrib = &(ci->vertex_input_state.attributes[i]);

                if (binding_value != attrib->binding) {
                    binding_value = attrib->binding;
                    ++input_binding_count;
                }
                u32 binding_index = input_binding_count - 1;
                input_bindings[binding_index].binding = binding_value;
                if (attrib->input_rate == VertexInputRate::VERTEX_ATTRIB_RATE_INSTANCE) {
                    input_bindings[binding_index].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                } else {
                    input_bindings[binding_index].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                }
                input_bindings[binding_index].stride +=
                    GetStrideFromVertexAttributeDescription(attrib->attrib_format, attrib->portion);

                input_attributes[input_attribute_count].location = attrib->location;
                input_attributes[input_attribute_count].binding = attrib->binding;
                input_attributes[input_attribute_count].format =
                    ToVkImageFormat(attrib->attrib_format, attrib->portion);
                input_attributes[input_attribute_count].offset = attrib->offset;
                ++input_attribute_count;
            }

            vertex_input_state_create_info.flags = 0;
            vertex_input_state_create_info.pNext = nullptr;
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = input_binding_count;
            vertex_input_state_create_info.pVertexBindingDescriptions = input_bindings.data();
            vertex_input_state_create_info.vertexAttributeDescriptionCount = input_attribute_count;
            vertex_input_state_create_info.pVertexAttributeDescriptions = input_attributes.data();

            graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        }

        // input assembly state
        {

            input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_state_create_info.flags = 0;
            input_assembly_state_create_info.pNext = nullptr;
            input_assembly_state_create_info.topology = ToVkPrimitiveTopology(ci->input_assembly_state.topology);
            input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

            graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
        }

        // tessllation state
        { graphics_pipeline_create_info.pTessellationState = nullptr; }

        // viewport state

        {
            view_port_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            view_port_state_create_info.flags = 0;
            view_port_state_create_info.pNext = nullptr;

            view_port.width = static_cast<f32>(ci->view_port_state.width);
            view_port.height = -static_cast<f32>(ci->view_port_state.height);
            view_port.x = 0.0f;
            view_port.y = -view_port.height;
            view_port.minDepth = 0.0f;
            view_port.maxDepth = 1.0f;

            VkExtent2D extent{};
            extent.width = ci->view_port_state.width;
            extent.height = ci->view_port_state.height;

            scissor.offset = {0, 0};
            scissor.extent = extent;

            view_port_state_create_info.viewportCount = 1;
            view_port_state_create_info.pViewports = &view_port;
            view_port_state_create_info.scissorCount = 1;
            view_port_state_create_info.pScissors = &scissor;

            graphics_pipeline_create_info.pViewportState = &view_port_state_create_info;
        }

        // rasterization state
        {

            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = ToVkPolygonMode(ci->rasterization_state.fill_mode);
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = ToVkCullMode(ci->rasterization_state.cull_mode);
            rasterization_state_create_info.frontFace = ToVkFrontFace(ci->rasterization_state.front_face);
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;

            graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
        }

        // TODO(hylu): mulitsampling
        {

            multi_sample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multi_sample_state_create_info.sampleShadingEnable = VK_FALSE;
            multi_sample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            // multisamplingStateCreateInfo.minSampleShading = 1.0f; // Optional
            // multisamplingStateCreateInfo.pSampleMask = nullptr; // Optional
            // multisamplingStateCreateInfo.alphaToCoverageEnable = VK_FALSE; //
            // Optional multisamplingStateCreateInfo.alphaToOneEnable = VK_FALSE; //
            // Optional

            graphics_pipeline_create_info.pMultisampleState = &multi_sample_state_create_info;
        }

        // color blend state
        {
            color_blend_attachment_state.resize(
                ci->render_target_formats.color_attachment_count); // TODO(hylu): reserve and construct
            for (auto &state : color_blend_attachment_state) {
                state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT;
                state.blendEnable = VK_FALSE;
            }

            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount = static_cast<u32>(color_blend_attachment_state.size());
            color_blend_state_create_info.pAttachments = color_blend_attachment_state.data();
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;
            graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
        }

        // depth stencil
        {

            depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_state_create_info.depthTestEnable = ci->depth_stencil_state.depth_test;
            depth_stencil_state_create_info.depthWriteEnable = ci->depth_stencil_state.depth_write;
            depth_stencil_state_create_info.depthCompareOp = ToVkCompareOp(ci->depth_stencil_state.depth_func);
            depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;

            graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
        }

        // dyanmic state
        { graphics_pipeline_create_info.pDynamicState = nullptr; }

        rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        rendering_create_info.colorAttachmentCount = ci->render_target_formats.color_attachment_count;

        Container::Array<VkFormat> formats(ci->render_target_formats.color_attachment_count, &stack_memory);
        for (u32 i = 0; i < ci->render_target_formats.color_attachment_count; i++) {
            formats[i] = ToVkImageFormat(ci->render_target_formats.color_attachment_formats[i]);
        }

        rendering_create_info.pColorAttachmentFormats = formats.data();
        if (ci->render_target_formats.has_depth)
            rendering_create_info.depthAttachmentFormat =
                ToVkImageFormat(ci->render_target_formats.depth_stencil_format);
        if (ci->render_target_formats.has_stencil)
            rendering_create_info.stencilAttachmentFormat =
                ToVkImageFormat(ci->render_target_formats.depth_stencil_format);

        graphics_pipeline_create_info.pNext = &rendering_create_info;
        // graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;

        graphics_pipeline_create_info.layout = m_pipeline_layout;

        CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_context.device, nullptr, 1, &graphics_pipeline_create_info, nullptr,
                                                  &m_pipeline));
    }
}

void VulkanPipeline::CreateComputePipeline() {
    assert(m_cs != nullptr);

    auto cs = reinterpret_cast<VulkanShader *>(m_cs);
    VkPipelineShaderStageCreateInfo shader_stage_create_info{};
    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage_create_info.module = cs->m_shader_module;
    shader_stage_create_info.pName = "main";

    VkComputePipelineCreateInfo compute_pipeline_create_info{};

    // cache

    compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    compute_pipeline_create_info.flags = 0;
    compute_pipeline_create_info.layout = m_pipeline_layout;
    compute_pipeline_create_info.stage = shader_stage_create_info;
    compute_pipeline_create_info.basePipelineHandle = nullptr;
    compute_pipeline_create_info.basePipelineIndex = 0;
    CHECK_VK_RESULT(
        vkCreateComputePipelines(m_context.device, nullptr, 1, &compute_pipeline_create_info, nullptr, &m_pipeline));
}

void VulkanPipeline::CreatePipelineLayout() {
    Container::Array<VulkanShader *> shader_modules;
    switch (m_create_info.type) {
    case Horizon::PipelineType::GRAPHICS:
        shader_modules.push_back(dynamic_cast<VulkanShader *>(m_vs));
        shader_modules.push_back(dynamic_cast<VulkanShader *>(m_ps));
        break;
    case Horizon::PipelineType::COMPUTE:
        shader_modules.push_back(dynamic_cast<VulkanShader *>(m_cs));
        break;
    default:
        break;
    }
    for (auto *shader_module : shader_modules) {
        for (const auto &shader_resource : shader_module->GetResources()) {
            Container::String key = shader_resource.name;

            // Since 'Input' and 'Output' resources can have the same name, we modify the key string
            if (shader_resource.type == ShaderResourceType::Input ||
                shader_resource.type == ShaderResourceType::Output) {
                key = Container::String{std::to_string(shader_resource.stages)} + "_" + key;
            }

            auto it = shader_resources.find(key);

            if (it != shader_resources.end()) {
                // Append stage flags if resource already exists
                it->second.stages |= shader_resource.stages;
            } else {
                // Create a new entry in the map
                shader_resources.emplace(key, shader_resource);
            }
        }
    }

    // Sift through the map of name indexed shader resources
    // Separate them into their respective sets
    for (auto &it : shader_resources) {
        auto &shader_resource = it.second;

        // Find binding by set index in the map.
        auto it2 = shader_sets.find(shader_resource.set);

        if (it2 != shader_sets.end()) {
            // Add resource to the found set index
            it2->second.push_back(shader_resource);
        } else {
            // Create a new set index and with the first resource
            shader_sets.emplace(shader_resource.set, std::vector<ShaderResource>{shader_resource});
        }
    }

    // Create a descriptor set layout for each shader set in the shader modules
    for (auto &shader_set_it : shader_sets) {
        descriptor_set_layouts.emplace_back(&device.get_resource_cache().request_descriptor_set_layout(
            shader_set_it.first, shader_modules, shader_set_it.second));
    }

    // Collect all the descriptor set layout handles, maintaining set order
    std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles;
    for (uint32_t i = 0; i < descriptor_set_layouts.size(); ++i) {
        if (descriptor_set_layouts[i]) {
            descriptor_set_layout_handles.push_back(descriptor_set_layouts[i]->get_handle());
        } else {
            descriptor_set_layout_handles.push_back(VK_NULL_HANDLE);
        }
    }

    // Collect all the push constant shader resources
    std::vector<VkPushConstantRange> push_constant_ranges;
    for (auto &push_constant_resource : get_resources(ShaderResourceType::PushConstant)) {
        push_constant_ranges.push_back(
            {push_constant_resource.stages, push_constant_resource.offset, push_constant_resource.size});
    }

    VkPipelineLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    create_info.setLayoutCount = static_cast<u32(descriptor_set_layout_handles.size());
    create_info.pSetLayouts = descriptor_set_layout_handles.data();
    create_info.pushConstantRangeCount = to_u32(push_constant_ranges.size());
    create_info.pPushConstantRanges = push_constant_ranges.data();

    // Create the Vulkan pipeline layout handle
    CHECK_VK_RESULT(vkCreatePipelineLayout(m_context.device, &create_info, nullptr, &m_pipeline_layout));

}


} // namespace Horizon::Backend