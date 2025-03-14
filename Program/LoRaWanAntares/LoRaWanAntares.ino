#include <lorawan.h>

//ABP Credentials
/*
  Notes:
  - select ABP Activation on ANTARES
  - select inherit to generate your keys
  - nwkSKey: 32 digit hex, you can put 16 first digits by first 16 digits your access key and add 16 digits with 0 (ex : abcdef01234567890000000000000000)
  - appSKey: 32 digit hex, put 16 first digits by 0 and put last 16 digits by last 16 digit your access key (ex : 0000000000000000abcdef0123456789)
*/
const char *devAddr = "Lora-Device-Address"; // Replace with the Device Address that you have in the Antares console
const char *nwkSKey = "Network-Session-Key"; // Replace with the Network Session Key that you have in the Antares console
const char *appSKey = "Application-Session-Key"; // Replace with the Application Session Key that you have in the Antares console

const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

char myStr[50];
char outStr[255];
byte recvStatus = 0;
int channel;

//ESPLORA3 Connections
const sRFM_pins RFM_pins = {
    .CS = 26,
    .RST = 0,
    .DIO0 = 4,
    .DIO1 = 27,
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_A);

  // Set Data Rate
  lora.setDataRate(SF10BW125);

  // set channel to random
  lora.setChannel(MULTI);

  // Set TxPower to 15 dBi (max)
  lora.setTxPower1(15);

  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
}

void loop() {
  // Check interval overflow
  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    sprintf(myStr, "Lora Counter-%d", counter);

    Serial.print("Sending : ");
    Serial.println(myStr);

    lora.sendUplink(myStr, strlen(myStr), 0, 5);
    counter++;
    channel = lora.getChannel();
    Serial.print(F("Ch : "));    Serial.print(channel); Serial.println(" ");
  }

  // Check Lora RX
  lora.update();

  recvStatus = lora.readData(outStr);
  if (recvStatus) {
    int counter = 0;

    for (int i = 0; i < recvStatus; i++)
    {
      if (((outStr[i] >= 32) && (outStr[i] <= 126)) || (outStr[i] == 10) || (outStr[i] == 13))
        counter++;
    }
    if (counter == recvStatus)
    {
      Serial.print(F("Received String : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(char(outStr[i]));
      }
    }
    else
    {
      Serial.print(F("Received Hex : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(outStr[i], HEX); Serial.print(" ");
      }
    }
    Serial.println();
  }

}