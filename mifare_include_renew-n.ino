#include <GyverOLED.h>
#include <GyverButton.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

bool ReadMode = true;
int Mode = 1; ////1-Чтение  2-Запись 3-Ввод ключа
GButton btnOK(3);             // кнопка OK
GButton btnUP(2);            // кнопка UP
GButton btnRIGHT(6);         // кнопка DOWN
GButton btnLEFT(5);         // кнопка LEFT
GButton btnDOWN(4);         // кнопка RiGHT



String type;
unsigned long timing;
int poz = 1;
byte nuidPICC[4] = {0xFA, 0xFA, 0xFA, 0xFA};

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define speakerPin A0       // + спикера
#define AllGnd A3           // -спикера

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;


void setup() {
  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(2);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
  //delay(1000);
  oled.setScale(1);
  // курсор на начало 3 строки
  oled.setCursor(0, 3);
  oled.print("Mifare Classic 1K  ");
  oled.setCursor(0, 4);
  oled.print("              ");
  oled.setCursor(0, 6);
  oled.print("Режим чтения");

  pinMode(speakerPin, OUTPUT);
  pinMode(AllGnd, OUTPUT); digitalWrite(AllGnd, LOW); // подключаем - пин спикера к земле
  btnOK.setTimeout(3000);

  Serial.begin(9600);
  //Sd_StartOK();  //Звук старта
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  MFRC522::PICC_TYPE_MIFARE_1K;

 //byte nuidPICC[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  
  for (byte i = 0; i < 6; i++) { // Наполняем ключ
     Serial.print(String(i));
     rfid.uid.uidByte[i] = (0xFA);       // Ключ по умолчанию 0xFFFFFFFFFFFF
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  //printHex(nuidPICC,0x4);
  printHex(rfid.uid.uidByte, 4);//////тут
  Serial.print("size=");
  Serial.println(rfid.uid.size);
  //printHex22(nuidPICC, 4);
  

}
void loop() {
  btnDOWN.tick();
  btnUP.tick();
  btnOK.tick();
  btnLEFT.tick();
  btnRIGHT.tick();

  if (Mode == 1) {
    ReadFunk();

  }

  if (Mode == 2) {
    WriteFunk();
  }

  if (Mode == 3) {
    EnterFunk();
  }


  if (btnLEFT.isSingle()) {
    --poz;
    Serial.println("btnLEFT)");
  }


  if (btnRIGHT.isSingle()) {
    ++poz;
    Serial.println("btnRIGHT)");
  }


  if (btnOK.isSingle()) {

    ++Mode;
    if (Mode > 3) Mode = 1;
    Serial.print("Режим ");  Serial.println(Mode);
    Serial.println("btnOKClick (OK)");
  }


  if (btnUP.isSingle()) {
    Serial.println("btnUPClick(+)");
    if (poz % 2 == 0) {
      rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] + 1;
    }
    else {
      rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] + 16;
    }
    nuidPICC[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)];
    printHex(rfid.uid.uidByte, rfid.uid.size);
  }

  if (btnDOWN.isSingle()) {
    Serial.println("btnDOWNClick(-)");
    if (poz % 2 == 0) {
      rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] - 1;
    }
    else {
      rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)] - 16;
    }
    nuidPICC[((poz / 2 + poz % 2) - 1)] = rfid.uid.uidByte[((poz / 2 + poz % 2) - 1)];
    printHex(rfid.uid.uidByte, rfid.uid.size);
  }


  if (btnOK.isHolded()) {
    Serial.println("Hold");
  }
}


void EnterFunk() {          /// РЕЖИМ ВВОДА
  oled.setScale(1);

  oled.setCursor(0, 4);
  oled.print("              ");
  oled.setCursor(0, 6);
  oled.print("Режим ввода  ");
  //Serial.println("Режим ввода");
  printHexEnter(rfid.uid.uidByte, 4);
}

void WriteFunk() {                                    /// РЕЖИМ ЗАПИСИ
  oled.setScale(1);
  oled.setCursor(0, 6);
  oled.print("Режим записи");
  Serial.println("Режим записи");


  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }


  // Dump UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();


  // Set new UID

  if ( rfid.MIFARE_SetUid(nuidPICC, (byte)4, true) ) {
    Serial.println(F("Wrote new UID to card."));
    oled.setScale(1);
    oled.setCursor(0, 4);
    oled.print("Запись успешна");
    Sd_WriteStep();
    Sd_WriteStep();
    Sd_WriteStep();
    Sd_WriteStep();


  }
  else {
    Serial.println(F("Ошибка записи."));
    oled.setScale(1);
    oled.setCursor(0, 4);
    oled.print("ОШИБКА Записи  ");
    Sd_ErrorBeep();
  }

  // Halt PICC and re-select it so DumpToSerial doesn't get confused
  rfid.PICC_HaltA();
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial() ) {
    Serial.println(F("Herabora."));
    return;
  }


}


