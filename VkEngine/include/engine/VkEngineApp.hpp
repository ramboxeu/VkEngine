#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"

namespace vke {

    template<typename T>
    using EngineResult = utils::Result<T, EngineError>;

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
        virtual int rankPhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features);
        EngineResult<void> createDevice();
        EngineResult<void> createSurface();
        EngineResult<std::vector<const char*>> getDeviceExtensions();
        EngineResult<void> createSwapchain();
        EngineResult<void> createImageViews();

        VKAPI_ATTR static VKAPI_CALL VkBool32 onVulkanDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message, void* data);
    public:
        VkEngineApp();
        ~VkEngineApp();

        EngineResult<void> create(int width, int height, const char* title);
        void run();
    };
}

#endif
