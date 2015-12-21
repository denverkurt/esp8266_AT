/*
 * Esp8266_AT.cpp - library for Arduino to work with esp8266 wifi SoC wia AT commands
 * See README file in this directory for more documentation
 *
 * Author: Sergey Prilukin sprilukin@gmail.com
 */

#include "Arduino.h"
#include <Esp8266_AT.h>
#include <Stream.h>

Esp8266AT::Esp8266AT(Stream *stream, Stream *dbgStream) {
    this->_stream = stream;
    this->_dbgStream = dbgStream;
}

boolean Esp8266AT::setup(String accessPoint, String password) {

    if (!this->executeCommandAndWaitForResult(AT_WIFI_CONNECTION_MODE_STA, AT_RESPONSE_OK, true)) {
        return false;
    }

    String command = String(AT_AP_CONNECT) + "\"" + accessPoint + "\",\"" + password + "\"";
    return this->executeCommandAndWaitForResult(command, AT_RESPONSE_OK, true);
}

boolean Esp8266AT::executeCommandAndWaitForResult(String command, String expectedResult, boolean prln) {
    if (prln) {
        this->_stream->println(command);
    } else {
        this->_stream->print(command);
    }

    if (this->_dbgStream) {
        this->_dbgStream->print(command);
    }

    char expectedResultCharArray[expectedResult.length()];
    expectedResult.toCharArray(expectedResultCharArray, expectedResult.length());

    boolean result = this->_stream->find(expectedResultCharArray);

    if (this->_dbgStream) {
        this->_dbgStream->println(result ? ": OK" : ": Error");
    }

    this->_finishLastCommand();

    return result;
}

void Esp8266AT::executeCommandAndIgnoreResult(String command) {
    this->_stream->println(command);
    this->_finishLastCommand();
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
    if (!this->executeCommandAndWaitForResult(SINGLE_CONNECTION_MODE, AT_RESPONSE_OK, true)) {
        return false;
    }

    String startConnection = String(TCP_START_CONNECTION) + ",\"" + ip + "\"," + String(port);
    if (!this->executeCommandAndWaitForResult(startConnection, AT_RESPONSE_OK, true)) {
        return false;
    }

    if (!this->executeCommandAndWaitForResult(
            String(TCP_SEND_MESSAGE) + String(message.length()),
            TCP_SEND_START_PROMPT, true)) {
        this->executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION);
        return false;
    }

    boolean result = this->executeCommandAndWaitForResult(message, expectedResponse, false);
    this->executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION);

    return result;
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
