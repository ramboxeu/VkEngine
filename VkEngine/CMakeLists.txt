find_package(SDL REQUIRED 2.0)
find_package(Vulkan 1.3 REQUIRED)

add_library(vkengine SHARED
    include/engine/VkEngineApp.hpp
    src/engine/VkEngineApp.cpp
)

target_link_libraries(vkengine PUBLIC ${Vulkan_LIBRARIES} ${SDL_LIBRARIES})
target_include_directories(vkengine PUBLIC include ${Vulkan_INCLUDE_DIRS} ${SLD_INCLUDE_DIRS})

