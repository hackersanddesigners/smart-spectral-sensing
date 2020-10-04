
#include <pcf8563.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include "sensor.h"
#include "esp_adc_cal.h"
#include <SparkFunLSM9DS1.h>
#include "image.h"     // include the image tab


// Uncomment this line and the sensor will be used
#define ENABLE_SENSOR
#define USE_PROTECTED_MEMBERS

#ifndef ST7735_SLPIN
#define ST7735_SLPIN    0x10
#define ST7735_DISPOFF  0x28
#endif

#define TP_PIN_PIN          33    //pin of the touch pad
#define TP_PWR_PIN          25    //pin of touch pad power (pullup)
#define I2C_SDA_PIN         21    // communication pin for i2c
#define I2C_SCL_PIN         22    // communication pin for i2c
#define LED_PIN             4     //onboard LED
#define RTC_INT_PIN         34
#define BATT_ADC_PIN        35
#define CHARGE_PIN          32
#define INT1_PIN_THS        38
#define INT2_PIN_DRDY       39
#define INTM_PIN_THS        37
#define RDYM_PIN            36
#define MOTOR_PIN           14

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
LSM9DS1 imu; // initiate sensor: create an LSM9DS1 object to use from here on.

char buff[256];     //create an array with 266 spots
bool initial = 1;

uint8_t func_select = 0;
uint8_t omm = 99;
uint8_t xcolon = 0;
uint32_t targetTime = 0;       // for next 1 second timeout
uint32_t colour = 0;
int vref = 1100;

bool pressed = false;           //variable to store button input (set to low)
uint32_t pressedTime = 0;       // variable to store how long the button is pressed
bool charge_indication = false;   //variable to store whether battery is charging
bool charge_show = false;       // variable to toggle charge indicator between on/off
uint8_t hh, mm, ss ;            // time set in hour, minute, secs

void configureLSM9DS1Interrupts() {   //configure the 3-axis sensor

  imu.configGyroInt(ZHIE_G, false, false);
  imu.configGyroThs(500, Z_AXIS, 10, true);
  imu.configAccelInt(XHIE_XL, false);
  imu.configAccelThs(20, X_AXIS, 1, false);
  imu.configInt(XG_INT1, INT1_IG_G | INT_IG_XL, INT_ACTIVE_LOW, INT_PUSH_PULL);
  imu.configInt(XG_INT2, INT_DRDY_XL | INT_DRDY_G, INT_ACTIVE_LOW, INT_PUSH_PULL);
  imu.configMagInt(XIEN, INT_ACTIVE_LOW, true);
  imu.configMagThs(10000);
}

uint16_t setupIMU() {      //function to set up the 3-axis sensor (Inertial Measurement Sensor)

  pinMode(INT2_PIN_DRDY, INPUT);
  pinMode(INT1_PIN_THS, INPUT);
  pinMode(INTM_PIN_THS, INPUT);
  pinMode(RDYM_PIN, INPUT);

  imu.settings.gyro.latchInterrupt = false;
  imu.settings.gyro.scale = 245;
  imu.settings.gyro.sampleRate = 1;
  imu.settings.accel.scale = 2;
  imu.settings.mag.scale = 4;
  imu.settings.mag.sampleRate = 0;

  // Call imu.begin() to initialize the sensor with given settings
  bool r =  imu.begin(LSM9DS1_AG_ADDR(1), LSM9DS1_M_ADDR(1), Wire);
  if (r) {
    imu.sleepGyro(false);
    return true;
  }
  return false;
}

void scanI2Cdevice(void)  {     //look for connected devices
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

void setupADC()     // ADC is procedure to be able to get analog readings
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

void setup(void)      // the usual setup we know :)
{
  Serial.begin(115200);

  tft.init();             //initialize screen
  tft.setRotation(1);     //set orientation of screen
  tft.setSwapBytes(true);

  //   Reduce the brightness for longer battery life
  ledcSetup(0, 1000, 8);
  ledcAttachPin(TFT_BL, 0);
  ledcWrite(0, 10);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);

#ifdef ENABLE_SENSOR

  // Turn on the IMU with configureIMU() (defined above)
  // check the return status of imu.begin() to make sure
  // it's connected.
  uint16_t status = setupIMU();
  if (status == false) {
    Serial.print("Failed to connect to IMU: 0x");
    Serial.println(status, HEX);
    while (1) ;
  }
#endif

  // After turning the IMU on, configure the interrupts:
  // configureLSM9DS1Interrupts();

  // Corrected ADC reference voltage
  setupADC();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour

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

void getIMU()
{
#ifdef ENABLE_SENSOR
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() ) {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();
  } else {
    Serial.println("Invalid gyroscope");
  }
  if ( imu.accelAvailable() ) {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();
  } else {
    Serial.println("Invalid accelerometer");
  }
  if ( imu.magAvailable() ) {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  } else {
    Serial.println("Invalid magnetometer");
  }
#endif
}

