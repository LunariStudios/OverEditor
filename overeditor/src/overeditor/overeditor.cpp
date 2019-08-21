#include <vulkan/vulkan.h>
#include <overeditor/application.h>
#include <plog/Log.h>
#include <overeditor/ecs/components/common.h>
#include <overeditor/graphics/shaders/shader.h>

using VertexLayout = overeditor::VertexLayout;
using DescriptorLayout = overeditor::DescriptorLayout;
using PushConstantsLayout = overeditor::PushConstantsLayout;

int main() {
    auto app = overeditor::Application();
    auto cube = app.entities.create();

    auto ctx = app.getDeviceContext();


    std::filesystem::path resDirectory = std::filesystem::current_path() / "res";
    LOG_INFO << "Using resources located at \"" << resDirectory.string() << "\"";
    auto &system = app.getRenderingSystem();
    auto &renderPass = system.get()->renderPass;
    overeditor::Shader shader(
            "standart",
            overeditor::ShaderSource(
                    resDirectory / "frag.spv",
                    std::vector<DescriptorLayout>(
                            {

                            }
                    ),
                    VertexLayout(),
                    PushConstantsLayout()
            ),
            overeditor::ShaderSource(
                    resDirectory / "vert.spv",
                    std::vector<DescriptorLayout>(
                            {
                                    DescriptorLayout(
                                            {
                                                    // Camera matrix
                                                    overeditor::DescriptorElement(
                                                            sizeof(CameraMatrices),
                                                            1,
                                                            vk::DescriptorType::eUniformBuffer,
                                                            true
                                                    ),
                                            }
                                    ),
                                    DescriptorLayout(
                                            {
                                                    // Model matrix
                                                    overeditor::DescriptorElement(
                                                            sizeof(glm::mat4),
                                                            1,
                                                            vk::DescriptorType::eUniformBuffer
                                                    )
                                            }
                                    )
                            }
                    ),
                    VertexLayout(
                            {
                                    overeditor::VertexElement(
                                            sizeof(float),
                                            3,
                                            vk::Format::eR32G32B32Sfloat
                                    )
                            }
                    ),
                    PushConstantsLayout(
                            {
                            }
                    )

            ),
            *ctx,
            renderPass
    );
    cube.assign<Transform>(
            glm::vec3(0, 0, 0) //Position
    );
    glm::vec3 cubeVertices[] = {
            glm::vec3(0.5, -0.5, 0.5),//0 BFL
            glm::vec3(-0.5, -0.5, 0.5),//1 BFR
            glm::vec3(-0.5, -0.5, -0.5),//2 BBL
            glm::vec3(0.5, -0.5, -0.5),//3 BBR
            glm::vec3(0.5, 0.5, 0.5),//4
            glm::vec3(-0.5, 0.5, 0.5),//5
            glm::vec3(-0.5, 0.5, -0.5),//6
            glm::vec3(0.5, 0.5, -0.5)//7
    };
    uint16_t cubeIndexes[] = {
            0, 1, 2, 2, 3, 0, //bottom
            4, 5, 6, 6, 7, 4,//top
            0, 1, 6, 6, 7, 0,//left
            1, 2, 5, 5, 6, 1,//right
            0, 1, 4, 4, 5, 0,//front
            2, 3, 6, 6, 7, 2 //back
    };
    auto b = overeditor::GeometryBuffer::create(
            shader.getVertSource().getShaderLayout(),
            *ctx,
            cubeVertices, 8,
            cubeIndexes, 36
    );
    vk::CommandPool secondaryPool = ctx->getDevice().createCommandPool(
            vk::CommandPoolCreateInfo(
                    vk::CommandPoolCreateFlagBits::eProtected,
                    ctx->getQueueContext()->getFamilyIndices().getGraphics().get()
            )
    );
    cube.assign_from_copy(
            Drawable::forGeometry(*ctx, shader, secondaryPool, renderPass, 0, b)
    );
    auto camera = app.entities.create();
    auto &swapchain = *ctx->getSwapChainContext();
    auto ext = swapchain.getSwapchainExtent();
    camera.assign<Transform>(
            glm::vec3(0, 0, 10),
            glm::quat_identity<float, glm::precision::defaultp>(),
            glm::vec3(1, 1, 1)
    );
    camera.assign_from_copy(
            Camera::createFor(
                    *ctx, renderPass, 100,
                    90,
                    (float) ext.width / ext.height
            )
    );

    overeditor::Event<float>::EventListener rotator = [&](float dt) {
        Transform &t = *cube.component<Transform>();
        glm::vec3 rot = glm::eulerAngles(t.rotation);
        rot.x += 0.1;
        t.rotation = glm::quat(rot);

    };
    app.getSceneTick().getEarlyStep() += &rotator;
    app.run();
}