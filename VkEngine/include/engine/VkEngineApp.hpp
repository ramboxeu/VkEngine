#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include <vulkan/vulkan.h>

#include <vector>

#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"

namespace vke {

    template<typename T>
    using EngineResult = utils::Result<T, EngineError>;

    class VkEngineApp {
        SDL_Window* mWindow;
        bool mRunning;
        VkInstance mInstance;

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
        EngineResult<void> createWindow(int width, int height, const char* title);
        EngineResult<void> createInstance(const char* name);
        EngineResult<std::vector<const char*>> getInstanceExtensions();
        EngineResult<void> checkExtensionsPresence(const std::vector<const char*>& extensions);
    public:
        VkEngineApp();
        ~VkEngineApp();

        EngineResult<void> create(int width, int height, const char* title);
        void run();
    };
}

#endif
