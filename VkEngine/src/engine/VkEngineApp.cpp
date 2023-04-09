#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>

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
        TRY(createInstance(title));

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
        vkDestroyInstance(mInstance, nullptr);

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

    EngineResult<void> VkEngineApp::createInstance(const char* name) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name;
        appInfo.pEngineName = "No Engine";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        appInfo.engineVersion =  VK_MAKE_API_VERSION(0, 0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        auto extsResults = getInstanceExtensions();
        if (!extsResults)
            return extsResults;

        std::vector<const char*> extensions = extsResults.getOk();

        VkInstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance)) {
            return EngineError::fromVkError(result);
        }

        return {};
    }

    EngineResult<std::vector<const char*>> VkEngineApp::getInstanceExtensions() {
        unsigned int count;
        if (!SDL_Vulkan_GetInstanceExtensions(mWindow, &count, nullptr)) {
            return EngineResult<std::vector<const char*>>::error(EngineError::fromSdlError(SDL_GetError()));
        }

        std::vector<const char*> extensions;
        extensions.reserve(count + 1);
        if (!SDL_Vulkan_GetInstanceExtensions(mWindow, &count, extensions.data())) {
            return EngineResult<std::vector<const char*>>::error(EngineError::fromSdlError(SDL_GetError()));
        }

        if (auto result = checkExtensionsPresence(extensions); !result) {
            return EngineResult<std::vector<const char*>>::error(result.getError());
        } else {
            return extensions;
        }
    }

    EngineResult<void> VkEngineApp::checkExtensionsPresence(const std::vector<const char*>& extensions) {
        uint32_t count;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr)) {
            return EngineError::fromVkError(result);
        }

        std::vector<VkExtensionProperties> available;
        available.reserve(count);
        if (VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data())) {
            return EngineError::fromVkError(result);
        }

        std::vector<const char*> required{extensions};
        for (auto it = required.begin(); it != required.end();) {
            const char* name = *it;

            for (const VkExtensionProperties& properties : available) {
                if (strcmp(name, properties.extensionName) == 0) {
                    required.erase(it);
                    break;
                }
            }

            it++;
        }

        if (!required.empty()) {
            return EngineError::extensionsNotPresent(std::move(required));
        }

        return {};
    }
}
