/*
  Telnet client

  This sketch connects to a telnet server.
  You need a telnet server to test this.
*/

#include "ZephyrClient.h"
#include "ZephyrEthernet.h"

// The IP address will be dependent on your local network:
IPAddress ip(192, 168, 1, 177);

// Example: To get the IP address of telehack.com (an example telnet server), run in a terminal:
// ping telehack.com
// or
// nslookup telehack.com
// Then use the returned IP address in the code.

// Enter the IP address of the server you're connecting to:
IPAddress server(1, 1, 1, 1);
int port = 23;  // Telnet port

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet;
// if you're using Processing's ChatServer, use port 10002):
ZephyrClient client;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }

  // in Zephyr system check if Ethernet is ready before proceeding to initialize
  while (Ethernet.linkStatus() != LinkON) {
    Serial.println("Waiting for link on");
    delay(100);
  }

  // start the Ethernet connection:
  Ethernet.begin(ip);

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (client.connected()) {
      client.print(inChar);
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // do nothing:
    while (true) {
      delay(1);
    }
  }
}
