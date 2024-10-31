#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
// softuart for sending data to esp32
// #include <SoftwareSerial.h>

//====================================================
/*
 * Cant Write to the card
 * so we will use the uid of the card to identify the card
 * and the database will store the data of the card by the uid
 */
// mrc522 not working, cant findout why, so using IR sensor instead
//====================================================
#define RST_PIN 9 // Configurable, see typical pin layout above
// 15 ss pin
#define SS_1_PIN 53
#define SS_2_PIN 49
#define SS_3_PIN 48
#define SS_4_PIN 47
#define SS_5_PIN 46
#define SS_6_PIN 45
#define SS_7_PIN 44
#define SS_8_PIN 43
#define SS_9_PIN 42
#define SS_10_PIN 41
#define SS_11_PIN 40
#define SS_12_PIN 39
#define SS_13_PIN 38
#define SS_14_PIN 37
#define SS_15_PIN 36
// #define SS_16_PIN 35
// #define SS_17_PIN 34
// #define SS_18_PIN 33
// #define SS_19_PIN 32
// #define SS_20_PIN 31
// #define SS_21_PIN 30
// #define SS_22_PIN 29
// #define SS_23_PIN 28
// #define SS_24_PIN 27
// #define SS_25_PIN 26
// #define SS_26_PIN 25
// #define SS_27_PIN 24
// #define SS_28_PIN 23
// #define SS_29_PIN 22
// #define SS_30_PIN 21

#define numofIR 15 // no longer user this

// re use the ss pin for ir sensor (coz the mfrc522 not working)
#define IR_PIN1 53
#define IR_PIN2 49
#define IR_PIN3 48
#define IR_PIN4 47
#define IR_PIN5 46
#define IR_PIN6 45
#define IR_PIN7 44
#define IR_PIN8 43
#define IR_PIN9 42
#define IR_PIN10 41
#define IR_PIN11 40
#define IR_PIN12 39
#define IR_PIN13 38
#define IR_PIN14 37
#define IR_PIN15 36

#define NR_OF_READERS 15

// #define ESP32_RX 2
// #define ESP32_TX 3

// TODO: to open the door and add the led to this pin and add buzzer to this
#define DOOR_PIN 8
#define BUZZER_PIN 7 // only ring for a while

#define FLOOR 1 // this is the floor 1

#define dw digitalWrite
#define dr digitalRead

#define TIME_CHECK_THING 2000 // check the thing every 1 second
//====================================================
byte ssPins[NR_OF_READERS] = {
    SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN, SS_5_PIN,
    SS_6_PIN, SS_7_PIN, SS_8_PIN, SS_9_PIN, SS_10_PIN,
    SS_11_PIN, SS_12_PIN, SS_13_PIN, SS_14_PIN, SS_15_PIN};
// SS_16_PIN, SS_17_PIN, SS_18_PIN, SS_19_PIN, SS_20_PIN,
// SS_21_PIN, SS_22_PIN, SS_23_PIN, SS_24_PIN, SS_25_PIN,
// SS_26_PIN, SS_27_PIN, SS_28_PIN, SS_29_PIN, SS_30_PIN};

byte irPins[numofIR] = { // no longer use this
    IR_PIN1, IR_PIN2, IR_PIN3, IR_PIN4, IR_PIN5,
    IR_PIN6, IR_PIN7, IR_PIN8, IR_PIN9, IR_PIN10,
    IR_PIN11, IR_PIN12, IR_PIN13, IR_PIN14, IR_PIN15};

byte lastThingPos[numofIR] = {0};
//====================================================
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
MFRC522::MIFARE_Key key;
// SoftwareSerial esp32_Serial(ESP32_RX, ESP32_TX);

byte mode = 0;
unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
unsigned long lastTurnBuzzerOn = 0;
unsigned long lastCheckThing = 0;
unsigned long lastRead = 0;
//====================================================
void dump_byte_array(byte *buffer, byte bufferSize);
void modeRead();
void writeToTag(byte numofByte, byte *data);
byte *readFromTag(byte block, MFRC522 rfid);
//====================================================
/**
 * Initialize.
 */
