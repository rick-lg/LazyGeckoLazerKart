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
#define VERSION_STR "!7.09.2025-d2.5-DEMO"


#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
#include <Arduino.h>
#include <LazyGeckoLazerKart.h>
#include <LGdevice_type.h>

//OTA=========================
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <Update.h>
#include <HTTPClient.h>
//OTA=========================

const byte DNS_PORT = 53;
DNSServer dnsServer;

WebServer server(80);
TaskHandle_t serverTaskHandle;

IPAddress apIP(192, 168, 4, 1);

String mqttClientId = "";

WiFiClient espClient;
PubSubClient client(espClient);

  char macStr[18];
String getHostName() {
  //iF WE WANT UNIQUE CAUSE EVERYONE IS ON THE SAME NETWROK
 /**/ 
  uint8_t mac[6];
  WiFi.macAddress(mac);
 // char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x-%02x-%02x-%02x-%02x-%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);




  return "LG:TG:" + String(macStr) +".local";
}

///I want SSID to build off of the MAC addr of the device
//Handled lower
//const char *ssid = "LG:TG:XX:XX:XX:XX:XX";
//const char *password = "boutablast";
bool isUpdating = false;


void performOTAUpdate(const char* url) {
  Serial.printf("Starting OTA update from URL: %s\n", url);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) { // Start update with max available size
      Serial.println("Not enough space to start OTA");
      http.end();
      return;
    }

    Serial.println("Begin OTA update...");

    size_t written = Update.writeStream(*stream);

    if (written == contentLength) {
      Serial.printf("Written %u bytes successfully\n", written);
    } else {
      Serial.printf("Written only %u/%u bytes. Update failed!\n", written, contentLength);
      http.end();
      return;
    }

    if (Update.end()) {
      if (Update.isFinished()) {
        Serial.println("OTA update finished successfully. Rebooting...");
        ESP.restart();
      } else {
        Serial.println("Update not finished? Something went wrong.");
      }
    } else {
      Serial.printf("Update failed. Error #: %u\n", Update.getError());
    }
  } else {
    Serial.printf("Failed to download firmware. HTTP code: %d\n", httpCode);
  }

  http.end();
}


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

      Serial.printf("Update UPLOAD_FILE_WRITE: %s\n", upload.filename.c_str());
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      isUpdating = false; // Stop blinking
      //Put something on the website when we are done
      if (Update.end(true)) {
        Serial.println("Update Success. Rebooting...");
      } else {
        Update.printError(Serial);
      }
    delay(3000);
    ESP.restart();
    }
  } else {
String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>LazyGecko LG-TAG OTA Updater</title>
  <style>
    :root {
      --bg: #f4f4f4;
      --fg: #222;
      --accent: #228B22;
      --card-bg: #ffffff;
      --border: #ccc;
      --font: 'Courier New', monospace;
    }

    body {
      margin: 0;
      padding: 20px;
      font-family: var(--font);
      background: var(--bg);
      color: var(--fg);
    }

    h2 {
      text-align: center;
      font-size: 1.6rem;
      letter-spacing: 1px;
      text-transform: uppercase;
    }
    h3 {
      text-align: center;
      font-size: 1.0rem;
      letter-spacing: 1px;
    }

    a.status {
      display: block;
      text-align: center;
      color: var(--accent);
      font-weight: bold;
      text-decoration: none;
      margin-bottom: 25px;
    }

    a.status:hover {
      text-decoration: underline;
    }

    form {
      background: var(--card-bg);
      border: 1px solid var(--border);
      padding: 20px;
      max-width: 400px;
      margin: 0 auto;
      border-radius: 8px;
      box-shadow: 2px 2px 10px rgba(0, 0, 0, 0.05);
      display: flex;
      flex-direction: column;
      gap: 15px;
    }

    input[type="file"] {
      padding: 8px;
      font-family: var(--font);
    }

    input[type="submit"] {
      background: var(--accent);
      border: none;
      padding: 10px;
      font-weight: bold;
      color: white;
      border-radius: 4px;
      cursor: pointer;
      transition: background 0.2s ease;
    }

    input[type="submit"]:hover {
      background: #e0269f;
    }

    progress {
      width: 100%;
      height: 20px;
      margin-top: 20px;
    }

    #status {
      text-align: center;
      margin-top: 10px;
      min-height: 1.2em;
      font-weight: bold;
    }
    /* Style submit button same as label */
    input[type="submit"] {
      background: var(--accent);
      border: none;
      padding: 15px;
      font-size: 1.4rem;
      font-weight: bold;
      color: white;
      border-radius: 8px;
      cursor: pointer;
      width: 100%;
      max-width: 400px;
      box-sizing: border-box;
      box-shadow: 2px 2px 6px rgba(0,0,0,0.15);
      transition: background 0.2s ease;
    }

    #file {
    position: absolute;
    width: 1px;
    height: 1px;
    padding: 0;
    margin: -1px;
    overflow: hidden;
    }
    
    .file-label {
    display: block;
    background-color: var(--accent);
    color: white;
    padding: 15px;
    font-size: 1.4rem;
    font-weight: bold;
    border-radius: 8px;
    cursor: pointer;
    text-align: center;
    user-select: none;
    width: 100%;
    max-width: 400px;
    margin-bottom: 15px;
    box-sizing: border-box;
    box-shadow: 2px 2px 6px rgba(0,0,0,0.15);
    transition: background-color 0.2s ease;
    }
    
    .button-link {
    display: block;
    background-color: var(--accent);
    color: white;
    padding: 15px;
    font-size: 1.4rem;
    font-weight: bold;
    border-radius: 8px;
    cursor: pointer;
    text-align: center;
    user-select: none;
    width: 100%;
    max-width: 400px;
    margin-bottom: 15px;
    box-sizing: border-box;
    box-shadow: 2px 2px 6px rgba(0,0,0,0.15);
    transition: background-color 0.2s ease;
    text-decoration: none;
    }
    
    
    
    
    
    .file-label:hover {
    background-color: #1e6f1e;
    }
    
    .file-label, .button-link, 
    input[type="submit"] {
      font-family: Arial, sans-serif;  or your chosen font */
      font-weight: bold;
      font-size: 1.4rem;
      line-height: 1.2; /* keep consistent */
    }
    @media (max-width: 480px) {
      form {
        padding: 15px;
        width: 100%;
      }

      body {
        padding: 10px;
      }
    }
  </style>
