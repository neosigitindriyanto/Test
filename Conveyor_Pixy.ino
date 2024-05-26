#include <Pixy2.h>
#include <Servo.h>

Pixy2 pixy;
Servo myServo;
#define IN1 6 // deklarasi pin IN1
#define IN2 7  // deklarasi pin IN2
#define IN3 8  // deklarasi pin IN3
#define IN4 9  // deklarasi pin IN4
#define ENA 5 // deklarasi pin ENA
#define ENB 10  // deklarasi pin ENB
const int servoPin = 9;

void setup() {
  // Konfigurasi pin-pin sebagai Output
    Serial.begin(9600);
  pixy.init();
  pixy.changeProg("color_connected_components");
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  myServo.attach(servoPin);
  myServo.write(90); // Set servo to neutral position
}

void loop() {
    pixy.ccc.getBlocks();
  if (pixy.ccc.numBlocks) {
    for (int i = 0; i < pixy.ccc.numBlocks; i++) {
      int signature = pixy.ccc.blocks[i].m_signature;
      Serial.print("Detected signature: ");
      Serial.println(signature);
      if (signature == 1) {
        myServo.write(135); // Rotate servo to 45 degrees
      } else if (signature == 2) {
        myServo.write(15); // Rotate servo to -45 degrees
      }
    }
  } else {
    myServo.write(90); // Reset servo to neutral position if no block detected
  }
  // Motor A dan B berputar ke kanan 2000 ms (2 detik) dengan kecepatan penuh
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 150); // Mengatur kecepatan motor A (0-255)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 150); // Mengatur kecepatan motor B (0-255)
  delay(2000); // Jeda 2 detik

  /* Motor A dan B berputar ke kiri 2000 ms (2 detik) dengan kecepatan penuh
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitWrite(ENA, 255); // Mengatur kecepatan motor A (255 = 100%)
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(ENB, 255); // Mengatur kecepatan motor B (255 = 100%)
  delay(2000); // Jeda 2 detik;

  // Motor A ke kiri dan motor B ke kanan 2000 ms (2 detik) dengan kecepatan 50%
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(ENA, 128); // Mengatur kecepatan motor A (128 = 50%)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  digitalWrite(ENB, 128); // Mengatur kecepatan motor B (128 = 50%)
  delay(2000); // Jeda 2 detik;

  // Motor A dan motor B berhenti selama 2000 ms (2 detik)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENB, LOW);
  delay(2000);*/
}