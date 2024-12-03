cmake_minimum_required(VERSION 3.28)
project(pipeline LANGUAGES CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CPM.cmake)

CPMAddPackage("gh:freetype/freetype#VER-2-13-2")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:ocornut/imgui@1.90.5")
CPMAddPackage("gh:cursey/safetyhook@0.4.1")
CPMAddPackage(
  NAME zydis
  GITHUB_REPOSITORY zyantific/zydis
  VERSION 4.1.0
  OPTIONS "ZYDIS_BUILD_EXAMPLES OFF"
  "ZYDIS_BUILD_TOOLS OFF"
  "ZYDIS_BUILD_DOXYGEN OFF"
)


add_library(pipeline SHARED
  # cheat
  pipeline-source/main.cpp

  #hooks
  pipeline-source/hooks/hooks.cpp
  pipeline-source/hooks/lists/directx.cpp
  pipeline-source/hooks/lists/prediction.cpp
  pipeline-source/hooks/lists/client.cpp
  pipeline-source/hooks/lists/studio.cpp
  pipeline-source/hooks/lists/surface.cpp
  pipeline-source/gui/menu/menu.cpp

 # sdk
 pipeline-include/valve/sdk/netvars.cpp
  pipeline-include/valve/sdk/entities.cpp
 pipeline-include/link.cpp
 pipeline-include/console/console.cpp
 pipeline-include/crashlog.cpp
 pipeline-source/factory/factory.cpp
 pipeline-include/drawsystem/drawsystem.cpp
  pipeline-include/valve/sdk/entities.cpp 
  pipeline-include/valve/sdk/interfaces.cpp 
  pipeline-include/valve/sdk/shared.cpp

)

if(imgui_ADDED)
  target_sources(pipeline PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
  )

  target_include_directories(pipeline PUBLIC
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends>)

  target_compile_definitions(pipeline PUBLIC
    "IMGUI_USER_CONFIG=\"${CMAKE_SOURCE_DIR}/pipeline-include/imconfig.hpp\""
    IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
  )
endif()

# Check build types so we can apply specific settings per build type.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(pipeline PRIVATE
    NOMINMAX GLM_FORCE_SWIZZLE GLM_FORCE_ALIGNED_GENTYPES PIPELINE_DEBUG PIPELINE_BETA
  )
  target_compile_options(pipeline PRIVATE
    /Od /Zi /MTd /arch:AVX2
  )
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  target_compile_definitions(pipeline PRIVATE
    NOMINMAX NDEBUG GLM_FORCE_SWIZZLE GLM_FORCE_ALIGNED_GENTYPES PIPELINE_BETA
  )
  target_compile_options(pipeline PRIVATE
    /O2 /Zi /MT /arch:AVX2
  )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(pipeline PRIVATE
    NOMINMAX NDEBUG GLM_FORCE_SWIZZLE GLM_FORCE_ALIGNED_GENTYPES PIPELINE_RELEASE
  )
  target_compile_options(pipeline PRIVATE
    /O2 /MT /arch:AVX
  )
else()
  message(FATAL_ERROR "Build type is not supported.")
endif()

target_include_directories(pipeline PRIVATE pipeline-include)
target_include_directories(pipeline PRIVATE pipeline-source/cheat)

target_link_libraries(pipeline PRIVATE safetyhook::safetyhook glm::glm freetype d3d9)