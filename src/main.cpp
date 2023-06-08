#include "BluetoothSerial.h"
#include <Arduino.h>
#include <ELMduino.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include <unordered_map>

#define SIZE(x) sizeof(x) / sizeof(x[0])

#define WIFI_NUM_RETRIES 10

BluetoothSerial SerialBT;
ELM327 elm327;
uint8_t elm327MacAddress[6] = {0x00, 0x1D, 0xA5, 0x68, 0x98, 0x8A};
static bool elm327Connected = false;
static bool bluetoothConnected = false;

// Replace with your network credentials
const char* ssid = "QuestTest";
const char* password = "12345678";

void Wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("ESP32 WIFI Connected to Access Point");
}

void Get_IPAddress(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WIFI Connected!");
  Serial.println("IP address of Connected WIFI: ");
  Serial.println(WiFi.localIP());
}

void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  int retries = 0;
  Serial.println("Disconnected from WIFI");
  Serial.print("Connection Lost Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Reconnecting...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    if (retries == WIFI_NUM_RETRIES ) {
      Serial.println("Reseting ESP...");
      ESP. restart();
    }
    delay(1000);
    Serial.println("Reconnecting to WiFi...");
    retries++;
  }
}

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// const char* ssid     = "ESP32-Access-Point";
// const char* password = "12345678";

struct engine_parameter {
  std::string param_name;
  uint8_t pid;
  std::function<float (ELM327&)> call_function;
  float value;
  float old_value;
};

engine_parameter engine_data[] = {
  { "rpm", ENGINE_RPM,            &ELM327::rpm, -1.0f, -1.0f },
  { "speed", VEHICLE_SPEED,         &ELM327::kph, -1.0f, -1.0f },
  { "coolantTemp", ENGINE_COOLANT_TEMP,   &ELM327::engineCoolantTemp, -1.0f, -1.0f },
};

bool connectBluetooth()
{
  while (!bluetoothConnected)
  {
    Serial.println("Connecting to Bluetooth...");
    if (SerialBT.connect(elm327MacAddress))
    {
      Serial.println("Bluetooth connected!");
      bluetoothConnected = true;
    }
    else
    {
      delay(200);
    }
  }
  
  return bluetoothConnected;
}

void sendJson(String jsonData)
{
  Serial.println(jsonData);
  webSocket.broadcastTXT(jsonData);
}

void init_WiFi()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(Wifi_connected,WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(Get_IPAddress, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.begin(ssid, password);
  Serial.println("Waiting for WIFI network...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  WiFi.onEvent(Wifi_disconnected,WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void setup()
{
  Serial.begin(115200);

  // WiFi.softAP(ssid, password);

  // IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  init_WiFi();

  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  server.serveStatic("/", SPIFFS, "/");
  Serial.println("SPIFFS mounted successfully");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");           
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });

  server.serveStatic("/", SPIFFS, "/");

  webSocket.begin();
  server.begin();

  SerialBT.begin("E90", true);
  SerialBT.setPin("1234");

  connectBluetooth();

  elm327.begin(SerialBT, false, 2000);

  // if (!SerialBT.connect("OBDII"))
  // {
  //   Serial.println("Couldn't connect to OBD scanner");
  //   while(1);
  // }
}

StaticJsonDocument<200> json_data;
 String jsonString;
bool requestSuccessfull = false;
void loop()
{
  webSocket.loop();

  for (int i = 0; i < SIZE(engine_data); i++) {
    while(!requestSuccessfull) {
      float tmp = engine_data[i].call_function(elm327);

      if (elm327.nb_rx_state == ELM_SUCCESS)
      {
        if(engine_data[i].old_value != tmp) {
          engine_data[i].value = tmp;
          json_data[engine_data[i].param_name.c_str()] = engine_data[i].value;
          serializeJson(json_data, jsonString);
          sendJson(JSON.stringify(jsonString));
          json_data.clear();
          jsonString = "";
          delay(10);
          requestSuccessfull = true;
        }
      } else if (elm327.nb_rx_state != ELM_GETTING_MSG) {
        elm327.printError();
      }
    }    
    requestSuccessfull = false;
  }
}