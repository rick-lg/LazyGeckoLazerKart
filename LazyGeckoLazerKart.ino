/*
 * LazyGeckoLazerKart.ino
 * Software used during OpenSauce 2024 Lazer Tag Enabled P.W.N.D. Race by LazyGecko 

 * Started with 'SimpleReceiver.cpp'
 *
 * Demonstrates receiving ONLY NEC protocol IR codes with IRremote
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************/
 //Sketchbook location under preferences c:\GitLG\LazyGeckoLazerKart


//    SELECT "ESP32 DEV MODULE" AS THE BOARD
#define VERSION_STR "6.15.2025 d2.0"


#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
#include <Arduino.h>
#include <LazyGeckoLazerKart.h>


//OTA=========================
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <Update.h>
//OTA=========================

WebServer server(80);
TaskHandle_t serverTaskHandle;

///I want SSID to build off of the MAC addr of the device
//Handled lower
//const char *ssid = "LG:TG:XX:XX:XX:XX:XX";
const char *password = "boutablast";
bool isUpdating = false;
void handleRoot() {
  if (server.method() == HTTP_POST) {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      isUpdating = true;
      digitalWrite(LG_CAR_LED_MOSFET_EN_ST, LOW); // Start with LED off
      Serial.printf("Update Start: %s\n", upload.filename.c_str());
      if (!Update.begin()) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      isUpdating = false; // Stop blinking
      if (Update.end(true)) {
        Serial.println("Update Success. Rebooting...");
      } else {
        Update.printError(Serial);
      }
    }
    delay(1000);
    ESP.restart();
  } else {
server.send(200, "text/html", R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 OTA Update</title>
    <style>
      body { font-family: Arial; padding: 20px; }
      progress { width: 100%; height: 30px; }
    </style>
  </head>
  <body>
    <h2>ESP32 OTA Update</h2>
    <a href="/status"> View Status Page</a>
    <form id="uploadForm">
      <input type="file" id="file" name="update" required><br><br>
      <input type="submit" value="Upload">
    </form>
    <br>
    <progress id="progressBar" value="0" max="100"></progress>
    <p id="status"></p>

    <script>
      const form = document.getElementById('uploadForm');
      const fileInput = document.getElementById('file');
      const progressBar = document.getElementById('progressBar');
      const statusText = document.getElementById('status');

      form.addEventListener('submit', function (e) {
        e.preventDefault();
        const file = fileInput.files[0];
        if (!file) return;

        const xhr = new XMLHttpRequest();
        xhr.open("POST", "/", true);

        xhr.upload.onprogress = function (e) {
          if (e.lengthComputable) {
            const percent = Math.round((e.loaded / e.total) * 100);
            progressBar.value = percent;
            statusText.textContent = `Uploading: ${percent}%`;
          }
        };

        xhr.onload = function () {
          if (xhr.status == 200) {
            statusText.textContent = "Upload complete. Rebooting ESP32...";
          } else {
            statusText.textContent = "Upload failed.";
          }
        };

        const formData = new FormData();
        formData.append("update", file);
        xhr.send(formData);
      });
    </script>
  </body>
  </html>
)rawliteral");
  }
}

// Task to handle HTTP requests on Core 0
void serverTask(void *parameter) {
  while (true) {
    server.handleClient();
    vTaskDelay(1); // prevent WDT reset
  }
}

// Optional: separate handler for upload stream (though we reused root)
void handleUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START || upload.status == UPLOAD_FILE_WRITE || upload.status == UPLOAD_FILE_END) {
    handleRoot();
  }
}

//Uncomment one of these
//#define LASER_ACTIVATED_BUBBLE_GUN (1)
//#define LASER_ACTIVATED_EAGLE_GUN  (1)
#define LASER_ACTIVATED_GOKART_GUN (1)


#ifdef LASER_ACTIVATED_BUBBLE_GUN
  #define OFF_BY_DEFAULT (1)
  #define TYPE_OF_TARTGET_STR "BUBBLE GUN MODE"
  #define MAX_LIFE (10)
  //Time to keep car dead
  #define DEATH_MS (5000)
  //Time car resist guns
  #define JESUS_MS (5000)

#elif LASER_ACTIVATED_EAGLE_GUN
  #define OFF_BY_DEFAULT (1)
  #define TYPE_OF_TARTGET_STR "EAGLE GUN MODE"
  #define MAX_LIFE (10)
  //Time to keep car dead
  #define DEATH_MS (5000)
  //Time car resist guns
  #define JESUS_MS (5000)

