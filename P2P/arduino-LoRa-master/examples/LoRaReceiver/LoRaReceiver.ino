//sample for ESPLORA3

#include <SPI.h>
#include <LoRa.h>

#define ss 26
#define rst 0
#define dio0 4

void setup() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(ss, rst, dio0);


  Serial.println("LoRa Receiver");
  if (!LoRa.begin(923E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSyncWord(0xF3);//Your Address

}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
