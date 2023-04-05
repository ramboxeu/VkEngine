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
        }
    }

    EngineError::EngineError(EngineError&& other) noexcept : mKind{other.mKind} {
        switch (other.mKind) {
            case Kind::NONE:
                break;
            case Kind::SDL:
                new (&mMessage) std::string(std::move(other.mMessage));
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
        }

        return stream;
    }

    EngineError EngineError::fromSdlError(const char* error) {
        return EngineError({error}, Kind::SDL);
    }

    EngineError::EngineError(std::string&& str, Kind kind) : mMessage{str}, mKind{kind} {
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
                break;
            case Kind::SDL:
                mMessage.std::string::~string();
                break;
        }

        mKind = Kind::NONE;
    }
}
