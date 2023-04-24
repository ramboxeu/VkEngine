#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include <vulkan/vulkan.h>
#include <string>

namespace vke {

    class ShaderModule {
        VkShaderModule mHandle;
        std::string mEntrypoint;

    public:
        ShaderModule(VkShaderModule handle, std::string entrypoint);
        ~ShaderModule();

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule(ShaderModule&& other) noexcept;

        ShaderModule& operator=(const ShaderModule&) = delete;
        ShaderModule& operator=(ShaderModule&&) = delete;

        const std::string& getEntrypoint() const;
        VkShaderModule getHandle() const;
    };
}

#endif