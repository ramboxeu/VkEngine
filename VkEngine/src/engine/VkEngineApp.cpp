#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <map>

#include "engine/vk/proxies.hpp"
#include "engine/VkEngineApp.hpp"
#include "engine/QueueFamilyIndexes.hpp"
#include "engine/SwapchainDetails.hpp"

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
        TRY(createSurface());
        TRY(findPhysicalDevice());
        TRY(createDevice());
        TRY(createSwapchain());
        TRY(createImageViews());
        TRY(createRenderPass());
        TRY(createShaderModules());
        TRY(createPipeline());
        TRY(createFramebuffers());
        TRY(createCommandPool());
        TRY(createCommandBuffers());
        TRY(createSemaphores());
        TRY(createFences());
        setMemoryTypes();

        onInit();

        return utils::Result<void, EngineError>::ok();
    }

    EngineResult<void> VkEngineApp::run() {
        mRunning = true;

        while (mRunning) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                handleWindowEvent(event);
            }

            if (EngineResult<void> result = renderFrame(); !result) {
                mRunning = false;
                cleanup();
                return result;
            }
        }

        cleanup();

        return {};
    }

    void VkEngineApp::handleWindowEvent(SDL_Event &event) {
        switch (event.type) {
            case SDL_QUIT:
                mRunning = false;
                break;
        }
    }

    void VkEngineApp::cleanup() {
        // Finish whatever device is doing right now before cleanup
        vkDeviceWaitIdle(mDevice);

        for (VkBuffer buffer : mBuffers) {
            vkDestroyBuffer(mDevice, buffer, nullptr);
        }
        mBuffers.clear();

        for (VkDeviceMemory memory : mMemoryAllocations) {
            vkFreeMemory(mDevice, memory, nullptr);
        }
        mMemoryAllocations.clear();

        for (VkSemaphore semaphore : mFrameSemaphores) {
            vkDestroySemaphore(mDevice, semaphore, nullptr);
        }
        mFrameSemaphores.clear();

        for (VkFence fence : mFrameFences) {
            vkDestroyFence(mDevice, fence, nullptr);
        }
        mFrameFences.clear();

        vkFreeCommandBuffers(mDevice, mCommandPool, mCommandBuffers.size(), mCommandBuffers.data());
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

        for (const VkFramebuffer& framebuffer : mFramebuffers) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }
        mFramebuffers.clear();

        for (const auto& entry : mShaderModules) {
            vkDestroyShaderModule(mDevice, entry.second.getHandle(), nullptr);
        }
        mShaderModules.clear();

        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
        vkDestroyPipeline(mDevice, mPipeline, nullptr);

        for (const VkImageView& view : mSwapchainImageViews) {
            vkDestroyImageView(mDevice, view, nullptr);
        }
        mSwapchainImageViews.clear();

        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
        mSwapchainImages.clear();

        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

        vkDestroyDevice(mDevice, nullptr);

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
                TRY(checkInstanceExtensionsPresence(layer, required));
            }
        }

        TRY(checkInstanceExtensionsPresence(nullptr, required));

        if (!required.empty()) {
            return EngineError::extensionsNotPresent(std::move(required));
        }

        return {};
    }

    EngineResult<void> VkEngineApp::checkInstanceExtensionsPresence(const char* layer, std::vector<const char*>& required) {
        uint32_t count;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layer, &count, nullptr)) {
            return EngineError::fromVkError(result);
        }

        std::vector<VkExtensionProperties> available;
        available.resize(count);
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layer, &count, available.data())) {
            return EngineError::fromVkError(result);
        }

        return checkExtensionsPresence(required, available);
    }

    EngineResult<void> VkEngineApp::checkDeviceExtensionsPresence(VkPhysicalDevice device, const char* layer, std::vector<const char*>& required) {
        uint32_t count;
        if (VkResult result = vkEnumerateDeviceExtensionProperties(device, layer, &count, nullptr)) {
            return EngineError::fromVkError(result);
        }

        std::vector<VkExtensionProperties> available;
        available.resize(count);
        if (VkResult result = vkEnumerateDeviceExtensionProperties(device, layer, &count, available.data())) {
            return EngineError::fromVkError(result);
        }

        return checkExtensionsPresence(required, available);
    }

    EngineResult<void> VkEngineApp::checkExtensionsPresence(std::vector<const char*>& required, const std::vector<VkExtensionProperties>& available) {
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
        if (!QueueFamilyIndexes::query(device, mSurface).isComplete()) {
            return 0;
        }

        if (auto exts = getDeviceExtensions()) {
            auto extensions = exts.getOk();

            if (!(checkDeviceExtensionsPresence(device, nullptr, extensions) && extensions.empty()))
                return 0;
        } else {
            return 0;
        }

        if (auto details = SwapchainDetails::query(device, mSurface)) {
            if (!details.getOk().isConfigurable())
                return 0;
        }

        return 1;
    }

    EngineResult<void> VkEngineApp::createDevice() {
        QueueFamilyIndexes indexes = QueueFamilyIndexes::query(mPhysicalDevice, mSurface);

        float priority = 1.0f;

        uint32_t queueCount = indexes.getGraphics() == indexes.getPresent() ? 1 : 2;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{queueCount};

        queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[0].pQueuePriorities = &priority;
        queueCreateInfos[0].queueFamilyIndex = indexes.getGraphics();
        queueCreateInfos[0].queueCount = 1;

        if (queueCount > 1) {
            queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[1].pQueuePriorities = &priority;
            queueCreateInfos[1].queueFamilyIndex = indexes.getPresent();
            queueCreateInfos[1].queueCount = 1;
        }

        std::vector<const char*> extensions;
        TRY(getDeviceExtensions()) extensions = result.getOk();

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pEnabledFeatures = &features;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;

        if (VkResult result = vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice)) {
            return EngineError::fromVkError(result);
        }

        vkGetDeviceQueue(mDevice, indexes.getGraphics(), 0, &mGraphicsQueue);

        if (queueCount > 1) {
            vkGetDeviceQueue(mDevice, indexes.getGraphics(), 0, &mPresentQueue);
        } else {
            mPresentQueue = mGraphicsQueue;
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createSurface() {
        if (!SDL_Vulkan_CreateSurface(mWindow, mInstance, &mSurface)) {
            return EngineError::fromSdlError(SDL_GetError());
        }

        return {};
    }

    EngineResult<std::vector<const char*>> VkEngineApp::getDeviceExtensions() {
        return std::vector<const char*>{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    EngineResult<void> VkEngineApp::createSwapchain() {
        if (auto details = SwapchainDetails::query(mPhysicalDevice, mSurface)) {
            VkSurfaceFormatKHR format = details->chooseFormat();
            VkPresentModeKHR mode = details->chooseMode();

            int width;
            int height;
            SDL_GetWindowSize(mWindow, &width, &height);

            VkExtent2D extent = details->chooseExtent(width, height);
            uint32_t minImageCount = details->chooseImageCount();

            QueueFamilyIndexes indexes = QueueFamilyIndexes::query(mPhysicalDevice, mSurface);

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = mSurface;
            createInfo.minImageCount = minImageCount;
            createInfo.imageFormat = format.format;
            createInfo.imageColorSpace = format.colorSpace;
            createInfo.imageArrayLayers = 1;
            createInfo.imageExtent = extent;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            uint32_t queueFamilyIndexes[2] = { indexes.getGraphics(), indexes.getPresent() };
            if (indexes.getGraphics() == indexes.getPresent()) {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 1;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
            }

            createInfo.pQueueFamilyIndices = queueFamilyIndexes;
            createInfo.preTransform = details->getCurrentTransform();
            createInfo.presentMode = mode;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (VkResult result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain)) {
                return EngineError::fromVkError(result);
            }

            mSwapchainExtent = extent;
            mSwapchainImageFormat = format.format;

            uint32_t imageCount;
            if (VkResult result = vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr)) {
                return EngineError::fromVkError(result);
            }

            mSwapchainImages.resize(imageCount);
            if (VkResult result = vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data())) {
                return EngineError::fromVkError(result);
            }

            return {};
        } else {
            return details;
        }
    }

    EngineResult<void> VkEngineApp::createImageViews() {
        mSwapchainImageViews.resize(mSwapchainImages.size());

        for (size_t i = 0, size = mSwapchainImages.size(); i < size; i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = mSwapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = mSwapchainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (VkResult result = vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageViews[i])) {
                return EngineError::fromVkError(result);
            }
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = mSwapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pAttachments = &colorAttachment;
        createInfo.attachmentCount = 1;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;

        if (VkResult result = vkCreateRenderPass(mDevice, &createInfo, nullptr, &mRenderPass)) {
            return EngineError::fromVkError(result);
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createShaderModules() {
        if (auto shaders = loadShaders()) {
            if (!shaders->contains(VK_SHADER_STAGE_VERTEX_BIT)) {
                return EngineError::missingVertexShader();
            }

            for (auto& entry : shaders.getOk()) {
                VkShaderModuleCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = entry.second.getSize();
                createInfo.pCode = entry.second.getCode();

                VkShaderModule handle;
                if (VkResult result = vkCreateShaderModule(mDevice, &createInfo, nullptr, &handle)) {
                    return EngineError::fromVkError(result);
                }

                mShaderModules.emplace(entry.first, ShaderModule{ handle, std::move(entry.second.getEntrypoint()) });
            }
        } else {
            return shaders;
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createPipeline() {
        std::vector<VkPipelineShaderStageCreateInfo> stages{mShaderModules.size()};


        for (size_t i = 0; const auto& entry : mShaderModules) {
            stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stages[i].stage = entry.first;
            stages[i].module = entry.second.getHandle();
            stages[i].pName = entry.second.getEntrypoint().c_str();
            stages[i].pSpecializationInfo = nullptr;

            i++;
        }

        VkVertexInputBindingDescription vertexInputBinding{};
        vertexInputBinding.binding = 0;
        vertexInputBinding.stride = sizeof(float) * 6;
        vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertexPositionAttribute{};
        vertexPositionAttribute.location = 0;
        vertexPositionAttribute.binding = 0;
        vertexPositionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexPositionAttribute.offset = 0;

        VkVertexInputAttributeDescription vertexColorAttribute{};
        vertexColorAttribute.location = 1;
        vertexColorAttribute.binding = 0;
        vertexColorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexColorAttribute.offset = sizeof(float) * 3;

        VkVertexInputAttributeDescription attributeDescriptions[] = {
                vertexPositionAttribute, vertexColorAttribute
        };

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInput.vertexAttributeDescriptionCount = 2;
        vertexInput.pVertexAttributeDescriptions = attributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mSwapchainExtent.width);
        viewport.height = static_cast<float>(mSwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset.x = 0.0f;
        scissor.offset.y = 0.0f;
        scissor.extent = mSwapchainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.sampleShadingEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &colorBlendAttachment;

        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 0;
        layoutCreateInfo.pSetLayouts = nullptr;
        layoutCreateInfo.pushConstantRangeCount = 0;
        layoutCreateInfo.pPushConstantRanges = nullptr;

        if (VkResult result = vkCreatePipelineLayout(mDevice, &layoutCreateInfo, nullptr, &mPipelineLayout)) {
            return EngineError::fromVkError(result);
        }

        VkGraphicsPipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.stageCount = stages.size();
        createInfo.pStages = stages.data();
        createInfo.pVertexInputState = &vertexInput;
        createInfo.pInputAssemblyState = &inputAssembly;
        createInfo.pTessellationState = nullptr;
        createInfo.pViewportState = &viewportState;
        createInfo.pRasterizationState = &rasterizer;
        createInfo.pMultisampleState = &multisampling;
        createInfo.pDepthStencilState = nullptr;
        createInfo.pColorBlendState = &colorBlend;
        createInfo.pDynamicState = &dynamicState;
        createInfo.layout = mPipelineLayout;
        createInfo.renderPass = mRenderPass;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = 0;

        if (VkResult result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline)) {
            return EngineError::fromVkError(result);
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createFramebuffers() {
        mFramebuffers.resize(mSwapchainImageViews.size());

        for (size_t i = 0, size = mSwapchainImageViews.size(); i < size; i++) {
            VkFramebufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = mRenderPass;
            createInfo.attachmentCount = 1;
            createInfo.pAttachments = &mSwapchainImageViews[i];
            createInfo.width = mSwapchainExtent.width;
            createInfo.height = mSwapchainExtent.height;
            createInfo.layers = 1;

            if (VkResult result = vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mFramebuffers[i])) {
                return EngineError::fromVkError(result);
            }
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createCommandPool() {
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = QueueFamilyIndexes::query(mPhysicalDevice, mSurface).getGraphics();

        if (VkResult result = vkCreateCommandPool(mDevice, &createInfo, nullptr, &mCommandPool)) {
            return EngineError::fromVkError(result);
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createCommandBuffers() {
        size_t size = MAX_CONCURRENT_FRAMES;
        mCommandBuffers.resize(size);

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = mCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = size;

        if (VkResult result = vkAllocateCommandBuffers(mDevice, &allocateInfo, mCommandBuffers.data())) {
            return EngineError::fromVkError(result);
        }

        return {};
    }

    EngineResult<void> VkEngineApp::renderFrame() {
        VkFence frameAvailableFence = mFrameFences[mCurrentFrame];

        // FENCE: wait for queue to finish
        if (VkResult result = vkWaitForFences(mDevice, 1, &frameAvailableFence, VK_TRUE, UINT64_MAX)) {
            return EngineError::fromVkError(result);
        }

        // FENCE: reset, we are processing this frame
        if (VkResult result = vkResetFences(mDevice, 1, &frameAvailableFence)) {
            return EngineError::fromVkError(result);
        }

        VkSemaphore imageAvailableSemaphore = mFrameSemaphores[mCurrentFrame * 2];
        VkSemaphore frameRenderedSemaphore = mFrameSemaphores[(mCurrentFrame * 2) + 1];

        // acquire next swapchain image
        uint32_t imageIndex;
        if (VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT32_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex)) {
            return EngineError::fromVkError(result);
        }

        VkCommandBuffer cmdBuffer = mCommandBuffers[mCurrentFrame];
        // reset buffer
        if (VkResult result = vkResetCommandBuffer(cmdBuffer, 0)) {
            return EngineError::fromVkError(result);
        }

        // begin command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = nullptr;
        if (VkResult result = vkBeginCommandBuffer(cmdBuffer, &beginInfo)) {
            return EngineError::fromVkError(result);
        }

        // setup dynamic state
        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(mSwapchainExtent.width);
        viewport.height = static_cast<float>(mSwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        VkRect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent = mSwapchainExtent;
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissors);
        // bind pipeline
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
        // begin render pass
        VkClearValue clearValue{};
        clearValue.color.float32[0] = 0.0;
        clearValue.color.float32[1] = 0.0;
        clearValue.color.float32[2] = 0.0;
        clearValue.color.float32[3] = 1.0;
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = mRenderPass;
        renderPassBeginInfo.framebuffer = mFramebuffers[imageIndex];
        renderPassBeginInfo.renderArea = scissors;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        // render
        render(cmdBuffer);
        // end render pass
        vkCmdEndRenderPass(cmdBuffer);
        // end command buffer
        vkEndCommandBuffer(cmdBuffer);
        // submit command buffer (resets frame busy fence, signals queue busy semaphore)
        // SEMAPHORE: signal that frame is rendered, wait for swapchain image
        // FENCE: signals when queue processing finishes
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &frameRenderedSemaphore;
        if (VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, frameAvailableFence)) {
            return EngineError::fromVkError(result);
        }

        // SEMAPHORE: wait for queue to finish
        // preset
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frameRenderedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mSwapchain;
        presentInfo.pImageIndices = &imageIndex;
        if (VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo)) {
            return EngineError::fromVkError(result);
        }

        mCurrentFrame = (mCurrentFrame + 1) % MAX_CONCURRENT_FRAMES;

        return {};
    }

    EngineResult<void> VkEngineApp::createSemaphores() {
        size_t size = MAX_CONCURRENT_FRAMES * 2;
        mFrameSemaphores.resize(size);

        for (size_t i = 0; i < size; i++) {
            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (VkResult result = vkCreateSemaphore(mDevice, &createInfo, nullptr, &mFrameSemaphores[i])) {
                return EngineError::fromVkError(result);
            }
        }

        return {};
    }

    EngineResult<void> VkEngineApp::createFences() {
        size_t size = MAX_CONCURRENT_FRAMES;
        mFrameFences.resize(size);

        for (size_t i = 0; i < size; i++) {
            VkFenceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (VkResult result = vkCreateFence(mDevice, &createInfo, nullptr, &mFrameFences[i])) {
                return EngineError::fromVkError(result);
            }
        }

        return {};
    }

    void VkEngineApp::render(VkCommandBuffer cmdBuffer) {}

    EngineResult<void> VkEngineApp::onInit() {
        return {};
    }

    EngineResult<Buffer> VkEngineApp::allocateBuffer(VkBufferUsageFlagBits bufferUsage, uint64_t size, BufferType type) {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = size;
        createInfo.usage = bufferUsage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;

        uint32_t queueFamilyIndex = QueueFamilyIndexes::query(mPhysicalDevice, mSurface).getGraphics();
        createInfo.pQueueFamilyIndices = &queueFamilyIndex;

        VkBuffer buffer;
        if (VkResult result = vkCreateBuffer(mDevice, &createInfo, nullptr, &buffer)) {
            return EngineResult<Buffer>::error(EngineError::fromVkError(result));
        }
        mBuffers.push_back(buffer);

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(mDevice, buffer, &requirements);

        MemoryType memType;
        switch (type) {
            case BufferType::SPEEDY:
                memType = mSpeedyMemType;
                break;
            case BufferType::STAGING:
                memType = mStagingMemType;
                break;
            case BufferType::UNIVERSAL:
                memType = mUniversalMemType;
                break;
        }

        uint32_t typeIndex = memType.getTypeIndex();

        if (!((requirements.memoryTypeBits >> typeIndex) & 1)) {
            std::cout << "[ENGINE] [WARN]: No memory type supports this buffer (" << buffer << ")\n";
        }

        VkDeviceMemory memory;
        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = typeIndex;
        vkAllocateMemory(mDevice, &allocateInfo, nullptr, &memory);
        mMemoryAllocations.push_back(memory);

        vkBindBufferMemory(mDevice, buffer, memory, 0);

        return Buffer(mDevice, buffer, memory, size, memType.isHostCoherent());
    }

    void VkEngineApp::setMemoryTypes() {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

        uint32_t speedyMemoryTypeIndex = VK_MAX_MEMORY_TYPES + 1;
        VkDeviceSize speedyMemorySize;
        uint32_t stagingMemoryTypeIndex = VK_MAX_MEMORY_TYPES + 1;
        VkDeviceSize stagingMemorySize = 0;
        bool stagingMemoryCoherent = false;
        uint32_t universalMemoryTypeIndex = VK_MAX_MEMORY_TYPES + 1;
        VkDeviceSize universalMemorySize = 0;
        bool universalMemoryCoherent = false;

        for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            VkMemoryType type = memoryProperties.memoryTypes[i];
            VkMemoryHeap heap = memoryProperties.memoryHeaps[type.heapIndex];

            if (type.propertyFlags == 0)
                continue;

            if ((type.propertyFlags & ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
                if (heap.size > speedyMemorySize) {
                    speedyMemoryTypeIndex = i;
                    speedyMemorySize = heap.size;
                }
            }

            if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                if (heap.size > stagingMemorySize) {
                    stagingMemoryTypeIndex = i;
                    stagingMemorySize = heap.size;
                    stagingMemoryCoherent = (bool)(type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                }
            }

            if (type.propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
                if (heap.size > universalMemorySize) {
                    universalMemoryTypeIndex = i;
                    universalMemorySize = heap.size;
                    universalMemoryCoherent = (bool)(type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                }
            }

        }

        if (universalMemoryTypeIndex == VK_MAX_MEMORY_TYPES + 1) {
            universalMemoryTypeIndex = stagingMemoryTypeIndex;
            universalMemoryCoherent = stagingMemoryCoherent;
        }

        mSpeedyMemType = MemoryType(speedyMemoryTypeIndex, false);
        mStagingMemType = MemoryType(stagingMemoryTypeIndex, stagingMemoryCoherent);
        mUniversalMemType = MemoryType(universalMemoryTypeIndex, universalMemoryCoherent);
    }
}
