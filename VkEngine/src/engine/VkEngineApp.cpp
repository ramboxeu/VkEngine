#include <SDL2/SDL.h>

#include "engine/VkEngineApp.hpp"

namespace vke {
    VkEngineApp::VkEngineApp() : mWindow{nullptr}, mRunning{false} {
        SDL_SetMainReady();
        SDL_Init(SDL_INIT_VIDEO);
    }

    VkEngineApp::~VkEngineApp() {
        SDL_Quit();
    }

    EngineResult<void> VkEngineApp::create(int width, int height, const char* title) {
        TRY(createWindow(width, height, title));

        return utils::Result<void, EngineError>::ok();
    }

    void VkEngineApp::run() {
        mRunning = true;

        while (mRunning) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                handleWindowEvent(event);
            }
        }

        cleanup();
    }

    void VkEngineApp::handleWindowEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_QUIT:
                mRunning = false;
                break;
        }
    }

    void VkEngineApp::cleanup() {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    EngineResult<void> VkEngineApp::createWindow(int width, int height, const char *title) {
        mWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);

        if (!mWindow) {
            return EngineError::fromSdlError(SDL_GetError());
        }

        return {};
    }
};
