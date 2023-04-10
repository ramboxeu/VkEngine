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

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
        EngineResult<void> createWindow(int width, int height, const char* title);
        EngineResult<void> createInstance(const char* name);
        EngineResult<std::vector<const char*>> getInstanceExtensions();
        EngineResult<void> checkExtensionsPresence(std::optional<std::vector<const char*>> layers, const std::vector<const char*>& extensions);
        EngineResult<void> checkExtensionsPresence(const char* layers, std::vector<const char*>& extensions);

        VKAPI_ATTR static VKAPI_CALL VkBool32 onVulkanDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message, void* data);
    public:
        VkEngineApp();
        ~VkEngineApp();

        EngineResult<void> create(int width, int height, const char* title);
        void run();
    };
}

#endif
