#include "Particle.h"
#include <lvgl.h>
#include <Adafruit_EPD_RK.h>

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(
    LOG_LEVEL_NONE, // Default logging level for all categories
    {
        {"app", LOG_LEVEL_ALL} // Only enable all log levels for the application
    });

#define SD_CS D2
#define SRAM_CS D3
#define EPD_CS D4
#define EPD_DC D5

#define EPD_RESET -1 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1  // can set to -1 to not use a pin (will wait a fixed delay)

/*Set to your screen resolution and rotation*/
#define HOR_RES 250
#define VER_RES 122
#define ROTATION LV_DISPLAY_ROTATION_0

Adafruit_SSD1680 epd(HOR_RES, VER_RES, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (HOR_RES * VER_RES / 5 * (LV_COLOR_DEPTH))

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Log.info(buf);
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, unsigned char *px_map)
{
    epd.clearBuffer();

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    // Invert the pixel map for eink display
    uint32_t px_map_size = w * h / 8;
    unsigned char inverted_px_map[px_map_size];
    for (uint32_t i = 0; i < px_map_size; i++)
        inverted_px_map[i] = ~px_map[i + 8]; // Skip the 8-byte header

    epd.drawBitmap(area->x1, area->y1, (uint8_t *)inverted_px_map, w, h, EPD_BLACK);
    epd.display();

    lv_display_flush_ready(disp);
}

/*use millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void setup()
{
    delay(1000);

    lv_init();
    lv_tick_set_cb(my_tick);
    lv_log_register_print_cb(my_print);

    lv_display_t *disp;
    disp = lv_display_create(HOR_RES, VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello Particle, I'm LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    Log.info("Starting display...");
    epd.begin();
    epd.clearBuffer();
    epd.display();
    Log.info("Setup done");
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);           /* let this time pass */
}