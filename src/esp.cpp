// #include <Arduino.h>
// #include <SoftwareSerial.h>
// #include <ESP8266WiFi.h>
// #include <ArduinoJson.h>
// #include <WebSocketsClient.h>
// #include <SocketIOclient.h>
// #include <WiFiManager.h>
// #include <SPI.h>
// #include <MFRC522.h>
// 
// //====================================================
// #define MEGA_RX D1
// #define MEGA_TX D2
// #define SS_PIN D8
// #define RST_PIN D0
// #define DEBUG
// // #define TIME_INTERVAL 2000
// #define TOPIC1 "/esp/new-card-found"
// #define TOPIC2 "/esp/got-person"
// #define TOPIC3 "/esp/pos-of-thing"
// #define SERVER "27.72.101.90"
// #define PORT 80
// //====================================================
// SoftwareSerial mega_Serial(MEGA_RX, MEGA_TX);
// SocketIOclient socketIO;
// MFRC522 rfid(SS_PIN, RST_PIN);
// MFRC522::MIFARE_Key key;
// //====================================================
// unsigned long uid = 0;
// int id = 0;
// int Floor = 0;
// byte nuidPICC[4];
// unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
// //====================================================
// void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
// {
//     String output = "";
//     output = String((char *)payload);
//     switch (type)
//     {
//     case sIOtype_DISCONNECT:
//         Serial.printf("[IOc] Disconnected!\n");
//         break;
//     case sIOtype_CONNECT:
//         Serial.printf("[IOc] Connected to url: %s\n", payload);
//         socketIO.send(sIOtype_CONNECT, "/");
//         break;
//     case sIOtype_EVENT:
//         Serial.printf("[IOc] get event: %s\n", payload);
//         if (output.indexOf("/web/got-person") > 0)
//         {
//             Serial.println("Got person");
//             mega_Serial.println("Open the door");
//         }
//         if (output.indexOf("/web/lock-door") > 0)
//         {
//             Serial.println("Lock door");
//             mega_Serial.println("Lock the door");
//         }
//         break;
//     case sIOtype_ACK:
//         Serial.printf("[IOc] get ack: %u\n", length);
//         break;
//     case sIOtype_ERROR:
//         Serial.printf("[IOc] get error: %u\n", length);
//         break;
//     case sIOtype_BINARY_EVENT:
//         Serial.printf("[IOc] get binary: %u\n", length);
//         break;
//     case sIOtype_BINARY_ACK:
//         Serial.printf("[IOc] get binary ack: %u\n", length);
//         break;
//     }
// }
// //-----------------------------------------------------------------------------
// void sendDataNewCardFound()
// {
//     DynamicJsonDocument doc(1024);
//     JsonArray array = doc.to<JsonArray>();
//     array.add(TOPIC1);
//     JsonObject data = array.createNestedObject();
//     data["uid"] = String(uid);
//     data["id"] = String(id);
//     data["floor"] = String(Floor);
//     String output;
//     serializeJson(doc, output);
//     socketIO.sendEVENT(output);
// }
// //-----------------------------------------------------------------------------
// void sendDataPosOfThing(String msg)
// {
//     DynamicJsonDocument doc(1024);
//     JsonArray array = doc.to<JsonArray>();
//     array.add(TOPIC3);
//     JsonObject data = array.createNestedObject();
//     data["posOfThing"] = String(msg);
//     String output;
//     serializeJson(doc, output);
//     socketIO.sendEVENT(output);
// }
// //-----------------------------------------------------------------------------
// void sentDataWhenCardFound()
// {
//     DynamicJsonDocument doc(1024);
//     JsonArray array = doc.to<JsonArray>();
//     array.add(TOPIC2);
//     JsonObject data = array.createNestedObject();
//     data["uid"] = String(uidDec);
//     String output;
//     serializeJson(doc, output);
//     socketIO.sendEVENT(output);
// }
// //-----------------------------------------------------------------------------
// /**
//    Helper routine to dump a byte array as hex values to Serial.
// */
// void printHex(byte *buffer, byte bufferSize)
// {
//     for (byte i = 0; i < bufferSize; i++)
//     {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], HEX);
//     }
// }
// 
// /**
//    Helper routine to dump a byte array as dec values to Serial.
// */
// void printDec(byte *buffer, byte bufferSize)
// {
//     for (byte i = 0; i < bufferSize; i++)
//     {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], DEC);
//     }
// }
// 
// //====================================================
// void setup()
// {
//     Serial.begin(9600);
//     mega_Serial.begin(9600);
//     //---------------------------------------------------------------------------
//     // init wifi
//     Serial.println("WiFi...");
//     WiFiManager wifiManager;
//     wifiManager.setTimeout(180); // wait 3 mins for config, or else use saved
//     if (!wifiManager.autoConnect("ESP8266"))
//     {
//         Serial.println("failed to connect and hit timeout");
//     }
//     else
//     {
//         Serial.println("Connected.");
//     }
//     //---------------------------------------------------------------------------
//     // server address, port and URL
//     socketIO.begin(SERVER, PORT, "/socket.io/?EIO=4"); // /socket.io/?EIO=4
//     socketIO.onEvent(socketIOEvent);
//     //---------------------------------------------------------------------------
//     // init rfid
//     SPI.begin();     // Init SPI bus
//     rfid.PCD_Init(); // Init MFRC522
//     Serial.println();
//     Serial.print(F("Reader :"));
//     rfid.PCD_DumpVersionToSerial();
//     for (byte i = 0; i < 6; i++)
//     {
//         key.keyByte[i] = 0xFF;
//     }
//     Serial.println();
//     Serial.println(F("This code scan the MIFARE Classic NUID."));
//     Serial.print(F("Using the following key:"));
//     printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
// }
// //====================================================
// void loop()
// {
//     socketIO.loop();
//     if (mega_Serial.available())
//     {
//         String data = mega_Serial.readStringUntil('\n');
//         if (data.startsWith("RFID: "))
//         {
//             int indexOfComma1 = data.indexOf(',');
//             int indexOfComma2 = data.indexOf(',', indexOfComma1 + 1);
//             id = data.substring(6, indexOfComma1).toInt();
//             uid = data.substring(indexOfComma1 + 1, indexOfComma2).toInt();
//             Floor = data.substring(indexOfComma2 + 1).toInt();
//             sendDataNewCardFound();
//         }
//         else if (data.startsWith("IR: "))
//         {
//             String msg = data.substring(4);
//             sendDataPosOfThing(msg);
//         }
//     }
// 
//     // check rfid
//     if (!rfid.PICC_IsNewCardPresent())
//     {
//         // if no card detected, lock the door
//         // mega_Serial.println("Lock");
//         return;
//     }
// 
//     // Verify if the NUID has been readed
//     if (!rfid.PICC_ReadCardSerial())
//         return;
// 
//     Serial.print(F("PICC type: "));
//     MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
//     Serial.println(rfid.PICC_GetTypeName(piccType));
// 
//     // Check is the PICC of Classic MIFARE type
//     if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
//         piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
//         piccType != MFRC522::PICC_TYPE_MIFARE_4K)
//     {
//         Serial.println(F("Your tag is not of type MIFARE Classic."));
//         return;
//     }
// 
//     // if the card is new, send data to server, else do nothing
//     if (rfid.uid.uidByte[0] != nuidPICC[0] ||
//         rfid.uid.uidByte[1] != nuidPICC[1] ||
//         rfid.uid.uidByte[2] != nuidPICC[2] ||
//         rfid.uid.uidByte[3] != nuidPICC[3])
//     {
//         uidDec = 0;
//         Serial.println(F("A new card has been detected."));
// 
//         // Store NUID into nuidPICC array
//         for (byte i = 0; i < 4; i++)
//         {
//             nuidPICC[i] = rfid.uid.uidByte[i];
//         }
// 
//         for (byte i = 0; i < rfid.uid.size; i++)
//         {
//             uidDecTemp = rfid.uid.uidByte[i];
//             uidDec = uidDec * 256 + uidDecTemp;
//         }
//         Serial.println(uidDec);
// 
//         Serial.println(F("The NUID tag is:"));
//         Serial.print(F("In hex: "));
//         printHex(rfid.uid.uidByte, rfid.uid.size);
//         Serial.println();
//         Serial.print(F("In dec: "));
//         printDec(rfid.uid.uidByte, rfid.uid.size);
//         Serial.println();
// 
//         sentDataWhenCardFound();
//     }
//     else
//         Serial.println(F("Card read previously."));
// 
//     // Halt PICC
//     rfid.PICC_HaltA();
// 
//     // Stop encryption on PCD
//     rfid.PCD_StopCrypto1();
// }
