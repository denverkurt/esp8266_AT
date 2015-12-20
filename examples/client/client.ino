#include <SoftwareSerial.h>
#include <esp8266_AT.h>

//pin numbers of helper debug software serial
#define rx_pin 2
#define tx_pin 3

// replace value with access point name
#define ap_name "write access point name here";

// replace value with access point password
#define ap_pass "write access point password here";


SoftwareSerial debugSerial(rx_pin, tx_pin);
Esp8266AT esp8266AT(&Serial, &debugSerial);

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.setTimeout(8000);

  // set the data rate for the SoftwareSerial port
  debugSerial.begin(115200);
  debugSerial.setTimeout(8000);
}

boolean connected = false;

void loop() {
    if (!connected) {
        if (!esp8266AT.setup(ap_name, ap_pass)) {
            debugSerial.println("Connection failed, please reset");
            while (1);
        } else {
            connected = true;
        }
    }

    if (!esp8266AT.get("192.168.1.46", 1337, "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n", "HTTP/1.1 200 OK")) {
        debugSerial.println("Last GET was failed");
    }

   delay(5000);
}
