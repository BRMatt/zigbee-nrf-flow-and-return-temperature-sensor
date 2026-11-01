#ifndef SCREEN_H__
#define SCREEN_H__

/** @file screen.h
 * @brief Shows probe readings on the SSD1306 OLED of the Seeed Expansion Base for XIAO.
 *
 * Only built when CONFIG_FART_SCREEN is enabled. Otherwise these are no-ops, so
 * callers don't need to check.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_FART_SCREEN)

/** @brief Initialises the screen and character framebuffer
 *
 *  @retval 0   If the operation was successful.
 *              Otherwise, a (negative) error code is returned.
 */
int screen_init(void);

/** @brief Clears the screen and shows a single line of text
 */
void screen_show_message(const char *message);

/** @brief Shows the temperature and resistance of both probes
 *
 *  @param  resistance_x. Probe resistance in ohms. Negative if it couldn't be read.
 */
void screen_show_readings(int16_t temperature_1, float resistance_1, int16_t temperature_2, float resistance_2);

#else

static inline int screen_init(void) { return 0; }
static inline void screen_show_message(const char *message) {}
static inline void screen_show_readings(int16_t temperature_1, float resistance_1, int16_t temperature_2, float resistance_2) {}

#endif

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H__ */
