#ifndef SWAPCHAINDETAILS_HPP
#define SWAPCHAINDETAILS_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "engine/EngineError.hpp"
#include "engine/utils/Result.hpp"

namespace vke {

    class SwapchainDetails {
        VkSurfaceCapabilitiesKHR caps;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> modes;

    public:

        static utils::Result<SwapchainDetails, EngineError> query(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool isConfigurable() const;
        VkSurfaceFormatKHR chooseFormat() const;
        VkPresentModeKHR chooseMode() const;
        VkExtent2D chooseExtent(int winWidth, int winHeight) const;
        uint32_t chooseImageCount() const;
        VkSurfaceTransformFlagBitsKHR getCurrentTransform() const;
    };
}

#endif
