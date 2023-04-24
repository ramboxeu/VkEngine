#ifndef ENGINE_ERROR_HPP
#define ENGINE_ERROR_HPP

#include <string>
#include <ostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace vke {

    class EngineError {
    public:
        enum class Kind {
            NONE,
            SDL,
            VULKAN,
            OS_ERROR,
            EXTENSIONS_NOT_PRESENT,
            NO_DEVICE,
            MISSING_VERTEX_SHADER
        };

        EngineError();
        EngineError(const EngineError& other);
        EngineError(EngineError&& other) noexcept;
        EngineError& operator=(const EngineError&);
        EngineError& operator=(EngineError&&) noexcept;
        ~EngineError() noexcept;

        friend std::ostream& operator<<(std::ostream& stream, EngineError& error);

        static EngineError fromSdlError(const char* error);
        static EngineError fromVkError(VkResult result);
        static EngineError extensionsNotPresent(std::vector<const char*> extensions);
        static EngineError noDevice();
        static EngineError fromOsError(std::error_code code);
        static EngineError missingVertexShader();
    private:
        EngineError(std::string&& str, Kind kind);
        EngineError(VkResult result, Kind kind);
        EngineError(std::vector<const char*>&& extensions, Kind kind);
        EngineError(std::error_code code, Kind kind);
        EngineError(Kind kind);

        void swap(EngineError&& other) noexcept;
        void copy(const EngineError& other);
        void clear() noexcept;

        union {
            std::string mMessage;
            VkResult mResult;
            std::vector<const char*> mExtensions;
            std::error_code mErrorCode;
        };
        Kind mKind;
    };
}

#endif
