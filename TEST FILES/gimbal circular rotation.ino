#include <Servo.h>

Servo servo1;  // Servo for one axis
Servo servo2;  // Servo for the other axis

int pos1 = 0;  // Variable to store the position of servo1
int pos2 = 0;  // Variable to store the position of servo2

void setup() {
  servo1.attach(9);  // Attach servo1 to pin 9
  servo2.attach(10); // Attach servo2 to pin 10
}

void loop() {
  // Circular movement pattern
  for (int angle = 0; angle <= 360; angle++) {
    pos1 = 90 + 35 * cos(radians(angle));  // Calculate position for servo1
    pos2 = 90 + 35 * sin(radians(angle));  // Calculate position for servo2
    
    servo1.write(pos1);  // Set the position of servo1
    servo2.write(pos2);  // Set the position of servo2
    
    delay(5);  // Delay to allow the servos to move
  }
}
