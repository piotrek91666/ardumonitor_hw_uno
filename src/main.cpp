#include <SPI.h>
#include <DHT22.h>
#include "address.h"

DHT22 myDHT22(DHT22_PIN);
EthernetClient ethClient;

String server_read(short bytes) {
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

void server_negotiate() {
  Serial.println("CNEG");
  char msg[13];

  // Server say Hello
  if (server_read(3).toInt() != 100) ethClient.stop();

  // Send your name
  sprintf(msg, "IAM:%s-%s", DEVICE_ID, FIRMWARE_VER);
  ethClient.print(msg);
  delay(250);

  // Confirmation from server
  if (server_read(3).toInt() != 120) ethClient.stop();
  ethClient.print("OK");
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
    delay(5000);
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

  // Delay time
  delay(5000);

  // Fetch data from sensors
  char s_data[64];

  // S00 - DHT22
  DHT22_ERROR_t dhtCode;
  dhtCode = myDHT22.readData();

  sprintf(s_data, "S000:['name':'DHT22','err':%i", dhtCode);

  if ((dhtCode == DHT_ERROR_NONE) || (dhtCode == DHT_ERROR_CHECKSUM)) {
    sprintf(s_data, "%s,'temp_c':%hi.%01hi,'humid':%hi.%01hi", s_data,
      myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt())%10,
      myDHT22.getHumidityInt()/10, abs(myDHT22.getHumidityInt())%10
      );
  }
  sprintf(s_data, "%s]", s_data);
  ethClient.print(s_data);

}