</head>
<body>

  <h2>LG Tag OTA Update</h2>
  <h3>[%VERSION_STR%]</h3>
  <h3>[%MAC_ADDR_STR%]</h3>
 
  <form id="uploadForm">
      
      <a href="/status" class="button-link">Status Page</a>
      
    <label for="file" class="file-label">Select Bin File</label>
    <input type="file" id="file" name="update" accept=".bin" required />
    <input type="submit" value="Upload Firmware" />
  </form>

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
        if (xhr.status === 200) {
          statusText.textContent = "✅ Upload complete. Rebooting ESP32...";
        } else {
          statusText.textContent = "❌ Upload failed.";
        }
      };

      const formData = new FormData();
      formData.append("update", file);
      xhr.send(formData);
    });
  </script>
</body>
</html>

)rawliteral";


html.replace("%VERSION_STR%", VERSION_STR);
html.replace("%MAC_ADDR_STR%", String(WiFi.macAddress()));



server.send(200, "text/html", html);



  }
}

void registerMsg(){
    String topic = "device/"+mqttClientId+"/register";
    String payload = "{";

    payload+= "\"mac\":\""+String(macStr)+"\",";
    payload+= "\"firmware\":\""+String(VERSION_STR)+"\",";
    payload+= "\"type\":\""+String(DEVICE_TYPE)+"\"";
    payload+= "}";


    client.publish(topic.c_str(), payload.c_str());
}

void setupMQTTSubscriptions() {
  Serial.println("Setting up MQTT subscriptions...");
  
  client.subscribe("lg-car/commands");  
  client.subscribe(String("device/"+mqttClientId+"/ota").c_str());
  client.subscribe(String("batch/" + String(DEVICE_TYPE) + "/ota").c_str());
  client.subscribe("batch/all/ota");

  Serial.println("MQTT subscriptions complete");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print(mqttClientId);
    Serial.print("..........");
    if (client.connect(mqttClientId.c_str())) {
      Serial.print("Connected as ");
      Serial.println(mqttClientId);
      
      // Set up multiple MQTT subscriptions for flexible update targeting
      setupMQTTSubscriptions();
      
      registerMsg();


    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s...");
      delay(5000);
    }
  }
}



