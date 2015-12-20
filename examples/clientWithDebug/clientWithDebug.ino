/*
 * An example of using esp8266_AT library with debug serial port.
 *
 * please set your acces point name and pass
 * please set your esp8266 chip speed
 * please replace ip, port request and response messages appropriately
 */
#include <SoftwareSerial.h>
#include <esp8266_AT.h>

//pin numbers of helper debug software serial
#define rx_pin 2
#define tx_pin 3

// replace value with access point name
#define ap_name "write access point name here"

// replace value with access point password
#define ap_pass "write access point password here"

//speed of the both wifi and debug serial ports should be the same.
//see speed for your version esp8266
#define SERIAL_SPEED 115200
#define SERIAL_TIMEOUT 8000

SoftwareSerial debugSerial(rx_pin, tx_pin);
Esp8266AT esp8266AT(&Serial, &debugSerial);

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_SPEED);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.setTimeout(SERIAL_TIMEOUT);

  // set the data rate for the SoftwareSerial port
  debugSerial.begin(SERIAL_SPEED);
}

boolean connected = false;

void loop() {
    if (!connected) {
        if (!esp8266AT.setup(ap_name, ap_pass)) {
            debugSerial.println("Connection failed, will repeat in 5 sec");
        } else {
            connected = true;
        }
    }

    if (connected) {
        //please replace ip, port request and ressponse messages appropriately
        if (!esp8266AT.post("192.168.1.46", 1337, "/", "application/x-www-form-urlencoded","key=value", 200)) {
            debugSerial.println("Last GET was failed");
        }
    }

    delay(5000);
}
