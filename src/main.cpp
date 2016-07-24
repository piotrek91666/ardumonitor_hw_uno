#include <SPI.h>
#include <DHT22.h>
#include "address.h"

bool conn_error_state = 0;

DHT22 myDHT22(DHT22_PIN);
EthernetClient ethClient;

void server_connect(IPAddress ip, int port) {
  if (ethClient.connect(ip, port)) {
    Serial.println("conn OK.");
  } else {
    Serial.println("conn FAILED!");
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(ERROR_PIN, OUTPUT);

  // Configure DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("dhcp FAILED!");
    for (;;)
      ;
  }

  server_connect(dst_server, dst_port);
  //Serial.println(Ethernet.localIP());

}

void loop() {
  // 0 - pass
  // 1 - renewed fail
  // 2 - renewed success
  // 3 - rebind fail
  // 4 - rebind success

  int eth_status = Ethernet.maintain();
  if ((eth_status == 2) || (eth_status == 4)) {
    ;
  }

  if (ethClient.connected() == 0) {
    digitalWrite(ERROR_PIN, HIGH);
    Serial.println("conn RETRY.");
    ethClient.flush();
    ethClient.stop();
    server_connect(dst_server, dst_port);
  }
  else {
    digitalWrite(ERROR_PIN, LOW);
  }

  String json_output = String("");

  // Sensor000 - DHT22
  DHT22_ERROR_t errorCode;
  delay(5000);
  errorCode = myDHT22.readData();

  json_output += "{'sensor000':{'name':'DHT22'";
  json_output += ",'error':" + String(errorCode);

  if ((errorCode == DHT_ERROR_NONE) || (errorCode == DHT_ERROR_CHECKSUM)) {
    json_output += ",'temp_c':" + String(myDHT22.getTemperatureC());
    json_output += ",'humidity':" + String(myDHT22.getHumidity());
  }

  json_output += "}}";
  Serial.println(json_output);
  ethClient.println(json_output);
}
