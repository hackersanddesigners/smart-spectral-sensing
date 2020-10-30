
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include <SparkFunLSM9DS1.h>

#define USE_PROTECTED_MEMBERS

#ifndef ST7735_SLPIN
#define ST7735_SLPIN    0x10
#define ST7735_DISPOFF  0x28
#endif


#define I2C_SDA_PIN         21    // communication pin for i2c
#define I2C_SCL_PIN         22    // communication pin for i2c

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
LSM9DS1 imu; // initiate sensor: create an LSM9DS1 object to use from here on.

char buff[256];     //create an array with 266 spots

uint8_t func_select = 0;
uint32_t colour = 0;

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

  // Turn on the IMU with configureIMU() (defined above)
  // check the return status of imu.begin() to make sure
  // it's connected.
  uint16_t status = setupIMU();
  if (status == false) {
    Serial.print("Failed to connect to IMU: 0x");
    Serial.println(status, HEX);
    while (1) ;
  }


  // After turning the IMU on, configure the interrupts:
  // configureLSM9DS1Interrupts();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
  
  // Lower MCU frequency can effectively reduce current consumption and heat
  setCpuFrequencyMhz(80);
}

void getIMU()
{

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

}

void IMU_Show() {

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
  delay(50);
}

void loop() {
  IMU_Show_Ball();        // ball that moves with the accellerometer values
}
