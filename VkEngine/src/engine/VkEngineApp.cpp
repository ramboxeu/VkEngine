#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <map>

#include "engine/vk/proxies.hpp"
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
        TRY(findPhysicalDevice());

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
        vke::vk::vkDestroyDebugUtilsMessengerEXT(mInstance, mMessenger, nullptr);
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL onVulkanDebugMessage2(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message, void* data);

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
        std::vector<const char*> layers{"VK_LAYER_KHRONOS_validation"};
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        TRY(checkExtensionsPresence(layers, extensions));

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerCreateInfo.pfnUserCallback = onVulkanDebugMessage;
        debugMessengerCreateInfo.pUserData = nullptr;

        createInfo.pNext = &debugMessengerCreateInfo;

        if (VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance)) {
            return EngineError::fromVkError(result);
        }

        if (VkResult result = vke::vk::vkCreateDebugUtilsMessengerEXT(mInstance, &debugMessengerCreateInfo, nullptr, &mMessenger)) {
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
        extensions.resize(count);
        if (!SDL_Vulkan_GetInstanceExtensions(mWindow, &count, extensions.data())) {
            return EngineResult<std::vector<const char*>>::error(EngineError::fromSdlError(SDL_GetError()));
        }

        return extensions;
    }

    EngineResult<void> VkEngineApp::checkExtensionsPresence(std::optional<std::vector<const char*>> layers, const std::vector<const char*>& extensions) {
        std::vector<const char*> required{extensions};

        if (layers.has_value()) {
            for (const char* layer : layers.value()) {
                TRY(checkExtensionsPresence(layer, required));
            }
        }

        TRY(checkExtensionsPresence(nullptr, required));

        if (!required.empty()) {
            return EngineError::extensionsNotPresent(std::move(required));
        }

        return {};
    }

    EngineResult<void> VkEngineApp::checkExtensionsPresence(const char* layer, std::vector<const char*>& required) {
        uint32_t count;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layer, &count, nullptr)) {
            return EngineError::fromVkError(result);
        }

        std::vector<VkExtensionProperties> available;
        available.resize(count);
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layer, &count, available.data())) {
            return EngineError::fromVkError(result);
        }

        for (auto it = required.begin(); it != required.end();) {
            const char* name = *it;

            bool advanced = false;
            for (const VkExtensionProperties& properties : available) {
                if (strcmp(name, properties.extensionName) == 0) {
                    it = required.erase(it);
                    advanced = true;
                    break;
                }
            }

            if (!advanced) it++;
        }

        return {};
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VkEngineApp::onVulkanDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message, void* data) {
        if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            return VK_FALSE;

        std::cout << "[ENGINE] ";

        switch (severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                std::cout << "[DEBUG]: ";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                std::cout << "[INFO]: ";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                std::cout << "[WARN]: ";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                std::cout << "[ERROR]: ";
                break;
        }

        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
            std::cout << "[Vulkan/General] ";
        }

        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            std::cout << "[Vulkan/Validation] ";
        }

        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            std::cout << "[Vulkan/Performance] ";
        }

        std::cout << message->pMessage << '\n';
        return VK_FALSE;
    }

    EngineResult<void> VkEngineApp::findPhysicalDevice() {
        uint32_t count = 0;
        if (VkResult result = vkEnumeratePhysicalDevices(mInstance, &count, nullptr)) {
            return EngineError::fromVkError(result);
        }

        std::vector<VkPhysicalDevice> devices;
        devices.resize(count);
        if (VkResult result = vkEnumeratePhysicalDevices(mInstance, &count, devices.data())) {
            return EngineError::fromVkError(result);
        }

        std::multimap<uint32_t, VkPhysicalDevice> map;
        for (VkPhysicalDevice device : devices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);

            int rank = rankPhysicalDevice(device, properties, features);

            if (rank != 0) {
                map.emplace(rank, device);
            }
        }

        if (map.empty()) {
            return EngineError::noDevice();
        }

        mPhysicalDevice = map.begin()->second;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
        std::cout << "[ENGINE] [DEBUG]: Found physical graphics device " << properties.deviceName << '\n';

        return {};
    }

    int VkEngineApp::rankPhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features) {
        return 1;
    }
}
