/*
 * Esp8266_AT.cpp - library for Arduino to work with esp8266 wifi SoC wia AT commands
 * See README file in this directory for more documentation
 *
 * Author: Sergey Prilukin sprilukin@gmail.com
 */

#include "Arduino.h"
#include <Esp8266_AT.h>

Esp8266AT::Esp8266AT(Stream *stream) {
    this->_stream = stream;
    this->_dbgStream = NULL;
}

Esp8266AT::Esp8266AT(Stream *stream, Stream *dbgStream) {
    this->_stream = stream;
    this->_dbgStream = dbgStream;
}

boolean Esp8266AT::setup(String accessPoint, String password) {

    if (!this->_executeCommandAndWaitForResult(AT_WIFI_CONNECTION_MODE_STA, AT_RESPONSE_OK)) {
        return false;
    }

    String command = String(AT_AP_CONNECT) + "\"" + accessPoint + "\",\"" + password + "\"\r\n";
    return this->_executeCommandAndWaitForResult(command, AT_RESPONSE_OK, DELAY_FOR_WIFI_STARTUP);
}

boolean Esp8266AT::post(String ip, int port, String path, String contentType, String payload, String expectedResponse) {
    String request = "POST " + path + " HTTP/1.0\r\n"
    += "Host: " + ip + ":" + String(port) + "\r\n"
    += "Content-Type: " + contentType + "\r\n"
    += "Content-Length: " + String(payload.length()) + "\r\n\r\n"
    += payload;

    return this->_request(ip, port, request, expectedResponse);
}

boolean Esp8266AT::_request(String ip, int port, String message, String expectedResponse) {
    if (!this->_createTCPConnection(ip, port, message.length())) {
        return false;
    }

    boolean result = this->_executeCommandAndWaitForResult(message, expectedResponse, MAX_TIMEOUT_FOR_TCP_REQUEST);
    //this->_executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION, MAX_TIMEOUT_FOR_AT_COMMANDS);

    if (this->_dbgStream) {
        this->_dbgStream->println(result ? DBG_RESPONSE_OK : DBG_RESPONSE_ERROR);
    }

    return result;
}

boolean Esp8266AT::_createTCPConnection(String ip, int port, int messageLength) {
    if (!this->_executeCommandAndWaitForResult(SINGLE_CONNECTION_MODE, AT_RESPONSE_OK)) {
        return false;
    }

    String startConnection = String(TCP_START_CONNECTION) + ",\"" + ip + "\"," + String(port) + "\r\n";
    if (!this->_executeCommandAndWaitForResult(startConnection, AT_RESPONSE_OK)) {
        return false;
    }

    if (!this->_executeCommandAndWaitForResult(
            String(TCP_SEND_MESSAGE) + String(messageLength) + "\r\n", TCP_SEND_START_PROMPT)) {
        this->_executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION, MAX_TIMEOUT_FOR_AT_COMMANDS);
        return false;
    }

    return true;
}

boolean Esp8266AT::_executeCommandAndWaitForResult(String command, String expectedResult) {
    this->_executeCommandAndWaitForResult(command, expectedResult, MAX_TIMEOUT_FOR_AT_COMMANDS);
}

boolean Esp8266AT::_executeCommandAndWaitForResult(String command, String expectedResult, unsigned int timeout) {

    if (this->_dbgStream) {
        this->_dbgStream->println(DBG_COMMAND);
        this->_dbgStream->println(command);
    }

    this->_stream->print(command);

    this->_stream->setTimeout(timeout);
    char expectedResultCharArray[expectedResult.length()];
    expectedResult.toCharArray(expectedResultCharArray, expectedResult.length());

    if (this->_dbgStream) {
        this->_dbgStream->println();
        this->_dbgStream->print("_freeRam: ");
        this->_dbgStream->println(this->freeRam());
    }

    return this->_stream->find(expectedResultCharArray);
}

void Esp8266AT::_executeCommandAndIgnoreResult(String command, unsigned int timeout) {
    if (this->_dbgStream) {
        this->_dbgStream->println(DBG_COMMAND);
        this->_dbgStream->println(command);
    }

    this->_stream->print(command);

    while(this->_stream->available()) {
        char ch = this->_stream->read();
        if (this->_dbgStream) {
            this->_dbgStream->print(ch);
        }
    }
}

int Esp8266AT::freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}