#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

const int RADIO_CS = 17;
const int RADIO_RESET = 20;
const int RADIO_DIO0 = 5;
const int RADIO_SCK = 18;
const int RADIO_MISO = 16;
const int RADIO_MOSI = 19;
//const int SPI = 0;

uint8_t myData[] = {0xDE, 0xAD, 0xBE, 0xEF};
size_t dataLength = sizeof(myData);

void onTxDone() {
  Serial.println("Sent");
}

int led = LED_BUILTIN;
void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  SPI.setCS(RADIO_CS);
  SPI.setSCK(RADIO_SCK);
  SPI.setMISO(RADIO_MISO);
  SPI.setMOSI(RADIO_MOSI);
  while (!Serial); 
  LoRa.setPins(RADIO_CS, RADIO_RESET, RADIO_DIO0);
  //MbedSPI spi0 = MbedSPI(0, 3, 2);
  //SPI.begin();
  LoRa.setSPI(SPI);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed");
  }
  LoRa.onTxDone(onTxDone);
  Serial.println("LoRa Initialized on GPIO 16-19");
}

// TODO:
// Find driver for radio
// find way to read input from serial
// read serial input from usb and send over radio
// print any received radio waves
void loop(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) { // Send every 5 seconds
    lastSend = millis();
    
    int begin = LoRa.beginPacket();
    Serial.println(begin);
    LoRa.write(myData, sizeof(myData));
    int end = LoRa.endPacket(true);
    Serial.println(end);
  }
}

