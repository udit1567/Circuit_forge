/*
  Link Status
  This sketch prints the ethernet link status. When the
  ethernet cable is connected the link status should go to "ON".
*/

#include <ZephyrEthernet.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  auto link = Ethernet.linkStatus();
  Serial.print("Link status: ");
  switch (link) {
    case Unknown:
      Serial.println("Unknown");
      break;
    case LinkON:
      Serial.println("ON");
      break;
    case LinkOFF:
      Serial.println("OFF");
      break;
  }
  delay(1000);
}
