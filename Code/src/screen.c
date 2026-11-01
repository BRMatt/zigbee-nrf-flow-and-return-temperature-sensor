#include "screen.h"

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(screen, LOG_LEVEL_INF);

#define SCREEN_LINES 4
#define LINE_LENGTH 24

// Anything above this is treated as a disconnected probe.
//
#define MAX_RESISTANCE_OHMS 999999

static const struct device *const screen = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static bool screen_ready = false;
static uint8_t line_height;

int screen_init(void)
{
	if (!device_is_ready(screen))
	{
		LOG_ERR("Screen %s not ready", screen->name);
		return -ENODEV;
	}

	if (display_set_pixel_format(screen, PIXEL_FORMAT_MONO10) != 0 &&
		display_set_pixel_format(screen, PIXEL_FORMAT_MONO01) != 0)
	{
		LOG_ERR("Screen doesn't support a monochrome pixel format");
		return -ENOTSUP;
	}

	int err = cfb_framebuffer_init(screen);

	if (err)
	{
		LOG_ERR("Framebuffer initialisation failed (err: %d)", err);
		return err;
	}

	// Pick the largest font that still fits SCREEN_LINES lines on the display.
	//
	uint16_t screen_height = cfb_get_display_parameter(screen, CFB_DISPLAY_HEIGH);
	uint8_t font_width;
	uint8_t font_height;

	for (uint8_t idx = 0; cfb_get_font_size(screen, idx, &font_width, &font_height) == 0; idx++)
	{
		if (font_height * SCREEN_LINES <= screen_height && font_height > line_height)
		{
			cfb_framebuffer_set_font(screen, idx);
			line_height = font_height;
		}
	}

	if (line_height == 0)
	{
		LOG_ERR("No font fits %d lines", SCREEN_LINES);
		return -EINVAL;
	}

	cfb_framebuffer_clear(screen, true);
	display_blanking_off(screen);

	screen_ready = true;

	return 0;
}

static void show_lines(char lines[][LINE_LENGTH], int count)
{
	if (!screen_ready)
	{
		return;
	}

	cfb_framebuffer_clear(screen, false);

	for (int i = 0; i < count; i++)
	{
		int err = cfb_print(screen, lines[i], 0, i * line_height);

		if (err)
		{
			LOG_ERR("Failed to print line %d (err: %d)", i, err);
		}
	}

	cfb_framebuffer_finalize(screen);
}

static void format_resistance(char *line, int probe, float resistance)
{
	if (resistance < 0 || resistance > MAX_RESISTANCE_OHMS)
	{
		snprintf(line, LINE_LENGTH, "R%d open", probe);
		return;
	}

	// Show as kilohms with two decimal places. Formatted as integers to avoid
	// needing float support in printf.
	//
	int ohms = (int)(resistance + 0.5f);

	snprintf(line, LINE_LENGTH, "R%d %d.%02dk", probe, ohms / 1000, (ohms % 1000) / 10);
}

void screen_show_message(const char *message)
{
	char lines[1][LINE_LENGTH];

	snprintf(lines[0], LINE_LENGTH, "%s", message);

	show_lines(lines, 1);
}

void screen_show_readings(int16_t temperature_1, float resistance_1, int16_t temperature_2, float resistance_2)
{
	char lines[SCREEN_LINES][LINE_LENGTH];

	snprintf(lines[0], LINE_LENGTH, "T1 %d C", temperature_1);
	format_resistance(lines[1], 1, resistance_1);
	snprintf(lines[2], LINE_LENGTH, "T2 %d C", temperature_2);
	format_resistance(lines[3], 2, resistance_2);

	show_lines(lines, SCREEN_LINES);
}
