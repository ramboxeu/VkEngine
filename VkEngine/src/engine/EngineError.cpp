#include "engine/EngineError.hpp"

namespace vke {

    EngineError::EngineError() : mKind{Kind::NONE} {
    }

    EngineError::EngineError(const EngineError& other) : mKind{other.mKind} {
        switch (other.mKind) {
            case Kind::NONE:
                break;
            case Kind::SDL:
                new (&mMessage) std::string(other.mMessage);
                break;
            case Kind::VULKAN:
                mResult = other.mResult;
                break;
            case Kind::EXTENSIONS_NOT_PRESENT:
                new (&mExtensions) std::vector<const char*>(other.mExtensions);
                break;
        }
    }

    EngineError::EngineError(EngineError&& other) noexcept : mKind{other.mKind} {
        switch (other.mKind) {
            case Kind::NONE:
                break;
            case Kind::SDL:
                new (&mMessage) std::string(std::move(other.mMessage));
                break;
            case Kind::VULKAN:
                mResult = other.mResult;
                break;
            case Kind::EXTENSIONS_NOT_PRESENT:
                new (&mExtensions) std::vector<const char*>(std::move(other.mExtensions));
                break;
        }
    }

    EngineError& EngineError::operator=(const EngineError& other) {
        if (&other != this) {
            mKind = other.mKind;
            switch (other.mKind) {
                case Kind::NONE:
                    clear();
                    break;
                case Kind::SDL:
                    mMessage = other.mMessage;
                    break;
                case Kind::VULKAN:
                    mResult = other.mResult;
                    break;
                case Kind::EXTENSIONS_NOT_PRESENT:
                    mExtensions = other.mExtensions;
                    break;
            }
        }

        return *this;
    }

    EngineError& EngineError::operator=(EngineError&& other) noexcept {
        if (&other != this) {
            mKind = other.mKind;
            switch (other.mKind) {
                case Kind::NONE:
                    clear();
                    break;
                case Kind::SDL:
                    mMessage = std::move(other.mMessage);
                    break;
                case Kind::VULKAN:
                    mResult = other.mResult;
                    break;
                case Kind::EXTENSIONS_NOT_PRESENT:
                    mExtensions = std::move(other.mExtensions);
                    break;
            }
        }

        return *this;
    }

    EngineError::~EngineError() noexcept {
        clear();
    }

    std::ostream& operator<<(std::ostream& stream, EngineError& error) {
        switch (error.mKind) {
            case EngineError::Kind::NONE:
                stream << "[NoError]";
                break;
            case EngineError::Kind::SDL:
                stream << "[SdlError] " << error.mMessage;
                break;
            case EngineError::Kind::VULKAN:
                stream << "[VulkanError] " << std::hex << error.mResult << std::dec;
                break;
            case EngineError::Kind::EXTENSIONS_NOT_PRESENT:
                stream << "[ExtensionsNotPresent] [";
                for (size_t i = 0, size = error.mExtensions.size(); i < size; i++) {
                    stream << error.mExtensions[i];
                    if(i < size - 1) stream << ", ";
                }
                stream << ']';
                break;
        }

        return stream;
    }

    EngineError EngineError::fromSdlError(const char* error) {
        return EngineError(std::string{error}, Kind::SDL);
    }

    EngineError EngineError::fromVkError(VkResult result) {
        return EngineError(result, Kind::VULKAN);
    }

    EngineError EngineError::extensionsNotPresent(std::vector<const char*> extensions) {
        return EngineError(std::move(extensions), Kind::EXTENSIONS_NOT_PRESENT);
    }

    EngineError::EngineError(std::string&& str, Kind kind) : mMessage{str}, mKind{kind} {
    }

    EngineError::EngineError(VkResult result, Kind kind) : mResult{result}, mKind{kind} {
    }

    EngineError::EngineError(std::vector<const char*>&& extensions, Kind kind) : mExtensions{std::move(extensions)}, mKind{kind} {
    }

    void EngineError::swap(EngineError&& other) noexcept {
        switch (other.mKind) {
            case Kind::NONE:
                break;
            case Kind::SDL:
                new (&mMessage) std::string(std::move(other.mMessage));
//                mMessage = std::move(other.mMessage);
                break;
        }

        other.mKind = Kind::NONE;
    }

    void EngineError::copy(const EngineError& other) {
        switch (other.mKind) {
            case Kind::NONE:
                break;
            case Kind::SDL:
                mMessage = other.mMessage;
                break;
        }
    }

    void EngineError::clear() noexcept {
        switch (mKind) {
            case Kind::NONE:
            case Kind::VULKAN:
                break;
            case Kind::SDL:
                mMessage.std::string::~string();
                break;
            case Kind::EXTENSIONS_NOT_PRESENT:
                mExtensions.std::vector<const char*>::~vector();
                break;
        }

        mKind = Kind::NONE;
    }
}
