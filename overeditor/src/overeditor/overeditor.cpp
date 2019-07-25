#include <vulkan/vulkan.h>
#include <overeditor/application.h>
#include <plog/Log.h>
#include <overeditor/ecs/components/common.h>
#include <overeditor/graphics/shaders/shader.h>

using VertexLayout = overeditor::utility::VertexLayout;
using DescriptorLayout = overeditor::utility::DescriptorLayout;

int main() {
    auto app = overeditor::Application();
    auto cube = app.entities.create();

    auto ctx = app.getDeviceContext();


    std::filesystem::path resDirectory = std::filesystem::current_path() / "res";
    LOG_INFO << "Using resources located at \"" << resDirectory.string() << "\"";
    auto &system = app.getRenderingSystem();
    auto &renderPass = system.get()->renderPass;
    overeditor::graphics::shaders::Shader shader(
            "standart",
            overeditor::graphics::shaders::ShaderSource(
                    resDirectory / "frag.spv", {
                    },
                    VertexLayout()
            ),
            overeditor::graphics::shaders::ShaderSource(
                    resDirectory / "vert.spv",
                    {
                            DescriptorLayout(
                                    {
                                            overeditor::utility::DescriptorElement(
                                                    sizeof(glm::mat4) * 2,
                                                    1,
                                                    vk::DescriptorType::eUniformBuffer,
                                                    true
                                            )
                                    }
                            ),
                            DescriptorLayout(
                                    {
                                            overeditor::utility::DescriptorElement(
                                                    sizeof(glm::mat4),
                                                    1,
                                                    vk::DescriptorType::eUniformBuffer
                                            )
                                    }
                            )
                    },
                    VertexLayout(
                            {
                                    overeditor::utility::VertexElement(
                                            sizeof(float),
                                            3,
                                            vk::Format::eR32G32B32Sfloat
                                    )
                            }
                    )
            ),
            *ctx,
            renderPass
    );
    cube.assign<Transform>(
            glm::vec3(10, 0, 20) //Position
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
    auto b = overeditor::graphics::GeometryBuffer::create(
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
            glm::vec3(0, 0, -10)
    );
    camera.assign<Camera>(
            Camera::forDevice(*ctx, 100, 90, (float) ext.width / ext.height)
    );

    app.run();
}