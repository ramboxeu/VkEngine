#include "engine/MemoryType.hpp"

namespace vke {
    MemoryType::MemoryType() : mTypeIndex{INVALID_TYPE_INDEX}, mHostCoherent{false} {
    }

    MemoryType::MemoryType(uint32_t typeIndex, bool hostCoherent) : mTypeIndex{typeIndex}, mHostCoherent{hostCoherent} {
    }

    uint32_t MemoryType::getTypeIndex() const {
        return mTypeIndex;
    }

    bool MemoryType::isHostCoherent() const {
        return mHostCoherent;
    }

    bool MemoryType::isValid() const {
        return mTypeIndex != INVALID_TYPE_INDEX;
    }
}