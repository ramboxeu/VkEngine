#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"

namespace vke {

    template<typename T>
    using EngineResult = utils::Result<T, EngineError>;

    class VkEngineApp {
        SDL_Window* mWindow;
        bool mRunning;

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
        EngineResult<void> createWindow(int width, int height, const char* title);
    public:
        VkEngineApp();
        ~VkEngineApp();

        EngineResult<void> create(int width, int height, const char* title);
        void run();
    };
}

#endif
