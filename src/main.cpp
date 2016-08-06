#include <SPI.h>
#include <DHT22.h>
#include <Adafruit_BMP085.h>
#include "address.h"

DHT22 myDHT22(DHT22_PIN);
Adafruit_BMP085 myBMP180;
EthernetClient ethClient;
bool BMP180_stat = true;
//char s_data[64];
char ftos_fix[6];

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
  if (server_read(3).toInt() != 110) ethClient.stop();
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

  if (myBMP180.begin())
  {
    BMP180_stat = false;
  }
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
  delay(2000);
char s_data[64];
  // S000 - DHT22
  DHT22_ERROR_t dhtCode;
  dhtCode = myDHT22.readData();

  sprintf(s_data, "S000:{'name':'DHT22','err':%i", dhtCode);

  if ((dhtCode == DHT_ERROR_NONE) || (dhtCode == DHT_ERROR_CHECKSUM)) {
    sprintf(s_data, "%s,'temp_c':%hi.%01hi,'humid':%hi.%01hi", s_data,
      myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt())%10,
      myDHT22.getHumidityInt()/10, abs(myDHT22.getHumidityInt())%10
      );
  }
  sprintf(s_data, "%s};", s_data);
  ethClient.write(s_data, strlen(s_data));

  // S001 - BMP180
  sprintf(s_data, "S001:{'name':'BMP180','err':%i", BMP180_stat);
  if (!BMP180_stat) {
    dtostrf(myBMP180.readTemperature(), 5, 2, ftos_fix);
    sprintf(s_data, "%s,'temp_c':%s,'press_pa':%lu", s_data,
      ftos_fix, myBMP180.readPressure());
  }
  sprintf(s_data, "%s};", s_data);
  ethClient.write(s_data, strlen(s_data));

}
