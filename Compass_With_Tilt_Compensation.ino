/*!
 * @file NavigationAngleRead.ino
 * @brief DFRobot's Manentic Sensor,This program continuously reads the accelerometer and magnetometer, 
 *        communicating the readings over the serial interface. You can display the readings with the Arduino Serial Monitor.
 * @n [Get the module here](http://www.dfrobot.com.cn/goods-326.html)
 * @n This example get the four lightest positions of the IR sources.
 * @n [Connection and Diagram](http://wiki.dfrobot.com.cn/index.php?title=(SKU:TOY0035)Gadgeteer_LSM303%E7%94%B5%E5%AD%90%E7%BD%97%E7%9B%98%E4%BC%A0%E6%84%9F%E5%99%A8)
 *
 * @copyright  [DFRobot](http://www.dfrobot.com), 2016
 * @copyright GNU Lesser General Public License
 *
 * @author [carl](carl.xu@dfrobot.com)
 * @version  V1.0
 * @date  2016-07-11
 */

//Required Library: https://github.com/DFRobot/MagneticSensorLsm303

#include <MagneticSensorLsm303.h>

MagneticSensorLsm303 compass;

void setup() {
  Serial.begin(9600);
  compass.init();
  compass.enable();
}

void loop() {
  compass.read();
  float heading = compass.getNavigationAngle();
  Serial.print("Navigation Angle:  ");
  Serial.println(heading,3);
  delay(500);  // delay for serial readability
}
