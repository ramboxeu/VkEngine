find_package(SDL2 REQUIRED)
find_package(Vulkan 1.3 REQUIRED)

add_library(vkengine SHARED
    include/engine/VkEngineApp.hpp
    src/engine/VkEngineApp.cpp
    include/engine/EngineError.hpp
    src/engine/EngineError.cpp
    include/engine/utils/Result.hpp
    src/engine/utils/Result.cpp
    src/engine/vk/proxies.hpp
    src/engine/vk/proxies.cpp
    include/engine/QueueFamilyIndexes.hpp
    src/engine/QueueFamilyIndexes.cpp
    include/engine/SwapchainDetails.hpp
    src/engine/SwapchainDetails.cpp
    include/engine/EngineResult.hpp
)

target_link_libraries(vkengine PUBLIC ${Vulkan_LIBRARIES} ${SDL2_LIBRARIES})
target_include_directories(vkengine PRIVATE include src ${Vulkan_INCLUDE_DIRS} ${SLD_INCLUDE_DIRS})
