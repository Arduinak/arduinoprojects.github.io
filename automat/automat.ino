#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN   9     // Pin RST na RC522
#define SS_PIN    10    // Pin SDA (SS) na RC522
#define BUZZER_PIN 8    // Pin pre bzučák
#define RELAY_PIN 7     // Pin pre relé
#define BUTTON_PIN 6    // Pin pre tlačidlo

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Vytvorenie inštancie MFRC522
LiquidCrystal_I2C lcd(0x27, 20, 4); // Nastavenie adresy LCD a rozmerov (20x4)

// UID kariet (vaše skutočné UID)
byte card1UID[] = {0x07, 0x82, 0xF1, 0xAE}; // Prvá karta
byte card2UID[] = {0xE3, 0xCC, 0x31, 0x1E}; // Druhá karta

bool serviceMode = false; // Stav servisného módu

void setup() {
  Serial.begin(9600);   // Inicializácia sériovej komunikácie
  SPI.begin();          // Inicializácia SPI komunikácie
  mfrc522.PCD_Init();   // Inicializácia MFRC522
  lcd.init();           // Inicializácia LCD
  lcd.backlight();      // Zapnutie podsvietenia LCD
  pinMode(BUZZER_PIN, OUTPUT); // Nastavenie bzučáka ako výstup
  pinMode(RELAY_PIN, OUTPUT);  // Nastavenie relé ako výstup
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Nastavenie tlačidla ako vstup s pull-up rezistorom

  lcd.setCursor(0, 0);
  lcd.print("Prilozte kartu...");
}

void loop() {
  // Kontrola, či je karta priložená
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Kontrola UID karty
    if (compareUID(mfrc522.uid.uidByte, card1UID)) {
      // Prvá karta
      if (!serviceMode) {
        activateServiceMode();
      }
    } else if (compareUID(mfrc522.uid.uidByte, card2UID)) {
      // Druhá karta
      if (!serviceMode) {
        runNormalProcess(); // Spustenie procesu "Polozte kelimok"
      }
    }

    // Zastavenie čítania karty
    mfrc522.PICC_HaltA();
  }

  // Kontrola tlačidla v servisnom móde
  if (serviceMode && digitalRead(BUTTON_PIN) == LOW) {
    runRelaySequence();
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10); // Čakanie, kým sa tlačidlo uvoľní
    }
  }
}

// Funkcia na aktiváciu servisného módu
void activateServiceMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Servisny mod");
  serviceMode = true;
  playBeep(600, 500); // Prehratie tónu 600 Hz na 500 ms
}

// Funkcia na deaktiváciu servisného módu
void deactivateServiceMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prilozte kartu...");
  serviceMode = false;
  playBeep(600, 500); // Prehratie tónu 600 Hz na 500 ms
}

// Funkcia na spustenie normálneho procesu (karta číslo 2)
void runNormalProcess() {
  while (true) { // Nekonečný cyklus, kým sa proces neopakuje
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Polozte kelimok");
    lcd.setCursor(0, 1);
    lcd.print(" A stlaite tlacidlo...");

    // Čakanie na stlačenie tlačidla
    while (digitalRead(BUTTON_PIN) == HIGH) {
      delay(10); // Čakanie, kým sa tlačidlo stlačí
    }

    // Spustenie relé na 8 sekúnd
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Capujem, Pockajte prosim");
    digitalWrite(RELAY_PIN, HIGH); // Zapnutie relé
    delay(8000); // Čakanie 8 sekúnd
    digitalWrite(RELAY_PIN, LOW);  // Vypnutie relé

    // Čakanie 10 sekúnd od začiatku procesu
    delay(10000); // Celkovo 10 sekúnd (8 sekúnd relé + 2 sekundy čakania)

    // Prehratie pesničky
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("D'akujeme");
    playMelody(); // Prehratie krátkej pesničky

    // Návrat do pôvodného stavu
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Prilozte kartu...");

    // Čakanie na priloženie karty pre opakovanie procesu
    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(10); // Čakanie na novú kartu
    }

    // Kontrola, či je priložená karta číslo 2
    if (compareUID(mfrc522.uid.uidByte, card2UID)) {
      break; // Ukončenie cyklu, ak je priložená karta číslo 2
    }
  }
}

// Funkcia na spustenie relé a sekvencie v servisnom móde
void runRelaySequence() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spustam rele...");
  digitalWrite(RELAY_PIN, HIGH); // Zapnutie relé
  delay(8000); // Čakanie 8 sekúnd
  digitalWrite(RELAY_PIN, LOW);  // Vypnutie relé

  delay(2000); // Čakanie 2 sekundy

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dakujeme!");
  playBeep(600, 500); // Prehratie tónu 600 Hz na 500 ms
  delay(100);
  playBeep(600, 500); // Prehratie tónu 600 Hz na 500 ms

  playMelody(); // Prehratie krátkej pesničky

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prilozte kartu...");
}

// Funkcia na porovnanie UID karty
bool compareUID(byte *uid1, byte *uid2) {
  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// Funkcia na prehratie tónu
void playBeep(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration); // Prehratie tónu
  delay(duration); // Čakanie počas trvania tónu
  noTone(BUZZER_PIN); // Vypnutie tónu
}

// Funkcia na prehratie krátkej pesničky
void playMelody() {
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // Noty pre pesničku
  int noteDurations[] = {200, 200, 200, 200, 200, 200, 200, 200}; // Trvanie not

  for (int i = 0; i < 8; i++) {
    tone(BUZZER_PIN, melody[i], noteDurations[i]);
    delay(noteDurations[i] * 1.30);
    noTone(BUZZER_PIN);
  }
}