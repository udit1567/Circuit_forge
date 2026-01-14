#include <Arduino_RouterBridge.h>

IPAddress localhost(127, 0, 0, 1);
BridgeTCPServer<> server(Bridge, localhost, 5678);

void setup() {

    if (!Bridge.begin()) {
        while (true) {}
    }

    if (!Monitor.begin()) {
        while (true) {}
    }

    server.begin();

}

void loop() {

    BridgeTCPClient<> client = server.accept();

    if (client.connected() == 1){
        Monitor.print("client ");
        Monitor.print(client.getId());
        Monitor.println(" connected");
    }

    if (client) {
        Monitor.println("A client established a connection");
    }

    while (client.connected()) {
        size_t len = client.available();
        if (len) {
            Monitor.println("Message received from client");
            uint8_t buff[len];
            client.read(buff, len);
            Monitor.write(buff, len);
        }
    }

    server.disconnect();    // Disconnects the client server-side

}
