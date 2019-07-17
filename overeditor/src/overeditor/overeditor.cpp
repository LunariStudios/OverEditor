#include <vulkan/vulkan.h>
#include <overeditor/application.h>
#include <plog/Log.h>
#include <overeditor/ecs/components/common.h>
#include <overeditor/graphics/shaders/shader.h>

int main() {
    auto app = overeditor::Application();
    auto cube = app.entities.create();
    cube.assign<Transform>(
            glm::vec3(10, 0, 20) //Position
    );
    overeditor::graphics::GeometryBuffer b((overeditor::graphics::GeometryLayout()));
    auto ctx = app.getDeviceContext();
    overeditor::graphics::shaders::Shader shader;
    std::filesystem::path resDirectory = std::filesystem::current_path() / "res";
    LOG_INFO << "Using resources located at \"" << resDirectory.string() << "\"";
    auto &system = app.getRenderingSystem();
    auto &renderPass = system.get()->renderPass;
    shader.initialize(*ctx, renderPass, resDirectory / "frag.spv", resDirectory / "vert.spv");
    vk::CommandPool secondaryPool = ctx->getDevice().createCommandPool(
            vk::CommandPoolCreateInfo(
                    vk::CommandPoolCreateFlagBits::eProtected,
                    ctx->getQueueContext()->getFamilyIndices().getGraphics().get()
            )
    );
    cube.assign_from_copy(
            Drawable::forGeometry(*ctx, shader.getPipeline(), secondaryPool, renderPass, 0, b)
    );
    app.run();
}