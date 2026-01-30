#include <Modulino.h>
#include <Arduino_RouterBridge.h>

ModulinoLatchRelay relay;

bool relayState = false;

void onGesture(String gesture) {
  if (gesture == "thumbs_up") {
    if (!relayState) {
      relay.set();
      relayState = true;
    }
  }
  else if (gesture == "thumbs_down") {
    if (relayState) {
      relay.reset();
      relayState = false;
    }
  }
}

void setup() {
  Bridge.begin();
  Modulino.begin();
  relay.begin();

  Bridge.provide("gesture", onGesture);
}

void loop() {
  // Nothing needed here — event driven
}
