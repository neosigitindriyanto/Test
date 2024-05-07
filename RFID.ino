#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5  // Sesuaikan dengan pin SS (atau RST) modul RC522
#define RST_PIN 27 // Sesuaikan dengan pin RST modul RC522

MFRC522 mfrc522(SS_PIN, RST_PIN); // Inisialisasi pustaka RC522 dengan pin yang sesuai

#define NUM_CARDS 1        // Jumlah kartu yang ingin didaftarkan
#define NFC_UID_SIZE 4      // Ukuran UID kartu NFC (pada modul RC522 UID hanya berukuran 4 byte)

byte registeredUIDs[NUM_CARDS][NFC_UID_SIZE] = {
  {0x53, 0xD3, 0x3C, 0x10}  // UID kartu 5
};

bool drawerLocked = true; // Mengunci laci secara default

void setup(void) {
  Serial.begin(115200);
  while (!Serial); // Tunggu hingga Serial Monitor terbuka
  SPI.begin();      // Inisialisasi SPI bus
  mfrc522.PCD_Init(); // Inisialisasi pustaka RC522
  Serial.println("Waiting for an NFC card ...");
}

void loop(void) {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Found an NFC card!");

    if (mfrc522.uid.size == NFC_UID_SIZE) {
      byte* uidData = mfrc522.uid.uidByte;
      if (isRegisteredCard(uidData)) {
        Serial.println("Access granted! Opening drawer...");
        openDrawer(); // Fungsi untuk membuka laci
      } else {
        Serial.println("Access denied! Unauthorized card.");
      }
    } else {
      Serial.println("Invalid NFC card.");
    }

    delay(1000); // Beri jeda agar kartu tidak terus-menerus terdeteksi saat ditempatkan di atas reader
    mfrc522.PICC_HaltA(); // Hentikan kartu NFC yang terdeteksi saat ini
  }
}

bool isRegisteredCard(byte* uidData) {
  for (int i = 0; i < NUM_CARDS; i++) {
    if (memcmp(uidData, registeredUIDs[i], NFC_UID_SIZE) == 0) {
      return true; // Kartu terdeteksi ada dalam daftar kartu terdaftar
    }
  }
  return false; // Kartu tidak ada dalam daftar kartu terdaftar
}

void openDrawer() {
  Serial.println("Laci terbuka");
  delay(1000);
}
