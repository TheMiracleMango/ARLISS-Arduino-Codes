#include <Adafruit_GPS.h> //Install Adafruit GPS library
#include <SoftwareSerial.h> //Install Software Serial library

SoftwareSerial mySerial (3,2); //Initialize Solfware Serial Port
Adafruit_GPS GPS (&mySerial); //Create GPS object

String NMEA1; //Variable for first NMEA sentence
String NMEA2; //Variable for second NMEA sentence
char c; //To read characters coming from the GPS

void setup ()
{
  Serial.begin(9600); //Turn on serial monitor
  GPS.begin(9600); //Turn on GPS at 9600 baud
  GPS.sendCommand("$PGCMD,33,0*6D"); //Turn off antenna update data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); //Set up update rate to 1Hz
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //GGA only
  delay(1000);
}

void loop()
{
  readGPS();
  delay(12); //Experiment with different delays to get good orderly data
}

void readGPS()
{
  clearGPS();

  while (!GPS.newNMEAreceived()) //Loop until you have a good NMEA sentence
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  NMEA1 = GPS.lastNMEA();

  while (!GPS.newNMEAreceived()) //Loop until you have a good NMEA sentence
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  NMEA2 = GPS.lastNMEA();

  Serial.println(NMEA1);
  Serial.println(NMEA2);
}

void clearGPS() //Clear old and corrupted data from Serial Port
{
  while (!GPS.newNMEAreceived()) //Loop until you have a good NMEA sentence
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence

  while (!GPS.newNMEAreceived()) //Loop until you have a good NMEA sentence
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence

  while (!GPS.newNMEAreceived()) //Loop until you have a good NMEA sentence
  {
    c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
}
