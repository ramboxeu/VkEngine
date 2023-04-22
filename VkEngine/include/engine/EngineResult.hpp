#ifndef ENGINERESULT_HPP
#define ENGINERESULT_HPP

#include "engine/utils/Result.hpp"
#include "engine/EngineError.hpp"

namespace vke {

    template<typename T>
    using EngineResult = utils::Result<T, EngineError>;
}

#endif
