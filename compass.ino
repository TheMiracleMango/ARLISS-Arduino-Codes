#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
 
/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);
 
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}
 
void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
  
  float Pi = 3.14159;
  
  // Calculate the angle of the vector y,x
  float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / Pi;
  
  // Normalize to 0-360
  if (heading < 0)
  {
    heading = 360 + heading;
  }


  float ytilt = (atan2(event.magnetic.z,event.magnetic.y) * 180) / Pi;  //uses arc tan of the y and z compass readings
  float xtilt = (atan2(event.magnetic.z,event.magnetic.x) * 180) / Pi;  //uses arc tan of the x and z compass readings 
  
  if (ytilt < 0)
  {
    ytilt = 360 + ytilt;
  }
  if (xtilt < 0)
  {
    xtilt = 360 + xtilt;
  }
  xtilt = 241 - xtilt;
  ytilt = 235 - ytilt;
  
  Serial.print("Magnetic x: ");
  Serial.println(event.magnetic.x);
  Serial.print("Magnetic y: ");
  Serial.println(event.magnetic.y);
  
  Serial.print("Compass Heading: ");
  Serial.println(heading);
  Serial.print("Y_Tilt: ");
  Serial.println(ytilt);
  Serial.print("X_Tilt: ");
  Serial.println(xtilt);
  Serial.println();
  delay(500);
}
