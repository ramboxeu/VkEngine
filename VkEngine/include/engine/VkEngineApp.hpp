#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <map>

#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"
#include "engine/ShaderFile.hpp"
#include "engine/EngineResult.hpp"
#include "engine/Buffer.hpp"
#include "engine/ShaderModule.hpp"
#include "engine/MemoryType.hpp"

namespace vke {

    class VkEngineApp {
        SDL_Window* mWindow;
        bool mRunning;
        VkInstance mInstance;
        VkDebugUtilsMessengerEXT mMessenger;
        VkPhysicalDevice mPhysicalDevice;
        VkDevice mDevice;
        VkQueue mGraphicsQueue;
        VkQueue mPresentQueue;
        VkSurfaceKHR mSurface;
        VkExtent2D mSwapchainExtent;
        VkFormat mSwapchainImageFormat;
        VkSwapchainKHR mSwapchain;
        std::vector<VkImage> mSwapchainImages;
        std::vector<VkImageView> mSwapchainImageViews;
        VkRenderPass mRenderPass;
        VkPipelineLayout mPipelineLayout;
        VkPipeline mPipeline;
        std::map<VkShaderStageFlagBits, ShaderModule> mShaderModules;
        std::vector<VkFramebuffer> mFramebuffers;
        VkCommandPool mCommandPool;
        std::vector<VkCommandBuffer> mCommandBuffers;
        std::vector<VkSemaphore> mFrameSemaphores;
        std::vector<VkFence> mFrameFences;
        int mCurrentFrame = 0;
        MemoryType mSpeedyMemType;
        MemoryType mStagingMemType;
        MemoryType mUniversalMemType;
        std::vector<VkBuffer> mBuffers;
        std::vector<VkDeviceMemory> mMemoryAllocations;

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
        EngineResult<void> createWindow(int width, int height, const char* title);
        EngineResult<void> createInstance(const char* name);
        EngineResult<std::vector<const char*>> getInstanceExtensions();
        EngineResult<void> checkExtensionsPresence(std::optional<std::vector<const char*>> layers, const std::vector<const char*>& extensions);
        EngineResult<void> checkExtensionsPresence(std::vector<const char*>& required, const std::vector<VkExtensionProperties>& available);
        EngineResult<void> checkDeviceExtensionsPresence(VkPhysicalDevice device, const char* layer, std::vector<const char*>& required);
        EngineResult<void> checkInstanceExtensionsPresence(const char* layer, std::vector<const char*>& required);
        EngineResult<void> findPhysicalDevice();
        EngineResult<void> createDevice();
        EngineResult<void> createSurface();
        EngineResult<std::vector<const char*>> getDeviceExtensions();
        EngineResult<void> createSwapchain();
        EngineResult<void> createImageViews();
        EngineResult<void> createRenderPass();
        EngineResult<void> createShaderModules();
        EngineResult<void> createPipeline();
        EngineResult<void> createFramebuffers();
        EngineResult<void> createCommandPool();
        EngineResult<void> createCommandBuffers();
        EngineResult<void> createSemaphores();
        EngineResult<void> createFences();
        EngineResult<void> renderFrame();
        void setMemoryTypes();

        VKAPI_ATTR static VKAPI_CALL VkBool32 onVulkanDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message, void* data);

    protected:
        enum class BufferType {
            SPEEDY,
            STAGING,
            UNIVERSAL
        };

        static constexpr int MAX_CONCURRENT_FRAMES = 2;

        virtual int rankPhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features);
        virtual EngineResult<std::map<VkShaderStageFlagBits, ShaderFile>> loadShaders() = 0;
        virtual void render(VkCommandBuffer cmdBuffer);
        virtual EngineResult<void> onInit();

        EngineResult<Buffer> allocateBuffer(VkBufferUsageFlagBits usage, uint64_t size, BufferType type = BufferType::UNIVERSAL);

    public:
        VkEngineApp();
        ~VkEngineApp();

        EngineResult<void> create(int width, int height, const char* title);
        EngineResult<void> run();
    };
}

#endif
