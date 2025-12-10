#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial mySerial(2, 3);  
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo servo;

// LED & Buzzer pins
#define GREEN_LED 4
#define RED_LED   5
#define BUZZER    6

void setup() {
  Serial.begin(9600);
  while (!Serial);

  servo.attach(9);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1); // EDIT: Increased text size
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Fingerprint Test"));
  display.display();
  delay(1500); // EDIT: Short delay to show startup message

  // LED & Buzzer setup
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Fingerprint sensor
  mySerial.begin(57600); 
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Sensor not found"));
    display.display();
    while (true);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Sensor found!"));
  display.display();
  delay(1500);

  finger.getTemplateCount();
  Serial.print(F("Sensor contains ")); 
  Serial.print(finger.templateCount); 
  Serial.println(F(" templates"));
  Serial.println(F("Waiting for valid finger..."));

  // EDIT: Display permanent waiting message
  display.clearDisplay();
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println(F("Waiting for"));
  display.println(F("Valid Fingerprint"));
  display.display();
  // END EDIT
}

void loop() {
  int result = getFingerprintIDez();

  if (result != -1) {
    display.clearDisplay();
    display.setTextSize(2); // EDIT: Big font for status message
    display.setCursor(0, 0);

    if (result > 0) {
      // Access Granted
      display.println(F("Access OK"));
      display.setTextSize(1);
      display.print(F("ID: "));
      display.println(result);
      display.display();

      digitalWrite(GREEN_LED, HIGH);
      tone(BUZZER, 1000); delay(200); noTone(BUZZER); // short beep
      
      servo.write(90);
      delay(3000);
      servo.write(0);
      
      digitalWrite(GREEN_LED, LOW);
    } 
    else {
      // Access Denied
      display.println(F("Access"));
      display.println(F("Denied"));
      display.display();
      
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 500); delay(100); noTone(BUZZER);
      delay(100);
      tone(BUZZER, 500); delay(100); noTone(BUZZER);
      
      delay(2000);
      digitalWrite(RED_LED, LOW);
    }

    // EDIT: After result, show waiting message again
    delay(1500);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.println(F("Waiting for"));
    display.println(F("Valid Fingerprint"));
    display.display();
    // END EDIT
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return 0;

  Serial.print(F("Found ID #")); 
  Serial.print(finger.fingerID);
  Serial.print(F(" with confidence of ")); 
  Serial.println(finger.confidence);

  return finger.fingerID;
}
