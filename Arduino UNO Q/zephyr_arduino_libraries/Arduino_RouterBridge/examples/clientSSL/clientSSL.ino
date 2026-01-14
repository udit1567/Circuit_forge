#include <Arduino_RouterBridge.h>

static const char ca_cert[] = {
"-----BEGIN CERTIFICATE-----\n"
"MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
"VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
"A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
"WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
"IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
"AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
"QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
"HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
"BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
"9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
"p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
"-----END CERTIFICATE-----\n"
};

BridgeTCPClient<> client(Bridge);

void setup() {

  if (!Bridge.begin()) {
    while (true) {}
  }

  if (!Monitor.begin()) {
    while (true) {}
  }

}

void loop() {
  Monitor.println("\nStarting connection to server...");

  /* if you get a connection, report back via monitor: */
  if (client.connectSSL("arduino.tips", 443, ca_cert) < 0) {
    Monitor.println("unable to connect to server");
    return;
  }

  Monitor.println("connected to server");
  /* Make aHTTP request: */
  size_t w = client.println("GET /asciilogo.txt HTTP/1.1");
  w += client.println("Host: arduino.tips");
  w += client.println("User-Agent: Arduino");
  w += client.println("Connection: close");
  w += client.println();

  /* if there are incoming bytes available  from the server,
   * read them and print them:
   */
  while (client.connected()) {
    size_t len = client.available();
    if (len) {
      uint8_t buff[len];
      client.read(buff, len);
      Monitor.write(buff, len);
    }
    delay(0);
  }

  /* if the server's disconnected, stop the client: */
  Monitor.println();
  Monitor.println("disconnecting from server.");
  client.stop();
  delay(1000);
}
