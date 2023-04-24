#ifndef SHADERFILE_HPP
#define SHADERFILE_HPP

#include <string>
#include <filesystem>
#include "engine/Shader.hpp"
#include "engine/EngineResult.hpp"

namespace vke {

    class ShaderFile {
        Shader::Type mType;
        std::string mEntrypoint;
        char* mCode;
        size_t mSize;

        ShaderFile(Shader::Type type, std::string&& entrypoint, char* code, size_t size);
    public:
        ShaderFile();
        ShaderFile(const ShaderFile& other);
        ShaderFile(ShaderFile&& other) noexcept;
        ~ShaderFile() noexcept;

        ShaderFile& operator=(const ShaderFile& other);
        ShaderFile& operator=(ShaderFile&& other) noexcept;

        static EngineResult<ShaderFile> loadFromFile(Shader::Type type, const std::filesystem::path& path, std::string entrypoint);

        Shader::Type getType() const;
        char* getBytes() const;
        uint32_t* getCode() const;
        size_t getSize() const;
        std::string& getEntrypoint();
    };
}

#endif
