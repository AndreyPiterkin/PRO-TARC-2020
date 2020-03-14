#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include "I2Cdev.h"
#include "MPU6050.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
 
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 5
 
#define SEALEVELPRESSURE_HPA (1013.25)
#define OUTPUT_READABLE_ACCELGYRO
 
Adafruit_BMP3XX bmp(BMP_CS); 
MPU6050 accelgyro(0x69); 

int16_t ax, ay, az;
float altitude;
float a_x, a_y, a_z;


 
void setup() {
  Serial.begin(115200);
    
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin(21,22);
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
  
  while (!Serial);
  Serial.println("BMP388 test");
 
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  Serial.println("Initializing I2C devices...");
   accelgyro.initialize();

   Serial.println("Testing device connections...");
   Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
}
 
void loop() {
  accelgyro.setFullScaleAccelRange(0);
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  accelgyro.getAcceleration(&ax, &ay, &az);
  
  Serial.print("Temperature = ");
  Serial.print(bmp.temperature);
  Serial.println(" *C");
 
  Serial.print("Pressure = ");
  Serial.print(bmp.pressure / 100.0);
  Serial.println(" hPa");
 
  Serial.print("Approx. Altitude = ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA)/0.3048);
  Serial.println(" ft");
 
  Serial.println();
  if(accelgyro.getFullScaleAccelRange() == 0) {
    a_x = ((float)ax)/16384;
    a_y = ((float)ay)/16384;
    a_z = ((float)az)/16384;
  } else if(accelgyro.getFullScaleAccelRange() == 1) {
    a_x = ((float)ax)/8192;
    a_y = ((float)ay)/8192;
    a_z = ((float)az)/8192;
  } else if(accelgyro.getFullScaleAccelRange() == 2) {
    a_x = ((float)ax)/4096;
    a_y = ((float)ay)/4096;
    a_z = ((float)az)/4096;
  } else if(accelgyro.getFullScaleAccelRange() == 3) {
    a_x = ((float)ax)/2048;
    a_y = ((float)ay)/2048;
    a_z = ((float)az)/2048;
  }
  

   #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print("a/g:\t");
        Serial.print(a_x); Serial.print("\t");
        Serial.print(a_y); Serial.print("\t");
        Serial.print(a_z); Serial.print("\t");
    #endif
  delay(500);
}
