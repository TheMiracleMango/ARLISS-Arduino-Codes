#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Servo.h>
 
/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

Servo servoL;
Servo servoR;

const int DATAPOINTS = 8;

const float activeZone = 2;

const float kP = 0.5;
const float kI = 0.2;
const float kD = 0.1;

const float integralActiveZone = 1;

const float target = 0;

float errorR_data[DATAPOINTS] = {0.0f};
float errorL_data[DATAPOINTS] = {0.0f};

float errorR;
float errorL;
float errorR_total;
float errorL_total;
float lastErrorR;
float lastErrorL;
float proportionR;
float proportionL;
float integralR;
float integralL;
float derivativeR;
float derivativeL;

float systemR;
float systemL;

int i = 0;
 
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

  servoR.attach(10);
  servoL.attach(9);
  servoR.write(90);
  servoL.write(90);
  delay(1000);
}
 
void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
  
  float Pi = 3.14159;
  
  // Calculate the angle of the vector y,x
  float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / Pi;
  
  Serial.print("Compass Heading: ");
  Serial.println(heading);
  Serial.println();






  //PID system for Servo

  //Take certain amount of data points, then average them out to smooth out the signal
  if (i < DATAPOINTS) {
    errorL = heading - target;
    errorR = target - heading;

    errorR_data[i] = errorR;
    errorL_data[i] = errorL;

    i++;
  }
  else {
    float sumR = 0;
    float sumL = 0;
    for (int j = 0; j < DATAPOINTS; j++){
      sumR += errorR_data[j];
      sumL += errorL_data[j];
    }
    errorR = sumR/DATAPOINTS;
    errorL = sumL/DATAPOINTS;

    i = 0;
  }

  //Prevent servo twitching when error is too small
  if (errorR < activeZone) errorR = 0;
  if (errorL < activeZone) errorL = 0;

  //Accumulate the error over a period of time (find area under the graph)
  if (errorR < integralActiveZone && errorR != 0) errorR_total += errorR;
  else errorR_total = 0;
  if (errorL < integralActiveZone && errorL != 0) errorL_total += errorL;
  else errorL_total = 0;

  //Prevent integral value from getting too big
  if (errorR_total > 50/kI) errorR_total = 50/kI;
  if (errorL_total > 50/kI) errorL_total = 50/kI;

  //Allow derivative value to adjust to zero quicker
  if (errorR == 0) derivativeR = 0;
  if (errorL == 0) derivativeL = 0;

  //Calculate PID values
  proportionR = errorR * kP;
  proportionL = errorL * kP;

  integralR = errorR_total * kI;
  integralL = errorL_total * kI;

  derivativeR = (errorR - lastErrorR) * kD;
  derivativeL = (errorL - lastErrorL) * kD;

  //Summing up all PID values
  systemR = proportionR + integralR + derivativeR;
  systemL = proportionL + integralL + derivativeL;

  //Prevent servos from rotating more than 90 degrees
  if (systemR < 0) systemR = 0;
  else if (systemR > 90) systemR = 90;
  if (systemL < 0) systemL = 0;
  else if (systemL > 90) systemL = 90;

  //Set servo power
  servoR.write(90 + systemR); //goes from 90 -> 180
  servoL.write(90 - systemL); //goes from 90 -> 0

  //Use to find derivative value
  lastErrorR = errorR;
  lastErrorL = errorL;
  
  Serial.println(systemR);
  Serial.println(systemL);
  
  delay(20);
}
