/*
 * An example of using esp8266_AT library which uses
 * RGB led to indicate of it's state.
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

// RGB led pins
#define redPin 11
#define greenPin 10
#define bluePin 9

Esp8266AT esp8266AT(&Serial);

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_SPEED);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.setTimeout(SERIAL_TIMEOUT);
}

boolean connected = false;

void loop() {

    if (!connected) {
        purple();

        if (!esp8266AT.setup(ap_name, ap_pass)) {
            red();
            while (1);
        } else {
            green();
            connected = true;
        }
    }

    blue();

    //please replace ip, port request and response messages appropriately
    if (!esp8266AT.get("192.168.1.46", 1337, "GET / HTTP/1.0\r\nHost: 192.168.1.46:1337\r\n\r\n", "HTTP/1.1 200 OK")) {
        red();
    } else {
        green();
    }

   delay(5000);
}

void red() {
    setColor(255, 0, 0);  // red
}

void green() {
    setColor(0, 255, 0);  // green
}

void blue() {
    setColor(0, 0, 255);  // blue
}

void purple() {
    setColor(80, 0, 80);  // purple
}

void setColor(int red, int green, int blue)
{
#ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
#endif
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}
