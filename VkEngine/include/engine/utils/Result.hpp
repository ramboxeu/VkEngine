#ifndef RESULT_HPP
#define RESULT_HPP

#include <memory>
#include <iostream>

namespace vke::utils {

    template<typename T, typename E>
    class Result {
    public:
        enum class Kind {
            NONE,
            OK,
            ERROR
        };

        Result(const Result<T, E>& other) : mKind{other.mKind} {
            switch(other.mKind) {
                case Kind::NONE:
                    break;
                case Kind::OK:
                    new (&mOk) T(other.mOk);
                    break;
                case Kind::ERROR:
                    new (&mError) E(other.mError);
                    break;
            }
        }

        Result(Result<T, E>&& other) noexcept : mKind{other.mKind} {
            switch(other.mKind) {
                case Kind::NONE:
                    break;
                case Kind::OK:
                    new (&mOk) T(std::move(other.mOk));
                    break;
                case Kind::ERROR:
                    new (&mError) E(std::move(other.mError));
                    break;
            }
        }

        Result& operator=(const Result<T, E>& other) {
            if (this != &other) {
                mKind = other.mKind;

                switch(other.mKind) {
                    case Kind::NONE:
                        clear();
                        break;
                    case Kind::OK:
                        mOk = other.mOk;
                        break;
                    case Kind::ERROR:
                        mError = other.mError;
                        break;
                }
            }

            return *this;
        }

        Result& operator=(Result<T, E>&& other) noexcept {
            if (this != &other) {
                mKind = other.mKind;

                switch(other.mKind) {
                    case Kind::NONE:
                        clear();
                        break;
                    case Kind::OK:
                        mOk = std::move(other.mOk);
                        break;
                    case Kind::ERROR:
                        mError = std::move(other.mError);
                        break;
                }
            }

            return *this;
        }

        ~Result() noexcept {
            clear();
        }

        bool isOk() const {
            return mKind == Kind::OK;
        }

        bool isError() const {
            return mKind == Kind::ERROR;
        }

        T& getOk() {
            if (isOk())
                return mOk;

            std::cerr << "[ENGINE] [FATAL]: ok() called, but Result is not OK\n";
            std::terminate();
        }

        const T& getOk() const {
            return getOk();
        }

        E& getError() {
            if (isError())
                return mError;

            std::cerr << "[ENGINE] [FATAL]: error() called, but Result is not ERROR\n";
            std::terminate();
        }

        const E& getError() const {
            return getError();
        }

        operator bool() const {
            return isOk();
        }

        static Result<T, E> ok(T&& ok) {
            return Result<T, E>(std::move(ok));
        }

        static Result<T, E> ok(const T& ok) {
            return Result<T, E>(ok);
        }

        static Result<T, E> error(E&& error) {
            return Result<T, E>(std::move(error), {});
        }

        static Result<T, E> error(const E& error) {
            return Result<T, E>(error, {});
        }

    protected:
        struct ErrorMarker {};

        Result(T&& ok) noexcept : mOk{std::move(ok)}, mKind{Kind::OK} {
        }

        Result(const T& ok) : mOk{ok}, mKind{Kind::OK} {
        }

        Result(E&& error, ErrorMarker _) : mError{std::move(error)}, mKind{Kind::ERROR} {
        }

        Result(const E& error, ErrorMarker _) : mError{error}, mKind{Kind::ERROR} {
        }

        void swap(Result<T, E>&& other) noexcept {
            switch(other.mKind) {
                case Kind::NONE:
                    break;
                case Kind::OK:
                    new (&mOk) T(std::move(other.mOk));
//                    mOk = other.mOk;
                    break;
                case Kind::ERROR:
                    new (&mError) E(std::move(other.mError));
                    break;
            }

            other.mKind = Kind::NONE;
        }

        void copy(const Result<T, E>& other) {
            switch (other.mKind) {
                case Kind::NONE:
                    break;
                case Kind::OK:
                    mOk = other.mOk;
                    break;
                case Kind::ERROR:
                    mError = other.mError;
                    break;
            }
        }

        void clear() noexcept {
            switch (mKind) {
                case Kind::NONE:
                    break;
                case Kind::OK:
                    mOk.T::~T();
                    break;
                case Kind::ERROR:
                    mError.E::~E();
                    break;
            }

            mKind = Kind::NONE;
        }

        union {
            T mOk;
            E mError;
        };
        Kind mKind;
    };

    struct OkMarker {};

    template<typename E>
    class Result<void, E> : public Result<OkMarker, E> {
    public:
        static Result<void, E> ok() {
            return Result<void, E>();
        }

        static Result<void, E> error(E&& error) {
            return Result<void, E>(std::move(error));
        }

        static Result<void, E> error(const E& error) {
            return Result<void, E>(error);
        }

    private:
        Result() : Result<OkMarker, E>(OkMarker{}) {
        }

        Result(E&& error) : Result<OkMarker, E>(std::move(error), {}) {
        }

        Result(const E& error) : Result<OkMarker, E>(error, {}) {
        }
    };
}

#endif
