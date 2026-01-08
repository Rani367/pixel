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

# Coverage configuration
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE AND NOT EMSCRIPTEN)
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage -fprofile-arcs -ftest-coverage)
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
    src/compiler/types.c
    src/compiler/typechecker.c
    src/compiler/ccodegen.c
)
target_link_libraries(pixel_compiler pixel_core pixel_vm)

# Runtime library (standard library)
add_library(pixel_runtime
    src/runtime/stdlib.c
)
target_link_libraries(pixel_runtime pixel_core pixel_vm)

# AOT Runtime library (for compiled Pixel programs)
add_library(pixel_aot_runtime
    src/aot_runtime/px_runtime.c
)
target_include_directories(pixel_aot_runtime PUBLIC src/aot_runtime)
target_link_libraries(pixel_aot_runtime m)

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
        -sUSE_SDL_TTF=2
        -sSDL2_IMAGE_FORMATS=["png","jpg"]
    )
else()
    # Native builds: Optional SDL2 support
    find_package(SDL2 QUIET)
    find_package(SDL2_image QUIET)
    find_package(SDL2_mixer QUIET)
    find_package(SDL2_ttf QUIET)

    if(SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_ttf_FOUND)
        message(STATUS "SDL2 found - enabling SDL2 backend")
        target_sources(pixel_pal PRIVATE src/pal/pal_sdl.c)
        target_compile_definitions(pixel_pal PRIVATE PAL_USE_SDL2)
        target_link_libraries(pixel_pal SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)
    else()
        message(STATUS "SDL2 not found - using mock backend only")
    endif()
endif()

# Engine library
add_library(pixel_engine
    src/engine/engine.c
    src/engine/engine_natives.c
    src/engine/physics.c
    src/engine/ui.c
    src/engine/ui_natives.c
    src/engine/ui_menus.c
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
if(EMSCRIPTEN OR (SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_ttf_FOUND))
    target_compile_definitions(pixel PRIVATE PAL_USE_SDL2)
endif()

# Emscripten-specific executable configuration
if(EMSCRIPTEN)
    # Use custom HTML shell template if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/web/shell.html")
        set(EMSCRIPTEN_SHELL "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html")
    else()
        set(EMSCRIPTEN_SHELL "")
    endif()

    # Emscripten linker flags
    set_target_properties(pixel PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "\
            -sUSE_SDL=2 \
            -sUSE_SDL_IMAGE=2 \
            -sUSE_SDL_MIXER=2 \
            -sUSE_SDL_TTF=2 \
            -sSDL2_IMAGE_FORMATS=[\"png\",\"jpg\"] \
            -sALLOW_MEMORY_GROWTH=1 \
            -sFORCE_FILESYSTEM=1 \
            -sEXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\",\"callMain\",\"FS\"] \
            -sEXPORTED_FUNCTIONS=[\"_main\",\"_emscripten_cancel_main_loop\"] \
            -sASYNCIFY=1 \
            -sASYNCIFY_STACK_SIZE=65536 \
            ${EMSCRIPTEN_SHELL} \
        "
    )
    message(STATUS "Emscripten build configured - output will be pixel.html")
endif()

# Coverage targets
if(ENABLE_COVERAGE AND NOT EMSCRIPTEN)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${LCOV} --zerocounters --directory . --ignore-errors unused,unsupported
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV} --capture --directory . --output-file coverage_raw.info
                    --ignore-errors unused,unsupported,gcov
            COMMAND ${LCOV} --extract coverage_raw.info '*/src/*' --output-file coverage.info
                    --ignore-errors unused,unsupported
            COMMAND ${GENHTML} coverage.info --output-directory coverage_report
                    --ignore-errors unused,unsupported
            COMMAND ${LCOV} --summary coverage.info --ignore-errors unused,unsupported
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )

        add_custom_target(coverage-check
            COMMAND sh -c "COVERAGE=$$(${LCOV} --summary coverage.info 2>&1 | grep lines | sed 's/.*: *\\([0-9.]*\\)%.*/\\1/'); test $$(echo \"$$COVERAGE >= 100\" | bc) -eq 1"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Checking coverage threshold (100% required)"
            DEPENDS coverage
        )
    endif()
endif()

# Tests (skip for Emscripten builds)
if(NOT EMSCRIPTEN)
    enable_testing()
    add_subdirectory(tests)
endif()
