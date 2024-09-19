cmake_minimum_required(VERSION 3.10)
project(GabexEngine VERSION 0.10)

# Specify C++ standard and build type
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Disable unnecessary GLFW build options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/src/EC"
    "${CMAKE_SOURCE_DIR}/include"
    "C:/VulkanSDK/1.3.283.0/Include"
    "${CMAKE_SOURCE_DIR}/include/volk" # Added VOLK include
)

# Specify the path for GLFW include and library directories
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/glfw/include")
set(GLFW_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/include/glfw/lib-mingw-w64")

link_directories(
    "C:/VulkanSDK/1.3.283.0/Lib"
    "C:/Program Files/Assimp/lib/x64"
    ${GLFW_LIBRARY_DIR}
)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/imguifiledialog/ImGuiFileDialog.cpp"
)

list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

set(IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/imgui_demo.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_vulkan.cpp"
    "${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/include/imguizmo/ImGuizmo.cpp"
)

# Add executable
add_executable(GabexEngine
    ${SOURCES}
    ${IMGUI_SOURCES}
    ${VOLK_SOURCE}  # Added VOLK source
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
)

# Define libraries
set(LIBS
    vulkan-1
    assimp-vc143-mt
    ${GLFW_LIBRARY_DIR}/libglfw3dll.a 
)

target_compile_definitions(GabexEngine PUBLIC VOLK_STATIC_DEFINE)

target_link_libraries(GabexEngine PUBLIC ${LIBS})

target_compile_definitions(GabexEngine PUBLIC DEBUG)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/DEBUG)
