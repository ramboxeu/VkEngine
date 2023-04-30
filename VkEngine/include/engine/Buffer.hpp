#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>
#include "engine/EngineResult.hpp"

namespace vke {

    class Buffer {
        VkDevice mDevice;
        VkBuffer mBuffer;
        VkDeviceMemory mBackingMemory;
        VkDeviceSize mSize;
        bool mNeedsFlushing;

    public:
        class MappedScope {
            friend Buffer;

            VkDevice mDevice;
            VkMappedMemoryRange mMappedRange;
            char* mData;
            VkDeviceSize mCursor;

            MappedScope(VkDevice device, VkMappedMemoryRange mappedRange, char* data);
        public:
            MappedScope();
            ~MappedScope();

            MappedScope(const MappedScope& other) = delete;
            MappedScope(MappedScope&& other) noexcept;

            MappedScope& operator=(const MappedScope& other) = delete;

            char& operator[](size_t index);

            void put(float* data, size_t size);
        };

        Buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory backingMemory, VkDeviceSize mSize, bool needsFlushing);
        Buffer();

        Buffer(const Buffer& other) = delete;
        Buffer(Buffer&& other) noexcept;
        ~Buffer();

        Buffer& operator=(const Buffer& other) = delete;
        Buffer& operator=(Buffer&& other) noexcept;

        VkBuffer& getHandle();
        EngineResult<MappedScope> map();
    };
}

#endif
