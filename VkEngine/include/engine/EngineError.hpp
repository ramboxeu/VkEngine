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
            EXTENSIONS_NOT_PRESENT,
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
    private:
        EngineError(std::string&& str, Kind kind);
        EngineError(VkResult result, Kind kind);
        EngineError(std::vector<const char*>&& extensions, Kind kind);

        void swap(EngineError&& other) noexcept;
        void copy(const EngineError& other);
        void clear() noexcept;

        union {
            std::string mMessage;
            VkResult mResult;
            std::vector<const char*> mExtensions;
        };
        Kind mKind;
    };
}

#endif
