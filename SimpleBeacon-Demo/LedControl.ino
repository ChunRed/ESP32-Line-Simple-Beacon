#include <FastLED.h>

#define NUM_LEDS 14
// 請依據您實際接線的 ESP32 腳位修改 DATA_PIN，例如 GPIO 2, 4, 12, 13 等
#define DATA_PIN 18

CRGB leds[NUM_LEDS];

void setupLEDs() {
  // WS2811 或 WS2815 燈帶的資料時序皆相容於 WS2812B
  // 這裡使用 WS2812B 協議，如果顏色順序不對 (例如紅綠相反)，可以將 GRB 改為 RGB
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  // 設定最高亮度 (0~255)
  FastLED.setBrightness(200);

  // 將所有燈珠設為全白
  fill_solid(leds, NUM_LEDS, CRGB::White);

  // 更新燈條顯示
  FastLED.show();
}
