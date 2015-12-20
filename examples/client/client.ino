#include <SoftwareSerial.h>
#include <esp8266_AT.h>

int redPin = 11;
int greenPin = 10;
int bluePin = 9;

SoftwareSerial mySerial(2, 3); // RX, TX
Esp8266AT esp8266AT(&Serial, &mySerial);

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.setTimeout(8000);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(115200);
  mySerial.setTimeout(8000);
}

int flag = 0;

void loop() {
    if (!esp8266AT.setup("nord", "asusn18urouter")) {
        mySerial.println("Connection failed, please reset");
        while (1);
    }

    if (!esp8266AT.get("192.168.1.46", 1337, "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n", "HTTP/1.1 200 OK")) {
        mySerial.println("Last GET was failed");
    }

   delay(5000);
}
