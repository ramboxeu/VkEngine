#include "engine/ShaderModule.hpp"

namespace vke {

    ShaderModule::ShaderModule(VkShaderModule handle, std::string entrypoint) : mHandle{handle}, mEntrypoint{std::move(entrypoint)} {
    }

    ShaderModule::~ShaderModule() {}

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept : mHandle{other.mHandle}, mEntrypoint{std::move(other.mEntrypoint)} {
        other.mHandle = VK_NULL_HANDLE;
    }

    const std::string& ShaderModule::getEntrypoint() const {
        return mEntrypoint;
    }

    VkShaderModule ShaderModule::getHandle() const {
        return mHandle;
    }

}