#elif LASER_ACTIVATED_GOKART_GUN

  //#define OFF_BY_DEFAULT (1)
  
  #define TYPE_OF_TARTGET_STR "GOKART GUN MODE"
  //Race 1 and 2 where at 10 health. 
  #define MAX_LIFE (15)
  //Time to keep car dead
  #define DEATH_MS (5000)
  //Time car resist guns
  // - VESC need more time to come back up
  #define JESUS_MS (10000)

#endif

int CAR_HEALTH = MAX_LIFE;

Adafruit_NeoPixel pixels(LG_CAR_HEALTH_BAR_COUNT, LG_CAR_HEALTH_BAR_PIN, NEO_GRB + NEO_KHZ800);

void HEALTH_BAR_JESUS_UPDATE(){
    for(int i=0; i<LG_CAR_HEALTH_BAR_COUNT; i++) {
      //YELLOW
        pixels.setPixelColor(i, pixels.Color(200, 200, 0));  
    }
  pixels.show();   // Send the updated pixel colors to the hardware.   
  
}


void HEALTH_BAR_UPDATE(){
  float percent = (float) CAR_HEALTH / (float)MAX_LIFE;
  
  int life_bars =  percent * LG_CAR_HEALTH_BAR_COUNT;
  Serial.println("Updating leds...");
  Serial.print("PERCENT...");
  Serial.println(percent);
  Serial.print("BARS...");
  Serial.println(life_bars);

  
  for(int i=0; i<LG_CAR_HEALTH_BAR_COUNT; i++) {
      if(i < life_bars){        
        //Green
        pixels.setPixelColor(i, pixels.Color(0, 200, 0));
      }else{        
        //Red
        pixels.setPixelColor(i, pixels.Color(200, 0, 0));
      }
  
  }
  pixels.show();   // Send the updated pixel colors to the hardware.    
}

void LaserGun_EnableCar(){
#ifndef OFF_BY_DEFAULT
  digitalWrite(LG_CAR_ENABLE_IO, HIGH);
  digitalWrite(LG_CAR_STATUS_IO, LOW);
  
  //LED ON when cart / grounds are connected
  digitalWrite(LG_CAR_LED_MOSFET_EN_ST, HIGH);
#else
  digitalWrite(LG_CAR_ENABLE_IO, LOW);
  digitalWrite(LG_CAR_STATUS_IO, HIGH);

  //LED ON when cart / grounds are connected
  digitalWrite(LG_CAR_LED_MOSFET_EN_ST, LOW);
#endif


}

void LaserGun_DisableCar(){
#ifndef OFF_BY_DEFAULT
  digitalWrite(LG_CAR_ENABLE_IO, LOW);
  digitalWrite(LG_CAR_STATUS_IO, HIGH);

  //LED ON when cart / grounds are connected
  digitalWrite(LG_CAR_LED_MOSFET_EN_ST, LOW);
#else
  digitalWrite(LG_CAR_ENABLE_IO, HIGH);
  digitalWrite(LG_CAR_STATUS_IO, LOW);
  
  //LED ON when cart / grounds are connected
  digitalWrite(LG_CAR_LED_MOSFET_EN_ST, HIGH);
#endif
}

void pulseRed(uint8_t wait) {
  
  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    pixels.fill(pixels.Color(pixels.gamma8(j), 0, 0));
    pixels.show();
    delay(wait);
    
  }  
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    pixels.fill(pixels.Color(pixels.gamma8(j), 0, 0));
    pixels.show();
    delay(wait);
  }
}

void pulseYellow(uint8_t wait) {
  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    pixels.fill(pixels.Color(pixels.gamma8(j),pixels.gamma8(j), 0));
    pixels.show();
    delay(wait);
  }  
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    pixels.fill(pixels.Color(pixels.gamma8(j), pixels.gamma8(j), 0));
    pixels.show();
    delay(wait);
  }
}

void LaserGun_KillLED_Sequence(int _time){  
  for(int i = 0; i < (_time/1000); i++){
    pulseRed(2);
  }
}
void LaserGun_JesusLED_Sequence(int _time){  
  for(int i = 0; i < (_time/500); i++){
    pulseYellow(1);
  }
}

void LaserGun_KillCar(){
  
  Serial.print("DISABLING CAR FOR ");
  Serial.print(DEATH_MS);
  Serial.println(" MS");
  
  LaserGun_DisableCar();
  
  LaserGun_KillLED_Sequence(DEATH_MS);
  //delay(DEATH_MS);
  LaserGun_ReviveCar();
}

