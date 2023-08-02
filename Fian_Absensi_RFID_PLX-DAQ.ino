#include <SPI.h>
#include <MFRC522.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SS_PIN 10
#define RST_PIN 9

LiquidCrystal_I2C lcd(0x27, 16, 2);

int const buzzer = 5;
int pb_mode = A0;

byte readCard[7];
//Add as many cards you want
int cards1[7] = {4 ,101 ,69 ,90 ,139 ,102 ,128};
int cards2[4] = {131 ,106 ,175 ,165};
int cards3[7] = {5 ,133 ,205 ,138 ,144 ,209 ,0};
int cards4[4] = {67 ,20 ,173 ,165};


int ID;
String nama;
String jabatan;
bool status_kartu = false;
bool sudah_absen = false;
boolean mode_pulang = false;

RTC_DS3231 rtc;
MFRC522 mfrc522(SS_PIN, RST_PIN);
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.begin(16,2);
  lcd.backlight();
  pinMode(pb_mode, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (! rtc.begin()) {
    Serial.println("RTC tidak terhubung, Cek kembali wiring!");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC tidak bekerja, Setel ulang waktu!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("CLEARSHEET");
  Serial.println("LABEL,ID,Date,Name,Card ID,Jabatan,Jam Masuk,Jam Keluar");

  lcd.setCursor(1, 0);
  lcd.print("SMP NEGERI 13");
  lcd.setCursor(0, 1);
  lcd.print("MALUKU TENGAH");

  delay(1500);
  lcd.clear();
  delay(500);
}

void loop() {
  DateTime now = rtc.now();

  if (digitalRead(pb_mode) == LOW) {

    mode_pulang = !mode_pulang;

    Serial.println("mode");
    delay(500);
  }

  lcd.setCursor(4, 1);
  printposisilcd(now.hour());
  lcd.print(":");
  printposisilcd(now.minute());
  lcd.print(":");
  printposisilcd(now.second());

  if (mode_pulang == true) {
    lcd.setCursor(2, 0);
    lcd.print("Absen Pulang");
  }
  else {
    lcd.setCursor(2, 0);
    lcd.print(" Absen Masuk");
  }

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  if (mfrc522.uid.uidByte[0] != readCard[0] ||
      mfrc522.uid.uidByte[1] != readCard[1] ||
      mfrc522.uid.uidByte[2] != readCard[2] ||
      mfrc522.uid.uidByte[3] != readCard[3] ) {

    Serial.println("");
    Serial.print("UID : ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      readCard[i] = mfrc522.uid.uidByte[i];
      Serial.print(readCard[i]);
      if (i < mfrc522.uid.size - 1) {
        Serial.print(" ,");
      }
      else {
        Serial.println("");
      }

      sudah_absen = false;
      status_kartu = true;

      if (readCard[i] == cards1[i]) {
        ID = 1;
        nama = "Aisa Tehupelasury";
        jabatan = "Kepala Sekolah";
      }
      else if (readCard[i] == cards2[i]) {
        ID = 2;
        nama = "Karim";
        jabatan = "Wakil";
      }
      else if (readCard[i] == cards3[i]) {
        ID = 3;
        nama = "Yamna Nahumarury";
        jabatan = "Sekertaris";
      }
      else if (readCard[i] == cards4[i]) {
        ID = 4;
        nama = "Sasa Lestaluhu";
        jabatan = "Bendahara";
      }

    }
  }
  else {
    sudah_absen = true;
    Serial.println("sudah absen");
    lcd.setCursor(2, 1);
    lcd.print("Sudah Absen");
  }

  if (status_kartu == true && sudah_absen == false) {
    if (mode_pulang == false) {
      Serial.print("DATA,");
      Serial.print(ID);
      Serial.print(",");
      printtanggal();
      Serial.print(",");
      Serial.print(nama);
      Serial.print(",");
      printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.print(",");
      Serial.print(jabatan);
      Serial.print(",");
      printwaktu();
      Serial.print(",");
      Serial.println("");
      lcd.setCursor(2, 1);
      lcd.print("Terima Kasih");
    }

    if (mode_pulang == true) {
      Serial.print("DATA,");
      Serial.print(ID);
      Serial.print(",");
      printtanggal();
      Serial.print(",");
      Serial.print(nama);
      Serial.print(",");
      printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.print(",");
      Serial.print(jabatan);
      Serial.print(",");
      Serial.print("");
      Serial.print(",");
      printwaktu();
      Serial.print(",");
      Serial.println("");
      lcd.setCursor(2, 1);
      lcd.print("Terima Kasih");
    }
  }

  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(2000);
  lcd.clear();
  delay(50);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}


void printtanggal() {
  DateTime now = rtc.now();
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print('/');
  Serial.print(now.day());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.year());
}

void printwaktu() {
  DateTime now = rtc.now();
  printposisi(now.hour());
  Serial.print(':');
  printposisi(now.minute());
  Serial.print(':');
  printposisi(now.second());
}

void printHex(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printposisi(int digits) {
  if (digits < 10)
    Serial.print("0");
  Serial.print(digits);
}

void printposisilcd(int digits) {
  if (digits < 10)
    lcd.print("0");
  lcd.print(digits);
}
