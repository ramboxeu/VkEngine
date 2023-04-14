#ifndef QUEUEFAMILYINDICIES_HPP
#define QUEUEFAMILYINDICIES_HPP

#include <cstdint>
#include <vulkan/vulkan.h>

namespace vke {
    class QueueFamilyIndexes {
    public:
        using IndexesMask = uint32_t;

        enum Index : uint32_t {
            GRAPHICS = 0x1,
            COMPUTE = 0x2,
            PRESENT = 0x4,
        };

        static QueueFamilyIndexes query(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool isComplete() const;
        bool hasIndex(Index index) const;
        bool hasGraphics() const;
        bool hasCompute() const;
        bool hasPresent() const;
        uint32_t getIndex(Index index) const;
        uint32_t getGraphics() const;
        uint32_t getCompute() const;
        uint32_t getPresent() const;

    private:
        QueueFamilyIndexes(uint32_t flags, uint32_t graphics, uint32_t compute, uint32_t present);

        uint32_t mFlags;
        uint32_t mGraphicsIndex;
        uint32_t mComputeIndex;
        uint32_t mPresentIndex;
    };
}

#endif
