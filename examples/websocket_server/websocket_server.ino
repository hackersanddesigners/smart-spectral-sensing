// timer/scheduler
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
#include <TaskSchedulerSleepMethods.h>

// networking
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
// Json
#include "AsyncJson.h" // install ArduinoJson via the library manager
#include "ArduinoJson.h"
// display
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
// accelerometer
#include <Wire.h>
#include <SparkFunLSM9DS1.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
LSM9DS1 imu;

// Pins for Wire
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
// Pins for IMU
#define INT1_PIN_THS 38
#define INT2_PIN_DRDY 39
#define INTM_PIN_THS 37
#define RDYM_PIN 36

char buff[256];

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

//Tasks
void updateIMU();
Task t1(100, TASK_FOREVER, &updateIMU); // timer 1, runs every 100ms forever
Scheduler runner;                       // timer scheduler

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len)
    {
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < info->len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < info->len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    }
    else
    {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
        if (info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

      if (info->opcode == WS_TEXT)
      {
        for (size_t i = 0; i < len; i++)
        {
          msg += (char)data[i];
        }
      }
      else
      {
        char buff[3];
        for (size_t i = 0; i < len; i++)
        {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if (info->final)
        {
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

const char *ssid = "your_ssid";
const char *password = "your_pass";
const char *hostName = "spectral";
const char *http_username = "admin";
const char *http_password = "admin";

DynamicJsonDocument doc(1024);

void setup()
{

  //  SPIFFS.format();
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Started");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREENYELLOW);
  tft.setTextSize(1);

  runner.init();
  runner.addTask(t1); // this task is our main loop where we check and broadcast the accelerometer data

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);

  uint16_t status = setupIMU();
  if (status == false)
  {
    Serial.print("Failed to connect to IMU: 0x");
    Serial.println(status, HEX);
    while (1)
      ;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(hostName);
  WiFi.begin(ssid, password);
  //  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //    Serial.printf("STA: Failed!\n");
  //    WiFi.disconnect(false);
  //    delay(1000);
  //    WiFi.begin(ssid, password);
  //  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    tft.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.println("Server running: ");
  tft.setTextSize(2);
  tft.setCursor(0, 15);
  tft.println(WiFi.localIP());

  MDNS.addService("http", "tcp", 80);

  SPIFFS.begin();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength())
    {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++)
    {
      AsyncWebHeader *h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++)
    {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isFile())
      {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      }
      else if (p->isPost())
      {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
      else
      {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char *)data);
    if (final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char *)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
  server.begin();
  t1.enable();
}

unsigned long last = 0;
bool sent = false;

void loop()
{
  ws.cleanupClients();
  runner.execute();
  if (millis() > 10000 && !sent)
  {
    //    ws.textAll("hello again");
    Serial.println("echo 2");
    sent = true;
  }
}

void updateIMU()
{

  getIMU();

  const size_t capacity = 4 * JSON_OBJECT_SIZE(3) + 20;
  DynamicJsonDocument doc(capacity);

  doc["acc"]["x"] = imu.calcAccel(imu.gx);
  doc["acc"]["y"] = imu.calcAccel(imu.gy);
  doc["acc"]["z"] = imu.calcAccel(imu.gz);
  doc["gyr"]["x"] = imu.calcGyro(imu.gx);
  doc["gyr"]["y"] = imu.calcGyro(imu.gy);
  doc["gyr"]["z"] = imu.calcGyro(imu.gz);
  doc["mag"]["x"] = imu.calcMag(imu.gx);
  doc["mag"]["y"] = imu.calcMag(imu.gy);
  doc["mag"]["z"] = imu.calcMag(imu.gz);

  String output;
  serializeJson(doc, output);

  ws.textAll(output); // websocket textAll broadcasts to all connected clients
}

uint16_t setupIMU()
{
  // Set up our Arduino pins connected to interrupts.
  // We configured all of these interrupts in the LSM9DS1
  // to be active-low.
  pinMode(INT2_PIN_DRDY, INPUT);
  pinMode(INT1_PIN_THS, INPUT);
  pinMode(INTM_PIN_THS, INPUT);

  // The magnetometer DRDY pin (RDY) is not configurable.
  // It is active high and always turned on.
  pinMode(RDYM_PIN, INPUT);

  // gyro.latchInterrupt controls the latching of the
  // gyro and accelerometer interrupts (INT1 and INT2).
  // false = no latching
  imu.settings.gyro.latchInterrupt = false;
  // Set gyroscope scale to +/-245 dps:
  imu.settings.gyro.scale = 245;
  // Set gyroscope (and accel) sample rate to 14.9 Hz
  imu.settings.gyro.sampleRate = 1;
  // Set accelerometer scale to +/-2g
  imu.settings.accel.scale = 2;
  // Set magnetometer scale to +/- 4g
  imu.settings.mag.scale = 4;
  // Set magnetometer sample rate to 0.625 Hz
  imu.settings.mag.sampleRate = 0;

  // Call imu.begin() to initialize the sensor and instill
  // it with our new settings.
  bool r = imu.begin(LSM9DS1_AG_ADDR(1), LSM9DS1_M_ADDR(1), Wire); // set addresses and wire port
  if (r)
  {
    imu.sleepGyro(false);
    return true;
  }
  return false;
}

void getIMU()
{

  // Update the sensor values whenever new data is available
  if (imu.gyroAvailable())
  {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();
  }
  else
  {
    Serial.println("Invalid gyroscope");
  }
  if (imu.accelAvailable())
  {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();
  }
  else
  {
    Serial.println("Invalid accelerometer");
  }
  if (imu.magAvailable())
  {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  }
  else
  {
    Serial.println("Invalid magnetometer");
  }
}

/*
  void IMU_Show()
  {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);

    getIMU();

    snprintf(buff, sizeof(buff), "--  ACC  GYR   MAG");
    tft.drawString(buff, 0, 0);
    snprintf(buff, sizeof(buff), "x %.2f  %.2f  %.2f", imu.calcAccel(imu.ax), imu.calcGyro(imu.gx), imu.calcMag(imu.mx));
    tft.drawString(buff, 0, 16);
    snprintf(buff, sizeof(buff), "y %.2f  %.2f  %.2f", imu.calcAccel(imu.ay), imu.calcGyro(imu.gy), imu.calcMag(imu.my));
    tft.drawString(buff, 0, 32);
    snprintf(buff, sizeof(buff), "z %.2f  %.2f  %.2f", imu.calcAccel(imu.az), imu.calcGyro(imu.gz), imu.calcMag(imu.mz));
    tft.drawString(buff, 0, 48);
    delay(200);
  } */
