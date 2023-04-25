#include "engine/ShaderFile.hpp"
#include <fstream>
#include <cstring>

namespace vke {

    ShaderFile::ShaderFile(std::string&& entrypoint, char* code, size_t size) : mEntrypoint{std::move(entrypoint)}, mCode{code}, mSize{size}  {
    }

    ShaderFile::ShaderFile() : mEntrypoint{}, mCode{nullptr}, mSize{0} {
    }

    ShaderFile::~ShaderFile() noexcept {
        delete[] mCode;
        mSize = 0;
    }

    ShaderFile::ShaderFile(const ShaderFile& other) :  mEntrypoint{other.mEntrypoint}, mSize{other.mSize} {
        mCode = new(std::align_val_t{4}) char[mSize];
        memcpy(mCode, other.mCode, mSize);
    }

    ShaderFile::ShaderFile(ShaderFile&& other) noexcept : mEntrypoint{std::move(other.mEntrypoint)}, mCode{other.mCode}, mSize{other.mSize} {
        other.mCode = nullptr;
        other.mSize = 0;
    }

    ShaderFile& ShaderFile::operator=(const ShaderFile& other) {
        if (this != &other) {
            this->ShaderFile::~ShaderFile();

            mEntrypoint = other.mEntrypoint;
            mCode = new(std::align_val_t{4}) char[mSize];
            mSize = other.mSize;

            memcpy(mCode, other.mCode, mSize);
        }

        return *this;
    }

    ShaderFile& ShaderFile::operator=(ShaderFile&& other) noexcept {
        if (this != &other) {
            this->ShaderFile::~ShaderFile();

            mEntrypoint = std::move(other.mEntrypoint);
            mCode = other.mCode;
            mSize = other.mSize;

            other.mCode = nullptr;
            other.mSize = 0;
        }

        return *this;
    }

    EngineResult<ShaderFile> ShaderFile::loadFromFile(const std::filesystem::path& path, std::string entrypoint) {
        std::fstream file(path);

        if (file.fail()) {
            return EngineResult<ShaderFile>::error(EngineError::fromOsError({errno, std::generic_category() }));
        }

        std::error_code code;
        size_t bytes = std::filesystem::file_size(path, code);

        if (code) {
            return EngineResult<ShaderFile>::error(EngineError::fromOsError(code));
        }

        char* buffer = new(std::align_val_t{4}) char[bytes];

        file.read(buffer, static_cast<std::streamsize>(bytes));

        if (!file.eof()) {
            std::fstream::pos_type pos = file.tellg();
            file.seekg(0, std::fstream::end);

            size_t oldBytes = bytes;
            bytes = file.tellg();

            char* oldBuffer = buffer;
            buffer = new(std::align_val_t{4}) char[bytes];

            std::memcpy(buffer, oldBuffer, oldBytes);

            file.seekg(pos);

            file.read(buffer + oldBytes, static_cast<std::streamsize>(bytes - oldBytes));
        }

        if (file.fail()) {
            return EngineResult<ShaderFile>::error(EngineError::fromOsError({errno, std::generic_category() }));
        }

        return ShaderFile(std::move(entrypoint), buffer, bytes);
    }

    char* ShaderFile::getBytes() const {
        return mCode;
    }

    uint32_t* ShaderFile::getCode() const {
        return reinterpret_cast<uint32_t*>(mCode);
    }

    size_t ShaderFile::getSize() const {
        return mSize;
    }

    std::string& ShaderFile::getEntrypoint() {
        return mEntrypoint;
    }
}
