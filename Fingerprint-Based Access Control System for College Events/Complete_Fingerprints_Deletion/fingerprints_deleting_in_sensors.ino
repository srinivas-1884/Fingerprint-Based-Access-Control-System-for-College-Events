#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger(&mySerial);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Clearing all fingerprints...");

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Sensor found!");
  } else {
    Serial.println("Sensor not found :(");
    while (1);
  }

  finger.emptyDatabase();
  Serial.println("All fingerprints deleted!");
}

void loop() {}
