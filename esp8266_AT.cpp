/*
 * Esp8266_AT.cpp - library for Arduino to work with esp8266 wifi SoC wia AT commands
 * See README file in this directory for more documentation
 *
 * Author: Sergey Prilukin sprilukin@gmail.com
 */

#include "Arduino.h"
#include <Esp8266_AT.h>
#include <Stream.h>

//wifi connection mode: AT+CWMODE=<mode>
//mode: 1 — STA, 2 — AP, 3 — BOTH

#define AT_WIFI_CONNECTION_MODE_STA "AT+CWMODE=1"
#define AT_WIFI_CONNECTION_MODE_AP "AT+CWMODE=2"
#define AT_WIFI_CONNECTION_MODE_BOTH "AT+CWMODE=3"

//wifi connect to AP command: AT+CWJAP="<ap name>","<password>"
#define AT_AP_CONNECT "AT+CWJAP="

//multiple connections mode AT+CIPMUX=<mode>
//mode: 0 - single connection, 1- multiple connection
#define SINGLE_CONNECTION_MODE "AT+CIPMUX=0"
#define MULTI_CONNECTION_MODE "AT+CIPMUX=1"

//tcp connection start: AT+CIPSTART="TCP","8.8.8.8",80
#define TCP_START_CONNECTION "AT+CIPSTART="
//start sending message over tcp
#define TCP_SEND_MESSAGE "AT+CIPSEND="
//prompt message after which we can start sending our request
#define TCP_SEND_START_PROMPT ">"
//close tcp connection
#define TCP_CLOSE_CONNECTION "AT+CIPCLOSE"

//AT OK response
#define AT_RESPONSE_OK "OK"

Esp8266AT::Esp8266AT(Stream* stream, Stream* dbgStream) {
    this->_stream = stream;
    this->_dbgStream = dbgStream;
}

boolean Esp8266AT::setup(String accessPoint, String password) {

    if (!this->executeCommandAndWaitForResult(AT_WIFI_CONNECTION_MODE_STA, AT_RESPONSE_OK)) {
        return false;
    }

    String command = String(AT_AP_CONNECT) + "\"" + accessPoint + "\",\"" + password + "\"";
    return this->executeCommandAndWaitForResult(command, AT_RESPONSE_OK);
}

boolean Esp8266AT::executeCommandAndWaitForResult(String command, String expectedResult) {
    this->_stream->println(command);
    this->_dbgStream->print(command);

    char expectedResultCharArray[expectedResult.length()];
    expectedResult.toCharArray(expectedResultCharArray, expectedResult.length());

    if (this->_stream->find(expectedResultCharArray)){
        this->_dbgStream->println(": OK");
        return true;
    }else{
        this->_dbgStream->println(": Error");
        return false;
    }
}

void Esp8266AT::executeCommandAndIgnoreResult(String command) {
    this->_stream->println(command);
    this->_dbgStream->println(command);
    while (this->_stream->available()) {
        this->_dbgStream->write(this->_stream->read());
    }
}

boolean Esp8266AT::get(String ip, int port, String request, String expectedResult) {
    if (!this->executeCommandAndWaitForResult(SINGLE_CONNECTION_MODE, AT_RESPONSE_OK)) {
        return false;
    }

    String message = String(TCP_START_CONNECTION) + "\"TCP\",\"" + ip + "\"," + String(port);
    if (!this->executeCommandAndWaitForResult(message, AT_RESPONSE_OK)) {
        return false;
    }

    if (!this->executeCommandAndWaitForResult(String(TCP_SEND_MESSAGE) + String(request.length()), TCP_SEND_START_PROMPT)) {
        this->executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION);
        return false;
    }

    if (!this->executeCommandAndWaitForResult(request, expectedResult)) {
        this->executeCommandAndIgnoreResult(TCP_CLOSE_CONNECTION);
        return false;
    }

    //Just for debugging purposes to read whole response
    while (this->_stream->available()) {
        this->_dbgStream->write(this->_stream->read());
    }
}
