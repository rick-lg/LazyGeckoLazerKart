
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>

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


// MQTT broker settings
const char* mqtt_server = "192.168.69.1";  // Replace with your MQTT broker IP
const int mqtt_port = 1883;
const char* mqtt_user = "";        // Optional (or leave blank "")
const char* mqtt_password = "";    // Optional



//Replaced with mqttClientId
//const char* mqtt_client_id = "LG-Car-Client";


//Updating OTAs
//1. Move the old otas to a different directory
//2. PowerShell
    //scp -r -O "C:\GitLG\LazyGeckoLazerKart\builds\*" root@192.168.69.1:/root/lg_tg/ota
//3. Update definitions in Node-Red
//4. Have teh devices reconnect to the server.

//Outstanding Issues:
// Connecting to the cloud shouldn't stiffle the rest of the car/device functions