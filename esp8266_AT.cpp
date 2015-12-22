/*
 * Esp8266_AT.cpp - library for Arduino to work with esp8266 wifi SoC wia AT commands
 * See README file in this directory for more documentation
 *
 * Author: Sergey Prilukin sprilukin@gmail.com
 */

#include "Arduino.h"
#include <Esp8266_AT.h>
#include <Stream.h>
#include <string.h>

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

    String command = String(AT_AP_CONNECT) + "\"" + accessPoint + "\",\"" + password + "\"";
    return this->_executeCommandAndWaitForResult(command, AT_RESPONSE_OK, DELAY_FOR_WIFI_STARTUP);
}

boolean Esp8266AT::get(String ip, int port, String path) {
    String request = "GET " + path + " HTTP/1.0\r\n";
    request += "Host: " + ip + ":" + String(port);
    request += "\r\n\r\n";
    return this->_request(ip, port, request, HTTP_200_OK);
}

boolean Esp8266AT::post(String ip, int port, String path, String contentType, String payload, int expectedResponseCode) {
    String request = "POST " + path + " HTTP/1.0\r\n";
    request += "Host: " + ip + ":" + String(port) + "\r\n";
    request += "Content-Type: " + contentType + "\r\n";
    request += "Content-Length: " + String(payload.length()) + "\r\n\r\n";
    request += payload;

    String expectedResponse;
    switch (expectedResponseCode) {
        case 200:
            expectedResponse = HTTP_200_OK;
            break;
        case 201:
            expectedResponse = HTTP_201_CREATED;
            break;
        default:
            if (this->_dbgStream) {
                this->_dbgStream->println("Not implemented HTTP response code: " + String(expectedResponseCode));
            }
            return false;
    }

    return this->_request(ip, port, request, expectedResponse);
}

boolean Esp8266AT::_request(String ip, int port, String message, String expectedResponse) {
    if (!this->_executeCommandAndWaitForResult(SINGLE_CONNECTION_MODE, AT_RESPONSE_OK)) {
        return false;
    }

    String startConnection = String(TCP_START_CONNECTION) + ",\"" + ip + "\"," + String(port);
    if (!this->_executeCommandAndWaitForResult(startConnection, AT_RESPONSE_OK)) {
        return false;
    }

    if (!this->_executeCommandAndWaitForResult(
            String(TCP_SEND_MESSAGE) + String(message.length()), TCP_SEND_START_PROMPT)) {
        this->_executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION, MAX_TIMEOUT_FOR_AT_COMMANDS);
        return false;
    }

    boolean result = this->_executeCommandAndWaitForResult(message, expectedResponse, MAX_TIMEOUT_FOR_TCP_REQUEST, false, true);
    //this->_executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION, MAX_TIMEOUT_FOR_AT_COMMANDS);

    return result;
}

boolean Esp8266AT::_executeCommandAndWaitForResult(String command, String expectedResult) {
    this->_executeCommandAndWaitForResult(command, expectedResult, MAX_TIMEOUT_FOR_AT_COMMANDS);
}

boolean Esp8266AT::_executeCommandAndWaitForResult(String command, String expectedResult, unsigned int timeout) {
    this->_executeCommandAndWaitForResult(command, expectedResult, timeout, true, false);
}

boolean Esp8266AT::_executeCommandAndWaitForResult(String command, String expectedResult, unsigned int timeout, boolean prln, boolean readWholeResult) {

    this->_stream->print(command);

    if (prln) {
        this->_stream->println();
    }

    if (this->_dbgStream) {
        this->_dbgStream->print(command);
    }

    String result = this->_readStringFromTheStream(timeout);

    if (this->_dbgStream && result) {
        this->_dbgStream->println(result);
    }

    return (result != NULL) && (result.indexOf(expectedResult) > -1);
}

void Esp8266AT::_executeCommandAndIgnoreResult(String command, unsigned int timeout) {

    this->_stream->println(command);

    String result = this->_readStringFromTheStream(timeout);

    if (this->_dbgStream && result) {
        this->_dbgStream->println(result);
    }
}

String Esp8266AT::_readStringFromTheStream(unsigned int timeout) {
    this->_stream->setTimeout(timeout);
    return this->_stream->readString();
}

void Esp8266AT::_finishLastCommand() {
    //Just for debugging purposes to read whole response

    while (this->_stream->available()) {
        byte data = this->_stream->read();
        if (this->_dbgStream) {
            this->_dbgStream->write(data);
        }
    }
}
