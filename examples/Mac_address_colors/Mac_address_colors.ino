/*
     Based this project: https://www.hackster.io/ferrithemaker/covid-19-personal-cumulative-risk-dosimeter-624019

     This sketch makes the ESP32 scan for wifi devices and creates a list of nearby devices.
     Each Wifi device has a unique number called the MAC address. This number is formatted like:
     XX.XX.XX.XX.XX.XX where each X is a hexadecimal number.
     We chop the MAC address into two pieces and use that as color representing the device.
     So for example ff:0f:24:2e:6a:c9 = #ff0f24 and #2E6AC9.
     The wristband displays all the wifi devices it was in contact with recently as gradients
     of their MAC address colors.
     To preserve the battery the devices sleeps 10 seconds after showing the colors.
     It may take a few cycles for devices to show up.
*/

#include <Arduino.h>

#include "WiFi.h"
#include "esp_wifi.h"
#include <HTTPClient.h>
#include <vector>
#include "string.h"
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library
// VERY IMPORTANT!!!
// You need to modify the library file User_Setup_Select.h
// comment the line #include <User_Setup.h>
// uncomment the line #include <User_Setups/Setup26_TTGO_T_Wristband.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

class MACPool {
  private:
    String mac;
    int signal;
    unsigned long time;
    bool newMAC;


  public:
    MACPool(String, int, unsigned long, bool);
    String getMAC();
    int getSignal();
    unsigned long getTime();
    void updateTime(unsigned long);
    bool getNewMAC();
    void updateNewMAC(bool);
};

MACPool::MACPool(String mac, int signal, unsigned long time, bool newMAC)
{
  this->mac = mac;
  this->signal = signal;
  this->time = time;
  this->newMAC = newMAC;
}

String MACPool::getMAC() {
  return this->mac;
}

int MACPool::getSignal() {
  return this->signal;
}

unsigned long MACPool::getTime() {
  return this->time;
}

void MACPool::updateTime(unsigned long time) {
  this->time = time;
}

void MACPool::updateNewMAC(bool nm) {
  this->newMAC = nm;
}

bool MACPool::getNewMAC() {
  return this->newMAC;
}


using namespace std;

unsigned int channel;

int SleepSecs = 10; // timer to wake up

float weight = 1.0; // weigh the index with real time data


const wifi_promiscuous_filter_t filt = {
  .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA
};


vector<MACPool> listOfMAC;

