/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>

int redPin = 11;
int greenPin = 10;
int bluePin = 9;

SoftwareSerial mySerial(2, 3); // RX, TX

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
  executeCommandAndWaitForResult("AT+CWMODE=1", "OK", 1, -1, purple, red);
  executeCommandAndWaitForResult("AT+CWJAP=\"nord\",\"asusn18urouter\"", "OK", 2, -1, green, red);

  callServer("192.168.1.46", 1337, "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n", "HTTP/1.1 200 OK");
  delay(5000);  

    /*while (Serial.available()) {
      mySerial.write(Serial.read());     
    }
    while (mySerial.available()) {
      Serial.write(mySerial.read());     
    }*/
}

void callServer(String ip, int port, String cmd, char* expectedResult) {
  if (flag == 2) {
  executeCommandAndWaitForResult("AT+CIPMUX=0", "OK", 3, 2, blue, red);
  executeCommandAndWaitForResult("AT+CIPSTART=\"TCP\",\"" + ip + "\"," + String(port), "OK", 4, -4, blue, red);
  executeCommandAndWaitForResult("AT+CIPSEND=" + String(cmd.length()), ">", 5, -5, blue, red);
  executeCommandAndWaitForResult(cmd, expectedResult, 6, -6, green, red);

   if (flag == -4 || flag == -5 || flag == -6) {
      executeCommandAndIgnoreResult("AT+CIPCLOSE");
   } else if (flag == 6) {
    // just read full response
     while (Serial.available()) {
      mySerial.write(Serial.read());     
    }
 
   }

   flag = 2;
  }
}

void executeCommandAndWaitForResult(String command, char* okMessage, byte flagValue, byte errorFlagValue, void (*okColor)(), void (*failColor)()) {
  if (flag == flagValue - 1) {
    Serial.println(command);
    mySerial.print(command);
  
    if(Serial.find(okMessage)){
      (*okColor)();
     mySerial.println(": OK");
     flag = flagValue;     
    }else{
     failColor();
     mySerial.println(": Error");   
     flag = errorFlagValue;    
    }
  }
}

void executeCommandAndIgnoreResult(String command) {
    Serial.println(command);
    mySerial.println(command);
    while (Serial.available()) {
      mySerial.write(Serial.read());     
    }
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

void aqua() {
  setColor(0, 255, 255);  // aqua
}

void yellow() {
  setColor(120, 80, 0);  // yellow
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
