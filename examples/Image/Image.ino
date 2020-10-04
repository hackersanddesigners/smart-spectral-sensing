
#include <pcf8563.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include "sensor.h"
#include "esp_adc_cal.h"
#include "image.h"
#include "charge.h"


#ifndef ST7735_SLPIN
#define ST7735_SLPIN    0x10
#define ST7735_DISPOFF  0x28
#endif

#define TP_PIN_PIN          33
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22
#define RTC_INT_PIN         34
#define BATT_ADC_PIN        35
#define TP_PWR_PIN          25
#define LED_PIN             4
#define CHARGE_PIN          32
#define INT1_PIN_THS        38
#define INT2_PIN_DRDY       39
#define INTM_PIN_THS        37
#define RDYM_PIN            36

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
PCF8563_Class rtc;

char buff[256];
bool rtcIrq = false;
bool initial = 1;
bool otaStart = false;

uint8_t func_select = 0;
uint8_t omm = 99;
uint8_t xcolon = 0;
uint32_t targetTime = 0;       // for next 1 second timeout
uint32_t colour = 0;
int vref = 1100;

bool pressed = false;

bool charge_indication = false;
bool charge_show = false;
uint8_t hh, mm, ss ;

#define SENSOR_SERVICE_UUID                 "4fafc301-1fb5-459e-8fcc-c5c9c331914b"
#define SENSOR_CHARACTERISTIC_UUID          "beb5483c-36e1-4688-b7f5-ea07361b26a8"

void scanI2Cdevice(void)
{
  uint8_t err, addr;
  int nDevices = 0;
  for (addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("I2C device found at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.print(addr, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (err == 4) {
      Serial.print("Unknow error at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Done\n");
}


void setupADC()
{
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  } else {
    Serial.println("Default Vref: 1100mV");
  }
}

void setupRTC()
{
  rtc.begin(Wire);
  //Check if the RTC clock matches, if not, use compile time
  rtc.check();

  RTC_Date datetime = rtc.getDateTime();
  hh = datetime.hour;
  mm = datetime.minute;
  ss = datetime.second;
}

void setup(void)
{
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0,  160, 80, image1);    //show image
  delay(2000);
  /*
    Reduce the brightness,
    can significantly reduce the current consumption
    when bright screen
    If you need to uncomment the comment
  */
  // ledcSetup(0, 1000, 8);
  // ledcAttachPin(TFT_BL, 0);
  // ledcWrite(0, 10);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);

  setupRTC();

  // Corrected ADC reference voltage
  setupADC();


  tft.fillScreen(TFT_TRANSPARENT);

  tft.setTextColor(TFT_YELLOW, TFT_TRANSPARENT); // Note: the new fonts do not draw the background colour

  targetTime = millis() + 1000;

  pinMode(TP_PIN_PIN, INPUT);
  //! Must be set to pull-up output mode in order to wake up in deep sleep mode
  pinMode(TP_PWR_PIN, PULLUP);
  digitalWrite(TP_PWR_PIN, HIGH);

  // Set the indicator to output
  pinMode(LED_PIN, OUTPUT);

  // Charging instructions, it is connected to IO32,
  // when it changes, you need to change the flag to know whether charging is in progress
  pinMode(CHARGE_PIN, INPUT_PULLUP);
  attachInterrupt(CHARGE_PIN, [] {
    charge_indication = true;
  }, CHANGE);

  // Check the charging instructions, if he is low, if it is true, then it is charging
  if (digitalRead(CHARGE_PIN) == LOW) {
    charge_indication = true;
  }

  // Lower MCU frequency can effectively reduce current consumption and heat
  setCpuFrequencyMhz(80);
}

String getVoltage()
{
  uint16_t v = analogRead(BATT_ADC_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  return String(battery_voltage) + "V";
}

void RTC_Show()       //Function to show Real Time Clock
{
  if (targetTime < millis()) {
    RTC_Date datetime = rtc.getDateTime();
    hh = datetime.hour;
    mm = datetime.minute;
    ss = datetime.second;
    targetTime = millis() + 1000;
    if (ss == 0 || initial) {
      initial = 0;
      tft.setTextColor(TFT_GREEN, TFT_TRANSPARENT);
      tft.setCursor (8, 60);
      tft.print(__DATE__); // This uses the standard ADAFruit small font
    }

    tft.setTextColor(TFT_BLUE, TFT_TRANSPARENT);
    tft.drawCentreString(getVoltage(), 120, 60, 1); // Next size up font 2


    // Update digital time
    uint8_t xpos = 6;
    uint8_t ypos = 0;
    if (omm != mm) { // Only redraw every minute to minimise flicker
      // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
      tft.setTextColor(0x39C4, TFT_TRANSPARENT);  // Leave a 7 segment ghost image, comment out next line!
      tft.setTextColor(TFT_TRANSPARENT, TFT_TRANSPARENT); // Set font colour to black to wipe image
      // Font 7 is to show a pseudo 7 segment display.
      // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
      tft.drawString("88:88", xpos, ypos, 7); // Overwrite the text to clear it
      tft.setTextColor(0xFBE0, TFT_TRANSPARENT); // Orange
      omm = mm;

      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 7);
      xpos += tft.drawNumber(hh, xpos, ypos, 7);
      xcolon = xpos;
      xpos += tft.drawChar(':', xpos, ypos, 7);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 7);
      tft.drawNumber(mm, xpos, ypos, 7);
    }

    if (ss % 2) { // Flash the colon
      tft.setTextColor(0x39C4, TFT_TRANSPARENT);
      xpos += tft.drawChar(':', xcolon, ypos, 7);
      tft.setTextColor(0xFBE0, TFT_TRANSPARENT);
    } else {
      tft.drawChar(':', xcolon, ypos, 7);
    }
  }

  if (charge_indication) {
    charge_indication = false;
    if (digitalRead(CHARGE_PIN) == LOW) {
      tft.pushImage(140, 55, 16, 16, charge);   //show pink thunderbolt charge icon
    } else {
      tft.fillRect(140, 55, 16, 16, TFT_TRANSPARENT);
    }
  }
}




void loop()
{

  if (digitalRead(TP_PIN_PIN) == HIGH) {
    if (!pressed) {
      initial = 1;
      targetTime = millis() + 1000;
      //            tft.fillScreen(TFT_TRANSPARENT);
      omm = 99;

      func_select = func_select + 1 > 2 ? 0 : func_select + 1;

      pressed = true;
    } else {
    }
  } else {
    pressed = false;
  }

  switch (func_select) {
    case 0:
      RTC_Show();
      break;
    case 1:
      tft.pushImage(0, 0,  160, 80, image1);
      break;
    case 2:
      tft.setTextColor(TFT_GREEN, TFT_TRANSPARENT);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Press again to wake up",  tft.width() / 2, tft.height() / 2 );
      Serial.println("Go to Sleep");
      delay(3000);
      tft.writecommand(ST7735_SLPIN);
      tft.writecommand(ST7735_DISPOFF);
      esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ANY_HIGH);
      delay(200);
      esp_deep_sleep_start();
      break;
    default:
      break;
  }
}
