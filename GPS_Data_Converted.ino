//Make sure to install the adafruit GPS library from https://github.com/adafruit/Adafruit-GPS-Library
#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library form the adafruit site above
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
SoftwareSerial mySerial(3, 2); //Initialize SoftwareSerial, and tell it you will be connecting through pins 2 and 3
Adafruit_GPS GPS(&mySerial); //Create GPS object
 
String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence
char c;       //Used to read the characters spewing from the GPS module

char latitude_char[20] = {'\0'};
char longitude_char[20] = {'\0'};
 
float AngleDiff(float Lat1, float Lat2, float Long1, float Long2);
void strConverter(char str[]);
void clearGPS();

void setup()  
{
  Serial.begin(115200);  //Turn on the Serial Monitor
  GPS.begin(9600);       //Turn GPS on at baud rate of 9600
  GPS.sendCommand("$PGCMD,33,0*6D"); // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  delay(1000);  //Pause
}

void loop()                     // run over and over again
{
  readGPS();  //This is a function we define below which reads two NMEA sentences from GPS
}

void readGPS(){  //This function will read and remember two NMEA sentences from GPS

  
  clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out

  
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
  
  Serial.println(NMEA1);
  Serial.println(NMEA2);

  dtostrf(GPS.latitude, 6, 4, latitude_char); //Converting Floats data into Char Array
  dtostrf(GPS.longitude, 6, 4, longitude_char);

  strConverter(latitude_char); //Add space between degree and minutes
  strConverter(longitude_char);

  Serial.println("");
  Serial.print("Latitude: ");
  //Serial.print(GPS.latitude,4);
  Serial.print(latitude_char);
  Serial.print(GPS.lat);
  Serial.println("");
  Serial.print("Longitude: ");
  //Serial.print(GPS.longitude,4);
  Serial.print(longitude_char);
  Serial.print(GPS.lon);
  Serial.println("");
  Serial.print("Altitude(m): ");
  Serial.println(GPS.altitude);
  Serial.println("");
  Serial.println("------------------------------");
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

float AngleDiff(float Lat1, float Lat2, float Long1, float Long2)
{
    float dy = lat2 - lat1;
    float dx = cosf(3.141592654/180*lat1)*(long2 - long1);
    float angle = atan2f(dy, dx);

    return angle;
}