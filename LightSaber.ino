#include "Wire.h"
#include "LSBT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

#include <L3G.h>

L3G gyro;
LSBT bt;
/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

void setup() {
  byte error, address;
  int nDevices = 0;
  Serial.begin(115200);
  Wire.begin();

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
  }


  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
  }
  /* set static */
  /* Disable auto-gain */
  mag.enableAutoRange(false);
  /* Set the magnetic gain static so we can later parse data on the
     receiver side*/
  mag.setMagGain(LSM303_MAGGAIN_4_0);

  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
  }

  gyro.enableDefault();

  Serial.println("Scanning for I2C devices ...");
  for(address = 0x01; address < 0x7f; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if(error != 2){
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found");
  }

  bt.setup();
}

void loop() {
  byte error, address;
  sensors_event_t m_event, event;
  mag.getEvent(&m_event);
  accel.getEvent(&event);
  gyro.read();
  Serial.printf("%f,%f,%f\n",m_event.magnetic.x,
                 m_event.magnetic.y,m_event.magnetic.z);


  /* Check if the sensor is saturating or not 
     if this message appears, we should change the static mag gain 
     TODO: transmit the sensor gain settings with the BTLE data to allow dynamic
     data parsing */
  if ((mag.raw.x >= 2040) | (mag.raw.x <= -2040) |
      (mag.raw.y >= 2040) | (mag.raw.y <= -2040) |
      (mag.raw.z >= 2040) | (mag.raw.z <= -2040) ) {
    Serial.println("saturation");
  }

    //while(1);
  /* send out our movement data */
  bt.publish(mag.raw, accel.raw, gyro.g);
}
