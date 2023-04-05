#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"

namespace vke {
    class VkEngineApp {
        SDL_Window* mWindow;
        bool mRunning;

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
    public:
        VkEngineApp();
        ~VkEngineApp();

        vke::utils::Result<void, vke::EngineError> create(int width, int height, const char* title);
        void run();
    };
}

#endif
