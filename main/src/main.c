
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* needed for usleep() */
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>

#if defined(_WIN32) && !defined(_MSC_VER)
#include <windows.h>
#endif
#endif
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_demos_ext.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t *hal_init(int32_t w, int32_t h);
static void high_res_exit_cb(lv_demo_high_res_api_t *api);
static void high_res_timer_delete_cb(lv_event_t *e);
static void high_res_clock_timer_cb(lv_timer_t *t);
static void high_res_make_abs_path(char *buf, uint32_t len, const char *rel);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern void freertos_main(void);

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int SDL_main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

#if 1
  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(1280, 720);

  /* ============ High-Res demo example ============ */
  char assets_path[512];
  char logo_path[512];
  char slides_path[512];
  high_res_make_abs_path(assets_path, sizeof(assets_path),
                         "lv_demos/src/high_res/assets");
  high_res_make_abs_path(logo_path, sizeof(logo_path),
                         "lv_demos/src/high_res/assets/img_lv_demo_high_res_lvgl_logo.png");
  high_res_make_abs_path(slides_path, sizeof(slides_path),
                         "lv_demos/src/high_res/assets/about_app_slides");

  lv_demo_high_res_api_t *api = lv_demo_high_res(
      assets_path,       /* assets folder */
      logo_path,         /* logo file */
      slides_path,       /* slides folder (about_app_slides) */
      high_res_exit_cb); /* called when the user logs out */

  /* Initial data for the UI (see lv_demo_high_res.h for all subjects) */
  lv_subject_set_int(&api->subjects.hour, 10);
  lv_subject_set_int(&api->subjects.minute, 30);
  lv_subject_set_pointer(&api->subjects.week_day_name, "Thursday");
  lv_subject_set_int(&api->subjects.month_day, 20);
  lv_subject_set_pointer(&api->subjects.month_name, "August");
  lv_subject_set_int(&api->subjects.temperature_outdoor, 28 * 10); /* 28.0 C */
  lv_subject_set_int(&api->subjects.temperature_indoor, 24 * 10);  /* 24.0 C */
  lv_subject_set_int(&api->subjects.volume, 60);
  lv_subject_set_int(&api->subjects.door, 0);

  /* Let the clock tick every second */
  lv_timer_t *clock_timer = lv_timer_create(high_res_clock_timer_cb, 1000, api);
  lv_obj_add_event_cb(api->base_obj, high_res_timer_delete_cb, LV_EVENT_DELETE, clock_timer);
#endif
  /* Other demos (uncomment a line and adjust hal_init to the matching resolution) */
  // clang-format off
  // hal_init(480, 320); lv_demo_ebike();
  // hal_init(384, 384); lv_demo_smartwatch();
  // hal_init(800, 480); lv_demo_multilang();
  // hal_init(800, 480); lv_demo_transform();
  // hal_init(800, 480); lv_demo_flex_layout();
  // hal_init(800, 480); lv_demo_scroll();
  // hal_init(1280, 720); lv_demo_widgets();
  // hal_init(1280, 720); lv_demo_music();
  // hal_init(1280, 720); lv_demo_stress();
  // hal_init(1280, 720); lv_demo_keypad_encoder();
  // hal_init(1280, 720); lv_demo_benchmark();
  // clang-format on

  while (1)
  {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
    usleep(5 * 1000);
  }

  return 0;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   HIGH-RES DEMO CALLBACKS
 **********************/

static void high_res_make_abs_path(char *buf, uint32_t len, const char *rel)
{
  char exe_path[512];
  DWORD n = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
  if (n == 0 || n >= sizeof(exe_path))
  {
    lv_snprintf(buf, len, "A:%s", rel); /*fallback: relative to CWD*/
    return;
  }

  /*main.exe lives in <project>/bin/, so go up two levels to the project root*/
  char *slash = strrchr(exe_path, '\\');
  if (slash)
    *slash = '\0';
  slash = strrchr(exe_path, '\\');
  if (slash)
    *slash = '\0';

  lv_snprintf(buf, len, "A:%s/%s", exe_path, rel);
}

static void high_res_exit_cb(lv_demo_high_res_api_t *api)
{
  lv_obj_delete(api->base_obj); /*Close the demo*/
}

static void high_res_timer_delete_cb(lv_event_t *e)
{
  lv_timer_t *timer = lv_event_get_user_data(e);
  lv_timer_delete(timer);
}

static void high_res_clock_timer_cb(lv_timer_t *t)
{
  lv_demo_high_res_api_t *api = lv_timer_get_user_data(t);
  int32_t minute = lv_subject_get_int(&api->subjects.minute) + 1;
  if (minute == 60)
  {
    minute = 0;
    int32_t hour = lv_subject_get_int(&api->subjects.hour) + 1;
    if (hour == 24)
      hour = 0;
    lv_subject_set_int(&api->subjects.hour, hour);
  }
  lv_subject_set_int(&api->subjects.minute, minute);
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t *hal_init(int32_t w, int32_t h)
{

  lv_group_set_default(lv_group_create());

  lv_display_t *disp = lv_sdl_window_create(w, h);

  lv_indev_t *mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);
  lv_display_set_default(disp);

  LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t *cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
  lv_indev_set_cursor(mouse, cursor_obj);           /*Connect the image  object to the driver*/

  lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);
  lv_indev_set_group(mousewheel, lv_group_get_default());

  lv_indev_t *kb = lv_sdl_keyboard_create();
  lv_indev_set_display(kb, disp);
  lv_indev_set_group(kb, lv_group_get_default());

  return disp;
}
