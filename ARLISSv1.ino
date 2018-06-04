#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library form the adafruit site above
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
#include <Wire.h>
#include <MagneticSensorLsm303.h> //Load custom library for compass with tilt compensation implemented
#include <Servo.h>

SoftwareSerial mySerial(3, 2); //Initialize SoftwareSerial, and tell it you will be connecting through pins 2 and 3
Adafruit_GPS GPS(&mySerial); //Create GPS object
MagneticSensorLsm303 compass; //Create compass object
Servo servoL;
Servo servoR;

//GPS Variables-------------------------------------------------

String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence

char latitude_char[20] = {'\0'};
char longitude_char[20] = {'\0'};
char c;
int N_degrees;
int E_degrees;
float N_minutes; 
float E_minutes;
float N_seconds;
float E_seconds;

//PID variables--------------------------------------------------

const int DATAPOINTS = 8;

const float activeZone = 5;

const float kP = 0.5;
const float kI = 0.1;
const float kD = 0.2;

const float integralActiveZone = 1;

const float N_target_seconds = 142365.233;
const float E_target_seconds = 431349.739;
float target;

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

float lastSystemR;
float lastSystemL;

int i = 0; //Counts how many data points the compass gathered.

const float Pi = 3.14159;

//---------------------------------------------------

void setup()  
{
  Serial.begin(9600);  //Turn on the Serial Monitor

  //Set up GPS
  GPS.begin(9600);       //Turn GPS on at baud rate of 9600
  GPS.sendCommand("$PGCMD,33,0*6D"); // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate

  //Set up compass
  compass.init();
  compass.enable();
  
  servoR.attach(10);
  servoL.attach(9);
  servoR.write(90);
  servoL.write(90);
  
  delay(1000);  //Pause
}

void loop(){  //This function will read and remember two NMEA sentences from GPS

  //------{Read GPS}------

  //clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out

  
  while(!GPS.newNMEAreceived()) { //Keep reading characters in this loop until a good NMEA sentence is received
    c=GPS.read(); //read a character from the GPS
  }
  GPS.parse(GPS.lastNMEA());  //Once you get a good NMEA, parse it
  NMEA1=GPS.lastNMEA();      //Once parsed, save NMEA sentence into NMEA1
  while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA2=GPS.lastNMEA();
  
  //Serial.println(NMEA1); //We don't have to print the NMEA sentences out, but we can if we want to
  //Serial.println(NMEA2);

  //NMEA sentences must be read prior to getting other datas, because the GPS depends on the NMEA sentences to extract meaningful data
  dtostrf(GPS.latitude, 6, 4, latitude_char); //Converting Floats data into Char Array
  dtostrf(GPS.longitude, 6, 4, longitude_char);

  strConverter(latitude_char); //Add space between degree and minutes
  strConverter(longitude_char);

  IntAndFloat(latitude_char, &N_degrees, &N_minutes);
  IntAndFloat(longitude_char, &E_degrees, &E_minutes);

  N_seconds = lower(N_minutes + lower(N_degrees));
  E_seconds = lower(E_minutes + lower(E_degrees));


  //------{Read Compass and find the mean}------

  compass.read();
  float heading = compass.getNavigationAngle();
  Serial.print("Navigation Angle:  ");
  if(heading > 180) heading -= 360;

  target = (atan2(N_target_seconds - N_seconds, E_target_seconds - E_seconds) * 180) / Pi;

  if (i < DATAPOINTS) {
    errorL = heading - target;
    errorR = target - heading;

    errorR_data[i] = errorR;
    errorL_data[i] = errorL;

    i++;
  }

  //------{PID}-------

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

  }

  //If the changes in system value is too small, the arduino will not change the system value (to prevent twitching)
  if ((lastSystemR - systemR) > (0 - activeZone) && (lastSystemR - systemR) < activeZone) systemR = lastSystemR;
  if ((lastSystemL - systemL) > (0 - activeZone) && (lastSystemL - systemL) < activeZone) systemL = lastSystemL;

  //Set servo power
  servoR.write(90 + systemR); //goes from 90 -> 180
  servoL.write(90 - systemL); //goes from 90 -> 0

  //Use to find derivative value
  lastErrorR = errorR;
  lastErrorL = errorL;
 
  //Use to take derivative of the system values
  lastSystemR = systemR;
  lastSystemL = systemL;

  //-------{Print information to monitor}-------

  Serial.println("");

  //Latitude Print
  Serial.print("Latitude: ");
  Serial.print(latitude_char);
  Serial.print(GPS.lat); // Print N for Latitude
  Serial.println("");
  
  Serial.print("Latitude Converted: ");
  Serial.print(N_degrees);
  Serial.print(" ");
  Serial.print(N_minutes);
  Serial.println("");
  
  Serial.print("Latitude in Seconds: ");
  Serial.println(N_seconds);
  Serial.println("");

  //Longitude Print
  Serial.print("Longitude: ");
  Serial.print(longitude_char);
  Serial.print(GPS.lon); // Print W for Longitude
  Serial.println("");

  Serial.print("Longitude Converted: ");
  Serial.print(E_degrees);
  Serial.print(" ");
  Serial.print(E_minutes);
  Serial.println("");
  
  Serial.print("Longitude in Seconds: ");
  Serial.println(E_seconds);
  Serial.println("");

  //Altitude Print
  Serial.print("Altitude(m): ");
  Serial.println(GPS.altitude);
  Serial.println("");

  //Others
  Serial.print("Angle: ");
  Serial.println(GPS.angle);
  //Serial.print("Speed: ");
  //Serial.println(GPS.speed);

  //PID
  Serial.print("Target Heading: ");
  Serial.println(target);
  Serial.print("Current Heading: ");
  Serial.println(heading,3);
  Serial.print("Right Servo's Value: ");
  Serial.println(systemR);
  Serial.print("Left Servo's Value: ");
  Serial.println(systemL);
  Serial.println("");

  Serial.println("------------------------------");

  //delay(10/DATAPOINTS);

  delay(1000);
}

void clearGPS() {  //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
  
  while(!GPS.newNMEAreceived()) {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  
  while(!GPS.newNMEAreceived()) {
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); 
}

void strConverter(char str[]){ //Convert GPS data into something Google Earth can read
  
  int strIndex = 0;
  int i;

  while (str[strIndex] != '\0') strIndex++; //Find the length of string
  
  while (str[strIndex - 1] != '.'){
    str[strIndex] = str[strIndex - 1];
    strIndex--;
  }
  for (i = 0; i < 3; i++){
    str[strIndex] = str[strIndex - 1];
    strIndex--;
  }
  str[strIndex] = ' ';
}


void IntAndFloat(char str[], int * degree, float * minutes)
{ 
  String deg;
  String mins;
  int i = 0;
  
  while(str[i] != ' ' && str[i] != '\0')
  {
    deg = deg + str[i++];
  }
  *degree = deg.toInt(); //toInt() function can only return a float precision 2

  i++;
  while(str[i] != '\0' && str[i] != 'N' && str[i] != 'E')
  {
    mins = mins + str[i++];
  }  
  *minutes = mins.toFloat();
}

float lower(float number){
  return number * 60.0f;
}
