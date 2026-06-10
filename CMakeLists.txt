cmake_minimum_required(VERSION 3.31)
project(godscraft)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -s")

add_definitions(-DGLFW_STATIC)

set(NOESIS_DIR ${CMAKE_SOURCE_DIR}/libs/NoesisGUI)

include_directories(include)
include_directories(libs/glfw/include)
include_directories(libs/glm)
include_directories(libs/vulkan/include)
include_directories(libs/stb)
include_directories(${NOESIS_DIR}/Include)
include_directories(${NOESIS_DIR}/Src/Packages/Render/VKRenderDevice/Include)
include_directories(${NOESIS_DIR}/Src/Packages/Render/VKRenderContext/Src)
include_directories(${NOESIS_DIR}/Src/Packages/Render/RenderContext/Include)
include_directories(${NOESIS_DIR}/Src/Packages/App/Providers/Include)
include_directories(${NOESIS_DIR}/Src/Packages/App/Providers/Src)

add_subdirectory(libs/glfw)

# Your sources
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES include/*.hpp)
file(GLOB_RECURSE NOESIS_VK_SOURCES
        ${NOESIS_DIR}/Src/Packages/Render/VKRenderDevice/Src/*.cpp
        ${NOESIS_DIR}/Src/Packages/Render/VKRenderContext/Src/*.cpp
        ${NOESIS_DIR}/Src/Packages/Render/RenderContext/Src/*.cpp
        ${NOESIS_DIR}/Src/Packages/App/Providers/Src/*.cpp
)

add_executable(godscraft
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${NOESIS_VK_SOURCES}
        include/Threads/IO.hpp
        src/Threads/IO.cpp
        include/Threads/Logger.hpp
        src/Threads/Logger.cpp
)

target_link_directories(godscraft PRIVATE
        ${NOESIS_DIR}/Bin/linux_x86_64
)

target_link_libraries(godscraft
        PRIVATE glfw
        PRIVATE vulkan
        PRIVATE pthread
        PRIVATE dl
        PRIVATE Noesis
)

add_custom_command(TARGET godscraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:godscraft>/resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:godscraft>/resources
)
