#include <algorithm>
#include "engine/SwapchainDetails.hpp"

namespace vke {

    utils::Result<SwapchainDetails, EngineError> SwapchainDetails::query(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapchainDetails details;

        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.caps)) {
            return utils::Result<SwapchainDetails, EngineError>::error(EngineError::fromVkError(result));
        }

        uint32_t count;
        if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr)) {
            return utils::Result<SwapchainDetails, EngineError>::error(EngineError::fromVkError(result));
        }

        details.formats.resize(count);
        if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, details.formats.data())) {
            return utils::Result<SwapchainDetails, EngineError>::error(EngineError::fromVkError(result));
        }

        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr)) {
            return utils::Result<SwapchainDetails, EngineError>::error(EngineError::fromVkError(result));
        }

        details.modes.resize(count);
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, details.modes.data())) {
            return utils::Result<SwapchainDetails, EngineError>::error(EngineError::fromVkError(result));
        }

        return details;
    }

    bool SwapchainDetails::isConfigurable() const {
        return !(modes.empty() || formats.empty());
    }

    VkSurfaceFormatKHR SwapchainDetails::chooseFormat() const {
        for (const VkSurfaceFormatKHR& format : formats) {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR SwapchainDetails::chooseMode() const {
        for (const VkPresentModeKHR& mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapchainDetails::chooseExtent(int winWidth, int winHeight) const {
        if (caps.currentExtent.width != UINT32_MAX) {
            return caps.currentExtent;
        } else {
            uint32_t width = std::clamp(static_cast<uint32_t>(winWidth), caps.minImageExtent.width, caps.maxImageExtent.width);
            uint32_t height = std::clamp(static_cast<uint32_t>(winHeight), caps.minImageExtent.height, caps.maxImageExtent.height);

            VkExtent2D extent{width, height};
            return extent;
        }
    }

    uint32_t SwapchainDetails::chooseImageCount() const {
        uint32_t count = caps.minImageCount + 1;

        if (caps.maxImageCount > 0 && count > caps.maxImageCount) {
            count = caps.maxImageCount;
        }

        return count;
    }

    VkSurfaceTransformFlagBitsKHR SwapchainDetails::getCurrentTransform() const {
        return caps.currentTransform;
    }
}