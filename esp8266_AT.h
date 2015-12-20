/*
 * Esp8266_AT.h - library for Arduino to work with esp8266 wifi SoC wia AT commands
 * See README file in this directory for more documentation
 *
 * Author: Sergey Prilukin sprilukin@gmail.com
 */

#ifndef Esp8266AT_h
#define Esp8266AT_h

#include "Arduino.h"
#include <Stream.h>

//uncomment to enable output to debug serial
//#define ESP8266_DEBUG

class Esp8266AT {
public:

    /**
     * Constructor
     * stream - serial port connected to esp8266 chip
     * dbgStream - serial port for debug purposes
     */
    Esp8266AT(Stream *stream, Stream *dbgStream);
    boolean executeCommandAndWaitForResult(String command, String expectedResult);
    void executeCommandAndIgnoreResult(String command);
    boolean setup(String accessPoint, String password);

    //HTTP methods emulation
    boolean get(String ip, int port, String path, String expectedResult);
private:
    Stream *_stream;
    Stream *_dbgStream;
};

#endif