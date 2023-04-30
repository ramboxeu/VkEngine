#include "engine/Buffer.hpp"

namespace vke {
    Buffer::Buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory backingMemory, VkDeviceSize size, bool needsFlushing) : mDevice{device}, mBuffer{buffer}, mBackingMemory{backingMemory}, mSize{size}, mNeedsFlushing{needsFlushing} {
    }

    Buffer::Buffer() : mDevice{VK_NULL_HANDLE}, mBuffer{VK_NULL_HANDLE}, mBackingMemory{VK_NULL_HANDLE}, mSize{0} {
    }

    Buffer::Buffer(Buffer&& other) noexcept : mDevice{other.mDevice}, mBuffer{other.mBuffer}, mBackingMemory{other.mBackingMemory}, mNeedsFlushing{other.mNeedsFlushing} {
        other.mDevice = VK_NULL_HANDLE;
        other.mBuffer = VK_NULL_HANDLE;
        other.mBackingMemory = VK_NULL_HANDLE;
    }

    Buffer::~Buffer() {
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept {
        if (this != &other) {
            mDevice = other.mDevice;
            mBuffer = other.mBuffer;
            mBackingMemory = other.mBackingMemory;
            mSize = other.mSize;
            mNeedsFlushing = other.mNeedsFlushing;

            other.mDevice = VK_NULL_HANDLE;
            other.mBuffer = VK_NULL_HANDLE;
            other.mBackingMemory = VK_NULL_HANDLE;
        }

        return *this;
    }

    VkBuffer& Buffer::getHandle() {
        return mBuffer;
    }

    EngineResult<Buffer::MappedScope> Buffer::map() {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = mBackingMemory;
        range.offset = 0;
        range.size = VK_WHOLE_SIZE;

        void* data;
        if (VkResult result = vkMapMemory(mDevice, mBackingMemory, 0, VK_WHOLE_SIZE, 0, &data)) {
            return EngineResult<Buffer::MappedScope>::error(EngineError::fromVkError(result));
        }

        return Buffer::MappedScope{mDevice, range, reinterpret_cast<char*>(data)};
    }

    Buffer::MappedScope::MappedScope(VkDevice device, VkMappedMemoryRange mappedRange, char* data) : mDevice{device}, mMappedRange{mappedRange}, mData{data}, mCursor{0} {
    }

    Buffer::MappedScope::MappedScope() : mDevice{VK_NULL_HANDLE}, mMappedRange{}, mData{nullptr}, mCursor{0} {
    }

    Buffer::MappedScope::MappedScope(Buffer::MappedScope&& other) noexcept : mDevice{other.mDevice}, mMappedRange{other.mMappedRange}, mData{other.mData}, mCursor{other.mCursor} {
        other.mDevice = VK_NULL_HANDLE;
        other.mData = nullptr;
        other.mCursor = 0;
    }

    Buffer::MappedScope::~MappedScope() {
        if (mData) {
            vkUnmapMemory(mDevice, mMappedRange.memory);
        }
    }

    void Buffer::MappedScope::put(float* data, size_t size) {
        size_t length = size * sizeof(float);
        char* bytes = reinterpret_cast<char*>(data);

        for (size_t i = 0; i < length; i++) {
            mData[mCursor++] = bytes[i];
        }
    }

    char& Buffer::MappedScope::operator[](size_t index) {
        return mData[index];
    }

}