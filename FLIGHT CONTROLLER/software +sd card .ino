#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>


#define LED 8
#define BUZZER 2
#define SD_CS 4  // Chip Select pin for SD card module


MPU6050 sensor;


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);


const int SERVO_PIN1 = 9;
const int SERVO_PIN2 = 10;


Servo sg90;
Servo sg90two;


const int MPU6050_RANGE = 17000;

const int SERVO_MIN_ANGLE = 0;  
const int SERVO_MAX_ANGLE = 180; 


const int FILTER_SIZE = 5; 
int axBuffer[FILTER_SIZE] = {0};
int ayBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;


float highestAltitude = -9999; 


const int EEPROM_ADDRESS = 0;

File dataFile;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  tone(BUZZER, 1000); 
  delay(100); 
  noTone(BUZZER); 

  
  sensor.initialize();
  if (sensor.testConnection()) {
    Serial.println("MPU6050 connected successfully");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1) {
      delay(500);
    }
  }

  
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {
      delay(500);
    }
  }

  
  sg90.attach(SERVO_PIN1);
  sg90two.attach(SERVO_PIN2);
  sg90.write(90);
  sg90two.write(90);


  float eepromAltitude;
  EEPROM.get(EEPROM_ADDRESS, eepromAltitude);
  if (isnan(eepromAltitude)) {
  
    eepromAltitude = -9999;
    EEPROM.put(EEPROM_ADDRESS, eepromAltitude);
  }
  highestAltitude = eepromAltitude;
  Serial.print("Highest Altitude retrieved from EEPROM: ");
  Serial.println(highestAltitude);

  
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    while (1) {
      delay(500);
    }
  }
  Serial.println("SD card initialized.");

  
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening datalog.txt");
  }
}

void loop() {
  
  digitalWrite(LED, HIGH);

  
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    float pressure = event.pressure;
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

  
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    
    float altitude = bmp.pressureToAltitude(1013.25, pressure);
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");

    
    if (dataFile) {
      dataFile.print("Pressure: ");
      dataFile.print(pressure);
      dataFile.print(" hPa, Temperature: ");
      dataFile.print(temperature);
      dataFile.print(" C, Altitude: ");
      dataFile.print(altitude);
      dataFile.println(" m");
      dataFile.flush(); 
    }

    
    if (altitude > highestAltitude) {
      highestAltitude = altitude;
      EEPROM.put(EEPROM_ADDRESS, highestAltitude); 
      Serial.print("New highest altitude recorded: ");
      Serial.println(highestAltitude);
    }
  }
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= 10000) { 
    Serial.print("Highest Altitude: ");
    Serial.println(highestAltitude);
    lastPrintTime = currentTime;
  }


  int16_t ax, ay, az, gx, gy, gz;
  sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  
  axBuffer[filterIndex] = ax;
  ayBuffer[filterIndex] = ay;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;


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


  delay(10);
}
