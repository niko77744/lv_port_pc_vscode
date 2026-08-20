cmake_minimum_required(VERSION 3.12.4)
cmake_policy(SET CMP0079 NEW)
project(lvgl C CXX)

# Main include files of the project
include_directories(${PROJECT_SOURCE_DIR}/main/inc)

# Define options for LVGL with default values (OFF)
option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" OFF)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Find and include SDL2 library
find_package(SDL2 REQUIRED)

# Remove ARM-specific compile and linker options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

# Add compile definitions based on the selected options
add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# Add LVGL subdirectory
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

# Add the standalone lv_demos library (lvgl/lv_demos) and link it to LVGL
add_subdirectory(lv_demos)
target_link_libraries(lvgl PUBLIC lv_demos_ext)

# Add the C3 Mini smartwatch demo (separate executable, own LVGL config)
option(BUILD_C3_MINI "Build the C3 Mini watch demo (bin/c3.exe)" ON)
if(BUILD_C3_MINI)
    add_subdirectory(c3_mini)
endif()

# Create the main executable (LVGL-only, no RTOS)
add_executable(main
    ${PROJECT_SOURCE_DIR}/main/src/main.c
    ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
)

# Define LVGL configuration as a simple include
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)

if(MSVC)
    target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES})
else()
    target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread)
endif()

# Custom target to run the executable
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

# Conditionally include and link SDL2_image if LV_USE_DRAW_SDL is enabled
if(LV_USE_DRAW_SDL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
    find_package(SDL2_image REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(main ${SDL2_IMAGE_LIBRARIES})
endif()

# Conditionally include and link libpng if LV_USE_LIBPNG is enabled
if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(main ${PNG_LIBRARIES})
endif()

# Conditionally include and link libjpeg-turbo if LV_USE_LIBJPEG_TURBO is enabled
if(LV_USE_LIBJPEG_TURBO)
    find_package(JPEG REQUIRED)
    target_include_directories(lvgl PUBLIC ${JPEG_INCLUDE_DIRS})
    target_link_libraries(main ${JPEG_LIBRARIES})
endif()

# Conditionally include and link FFmpeg libraries if LV_USE_FFMPEG is enabled
if(LV_USE_FFMPEG)
    target_link_libraries(main avformat avcodec avutil swscale z)
endif()

# Conditionally include and link FreeType if LV_USE_FREETYPE is enabled
if(LV_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(main ${FREETYPE_LIBRARIES})
endif()

# On Windows, GUI applications do not show a console by default,
# which hides log output. This ensures a console is available for logging.
if (WIN32)
    if (MSVC)
        target_link_options(main PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(main PRIVATE "-mconsole")
    endif()
endif()

# Apply additional compile options if the build type is Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")

    target_compile_options(lvgl PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        # -Wmissing-prototypes
        -Wpointer-arith
        -Wmultichar
        -Wno-pedantic # ignored for now, we convert functions to pointers for properties table.
        -Wreturn-type
        -Wshadow
        -Wshift-negative-value
        -Wsizeof-pointer-memaccess
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-shadow>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-double-promotion>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-float-conversion>
    )
    # ThorVG sources are third-party C++ code; keep the strict checks for LVGL's own code only.
    # Suppress the four noisy warning classes for C++ (all ThorVG) sources.

if (ASAN)
    message(STATUS "AddressSanitizer enabled")

    # Add AddressSanitizer flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
else()
    message(STATUS "AddressSanitizer disabled")
endif()
endif()