void LaserGun_ReviveCar(){
  
  Serial.print("ENABLING CAR. SAFE FOR ");
  Serial.print(JESUS_MS);
  Serial.println(" MS");

  CAR_HEALTH = MAX_LIFE;
  LaserGun_EnableCar();
  /*
  HEALTH_BAR_JESUS_UPDATE();
  delay(JESUS_MS);  
  */
  LaserGun_JesusLED_Sequence(JESUS_MS);
  
  HEALTH_BAR_UPDATE();
  Serial.println("DAMAGE REENABLED... LOOKING FOR SHOTS");
}

int LaserGun_CarShot(int8_t _damage){
  CAR_HEALTH -= _damage;

  CAR_HEALTH = (CAR_HEALTH < 0)? 0: CAR_HEALTH;
  CAR_HEALTH = (CAR_HEALTH > MAX_LIFE)? MAX_LIFE: CAR_HEALTH;

  Serial.print("CAR_HEALTH ");
  Serial.println(CAR_HEALTH);
  
  HEALTH_BAR_UPDATE();
  
  if(CAR_HEALTH <= 0){
    LaserGun_KillCar();
  }
  return CAR_HEALTH;
}

#define BLUE_GUN01 (0x048800)
#define BLUE_GUN02 (0xC08000)
#define BLUE_GUN03 (0xC08000)
#define BLUE_GUN04 (0x008800)


#define BLUE_GUN01_2025       (0x40078)
#define BLUE_GUN_ROCKET_2025  (0x24078)

int damg = 0;
void LaserGun_CheckMessage(int _data){

  switch(_data){
    case BLUE_GUN01_2025:
    case BLUE_GUN01:    
      Serial.println("Shots Fired from BLUE_GUN01");
      damg = 1;
      LaserGun_CarShot(damg);
      break;
    case BLUE_GUN02: //Gun 3 has the same code for some reason
      break;
    case BLUE_GUN_ROCKET_2025:
    case BLUE_GUN04:  //Rocket Launcher = Healing Launcher
      damg = -1 * (MAX_LIFE / 4);  
      LaserGun_CarShot(damg);
      break;
    default:
      return;
      break;
  }  

  HEALTH_BAR_UPDATE();
}





