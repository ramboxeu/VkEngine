#include <vector>
#include "engine/QueueFamilyIndexes.hpp"

namespace vke {

    QueueFamilyIndexes QueueFamilyIndexes::query(VkPhysicalDevice device, VkSurfaceKHR surface) {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

        std::vector<VkQueueFamilyProperties> families;
        families.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

        uint32_t graphicsIndex;
        uint32_t computeIndex;
        uint32_t presentIndex;
        uint32_t flags = 0;
        for (size_t i = 0, size = families.size(); i < size; i++) {
            VkQueueFamilyProperties family = families[i];

            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsIndex = i;
                flags |= QueueFamilyIndexes::GRAPHICS;
            }

            if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                computeIndex = i;
                flags |= QueueFamilyIndexes::COMPUTE;
            }

            VkBool32 surfaceSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surfaceSupported);
            if (surfaceSupported) {
                presentIndex = i;
                flags |= QueueFamilyIndexes::PRESENT;
            }
        }

        return QueueFamilyIndexes{ flags, graphicsIndex, computeIndex, presentIndex };
    }

    bool QueueFamilyIndexes::isComplete() const {
        return mFlags & GRAPHICS && mFlags & PRESENT;
    }

    bool QueueFamilyIndexes::hasIndex(Index index) const {
        return mFlags & index;
    }

    bool QueueFamilyIndexes::hasGraphics() const {
        return hasIndex(GRAPHICS);
    }

    bool QueueFamilyIndexes::hasCompute() const {
        return hasIndex(COMPUTE);
    }

    bool QueueFamilyIndexes::hasPresent() const {
        return hasIndex(PRESENT);
    }

    uint32_t QueueFamilyIndexes::getIndex(Index index) const {
        switch (index) {
            case GRAPHICS:
                return getGraphics();
            case COMPUTE:
                return getCompute();
            case PRESENT:
                return getPresent();
        }
    }

    uint32_t QueueFamilyIndexes::getGraphics() const {
        return mGraphicsIndex;
    }

    uint32_t QueueFamilyIndexes::getPresent() const {
        return mPresentIndex;
    }

    uint32_t QueueFamilyIndexes::getCompute() const {
        return mComputeIndex;
    }

    QueueFamilyIndexes::QueueFamilyIndexes(uint32_t flags, uint32_t graphics, uint32_t compute, uint32_t present) : mFlags{flags}, mGraphicsIndex{graphics}, mComputeIndex{compute}, mPresentIndex{present}  {
    }

}