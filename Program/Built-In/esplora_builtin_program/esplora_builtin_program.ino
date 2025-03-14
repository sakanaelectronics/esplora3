#include <Arduino.h>
#include <lorawan.h>
#include <SPI.h>
#include <LoRa.h>

#define led 2

const sRFM_pins RFM_pins = {
    .CS = 26,
    .RST = 0,
    .DIO0 = 4,
    .DIO1 = 27,
};

/*
const char *devAddr = "9566412c";
const char *nwkSKey = "fe6d60137e6a87950000000000000000";
const char *appSKey = "0000000000000000f030e2b83c660e38";
*/

String devAddr;
String nwkSKey;
String appSKey;

char myStr[50];
byte outStr[255];
byte recvStatus = 0;
int port, channel, freq;
bool newmessage = false;

int step = 0;
int mode = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  //delay(1000);
  Serial.println("______________________________________");
  Serial.println("++++++++++++++++++++++++++++++++++++++");
  Serial.println();
  Serial.println("***  SAKANA ELECTRONICS  ***");
  Serial.println("* ----- ESPLORA 3 TESTING PROGRAM ----- *");
  delay(500);

  Serial.println("Mode : 1 (LoRaWAN), 2 (P2P as sender), 3 (P2P as receiver) ");
  while (step == 0)
  {
    digitalWrite(led, LOW);
    while (Serial.available())
    {
      digitalWrite(led, HIGH);

      String inString = Serial.readString();
      if (inString == "1")
      {
        Serial.println("Set to LoRaWAN");
        mode = 1;
      }

      else if (inString == "2")
      {
        Serial.println("Set to P2P as sender");
        mode = 2;
      }

      else if (inString == "3")
      {
        Serial.println("Set to P2P as receiver");
        mode = 3;
      }

      else
      {
        Serial.println("Invalid mode");
      }

      if (mode == 1 || mode == 2 || mode == 3)
      {
        step = 1;
      }
    }
  }

  if (mode == 1) // lorawan
  {
    while (step == 1)
    {
      Serial.println("LoRaWAN Mode");
      Serial.println("Class : C");
      Serial.println("Activasion mode : ABP");

      if (!lora.init())
      {
        Serial.println("RFM95 not detected");
        delay(5000);
        return;
      }
      // Set LoRaWAN Class change CLASS_A or CLASS_C
      lora.setDeviceClass(CLASS_C);

      // Set Data Rate
      lora.setDataRate(SF10BW125);

      // Set FramePort Tx
      lora.setFramePortTx(5);

      // set channel to random
      lora.setChannel(MULTI);

      // Set TxPower to 15 dBi (max)
      lora.setTxPower(15);

      step = 2;
    }

    Serial.println("Set Network Session Key: ");
    while (step == 2)
    {
      digitalWrite(led, LOW);

      while (Serial.available())
      {
        String inString = Serial.readString();
        if (inString.length() == 32)
        {
          digitalWrite(led, HIGH);
          Serial.println("Network Session Key set to: " + inString);
          nwkSKey = inString;
          step = 3;
        }
        else
        {
          Serial.println("Please input 32 digit Network Session Key");
          Serial.println("Set Network Session Key: ");
        }
      }
    }

    Serial.println("Set Application Session Key: ");

    while (step == 3)
    {
      digitalWrite(led, LOW);

      while (Serial.available())
      {
        digitalWrite(led, HIGH);

        String inString = Serial.readString();
        if (inString.length() == 32)
        {
          Serial.println("Application Session Key set to: " + inString);
          appSKey = inString;
          step = 4;
        }
        else
        {
          Serial.println("Please input 32 digit Application Session Key");
          Serial.println("Set Application Session Key: ");
        }
      }
    }

    Serial.println("Set Device Address: ");
    while (step == 4)
    {

      while (Serial.available())
      {
        digitalWrite(led, HIGH);

        String inString = Serial.readString();
        if (inString.length() == 8)
        {
          Serial.println("Device address set to: " + inString);
          devAddr = inString;
          step = 5;
        }
        else
        {
          Serial.println("Please input 8 digit device address");
          Serial.println("Set Device Address: ");
        }
      }
      digitalWrite(led, LOW);
    }

    lora.setNwkSKey(nwkSKey.c_str());
    lora.setAppSKey(appSKey.c_str());
    lora.setDevAddr(devAddr.c_str());

    Serial.println("Set LoRaWAN success");
  }

  if (mode == 2 || mode == 3)
  {
    LoRa.setPins(26, 0, 4);

    if (!LoRa.begin(923E6))
    {
      Serial.println("Starting LoRa failed!");
      while (1)
        ;
    }

    LoRa.setSyncWord(0xF3); // Your Address

    if (mode == 2)
      Serial.println(" ESPLORA3 in P2P mode as sender");

    if (mode == 3)
      Serial.println(" ESPLORA3 in P2P mode as receiver");
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  while (mode == 1)
  {

    if (step == 5)
    {

      Serial.println("Input your payload: ");
      step = 6;
    }

    if (step == 6)
    {
      digitalWrite(led, LOW);

      while (Serial.available())
      {

        digitalWrite(led, HIGH);
        String inString = Serial.readString();
        Serial.print("Sending: ");
        Serial.println(inString);
        sprintf(myStr, inString.c_str());
        lora.sendUplink(myStr, strlen(myStr), 0);
        port = lora.getFramePortTx();
        channel = lora.getChannel();
        freq = lora.getChannelFreq(channel);
        Serial.print(F("fport: "));
        Serial.print(port);
        Serial.print(" ");
        Serial.print(F("Ch: "));
        Serial.print(channel);
        Serial.print(" ");
        Serial.print(F("Freq: "));
        Serial.print(freq);
        Serial.println(" ");
        delay(2000);
        Serial.println("Payload send!");
        step = 5;
      }
      lora.update();
    }
  }

  while (mode == 2)
  {
    if (step == 1)
    {

      Serial.println("Input your payload: ");
      step = 2;
    }
    if (step == 2)
    {
      digitalWrite(led, LOW);

      while (Serial.available())
      {
        digitalWrite(led, HIGH);

        String inString = Serial.readString();
        Serial.println(inString);
        LoRa.beginPacket();
        LoRa.print(inString);
        LoRa.endPacket();
        Serial.println("Payload send!");
        step = 1;
      }
    }
  }
  while (mode == 3)
  {
    int packetSize = LoRa.parsePacket();
    digitalWrite(led, LOW);

    if (packetSize)
    {
      // received a packet
      Serial.print("Received packet '");

      // read packet
      while (LoRa.available())
      {
        digitalWrite(led, HIGH);
        Serial.print((char)LoRa.read());
      }

      // print RSSI of packet
      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
    }
  }
}