#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library form the adafruit site above
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
SoftwareSerial mySerial(3, 2); //Initialize SoftwareSerial, and tell it you will be connecting through pins 2 and 3
Adafruit_GPS GPS(&mySerial); //Create GPS object

String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence

char latitude_char[20] = {'\0'};
char longitude_char[20] = {'\0'};
char c;

const float Pi = 3.14159;

int N_degrees;
int E_degrees;
float N_minutes; 
float E_minutes;
float N_seconds;
float E_seconds;

const float N_target_seconds = 142365.2328;
const float E_target_seconds = 431349.7392;
float target;

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

  //target = (atan2(N_target_seconds - N_seconds, E_target_seconds - E_seconds) * 180) / Pi;
  float longitude_diff = E_seconds - E_target_seconds;
  float y = sin(longitude_diff) * cos(N_target_seconds);
  float x = (cos(N_seconds) * sin(N_target_seconds)) - (sin(N_seconds) * cos(N_target_seconds) * cos(longitude_diff));
  target = (atan2(y, x)*180) / Pi;

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
  //Serial.print("Angle: ");
  //Serial.println(GPS.angle);
  //Serial.print("Speed: ");
  //Serial.println(GPS.speed);

  //PID
  Serial.print("Target Heading: ");
  Serial.println(target);
  Serial.println(N_target_seconds - N_seconds);
  Serial.println(longitude_diff);
  
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