// Task to handle HTTP requests on Core 0
void serverTask(void *parameter) {
  while (true) {
    //dnsServer.processNextRequest();                       //NEW

    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop();

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
//Handled for Bulk building
//#define LASER_ACTIVATED_ADLIAS_BLOWER_GUN (1)
//#define LASER_ACTIVATED_FOG_GUN (1)
//#define LASER_ACTIVATED_WATER_GUN (1)
//#define LASER_ACTIVATED_BUBBLE_GUN (1)
//#define LASER_ACTIVATED_EAGLE_GUN  (1)
//#define LASER_ACTIVATED_GOKART_GUN (1)

 
#ifdef LASER_ACTIVATED_ADLIAS_BLOWER_GUN

  #define OFF_BY_DEFAULT (1)
  #define TYPE_OF_TARTGET_STR "ADLIAS_BLOWER MODE"
  #define MAX_LIFE (10)
  //Time to keep car dead
  #define DEATH_MS (5000)
  //Time car resist guns
  #define JESUS_MS (5000)

#elif LASER_ACTIVATED_FOG_GUN

  #define OFF_BY_DEFAULT (1)
  #define TYPE_OF_TARTGET_STR "FOG MACHINE MODE"
  #define MAX_LIFE (10)
  //Time to keep car dead
  #define DEATH_MS (15000)
  //Time car resist guns
  #define JESUS_MS (5000)

#elif LASER_ACTIVATED_WATER_GUN

  #define OFF_BY_DEFAULT (1)
  #define TYPE_OF_TARTGET_STR "WATER GUN MODE"
  #define MAX_LIFE (10)
  //Time to keep car dead
  #define DEATH_MS (1500)
  //Time car resist guns
  #define JESUS_MS (5000)

#elif LASER_ACTIVATED_BUBBLE_GUN
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
void registerDamage(int8_t _damage){
    String topic = "device/"+mqttClientId+"/damage";
    String payload = "{";

    payload+= "\"mac\":\""+String(macStr)+"\",";
    payload+= "\"type\":\""+String(DEVICE_TYPE)+"\",";
    payload+= "\"damage\":\""+String(_damage)+"\",";
    payload+= "\"health\":\""+String(CAR_HEALTH)+"\"";
    payload+= "}";

    Serial.print("Publishing ...");
    Serial.println(payload);


    client.publish(topic.c_str(), payload.c_str());
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
  registerDamage(-MAX_LIFE);
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

  registerDamage(_damage);

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

String getOtaUrl(const String& json) {
  const String key = "\"otaurl\":\"";
  int start = json.indexOf(key);
  if (start == -1) return "";  // not found

  start += key.length();
  int end = json.indexOf("\"", start);
  if (end == -1) return "";    // no closing quote found

  return json.substring(start, end);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert topic to String for easy handling
  String topicStr = String(topic);

  // Print topic for debugging
  Serial.print("Topic: ");
  Serial.println(topicStr);

  // Check if topic ends with "/ota"
  if (topicStr.endsWith("/ota")) {
    // Convert payload to string
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }


    String url = getOtaUrl(message);

    // Print the OTA URL
    Serial.print("OTA URL received: ");
    Serial.println(url);
    
    // Add random delay for batch updates to prevent network congestion
    if (topicStr.startsWith("batch/")) {
      uint32_t delay_ms = esp_random() % 30000;  // 0-30 second delay
      Serial.print("Batch update detected, delaying: ");
      Serial.print(delay_ms);
      Serial.println(" ms");
      vTaskDelay(delay_ms/portTICK_PERIOD_MS);

    }
    
    performOTAUpdate(url.c_str());
    //Pass this to the update module?

  }
}

TaskHandle_t wifiTaskHandle = NULL;
bool wifi_enabled = false;


void WiFiTask(void * parameter) {

  const char* ssid = "LG-Router";
  const char* password = "supermansucks";

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    uint32_t jitter_ms = esp_random() % 5000;  // up to 5000 ms
    uint32_t total_delay = 5000;
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      wifi_enabled = false;
      Serial.println("WiFi disconnected. Attempting reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);

      int retries = 0;
      while (WiFi.status() != WL_CONNECTED && retries < 20) { // 10 seconds max
        delay(500);
        Serial.print(".");
        retries++;
      }
    
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // Build hostname and start mDNS
        String hostname = getHostName();
        mqttClientId = String(macStr);
        mqttClientId.replace("-", "");  // Remove colons for compatibility

        Serial.print("Hostname: ");
        Serial.println(hostname);
        
        wifi_enabled = true;

        //Wait to do this
        client.setServer(mqtt_server, mqtt_port);
        client.setCallback(mqttCallback);  // Optional: if you want to handle messages
      }else {
          Serial.println("\nFailed to reconnect to WiFi.");    
          jitter_ms = esp_random() % 5000;  // up to 5000 ms
          total_delay = 5000 + jitter_ms;
          Serial.print("\nRetry in....");
          Serial.println(total_delay);
      }
    }
        // Base delay: 5 sec, add jitter: 0–5 sec
    vTaskDelay(total_delay / portTICK_PERIOD_MS);
  }

  // Delete this task after WiFi connects
  vTaskDelete(NULL);
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


    Serial.println("CHECKING: Enabling Car...");
    digitalWrite(LG_CAR_LED_IR_RX_ST, LOW);
    LaserGun_ReviveCar();
    
   
    // Start WiFi connection task (non-blocking)
      xTaskCreatePinnedToCore(
        WiFiTask,
        "WiFiTask",
        4096,
        NULL,
        1,
        &wifiTaskHandle,
        1 // Run on core 1 for example
      );

   // dnsServer.start(DNS_PORT, "*", apIP);  // catch-all DNS

    // Start Web Server
     // Route and OTA upload handler
    server.on("/", HTTP_ANY, handleRoot, handleUpload);

    // Add /status route handler
    server.on("/status", HTTP_GET, handleStatus);
    
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

void handleStatus() {
 
    bool default_output = false;
#ifdef OFF_BY_DEFAULT
  default_output = true;
#endif
String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>ESP32 Status</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f9f9f9;
      color: #222;
      margin: 20px;
      max-width: 600px;
    }
    a {
      color: #228B22;
      text-decoration: none;
      font-weight: bold;
      margin-bottom: 20px;
      display: inline-block;
    }
    a:hover {
      text-decoration: underline;
    }
    h1, h2 {
      border-bottom: 2px solid #228B22;
      padding-bottom: 5px;
    }
    ul {
      list-style-type: none;
      padding: 0;
    }
    li {
      background: #fff;
      margin-bottom: 8px;
      padding: 10px;
      border-radius: 5px;
      box-shadow: 0 1px 3px rgba(0,0,0,0.1);
    }
    p {
      background: #fff;
      padding: 10px;
      border-radius: 5px;
      box-shadow: 0 1px 3px rgba(0,0,0,0.1);
      margin: 6px 0;
    }
  </style>
</head>
<body>
  <a href="/">&#8592; View OTA Page</a>
  <h1>Status</h1>
  <ul>
)rawliteral";

