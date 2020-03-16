#include <Wire.h>
#include <SPI.h>
#include "SPIFFS.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP3XX.h"
#include "I2Cdev.h"
#include "MPU6050.h"
 
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 5
#define EEPROM_SIZE 64
 
#define SEALEVELPRESSURE_HPA (1013.25)

 //define the BMP388 (the altimeter) and the MPU6050 (the accelerometer), with the MPU set to AD0 high
Adafruit_BMP3XX bmp(BMP_CS); 
MPU6050 accelgyro(0x69); 

//accelerometer outputs
int16_t ax, ay, az;
float a_x, a_y, a_z;

float initAltitude;
float currentAltitude;
float maxAltitude;
float temp; //in degrees Celsius
float metersPerFeet = 0.3048;

unsigned long initTime;
unsigned long currTime;

void setup() {
  Serial.begin(115200);
    
  Wire.begin(21,22);
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }
  //initializes the SPI file storage system
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  //set bmp388 (altimeter) oversampling and data rate
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);
  bmp.setOutputDataRate(BMP3_ODR_200_HZ);

  //take initial altitude reading
  bmp.performReading();
  initAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  maxAltitude = -1000.00;

  //set the accelerometer range to +/- 16g
  accelgyro.setFullScaleAccelRange(3);

  //create the file for flight1
  //change the number of the flight (format: flightx.txt)
  File flight1 = SPIFFS.open("/flight1.txt", FILE_WRITE);
  if(!flight1) {
    Serial.println("Error opening /flight1.txt");  
  }
  
  //writes the header, with columns for each of the measured values
  if (flight1.println("Flight 1 Data: Time : Altitude : Temp : Accel X : Accel Y : Accel Z")) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
  flight1.close();
}
 
void loop() {
  //open the flight1 file to append data
  File flight1 = SPIFFS.open("/flight1.txt", FILE_APPEND);
  if(!flight1){
      Serial.println("Failed to open file for reading");
      return;
  }
  
  //take readings for temperature, altitude, and acceleration <x,y,z>
  bmp.performReading();
  accelgyro.getAcceleration(&ax, &ay, &az);
  temp = bmp.temperature;
  currentAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA)/metersPerFeet; //altitude in feet

  if(currentAltitude >= maxAltitude) maxAltitude = currentAltitude; //find cumulative max altitude

  //scale the raw acceleration output to represent actual g values for acceleration
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

  //print all values to file
  flight1.print(currTime-initTime); //TODO: timer to be implemented
  flight1.print(" : ");
  flight1.print(currentAltitude);
  flight1.print(" : ");
  flight1.print(a_x);
  flight1.print(" : ");
  flight1.print(a_y);
  flight1.print(" : ");
  flight1.print(a_z);
  flight1.println();

  flight1.close();
  delay(5);
}