void IMU_Show() {

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
#ifdef ENABLE_SENSOR
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
#else
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println("Sensor is not enable");
  delay(200);
#endif

}

float my_min = 1;
float my_max = 0;

float mx_min = 1;
float mx_max = 0;

void IMU_Show_Ball()            // function where ball is positioned in coordinates of accellerometer sensor vals
{

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK); // uncomment this line to draw
  tft.setTextDatum(TL_DATUM);
#ifdef ENABLE_SENSOR
  getIMU();
  float my = imu.calcMag(imu.my);
  if ( my < my_min ) my_min = my;
  if ( my > my_max ) my_max = my;

  float mx = imu.calcMag(imu.mx);
  if ( mx < mx_min ) mx_min = mx;
  if ( mx > mx_max ) mx_max = mx;

  int x = map( my * 1000, my_min * 1000, my_max * 1000, -70, 70 );
  int y = map( mx * 1000, mx_min * 1000, mx_max * 1000, 35, -35 );

  tft.drawCircle( 80 + x, 40 + y, 4, TFT_RED);

  snprintf(buff, sizeof(buff), "%.2f | %.2f | %.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
  tft.drawString(buff, 0, 0);
  delay(200);
#else
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println("Sensor is not enable");
  delay(200);
#endif

}


void loop()

/*
  Main program:
  three switch cases to loop between by pressing the button
  the last case put the display to sleep
  pressing it again turns it back on
*/


{
  if (digitalRead(TP_PIN_PIN) == HIGH) {    // if it senses that the button is pressed...
    if (!pressed) {
      initial = 1;
      targetTime = millis() + 1000;
      tft.fillScreen(TFT_BLACK);                //    turn on the screen...
      omm = 99;

      digitalWrite(LED_PIN, HIGH);            // turn on the LED for feedback...
      delay(100);
      digitalWrite(LED_PIN, LOW);

      func_select = func_select + 1 > 3 ? 0 : func_select + 1;    // .. and go to the next case

      pressed = true;
      pressedTime = millis();
      if (func_select == 2) {

        tft.pushImage(0, 0,  160, 80, image1);      // display splash image for 5 secs, see image tab
        delay(500);

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 30);
        tft.println("Your function here");
        delay(500);

        tft.pushImage(0, 0,  160, 80, image1);      // display splash image for 5 secs, see image tab
        delay(500);

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 30);
        tft.println("Your function here");
        delay(5000);
      }
    } else {

    }
  } else {
    pressed = false;
  }

  switch (func_select) {      // depending on the case that is active, activate one of the following functions

    case 0:
      IMU_Show_Ball();        // ball that moves with the accellerometer values
      break;

    case 1:
      IMU_Show();             // shows table with all sensor values
      break;

    case 3:                   // or turn everything off and go to sleep

#if defined(ENABLE_SENSOR) && defined(USE_PROTECTED_MEMBERS)
      imu.settings.gyro.lowPowerEnable = true;
      imu.settings.gyro.enableX = false;
      imu.settings.gyro.enableY = false;
      imu.settings.gyro.enableZ = false;
      imu.settings.gyro.enabled = false;

      imu.settings.mag.enabled = false;
      imu.settings.mag.lowPowerEnable = true;
      imu.settings.mag.operatingMode = 11;

      imu.settings.accel.enabled = false;
      imu.settings.accel.enableX = false;
      imu.settings.accel.enableY = false;
      imu.settings.accel.enableZ = false;

      imu.settings.temp.enabled = false;

      imu.initMag();
      imu.initAccel();
      imu.initGyro();
#else
      imu.sleepGyro();
#endif

      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Going to sleep",  tft.width() / 2, tft.height() / 2 );
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
