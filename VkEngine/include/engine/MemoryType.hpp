#ifndef MEMORYTYPE_HPP
#define MEMORYTYPE_HPP

#include <cstdint>
#include <vulkan/vulkan.h>

namespace vke {

    class MemoryType {
        static constexpr uint32_t INVALID_TYPE_INDEX = VK_MAX_MEMORY_TYPES + 1;

        uint32_t mTypeIndex;
        bool mHostCoherent;

    public:
        MemoryType();
        MemoryType(uint32_t typeIndex, bool hostCoherent);

        uint32_t getTypeIndex() const;
        bool isHostCoherent() const;
        bool isValid() const;
    };
}

#endif