void ReadFunk() {                       /// РЕЖИМ ЧТЕНИЯ

  oled.setScale(1);

  oled.setCursor(0, 4);
  oled.print("              ");
  oled.setCursor(0, 6);
  oled.print("Режим чтения  ");
  //Serial.println("Режим чтения");
  printHex(rfid.uid.uidByte, 4);

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType == MFRC522::PICC_TYPE_MIFARE_1K) {
    type = "Mifare Classic 1K  ";
    oled.setScale(1);
    oled.setCursor(0, 3);
    oled.print(type);
    Serial.println(F("Type Mifare 1K"));
  }
  if (piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
    type = "Mifare Classic 4K  ";
    oled.setScale(1);
    oled.setCursor(0, 3);
    oled.print(type);
    Serial.println(F("Type Mifare 4K"));
  }
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI) {
    type = "Mifare Classic MINI";
    oled.setScale(1);
    oled.setCursor(0, 3);
    oled.print(type);
    Serial.println(F("Type Mifare MINI"));
  }
  if (piccType == MFRC522::PICC_TYPE_MIFARE_UL) {
    type = "Mifare UltraLight";
    oled.setScale(1);
    oled.setCursor(0, 3);
    oled.print(type);
    Serial.println(F("Type Mifare UltraLight"));
  }
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    type = "Unknown           ";
    oled.setScale(1);
    oled.setCursor(0, 3);
    oled.print(type);
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    Sd_ErrorBeep();
  }



  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  
  
  Serial.println();


  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  beep_ok();
}



void printHex(byte *buffer, byte bufferSize) {
  oled.setScale(1);   // масштаб текста (1..4)
  oled.setCursor(0, 0);       // курсор в -1,0
  for (byte i = 0; i < bufferSize; i++) {
    if (i == 0) {
      Serial.print(buffer[i] < 0x10 ? "0" : "");
      oled.print(buffer[i] < 0x10 ? "0" : "");
    }
    else {
      Serial.print(buffer[i] < 0x10 ? ":0" : ":");
      oled.print(buffer[i] < 0x10 ? ":0" : ":");
    }

    Serial.print(buffer[i], HEX);
    oled.print(buffer[i], HEX);

  }
  if (bufferSize < 0x5) { ////затираем лишние символы если длинна ключа меньше 4 бита
    oled.setScale(1);   // масштаб текста (1..4)
    oled.setCursor(66, 0);       // курсор в -1,0
    oled.print("         ");
  }

}






void printHexEnter(byte *buffer, byte bufferSize) {
  oled.setScale(1);   // масштаб текста (1..4)
  oled.setCursor(0, 0);       // курсор в -1,0
  if (millis() - timing > 500) {

    for (byte i = 0; i < bufferSize; i++) {
      if (i == 0) {
        // Serial.print(buffer[i] < 0x10 ? "0" : "");
        oled.print(buffer[i] < 0x10 ? "0" : "");
      }
      else {
        // Serial.print(buffer[i] < 0x10 ? ":0" : ":");
        oled.print(buffer[i] < 0x10 ? ":0" : ":");
      }

      // Serial.print(buffer[i], HEX);
      oled.print(buffer[i], HEX);

    }
  }
  if (bufferSize < 0x5) { ////затираем лишние символы если длинна ключа меньше 4 бита
    oled.setScale(1);   // масштаб текста (1..4)
    oled.setCursor(66, 0);       // курсор в -1,0
    oled.print("         ");
  }

  if (millis() - timing > 1000) { // Вместо 10000 подставьте нужное вам значение паузы
    timing = millis();
    Serial.println ("1 second");
    oled.setCursor(((poz / 2 + poz + poz % 2 - 1) * 6) - 6, 0); // курсор в -1,0
    oled.print(" ");
  }


}




//***************** звуки****************

void Sd_WriteStep() { // звук "очередной шаг"
  for (int i = 2500; i < 6000; i = i * 1.5) {
    tone(speakerPin, i);
    delay(10);
  }
  noTone(speakerPin);
}

void Sd_ErrorBeep() {  // звук "ERROR"
  for (int j = 0; j < 3; j++) {
    for (int i = 1000; i < 2000; i = i * 1.1) {
      tone(speakerPin, i);
      delay(10);
    }
    delay(50);
    for (int i = 1000; i > 500; i = i * 1.9) {
      tone(speakerPin, i);
      delay(10);
    }
    delay(50);
  }
  noTone(speakerPin);
}

void Sd_StartOK() {  // звук "Успешное включение"
  Serial.println("Hello Sanya");
  delay(150);
  tone(speakerPin, 1318, 300);
  delay(300);
  tone(speakerPin, 1318, 150);
  delay(300);
  tone(speakerPin, 1046, 150);
  delay(150);
  tone(speakerPin, 1318, 300);
  delay(300);
  tone(speakerPin, 1568, 600);
  delay(600);
  tone(speakerPin, 784, 600);
  delay(600);
}

void beep_ok() {  // звук ОК
  for (int ib = 400; ib < 6000; ib = ib * 1.5) {
    tone(speakerPin, ib);
    delay(20);
  }
  noTone(speakerPin);
}
