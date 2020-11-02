### Websocket server on your wrist

This is a very rough first version. YMMV.

This example connects to your local wifi and starts a server. Connect your browser (or your phone) to the the IP Address on the display after it connects.<br />
The browser makes a websocket connection to the wristband and the wristband will broadcast the IMU data to all connected clients.<br/>
For now the data get displayed as a graph, but you can use this as a starting point for generating sounds for example.<br/>
The band get super hot and the battery life will be terrible. It can probably be improved a lot, but don't expect too much, you're running a webserver :)

## Libraries

- TaskScheduler - install through the library manager > Sketch - Libraries > Library Manager.
- ArduinoJson - install as above
- ESPAsyncWebServer - Download https://github.com/me-no-dev/ESPAsyncWebServer and move it to your Arduino libraries directory
  I think SPIFFS is part of ESPAsyncWebServer.<br />
  The other libraries should already be installed, otherwise search for them in the library manager.

## WIFI

Change the lines:

    const char *ssid = "your_ssid";
    const char *password = "your_pass";

to represent your network settings.

## Uploading the site

The website we're serving will run of the Flash memory on the ESP32.

To install the plugin follow the instructions here:
https://github.com/me-no-dev/arduino-esp32fs-plugin#installation

After installation you can select Tools > ESP32 Sketch data upload. This will upload the /data directory inside our sketch folder.

## Editing the site.

To make editing easier, you can connect to http://<your_server_up>/edit to edit pages later. Username and pass are both 'admin'.
