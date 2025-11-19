cmake_minimum_required(VERSION 3.16)
project(pixel C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build types
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

# Strict warnings
add_compile_options(
    -Wall -Wextra -Werror
    -Wno-unused-parameter
    -Wshadow
    -Wdouble-promotion
    -Wformat=2
    -Wundef
)

# Sanitizers for debug builds (not supported on Emscripten)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT EMSCRIPTEN)
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Core library
add_library(pixel_core
    src/core/arena.c
    src/core/strings.c
    src/core/table.c
    src/core/error.c
    src/core/log.c
)
target_include_directories(pixel_core PUBLIC src)

# VM library
add_library(pixel_vm
    src/vm/value.c
    src/vm/object.c
    src/vm/opcodes.c
    src/vm/chunk.c
    src/vm/debug.c
    src/vm/gc.c
    src/vm/vm.c
)
target_link_libraries(pixel_vm pixel_core m)

# Compiler library
add_library(pixel_compiler
    src/compiler/token.c
    src/compiler/lexer.c
    src/compiler/ast.c
    src/compiler/parser.c
    src/compiler/symbols.c
    src/compiler/analyzer.c
    src/compiler/codegen.c
)
target_link_libraries(pixel_compiler pixel_core pixel_vm)

# Runtime library (standard library)
add_library(pixel_runtime
    src/runtime/stdlib.c
)
target_link_libraries(pixel_runtime pixel_core pixel_vm)

# Platform Abstraction Layer (PAL)
# Mock backend is always available; SDL2 backend is optional
add_library(pixel_pal
    src/pal/pal.c
    src/pal/pal_mock.c
)
target_include_directories(pixel_pal PUBLIC src)

# Emscripten uses its own SDL2 ports
if(EMSCRIPTEN)
    message(STATUS "Emscripten detected - using SDL2 ports for web build")
    target_sources(pixel_pal PRIVATE src/pal/pal_sdl.c)
    target_compile_definitions(pixel_pal PRIVATE PAL_USE_SDL2)
    # Emscripten SDL2 ports - compile flags
    target_compile_options(pixel_pal PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_IMAGE=2
        -sUSE_SDL_MIXER=2
        -sSDL2_IMAGE_FORMATS=["png","jpg"]
    )
else()
    # Native builds: Optional SDL2 support
    find_package(SDL2 QUIET)
    find_package(SDL2_image QUIET)
    find_package(SDL2_mixer QUIET)

    if(SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND)
        message(STATUS "SDL2 found - enabling SDL2 backend")
        target_sources(pixel_pal PRIVATE src/pal/pal_sdl.c)
        target_compile_definitions(pixel_pal PRIVATE PAL_USE_SDL2)
        target_link_libraries(pixel_pal SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer)
    else()
        message(STATUS "SDL2 not found - using mock backend only")
    endif()
endif()

# Engine library
add_library(pixel_engine
    src/engine/engine.c
    src/engine/engine_natives.c
    src/engine/physics.c
)
target_link_libraries(pixel_engine
    pixel_core
    pixel_vm
    pixel_runtime
    pixel_pal
)

# Main executable
add_executable(pixel src/main.c)
target_link_libraries(pixel
    pixel_core
    pixel_compiler
    pixel_vm
    pixel_runtime
    pixel_engine
)

# Define PAL_USE_SDL2 for main executable when using SDL2
if(EMSCRIPTEN OR (SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_F