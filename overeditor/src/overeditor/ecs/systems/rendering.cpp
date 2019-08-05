#include <overeditor/ecs/systems/rendering.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <tuple>

namespace overeditor::systems::graphics {
    typedef std::tuple<Drawable, glm::mat4> RenderingTuple;

    vk::CommandBuffer create_bind(
            const vk::Device &device,
            const vk::CommandPool &commandPool
    ) {
        vk::CommandBuffer buf;
        auto i = vk::CommandBufferAllocateInfo(
                commandPool,
                vk::CommandBufferLevel::eSecondary,
                1
        );
        device.allocateCommandBuffers(
                &i,
                &buf
        );
        return buf;
    }

    vk::CommandBuffer get_bind(
            Camera &camera,
            const vk::Device &device,
            const vk::CommandPool &pool,
            const vk::RenderPass &renderPass,
            const overeditor::graphics::shaders::Shader *shader,
            glm::mat4 mvp
    ) {
        auto &bufs = camera.shaderBinds;
        vk::CommandBuffer buf;
        if (bufs.count(shader) == 0) {

            buf = create_bind(device, pool);
            bufs[shader] = buf;
        } else {
            buf = bufs[shader];
        }

        auto iInfo = vk::CommandBufferInheritanceInfo(
                renderPass, 0
        );
        buf.begin(
                vk::CommandBufferBeginInfo(
                        (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse |
                        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
                        &iInfo
                )
        );
        /*buf.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                shader.getLayout(), 0,
                {camera.matricesSet},
                {}
        );*/
        LOG_INFO << "Yay: " << glm::to_string(mvp);
        buf.pushConstants(
                shader->getLayout(),
                vk::ShaderStageFlagBits::eVertex,
                 0,
                sizeof(glm::mat4),
                &mvp
        );
        buf.end();
        return buf;
    }

    std::vector<vk::CommandBuffer> &
    find_or_create(
            std::map<overeditor::graphics::shaders::Shader *, std::vector<vk::CommandBuffer>> &map,
            overeditor::graphics::shaders::Shader *pShader
    ) {
        auto it = map.count(pShader);
        if (it > 0) {
            return map.at(pShader);
        }
        return map[pShader];
    }

    void RenderingSystem::render(
            Camera &camera,
            const Transform &cameraTransform,
            entityx::EntityManager &entities
    ) {

        std::vector<RenderingTuple> toRender;

        //Re-record buffer
        entityx::ComponentHandle<Transform> transform;
        entityx::ComponentHandle<Drawable> drawable;
        for (entityx::Entity e : entities.entities_with_components(transform, drawable)) {
            auto &t = *transform;
            auto &d = *drawable;
            auto model = glm::translate(glm::toMat4(t.rotation)/* * glm::scale(glm::mat4(1), t.scale)*/, t.position);
            LOG_INFO << "Model: " << glm::to_string(model);
            toRender.emplace_back(d, model);
        }
        if (toRender.empty()) {
            //Nothing to draw
            LOG_INFO << "WWGP";
            return;
        }

        const auto &device = context->getDevice();
        auto view = glm::lookAt(
                cameraTransform.position,
                cameraTransform.position + Transform::getForward(cameraTransform),
                Transform::getUp(cameraTransform)
        );
        auto projection = glm::perspective(
                glm::radians(camera.fieldOfView),
                camera.aspectRatio,
                0.1F,
                camera.depth
        );
        LOG_INFO << "View: " << glm::to_string(view);
        LOG_INFO << "Projection: " << glm::to_string(projection);

        std::vector<vk::CommandBuffer> secondaryBuffers;
        std::map<overeditor::graphics::shaders::Shader *, std::vector<vk::CommandBuffer>> executionMap;

        //Draw buffers
        for (auto &element : toRender) {
            Drawable &d = std::get<0>(element);
            glm::mat4 model = std::get<1>(element);
            std::vector<vk::CommandBuffer> &vec = find_or_create(
                    executionMap,
                    const_cast<overeditor::graphics::shaders::Shader *>(d.shader)
            );
            vec.push_back(
                    get_bind(
                            camera, device, pool, renderPass, d.shader, projection * view * model
                    )
            );
            vec.push_back(d.buf);
        }
        for (const std::pair<overeditor::graphics::shaders::Shader *const, std::vector<vk::CommandBuffer>> &element : executionMap) {
            const auto &bufs = element.second;
            secondaryBuffers.insert(
                    secondaryBuffers.end(),
                    bufs.begin(),
                    bufs.end()
            );
        }
        const auto &swapchain = context->getSwapChainContext()->getSwapchain();
        uint32_t imageIndex;
        device.acquireNextImageKHR(
                swapchain,
                std::numeric_limits<uint64_t>::max(),
                imageAvailableSemaphore,
                nullptr,
                &imageIndex
        );
        primaryBuffer.begin(
                vk::CommandBufferBeginInfo(
                        (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse
                )
        );

        vk::ClearValue value = vk::ClearColorValue(
                std::array<float, 4>(
                        {
                                0.0F, 0.0F, 0.0F, 1.0F
                        }
                )
        );
        /*primaryBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                nullptr, 0,
                {camera.matricesSet},
                {}
        );*/
        primaryBuffer.beginRenderPass(
                vk::RenderPassBeginInfo(
                        renderPass,
                        framebuffers[imageIndex],
                        vk::Rect2D(vk::Offset2D(),
                                   context->getSwapChainContext()->getSwapchainExtent()),
                        1,
                        &value
                ),
                vk::SubpassContents::eSecondaryCommandBuffers
        );


        primaryBuffer.executeCommands(secondaryBuffers);
        primaryBuffer.endRenderPass();
        primaryBuffer.end();
        // Submit
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo info = vk::SubmitInfo(
                1, &imageAvailableSemaphore, waitStages,
                1, &primaryBuffer,
                1, &renderFinishedSemaphore
        );
        auto &queue = context->getQueueContext()->getGraphicsQueue();
        vkAssertOk(
                queue.submit(1, &info, nullptr)
        )
        LOG_INFO << "Submitted";
        vkAssertOk(
                queue.presentKHR(
                        vk::PresentInfoKHR(
                                1, &renderFinishedSemaphore,
                                1, &swapchain,
                                &imageIndex
                        )
                )
        )
        queue.waitIdle();
    }

    RenderingSystem::RenderingSystem(const overeditor::graphics::DeviceContext &context) {
        RenderingSystem::context = &context;
        auto scContext = context.getSwapChainContext();
        vk::AttachmentDescription colorAttachment(
                (vk::AttachmentDescriptionFlags) 0, // Flags
                scContext->getSwapchainFormat(), // Format
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::ePresentSrcKHR
        );
        vk::AttachmentReference colorAttachmentRef(
                0,
                vk::ImageLayout::eColorAttachmentOptimal
        );
        vk::SubpassDescription subpass(
                (vk::SubpassDescriptionFlags) 0,
                vk::PipelineBindPoint::eGraphics,
                0, nullptr,
                1, &colorAttachmentRef
        );
        auto dep = vk::SubpassDependency(
                VK_SUBPASS_EXTERNAL, 0,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                (vk::AccessFlags) 0,
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
        );
        auto &device = context.getDevice();
        pool = device.createCommandPool(
                vk::CommandPoolCreateInfo(
                        (vk::CommandPoolCreateFlags) vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                        context.getQueueContext()->getFamilyIndices().getGraphics().get()
                )
        );
        renderPass = device.createRenderPass(
                vk::RenderPassCreateInfo(
                        (vk::RenderPassCreateFlags) 0,
                        1, &colorAttachment,
                        1, &subpass,
                        1, &dep
                )
        );
        primaryBuffer = device.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(
                        pool,
                        vk::CommandBufferLevel::ePrimary,
                        1
                )
        )[0];
        auto &imgs = scContext->getSwapchainImages();
        size_t count = imgs.size();
        framebuffers.reserve(count);
        auto ex = scContext->getSwapchainExtent();
        for (size_t i = 0; i < count; i++) {
            auto &view = imgs[i].getView();
            framebuffers.emplace_back(
                    device.createFramebuffer(
                            vk::FramebufferCreateInfo(
                                    (vk::FramebufferCreateFlags) 0,
                                    renderPass,
                                    1, &view,
                                    ex.width, ex.height, 1
                            )
                    )
            );
        }
        imageAvailableSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
        renderFinishedSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
    }

    void RenderingSystem::update(
            entityx::EntityManager &entities,
            entityx::EventManager &events,
            entityx::TimeDelta dt
    ) {
        entityx::ComponentHandle<Camera> camera;
        entityx::ComponentHandle<Transform> cameraTransform;
        for (auto e : entities.entities_with_components(camera, cameraTransform)) {
            render(*camera, *cameraTransform, entities);
        }
    }

}