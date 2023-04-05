#ifndef ENGINE_ERROR_HPP
#define ENGINE_ERROR_HPP

#include <string>
#include <ostream>

namespace vke {

    class EngineError {
    public:
        enum class Kind {
            NONE,
            SDL
        };

        EngineError();
        EngineError(const EngineError& other);
        EngineError(EngineError&& other) noexcept;
        EngineError& operator=(const EngineError&);
        EngineError& operator=(EngineError&&) noexcept;
        ~EngineError() noexcept;

        friend std::ostream& operator<<(std::ostream& stream, EngineError& error);

        static EngineError fromSdlError(const char* error);
    private:
        EngineError(std::string&& str, Kind kind);

        void swap(EngineError&& other) noexcept;
        void copy(const EngineError& other);
        void clear() noexcept;

        union {
            std::string mMessage;
        };
        Kind mKind;
    };
}

#endif
