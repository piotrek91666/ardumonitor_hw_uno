This is a remote sensor sender based on Arduino + Ethernet Shield

You need create file src/address.h like:
```c
#ifndef ADDRESS_H
#define ADDRESS_H

//#include <SPI.h>
#include <Ethernet.h>
//#include <DHT22.h>

#define DHT22_PIN 2
#define ERROR_PIN 3

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
IPAddress dst_server(1, 1, 1, 1);
int dst_port = 999999;

#endif
```

Also for this example you must install Ethernet and DHT22 libraries.