void setup()
{

  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial)
    ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  Serial2.begin(9600);

  SPI.begin(); // Init SPI bus
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    pinMode(ssPins[reader], OUTPUT);
    dw(ssPins[reader], HIGH);
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  pinMode(DOOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(DOOR_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  for (int i = 0; i < 15; i++)
  {
    pinMode(irPins[i], INPUT);
  }
}
//====================================================
/**
 * Main loop.
 */
void loop()
{
  // check if the buzzer is on for 3 seconds
  if ((millis() - lastTurnBuzzerOn > 3000) && dr(BUZZER_PIN) == HIGH)
  {
    dw(BUZZER_PIN, LOW);
  }

  if (Serial2.available())
  {
    String data = Serial2.readStringUntil('\n');
    if (data.startsWith("Open"))
    {
      // open the door and ring the alarm for 3s
      digitalWrite(DOOR_PIN, HIGH);
      dw(BUZZER_PIN, HIGH);
      lastTurnBuzzerOn = millis();
    }
    else if (data.startsWith("Lock")) // lock the door
    {
      digitalWrite(DOOR_PIN, LOW);
    }
  }
  if (millis() - lastRead > 2000)
  {
    // modeRead();
    lastRead = millis();
  }

  // check the IR (this will be main)
  if (millis() - lastCheckThing > TIME_CHECK_THING)
  {
    String msg = "IR: ";
    for (int i = 0; i < numofIR; i++)
    {
      lastThingPos[i] = dr(irPins[i]);
      msg += String(lastThingPos[i]);
    }
    Serial2.println(msg);

    lastCheckThing = millis();
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void modeRead()
{
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    // Look for new cards or even old card remain at its position.
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial())
    {
      uidDec = 0;
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      for (byte i = 0; i < mfrc522[reader].uid.size; i++)
      {
        uidDecTemp = mfrc522[reader].uid.uidByte[i];
        uidDec = uidDec * 256 + uidDecTemp;
      }
      Serial.println(uidDec);
      // send uid to esp32
      String msg = "RFID: " + String(reader + 1) + "," + String(uidDec) + "," + String(FLOOR); // send if the thing is there or not
      Serial2.println(msg);

      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      // Halt PICC, stop reading the tag/card
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    }
    else
    {
      // to show that no card is read.
      String msg = "RFID: " + String(reader + 1) + "," + String(0) + "," + String(FLOOR); // send if the thing is there or not
      Serial2.println(msg);
      Serial.println(F("No card read"));
    }
    delay(50);
  }
}

void writeToTag(byte numofByte, byte *data)
{
  // check if any card is present in the rfid[0] and write to it
  if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
  {
    Serial.print(F("Reader "));
    Serial.print(0);
    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F(": Card UID:"));
    dump_byte_array(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522[0].PICC_GetType(mfrc522[0].uid.sak);
    Serial.println(mfrc522[0].PICC_GetTypeName(piccType));

    // Halt PICC, stop reading the tag/card
    mfrc522[0].PICC_HaltA();
    // Stop encryption on PCD
    mfrc522[0].PCD_StopCrypto1();
    // write to card
    byte block = 1;
    byte buffer[16];
    byte len = 16;
    for (byte i = 0; i < numofByte; i++)
    {
      buffer[i] = data[i];
    }
    byte status;
    status = mfrc522[0].MIFARE_Write(block, buffer, len);
    if (status == MFRC522::STATUS_OK)
    {
      Serial.println(F("Write data to card success"));
    }
    else
    {
      Serial.println(F("Write data to card failed"));
    }
  }
}
/**
 * Read data from tag which has been written before and being present in the matrix of rfids
 * @param block: block to read
 * @param rfid: the rfid which the tag is present
 * @return byte*: data read from tag
 */
byte *readFromTag(byte block, MFRC522 mfrc522)
{
  byte buffer[18];
  byte size = sizeof(buffer);

  // authenticate block
  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return NULL;
  }

  if (mfrc522.MIFARE_Read(block, buffer, &size) == MFRC522::STATUS_OK)
  {
    return buffer;
  }
  else
  {
    return NULL;
  }
}