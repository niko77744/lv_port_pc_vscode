# ============================================================================
#  C3 Mini smartwatch demo (fbiego/esp32-c3-mini)
#  A complete LVGL 9 watch UI: watchfaces, apps, games, notifications...
#
#  It is built as a SEPARATE executable (bin/c3.exe) with its OWN LVGL
#  instance, because the watchfaces require LV_COLOR_DEPTH == 16 while the
#  main project (high_res demo etc.) uses LV_COLOR_DEPTH == 32.
#  Nothing here touches the existing `main` executable.
# ============================================================================

set(C3_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(C3_LVGL_ROOT ${PROJECT_SOURCE_DIR}/lvgl)

# ---------------------------------------------------------------------------
# 1) A private LVGL build configured by c3_mini/include/lv_conf.h
# ---------------------------------------------------------------------------
file(GLOB_RECURSE C3_LVGL_SOURCES CONFIGURE_DEPENDS ${C3_LVGL_ROOT}/src/*.c)

add_library(lvgl_c3 STATIC ${C3_LVGL_SOURCES})

target_include_directories(lvgl_c3 PUBLIC
    ${C3_ROOT}/include                 # lv_conf.h (LV_COLOR_DEPTH=16) must come first
    ${C3_LVGL_ROOT}/src
    ${C3_LVGL_ROOT}
    ${SDL2_INCLUDE_DIRS}
)

target_compile_definitions(lvgl_c3 PUBLIC
    LV_CONF_INCLUDE_SIMPLE
    LV_USE_SDL=1
    LV_USE_QRCODE=1
    LV_MEM_CUSTOM=1
    LV_DRV_NO_CONF
    SDL_HOR_RES=240
    SDL_VER_RES=240
    SDL_ZOOM=1
)

# ---------------------------------------------------------------------------
# 2) The C3 Mini app (UI + apps + games + watchfaces) and the SDL2 HAL
# ---------------------------------------------------------------------------
file(GLOB_RECURSE C3_SOURCES CONFIGURE_DEPENDS
    ${C3_ROOT}/src/*.c
    ${C3_ROOT}/src/*.cpp
)
list(APPEND C3_SOURCES ${C3_ROOT}/hal/sdl2/app_hal.cpp)

add_executable(c3 ${C3_SOURCES})

target_include_directories(c3 PRIVATE
    ${C3_ROOT}/include
    ${C3_ROOT}/hal/sdl2
    ${C3_ROOT}/src
    ${C3_LVGL_ROOT}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(c3 PRIVATE lvgl_c3 ${SDL2_LIBRARIES})
if(NOT MSVC)
    target_link_libraries(c3 PRIVATE m pthread)
endif()

set_target_properties(c3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

if(WIN32 AND NOT MSVC)
    target_link_options(c3 PRIVATE -mconsole)
endif()

# Custom target to run the watch demo
add_custom_target(run_c3 COMMAND ${PROJECT_SOURCE_DIR}/bin/c3 DEPENDS c3)
