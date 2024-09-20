#include <Arduino.h>
#include "Arduino_LED_Matrix.h"
#include "WiFiS3.h"

#include "MecatroUtils.h"
#include "Telemetry.h"

#define MAX_TELEMETRY_VARIABLES 100
unsigned int nTelemetryVariables = 0;
uint8_t telemetryBuffer[5 + 4 * MAX_TELEMETRY_VARIABLES] = {'@'};
String telemetryVariableNames[MAX_TELEMETRY_VARIABLES];
Stream *telemetryStream;

WiFiServer server(80);
WiFiClient telemetryClient;

ArduinoLEDMatrix matrix;
const uint32_t fullOn[] = {
	0xffffffff,
	0xffffffff,
	0xffffffff
};

namespace mecatro{
    // Private function: try to connect to the telemetry client, on success, send the telemetry header.
    bool tryConnectTelemetryClient(unsigned char const controlPeriodMs)
    {
      bool connected = false;
      telemetryClient = server.available();
      if (telemetryClient)
      {
        if (telemetryClient.available())
        {
            if (telemetryClient.read() == 's')
            {
              connected = true;
              telemetryStream = &telemetryClient;
            }
        }
      }

      if (Serial.available())
      {
        if (Serial.read() == 's')
        {
              connected = true;
              telemetryStream = &Serial;
        }
      }

      if (connected)
      {
        // Send desired update period.
        telemetryStream->println();
        telemetryStream->print("mecatro@");
        telemetryStream->print(1000 * controlPeriodMs);
        // Send header: @-separated list of names
        for (unsigned int i = 0; i < nTelemetryVariables; i++)
        {
          telemetryStream->print("@");
          telemetryStream->print(telemetryVariableNames[i]);
        }
        telemetryStream->println();
      }
      return connected;
    }

    void initTelemetry(char* const wifiSSID, char* const wifiPassword, unsigned int const numberOfVariables, String* variableNames, unsigned char const controlPeriodMs)
    {
      matrix.begin();
      matrix.loadFrame(fullOn);
      // Look to see if user send the 's' command already: in this case there is no need to start WiFi or wait any longer.
      if (Serial.available())
        if (Serial.read() == 's')
        {
          telemetryStream = &Serial;
          return;
        }
      // Hack for custom firmware: manually set baudrate at 2Mbps
      modem.begin();
      Serial2.print("AT+UART=2000000\r\n");
      Serial2.flush();
      // Give a bit of time for the ESP32 to switch frequency.
      delay(10);
      Serial2.begin(2000000);
      std::string res = "";
      modem.write(std::string(PROMPT(_SOFTRESETWIFI)),res, "%s" , CMD(_SOFTRESETWIFI));
      delay(5);
      modem.write(std::string(PROMPT(_SOFTRESETWIFI)),res, "%s" , CMD(_SOFTRESETWIFI));
      delay(5);

      // Setup WiFi
      WiFi.config(IPAddress(192,168,4,1));

      if (WiFi.beginAP(wifiSSID, wifiPassword) != WL_AP_LISTENING)
      {
          Serial.println("Creating access point failed.");
          while (true) ;;
      }
      server.begin();

      nTelemetryVariables = min(numberOfVariables, MAX_TELEMETRY_VARIABLES);
      for (int i = 0; i < nTelemetryVariables; i++)
        telemetryVariableNames[i] = variableNames[i];

      // Turn off led screen.
      matrix.clear();

      // Wait for connection and start signal
      while (!tryConnectTelemetryClient(controlPeriodMs))
        delay(10);
    }

    void log(unsigned int const variableId, float const variableValue)
    {
      // Simply store variable in buffer, telemetry in done in main loop.
      if (variableId < nTelemetryVariables)
        memcpy(telemetryBuffer + 5 + 4 * variableId, (uint8_t*)(&variableValue), 4);
    }

    void sendTelemetry()
    {
      // To limit bandwidth, telemetry consists of raw-data:
      // - '@' character as line start
      // - timestamp, microseconds, 4 bytes (uint32)
      // - each variable, over 4 bytes (float)
      auto m = micros();
      memcpy(telemetryBuffer + 1, (uint8_t*)(&tickTime), 4);
      telemetryStream->write(telemetryBuffer, 5 + 4 * nTelemetryVariables);
    }

    void recieveGains(unsigned char const nGains, float *gainsArray)
    {
      bool done = false;
      while (!done)
      {
        if (telemetryStream->available())
        {
            if (telemetryStream->read() != 0xFF)
              continue;
            // Read all gains.
            unsigned char message[4 * nGains + 1];
            for (int i = 0; i < 4 * nGains + 1; i++)
            {
              while (!telemetryStream->available())
                ;;
              message[i] = telemetryStream->read();
            }
            // Verify checksum
            unsigned char chk = 0;
            for (int i = 0; i < 4 * nGains; i++)
              chk += message[i];
            chk = 255 - chk;
            if (chk != message[4 * nGains])
              continue;
            // Decode floats
            for (int i = 0; i < nGains; i++)
            {
              gainsArray[i] = *reinterpret_cast<float*>(message + 4 * i);
            }
            done = true;
        }
      }
    }
};