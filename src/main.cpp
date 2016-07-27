#include <SPI.h>
#include <DHT22.h>
#include "address.h"

DHT22 myDHT22(DHT22_PIN);
EthernetClient ethClient;

String server_read(EthernetClient etc_cli, short bytes) {
  char c;
  String msg = "";
  short i = 0;
  while (ethClient.available()) {
    c = ethClient.read();
    if (i < bytes) {
      msg += c;
      i++;
    }
  }
  return msg;
}

bool server_negotiate() {
  Serial.println("CNEG");
  char msg[13];

  // Server say Hello
  if (server_read(ethClient, 3).toInt() != 100) return false;

  // Send your name
  sprintf(msg, "IAM:%s-%s", DEVICE_ID, FIRMWARE_VER);
  ethClient.print(msg);
  delay(250);

  // Confirmation from server
  if (server_read(ethClient, 3).toInt() != 120) return false;
  ethClient.print("OK");
  return true;
}

void server_connect(IPAddress ip, int port) {
  // Connection 5 tries
  for (short t = 0; t <= 4; t++) {

    // Get IP / DHCP Renew
    Ethernet.maintain();

    // Let's connect
    if (ethClient.connect(ip, port)) {
      delay(1000);
      break;
    } else Serial.println("CFAIL");

    // If 5 tries are failed, hang program.
    if (t>=4) for (;;);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ERROR_PIN, OUTPUT);

  // Configure DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DFAIL");
    for (;;);
  }

  server_connect(dst_server, dst_port);
  server_negotiate();
}

void loop() {
  // Keep connection
  if (ethClient.connected() == 0) {
    digitalWrite(ERROR_PIN, HIGH);
    ethClient.flush();
    ethClient.stop();
    server_connect(dst_server, dst_port);
    server_negotiate();
  }
  else {
    digitalWrite(ERROR_PIN, LOW);
  }

  String json_output = "";

  // Sensor000 - DHT22
  DHT22_ERROR_t errorCode;
  delay(5000);
  errorCode = myDHT22.readData();

  json_output += "{'s000':{'name':'DHT22'";
  json_output += ",'err':" + String(errorCode);

  if ((errorCode == DHT_ERROR_NONE) || (errorCode == DHT_ERROR_CHECKSUM)) {
    json_output += ",'temp_c':" + String(myDHT22.getTemperatureC());
    json_output += ",'humidity':" + String(myDHT22.getHumidity());
  }

  json_output += "}}";
  //Serial.println(json_output);
  ethClient.print(json_output);

}
