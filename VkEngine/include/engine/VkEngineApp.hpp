#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

namespace vke {
    class VkEngineApp {
        SDL_Window* mWindow;
        bool mRunning;

        void handleWindowEvent(SDL_Event& event);
        void cleanup();
    public:
        VkEngineApp();
        ~VkEngineApp();

        bool create(int width, int height, const char* title);
        void run();
    };
}

#endif