void setup() {

    
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));


    //Stats about the car
   
    Serial.println("=================================");
    Serial.println("TARGET CONFIGURATIONS");
    Serial.println("================================="); 
    Serial.print("Version: ");
    Serial.println(VERSION_STR);

    Serial.print("Mode: ");
    Serial.println(TYPE_OF_TARTGET_STR);

    Serial.print("Default State: ");
  #ifdef OFF_BY_DEFAULT
    Serial.println(" OFF");
  #else
    Serial.println(" ON");
  #endif
    Serial.print("Max Health: ");
    Serial.println(MAX_LIFE);
    Serial.print("'Health Bar LED Count: ");
    Serial.println(LG_CAR_HEALTH_BAR_COUNT);
    Serial.print("'Death' Time (ms): ");
    Serial.println(DEATH_MS);
    Serial.print("'JESUS' Time (ms): ");
    Serial.println(JESUS_MS);
    


    Serial.println("------------------------");
    Serial.println(" PINOUT ");
    Serial.println("------------------------");
    Serial.print("IR RX: ");
    Serial.println(STR(IR_RECEIVE_PIN_ESP));
    Serial.print("FET ENABLE: ");
    Serial.println(STR(LG_CAR_ENABLE_IO));
    Serial.print("STATUS LED: ");
    Serial.println(STR(LG_CAR_STATUS_IO));
    Serial.print("LED STRIP: ");
    Serial.println(STR(LG_CAR_HEALTH_BAR_PIN));
    Serial.println("=================================");

    pinMode(LG_CAR_ENABLE_IO, OUTPUT);   
    pinMode(LG_CAR_STATUS_IO, OUTPUT);  

    pinMode(LG_CAR_LED_MOSFET_EN_ST, OUTPUT);   
    pinMode(LG_CAR_LED_IR_RX_ST, OUTPUT); 

    digitalWrite(LG_CAR_LED_IR_RX_ST, LOW);
    digitalWrite(LG_CAR_LED_MOSFET_EN_ST, LOW);

    delay (1000);
    digitalWrite(LG_CAR_LED_IR_RX_ST, HIGH);
    delay (1000);
    digitalWrite(LG_CAR_LED_MOSFET_EN_ST, HIGH);
    delay (1000);

    Serial.println("CHECKING: Disabling Car for 5 seconds...");
    LaserGun_DisableCar();
    delay (5000);

    Serial.println("CHECKING: Enabling Car...");
    digitalWrite(LG_CAR_LED_IR_RX_ST, LOW);
    LaserGun_ReviveCar();
    
    //Setup Wifi for OTA in AP Mode
    WiFi.mode(WIFI_AP);                  // Important!
    WiFi.softAP("t", "123");     // Start AP
    // Get the MAC address and format it
    String mac = WiFi.softAPmacAddress();
    Serial.print("MACADDR: ");
    Serial.println(mac);

    String ssid = "LG:TG:" + mac;

    Serial.print("SSID Set to : ");
    Serial.println(ssid);

    WiFi.softAP(ssid.c_str(), password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP()); //Is this constant?

    // Start Web Server
     // Route and OTA upload handler
    server.on("/", HTTP_ANY, handleRoot, handleUpload);

    // Add /status route handler
    server.on("/status", HTTP_GET, []() {


int CAR_HEALTH = MAX_LIFE;
    bool default_output = false;
#ifdef OFF_BY_DEFAULT
  default_output = true;
#endif

      String html = "<!DOCTYPE html><html><head><title>ESP32 Status</title></head><body>";
      html += "<a href=""""/""""> View OTA Page</a>";
      html += "<h1>Status</h1><ul>";
      html += "<li>TYPE_OF_TARTGET_STR: " + String(TYPE_OF_TARTGET_STR) + "</li>";
      html += "<li>OFF_BY_DEFAULT: " + String(default_output) + "</li>";
      html += "<li>MAX_LIFE: "  + String(MAX_LIFE) + "</li>";
      html += "<li>DEATH_MS: "  + String(DEATH_MS) + "</li>";
      html += "<li>JESUS_MS : " + String(JESUS_MS) + "</li>";
      html += "<li>CAR_HEALTH : " + String(CAR_HEALTH) + "</li>";
      html += "</ul>";
      html += "<h2>Outputs</h2>";
      html += "<p>LG_CAR_ENABLE_IO = "        + String(digitalRead(LG_CAR_ENABLE_IO)) + "</p>";
      html += "<p>LG_CAR_STATUS_IO = "        + String(digitalRead(LG_CAR_STATUS_IO)) + "</p>";
      html += "<p>LG_CAR_LED_MOSFET_EN_ST = " + String(digitalRead(LG_CAR_LED_MOSFET_EN_ST)) + "</p>";
      html += "<p>LG_CAR_LED_IR_RX_ST = "     + String(digitalRead(LG_CAR_LED_IR_RX_ST)) + "</p>";
      html += "</body></html>";

      server.send(200, "text/html", html);
    });
    
    server.begin();

    Serial.println("Web server started");
    // Start server handling on Core 0
    xTaskCreatePinnedToCore(
        serverTask,        // Function
        "WebServerTask",   // Name
        4096,              // Stack size
        NULL,              // Params
        1,                 // Priority
        &serverTaskHandle, // Handle
        0                  // Core 0
      );
    

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    //IrReceiver.begin(IR_RECEIVE_PIN_ESP, ENABLE_LED_FEEDBACK);
    IrReceiver.begin(IR_RECEIVE_PIN_ESP, false);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN_ESP)));

    
    pixels.begin();
    pixels.show(); 


#ifndef OFF_BY_DEFAULT
    Serial.println(VERSION_STR);
    Serial.println(">>OUTPUT ENABLED BY DEFAULT<<");
#else
    Serial.println(VERSION_STR);
    Serial.println("<<OUTPUT DISABLED BY DEFAULT>>");
#endif
   // LaserGun_KillLED_Sequence(DEATH_MS);
   
    
}


unsigned long lastBlinkTime = 0;
bool ledState = false;
void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     *
     * E.g. command is in IrReceiver.decodedIRData.command
     * address is in command is in IrReceiver.decodedIRData.address
     * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
     */
     
    //server.handleClient();

  if (isUpdating) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 250) { // Blink interval
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LG_CAR_LED_MOSFET_EN_ST, ledState);
    }
  }else{
      digitalWrite(LG_CAR_LED_IR_RX_ST, LOW);
      if (IrReceiver.decode()) {

        digitalWrite(LG_CAR_LED_IR_RX_ST, HIGH);
          /*
          * Print a summary of received data
          */

          if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
              Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
              // We have an unknown protocol here, print extended info
              IrReceiver.printIRResultRawFormatted(&Serial, true);
              IrReceiver.resume(); // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
          } else {
              IrReceiver.resume(); // Early enable receiving of the next IR frame
              IrReceiver.printIRResultShort(&Serial);
              IrReceiver.printIRSendUsage(&Serial);
          }
          Serial.println();
  #ifdef DEBUG_YALL
  #endif
          
          Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
          LaserGun_CheckMessage(IrReceiver.decodedIRData.decodedRawData);
          
      }
  }
}