html += "<li>MAC: " + String(WiFi.macAddress()) + "</li>";
html += "<li>VERSION_STR: " + String(VERSION_STR) + "</li>";
html += "<li>TYPE_OF_TARTGET_STR: " + String(TYPE_OF_TARTGET_STR) + "</li>";
html += "<li>OFF_BY_DEFAULT: " + String(default_output) + "</li>";
html += "<li>MAX_LIFE: "  + String(MAX_LIFE) + "</li>";
html += "<li>DEATH_MS: "  + String(DEATH_MS) + "</li>";
html += "<li>JESUS_MS: " + String(JESUS_MS) + "</li>";
html += "<li>CAR_HEALTH: " + String(CAR_HEALTH) + "</li>";

html += R"rawliteral(
  </ul>
  <h2>Outputs</h2>
)rawliteral";

html += "<p>LG_CAR_ENABLE_IO = "        + String(digitalRead(LG_CAR_ENABLE_IO)) + "</p>";
html += "<p>LG_CAR_STATUS_IO = "        + String(digitalRead(LG_CAR_STATUS_IO)) + "</p>";
html += "<p>LG_CAR_LED_MOSFET_EN_ST = " + String(digitalRead(LG_CAR_LED_MOSFET_EN_ST)) + "</p>";
html += "<p>LG_CAR_LED_IR_RX_ST = "     + String(digitalRead(LG_CAR_LED_IR_RX_ST)) + "</p>";

html += R"rawliteral(
</body>
</html>
)rawliteral";


      server.send(200, "text/html", html);
    
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

    //DEBUGGING THE WIFI COLLISION
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 1000) { // Blink interval
      lastBlinkTime = currentMillis;
      ledState = !ledState;
     // LaserGun_CarShot(5);
    }

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
