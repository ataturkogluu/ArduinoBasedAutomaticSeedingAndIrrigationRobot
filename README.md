# ArduinoBasedAutomaticSeedingAndIrrigationRobot
ARDUINO BASED AUTOMATIC SEEDING AND IRRIGATION ROBOT
The robot is based on Arduino Uno. In the project, there is a drilling mechanism to drill
the soil and the robot is designed to allow the seeds to fall into the drilled part. CD-ROM
mechanism and DC motor are used for drilling process. In the seeding mechanism, a
seeding syringe and a servo motor were used. The outputs taken with temperature, soil
moisture and humidity sensors are transferred to the mobile application with the
ESP8266 wifi module. ESP8266 wifi module is a self-contained module with built-in
TCP/IP protocol, which can give any microcontroller access to wifi network. The robot
has four DC motor controlled wheels and one caster wheel. With the help of the
ultrasonic sensor, the robot detects if there is an obstacle in front of the vehicle and
proceeds. It is transmitted to the user through the mobile application that he encounters
with the obstacle. If the obstacle is removed, the robot continues to move forward. The
robot is programmed to advance by drilling the soil at regular intervals and
simultaneously releasing seeds. The seed robot performs the seeding process by
scanning a certain area. It acts completely autonomously while performing this
operation. In order to do this, the length and width information of the field must be
shared with the robot. While the robot is moving, it also performs irrigation with the
irrigation mechanism. The user can access the information of the autonomous vehicle
through the mobile application. The mobile application is built using the Flutter
language. The movement of the robot is designed to be controlled also via the mobile
application in the desired situation.
