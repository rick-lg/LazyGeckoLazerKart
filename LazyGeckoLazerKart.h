
#include <Adafruit_NeoPixel.h>

#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

//ESP32
#define IR_RECEIVE_PIN_ESP      ( 4)
#define LG_CAR_ENABLE_IO        (16)
#define LG_CAR_HEALTH_BAR_PIN   ( 5)
#define LG_CAR_STATUS_IO        (19)
//On Board Status Pins
#define LG_CAR_LED_MOSFET_EN_ST (14)
#define LG_CAR_LED_IR_RX_ST     (12)


//How many LEDS
#define LG_CAR_HEALTH_BAR_COUNT   (20)
