#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Servo.h>

// Pin definitions
#define LED 8
#define BUZZER 11

// MPU6050 sensor
MPU6050 sensor;

// BMP180 sensor
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified();

// Servo pins
const int SERVO_PIN1 = 9;
const int SERVO_PIN2 = 10;

// Create servo objects
Servo sg90;
Servo sg90two;

// MPU6050 range
const int MPU6050_RANGE = 17000;

// Servo angle range
const int SERVO_MIN_ANGLE = 0;  // Increased movement range
const int SERVO_MAX_ANGLE = 180; // Increased movement range

// Variables for filtering
const int FILTER_SIZE = 5;  // Reduce filter size for more responsiveness
int axBuffer[FILTER_SIZE] = {0};
int ayBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize buzzer and LED
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  tone(BUZZER, 1000); // Set the voltage to high and make a noise at 1000 Hz
  delay(50); // Wait for 50 milliseconds
  noTone(BUZZER); // Set the voltage to low and make no noise

  // Initialize MPU6050
  sensor.initialize();
  if (sensor.testConnection()) {
    Serial.println("MPU6050 connected successfully");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1) {
      delay(500);
    }
  }

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {
      delay(500);
    }
  }

  // Initialize servos to 90 degrees (perpendicular position)
  sg90.attach(SERVO_PIN1);
  sg90two.attach(SERVO_PIN2);
  sg90.write(90);
  sg90two.write(90);
}

void loop() {
  // Turn on LED
  digitalWrite(LED, HIGH);

  // Read BMP180 data
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    Serial.print("Pressure: ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    // Read temperature
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    // Calculate altitude
    float altitude = bmp.pressureToAltitude(1013.25, event.pressure);
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");
  }

  // Read MPU6050 data
  int16_t ax, ay, az, gx, gy, gz;
  sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Add new data to filter buffers
  axBuffer[filterIndex] = ax;
  ayBuffer[filterIndex] = ay;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;

  // Compute filtered values
  long axSum = 0;
  long aySum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    axSum += axBuffer[i];
    aySum += ayBuffer[i];
  }
  int axFiltered = axSum / FILTER_SIZE;
  int ayFiltered = aySum / FILTER_SIZE;

  Serial.print("ax: "); Serial.print(axFiltered); Serial.print(" ");
  Serial.print("ay: "); Serial.print(ayFiltered); Serial.print(" ");
  Serial.print("az: "); Serial.print(az); Serial.print(" ");
  Serial.print("gx: "); Serial.print(gx); Serial.print(" ");
  Serial.print("gy: "); Serial.print(gy); Serial.print(" ");
  Serial.print("gz: "); Serial.println(gz);

  // Map filtered MPU6050 data to servo angles with increased sensitivity
  int servo_angle1 = map(axFiltered, -MPU6050_RANGE, MPU6050_RANGE, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  int servo_angle2 = map(ayFiltered, -MPU6050_RANGE, MPU6050_RANGE, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

  // Constrain the angles to ensure they do not exceed the 0 to 180 degree limit
  servo_angle1 = constrain(servo_angle1, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  servo_angle2 = constrain(servo_angle2, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

  // Set servo positions
  sg90.write(servo_angle1);
  sg90two.write(servo_angle2);


  // Turn off LED
  digitalWrite(LED, LOW);

  // Short delay before next loop iteration
  delay(10);
}
