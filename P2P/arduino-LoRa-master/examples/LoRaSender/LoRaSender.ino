//sample for ESPLORA3 

#include <SPI.h>
#include <LoRa.h>

#define ss 26 
#define rst 0
#define dio0 4

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(ss, rst, dio0);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(923E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSyncWord(0xF3);//Your Address

}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.print(". HALLO ESPLORA 3 ");
  LoRa.endPacket();

  counter++;

  delay(5000);
}
