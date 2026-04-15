// #include <Arduino.h>
// #include <LoRa.h>
// #include <SPI.h>

// const int RADIO_CS = 17;
// const int RADIO_RESET = 20;
// const int RADIO_DIO0 = 5;
// const int RADIO_SCK = 18;
// const int RADIO_MISO = 16;
// const int RADIO_MOSI = 19;
// //const int SPI = 0;

// uint8_t myData[] = {0xDE, 0xAD, 0xBE, 0xEF};
// size_t dataLength = sizeof(myData);

// void onTxDone() {
//   Serial.println("Sent");
// }

// int led = LED_BUILTIN;
// void setup(){
//   pinMode(LED_BUILTIN, OUTPUT);
//   Serial.begin(115200);
//   SPI.setCS(RADIO_CS);
//   SPI.setSCK(RADIO_SCK);
//   SPI.setMISO(RADIO_MISO);
//   SPI.setMOSI(RADIO_MOSI);
//   while (!Serial); 
//   LoRa.setPins(RADIO_CS, RADIO_RESET, RADIO_DIO0);
//   SPI.begin();
//   LoRa.setSPI(SPI);
//   if (!LoRa.begin(915E6)) {
//     Serial.println("Starting LoRa failed");
//   }
//   LoRa.onTxDone(NULL);
//   Serial.println("LoRa Initialized on GPIO 16-19");
// }

// // TODO:
// // Find driver for radio
// // find way to read input from serial
// // read serial input from usb and send over radio
// // print any received radio waves
// void loop(){
//   digitalWrite(LED_BUILTIN, HIGH);
//   delay(500);
//   digitalWrite(LED_BUILTIN, LOW);
//   delay(500);

//   static unsigned long lastSend = 0;
//   if (millis() - lastSend > 5000) { // Send every 5 seconds
//     lastSend = millis();
    
//     int begin = LoRa.beginPacket();
//     Serial.println(begin);
//     LoRa.write(myData, sizeof(myData));
//     int end = LoRa.endPacket();
//     Serial.println(end);
//   }
// }





//send the 5 numbers
//packet mode: explicit
//1 byte of data

#include <SPI.h>
#include <LoRa.h>

#define LORA_CS     17
#define LORA_RESET  20
#define LORA_DIO0   5
#define LORA_DIO1   4

#define LORA_SCK    18
#define LORA_MISO   16
#define LORA_MOSI   19

uint8_t myData[] = {"a"};
bool transmitting = false;

char data[256];

void onTxDone() {
  Serial.println("TX Done");
  transmitting = false;
  LoRa.receive();
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;
  Serial.print("Received: ");

  while (LoRa.available()) {
    int incoming = LoRa.read();
    Serial.print(incoming);
    Serial.print(" ");
  }

  Serial.println();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while(!Serial);
  Serial.setTimeout(10);

  Serial.println("Starting LoRa");

  // Configure SPI pins
  SPI.setSCK(LORA_SCK);
  SPI.setMISO(LORA_MISO);
  SPI.setMOSI(LORA_MOSI);
  SPI.begin();

  // Configure LoRa pins
  LoRa.setPins(LORA_CS, LORA_RESET, LORA_DIO0);
  LoRa.setSPI(SPI);

  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa init failed");
    while (1);
  }

  Serial.println("LoRa init success");

  LoRa.onTxDone(onTxDone);
  Serial.println("LoRa Initialized on GPIO 16-19");
  LoRa.onReceive(onReceive);

  LoRa.receive();
}



void loop() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(500);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(500);
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "united") {
      if (transmitting) {
        Serial.println("Wait! Radio is still busy sending...");
      }

      else{
        Serial.println("Command recognized: Sending LoRa packet...");
        transmitting=true;
        LoRa.beginPacket();
        LoRa.write(myData, sizeof(myData));
        if (LoRa.endPacket(true)) {
          Serial.println("Async send started.");
        } 
        else {
          Serial.println("Error: Radio hardware rejected the packet.");
          transmitting=false;
          LoRa.receive();
        }
      }
    }
    else{
      Serial.println("Wrong input");
    }
  }
  delay(1);
}

// void loop() {
//   String input;
//   if (Serial.available()>=1){
//     input = Serial.readStringUntil('\n');
//   }
//   if (input && input=="united"){
//     LoRa.beginPacket();
//     LoRa.write(myData, sizeof(myData));
//     LoRa.endPacket(true);   // async transmit
//   }
//   // if (millis() - lastSend > 1000) {

//   //   lastSend = millis();

//   //   Serial.println("Sending packet");

//   //   LoRa.beginPacket();
//   //   LoRa.write(myData, sizeof(myData));
//   //   LoRa.endPacket(true);   // async transmit
//   // }
// }

