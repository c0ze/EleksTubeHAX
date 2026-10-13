#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "GLOBAL_DEFINES.h"
#include "TFTs.h"

/*
 * Shows a short text notification on the displays, one character per digit.
 * Text longer than NUM_DIGITS scrolls from right to left.
 * While a notification is active, the clock digits are not redrawn.
 */

#define NOTIFICATION_MAX_TEXT_LEN 64
#define NOTIFICATION_DEFAULT_DURATION_SEC 10
#define NOTIFICATION_MAX_DURATION_SEC 3600
#define NOTIFICATION_DEFAULT_COLOR 0xFBC0 // Nixie orange (RGB 255, 122, 0)
#define NOTIFICATION_DEFAULT_SCROLL_MS 350
#define NOTIFICATION_MIN_SCROLL_MS 100

class Notification
{
public:
  // duration_sec == 0 keeps the notification until clear() is called.
  void show(const char *text, uint16_t duration_sec, uint16_t color, uint16_t scroll_ms);
  void clear() { active = false; }
  bool isActive() { return active; }

  // Call from the main loop. Returns true once, when the notification has ended and the clock must be redrawn.
  bool loop();

private:
  void draw();
  void drawCharOnDigit(uint8_t position, char c);

  char text[NOTIFICATION_MAX_TEXT_LEN + 1] = "";
  uint8_t text_len = 0;
  uint16_t color = NOTIFICATION_DEFAULT_COLOR;
  uint16_t scroll_ms = NOTIFICATION_DEFAULT_SCROLL_MS;
  uint32_t duration_ms = 0;
  uint32_t started_at = 0;
  uint32_t last_step_at = 0;
  uint8_t scroll_offset = 0;
  bool active = false;
  bool was_active = false;
};

extern Notification notification;

#endif // NOTIFICATION_H