typedef struct {
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct {
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;


/*
    Convert the mac address to a color.
    So, for example, the Mac BE:30:D9:4A:EE:FF
    would turn into #BE30D9 and #4AEEFF.

    String mac - mac address to convert
    boolean getSecond - if true it returns the second color.
*/

uint32_t MACToColor( String mac, boolean getSecond ) {
  int r, g, b, r2, g2, b2;
  const char* mac_char = mac.c_str();
  sscanf (mac_char, "%x:%x:%x:%x:%x:%x", &r, &g, &b, &r2, &g2, &b2);
  if ( getSecond ) return tft.color565( r2, g2, b2 );//return toRGB565Int( r2, g2, b2 );
  return tft.color565( r, g, b ); // toRGB565Int( r, g, b );
}

//uint32_t toRGB565Int(int r, int g, int b) {
//  return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
//}

void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {

  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  int signal = p->rx_ctrl.rssi;

  MacAddr mac_add = (MacAddr)wh->sa;
  String sourceMac;
  for (int i = 0; i < sizeof(mac_add.mac); i++) {
    String macDigit = String(mac_add.mac[i], HEX);
    if (macDigit.length() == 1) {
      macDigit = "0" + macDigit;
    }

    sourceMac += macDigit;
    if (i != sizeof(mac_add.mac) - 1) {
      sourceMac += ":";
    }
  }

  sourceMac.toUpperCase();

  if (signal > -70) { // signal level threshold

    // Prevent duplicates
    for (int i = 0; i < listOfMAC.size(); i++) {
      if (sourceMac == listOfMAC[i].getMAC()) {
        listOfMAC[i].updateTime(millis()); // update the last time MAC found
        listOfMAC[i].updateNewMAC(false);
        return;
      }
    }

    // new MAC

    listOfMAC.push_back(MACPool(sourceMac, signal, millis(), true));

    Serial.println(listOfMAC[listOfMAC.size() - 1].getMAC());

    // purge outdated MACs

    for (auto it = listOfMAC.begin(); it != listOfMAC.end(); ) {
      if (millis() - it->getTime() > 300000) { // remove if older than 5min
        it = listOfMAC.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void setup(void) {
  Serial.begin(9600);
  pinMode(25, PULLUP); // button power
  pinMode(27, OUTPUT); // screen backlight
  digitalWrite(27, LOW); // screen off
  pinMode(33, PULLUP); // button

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x000000);

  esp_sleep_enable_timer_wakeup(SleepSecs * 1000000);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)33, 1); //1 = Low to High, 0 = High to Low. Pin pulled HIGH
}

void loop() {
  Serial.println("System sleeps");
  delay(100);
  esp_light_sleep_start();
  Serial.println("System awakes");

  // scan for wifi networks
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  //Serial.println("Enable WiFi");
  // set WiFi in promiscuous mode
  esp_wifi_set_mode(WIFI_MODE_STA);            // Promiscuous works only with station mode
  //  esp_wifi_set_mode(WIFI_MODE_NULL);
  // power save options
  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_start();
  esp_wifi_set_max_tx_power(-4);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);   // Set up promiscuous callback
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  for (int loops = 0; loops < 10; loops++) {
    for (channel = 0; channel < 12; channel++) {
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
      delay(100);
    }
    delay(100);
  }

  Serial.println("Turn on screen!");
  digitalWrite(27, HIGH);
  tft.fillScreen(0x000000);

  if ( listOfMAC.size() > 0 ) {

    for ( int i = 0; i < listOfMAC.size(); i++ ) {
      String mac = listOfMAC[ i ].getMAC();
      uint32_t color1 = MACToColor( mac, false );
      uint32_t color2 = MACToColor( mac, true );

      // fill screen with two rects in the colors
      //      tft.fillRect( 0, 0, tft.width() / 2, tft.height(), color1 );
      //      tft.fillRect(tft.width() / 2, 0, tft.width() / 2, tft.height(), color2 );

      // fill the screen with a gradient between the two colors
      drawGradient( color1, color2 );
      tft.setCursor(0, 0, 2);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      tft.println(mac);
      delay( 5000 );
    }
  }


  Serial.println("Turn off screen!");
  //Serial.println(millis());
  digitalWrite(27, LOW);
  delay(100);
}

/*
   Tries (but fails sometimes) to draw a gradient
   between the two colors.
*/
void drawGradient( int color1, int color2 ) {
  uint8_t r1, g1, b1, r2, g2, b2;
  r1 = (color1 >> 8) & 0xF8; r1 |= (r1 >> 5); // rgb565 to 8bit values
  g1 = (color1 >> 3) & 0xFC; g1 |= (g1 >> 6);
  b1 = (color1 << 3) & 0xF8; b1 |= (b1 >> 5);

  r2 = (color2 >> 8) & 0xF8; r2 |= (r2 >> 5); // same for second color
  g2 = (color2 >> 3) & 0xFC; g2 |= (g2 >> 6);
  b2 = (color2 << 3) & 0xF8; b2 |= (b2 >> 5);

  int w = tft.width();
  for ( int i = 0; i < w; i++ ) {
    uint8_t r, g, b;
    r = floor( lerp( i, 0, w, r1, r2 ) ); // interpolate between the colors for each of the 160 pixels
    g = floor( lerp( i, 0, w, g1, g2 ) );
    b = floor( lerp( i, 0, w, b1, b2 ) );
    uint16_t colour = tft.color565( r, g, b ); // convert back to rgb565
    tft.drawFastVLine(i, 0, tft.height(), colour);
  }
}

// Linear interpolation of y value given min/max x, min/max y, and x position.
float lerp(float x, float x0, float x1, float y0, float y1) {
  // Clamp x within x0 and x1 bounds.
  x = x > x1 ? x1 : x;
  x = x < x0 ? x0 : x;
  // Calculate linear interpolation of y value.
  return y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
}
