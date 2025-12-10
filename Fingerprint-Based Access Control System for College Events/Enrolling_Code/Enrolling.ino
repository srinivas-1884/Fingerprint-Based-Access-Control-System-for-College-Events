#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

SoftwareSerial mySerial(2, 3);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define MAX_STUDENTS 127
#define ROLL_LEN 20

// ----------------------------------------------------
void setup() {
  Serial.begin(9600);
  mySerial.begin(57600);
  finger.begin(57600);
  delay(1000);

  if (finger.verifyPassword()) {
    Serial.println("Sensor Ready");
  } else {
    Serial.println("Sensor Not Found");
    while (1);
  }
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("ENROLL:")) {
      String roll = cmd.substring(7);
      enrollStudentFlow(roll);
    } else if (cmd.startsWith("DELETE:")) {
      String roll = cmd.substring(7);
      deleteUserByRoll(roll);
    } else if (cmd == "LIST") {
      listAllUsers();
    }
  }
}

// ----------------------------------------------------
// EEPROM Functions
void saveRollToEEPROM(int id, String roll) {
  int addr = (id - 1) * ROLL_LEN;
  for (int i = 0; i < ROLL_LEN; i++) {
    EEPROM.write(addr + i, (i < roll.length()) ? roll[i] : 0);
  }
}

String readRollFromEEPROM(int id) {
  int addr = (id - 1) * ROLL_LEN;
  String roll = "";
  for (int i = 0; i < ROLL_LEN; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0) break;
    roll += c;
  }
  return roll;
}

int findIDByRoll(String roll) {
  for (int i = 1; i <= MAX_STUDENTS; i++) {
    String r = readRollFromEEPROM(i);
    if (r == roll) return i;
  }
  return 0;
}

// ----------------------------------------------------
bool captureFingerImage(int num) {
  int p = -1;
  for (int i = 0; i < 10; i++) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) break;
    delay(500);
  }
  if (p != FINGERPRINT_OK) return false;

  p = finger.image2Tz(num);
  return (p == FINGERPRINT_OK);
}

// ----------------------------------------------------
void enrollStudentFlow(String roll) {
  Serial.println("Starting enrollment...");

  int id = 0;
  for (int i = 1; i <= MAX_STUDENTS; i++) {
    if (finger.loadModel(i) != FINGERPRINT_OK) {
      id = i;
      break;
    }
  }

  if (id == 0) {
    Serial.println("ENROLL_FAIL_NO_SPACE");
    return;
  }

  if (findIDByRoll(roll) != 0) {
    Serial.println("ENROLL_FAIL_DUPLICATE_ROLL");
    return;
  }

  Serial.println("Place finger on the sensor (first scan)...");
  if (!captureFingerImage(1)) {
    Serial.println("ENROLL_FAIL_IMAGE1");
    return;
  }

  int searchRes = finger.fingerFastSearch();
  if (searchRes == FINGERPRINT_OK) {
    Serial.println("ENROLL_FAIL_DUPLICATE_FINGER");
    return;
  }

  Serial.println("Remove finger...");
  delay(1500);

  Serial.println("Place the SAME finger again (second scan)...");
  if (!captureFingerImage(2)) {
    Serial.println("ENROLL_FAIL_IMAGE2");
    return;
  }

  int p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("ENROLL_FAIL_MATCH");
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    saveRollToEEPROM(id, roll);
    Serial.print("ENROLL_SUCCESS:");
    Serial.println(roll);
  } else {
    Serial.println("ENROLL_FAIL_STORE");
  }
}

// ----------------------------------------------------
void deleteUserByRoll(String roll) {
  int id = findIDByRoll(roll);
  if (id == 0) {
    Serial.println("DELETE_FAIL_NOT_FOUND");
    return;
  }
  finger.deleteModel(id);
  for (int i = 0; i < ROLL_LEN; i++) EEPROM.write((id - 1) * ROLL_LEN + i, 0);
  Serial.print("DELETE_SUCCESS:");
  Serial.println(roll);
}

// ----------------------------------------------------
void listAllUsers() {
  Serial.println("LIST_BEGIN");
  for (int i = 1; i <= MAX_STUDENTS; i++) {
    String r = readRollFromEEPROM(i);
    if (r.length() > 0) {
      Serial.print("ID:");
      Serial.print(i);
      Serial.print(",ROLL:");
      Serial.println(r);
    }
  }
  Serial.println("LIST_END");
}