add_executable(gears
    src/main.cpp
)

target_link_libraries(gears PRIVATE vkengine)
target_include_directories(gears PRIVATE VkEngine/include) 
