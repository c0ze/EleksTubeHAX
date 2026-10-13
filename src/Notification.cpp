#include "Notification.h"

Notification notification;

// Display positions from left to right, independent of the hardware's digit indexing.
static const uint8_t digit_at_position[NUM_DIGITS] = {HOURS_TENS, HOURS_ONES, MINUTES_TENS, MINUTES_ONES, SECONDS_TENS, SECONDS_ONES};

// Font 4 is 26 px high; scale it to fill most of the display.
#define NOTIFICATION_FONT 4
#define NOTIFICATION_TEXT_SIZE ((TFT_WIDTH >= 120) ? 4 : 2)

void Notification::show(const char *text_, uint16_t duration_sec, uint16_t color_, uint16_t scroll_ms_)
{
  // Font 4 only has printable ASCII; replace everything else (e.g. UTF-8 bytes) with '?'.
  text_len = 0;
  for (const char *p = text_; *p != '\0' && text_len < NOTIFICATION_MAX_TEXT_LEN; p++)
  {
    text[text_len++] = (*p >= ' ' && *p <= '~') ? *p : '?';
  }
  text[text_len] = '\0';

  if (duration_sec > NOTIFICATION_MAX_DURATION_SEC)
    duration_sec = NOTIFICATION_MAX_DURATION_SEC;
  duration_ms = (uint32_t)duration_sec * 1000;
  color = color_;
  scroll_ms = (scroll_ms_ < NOTIFICATION_MIN_SCROLL_MS) ? NOTIFICATION_MIN_SCROLL_MS : scroll_ms_;
  scroll_offset = 0;
  started_at = millis();
  last_step_at = started_at;
  active = true;
  was_active = true;

  Serial.print("Showing notification: ");
  Serial.println(text);
  draw();
}

bool Notification::loop()
{
  if (!active)
  {
    bool ended = was_active; // ended via clear()
    was_active = false;
    return ended;
  }

  uint32_t now = millis();
  if (duration_ms != 0 && (now - started_at) >= duration_ms)
  {
    active = false;
    was_active = false;
    return true;
  }

  if (text_len > NUM_DIGITS && (now - last_step_at) >= scroll_ms)
  {
    last_step_at = now;
    // Scroll through the text followed by a blank screen width, then start over.
    scroll_offset = (scroll_offset + 1) % (text_len + NUM_DIGITS);
    draw();
  }
  return false;
}

void Notification::draw()
{
  // Short text is centered; long text shows a window starting at scroll_offset.
  int16_t first = (text_len <= NUM_DIGITS) ? -((NUM_DIGITS - text_len) / 2) : scroll_offset;
  uint8_t cycle = text_len + NUM_DIGITS;

  for (uint8_t position = 0; position < NUM_DIGITS; position++)
  {
    int16_t index = first + position;
    if (text_len > NUM_DIGITS)
      index %= cycle;
    char c = (index >= 0 && index < text_len) ? text[index] : ' ';
    drawCharOnDigit(position, c);
  }
}

void Notification::drawCharOnDigit(uint8_t position, char c)
{
  tfts.chip_select.setDigit(digit_at_position[position]);
  tfts.fillScreen(TFT_BLACK);

  if (c != ' ')
  {
    uint16_t dimmed_color = color;
#ifndef DIM_SKIP_SOFTWARE_ALPHA // hardware dimming handles brightness on its own
    if (tfts.dimming < 255)
      dimmed_color = tfts.alphaBlend(tfts.dimming, color, TFT_BLACK);
#endif
    char str[2] = {c, '\0'};
    tfts.setTextColor(dimmed_color, TFT_BLACK);
    tfts.setTextSize(NOTIFICATION_TEXT_SIZE);
    tfts.setTextDatum(MC_DATUM);
    tfts.drawString(str, TFT_WIDTH / 2, TFT_HEIGHT / 2, NOTIFICATION_FONT);
    tfts.setTextDatum(TL_DATUM);
    tfts.setTextSize(1);
  }

#ifdef CS_DIRECT_GPIO
  tfts.chip_select.update();
#endif
}
