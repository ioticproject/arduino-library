#ifndef IOT_SERVER_H
#define IOT_SERVER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include <functional>
#include <string>

#include "structs.hpp"

using std::function;
using std::string;

namespace iot {
class Server {
   private:
    WiFiServer* server = NULL;
    function<void(start_t)> on_init;

   public:
    Server(string device_tag, function<void(start_t)> on_init)
        : on_init(on_init) {
        init(device_tag);
    }

    ~Server(){
        server->close();
        delete server;
    }

    void init(string device_tag) {
        string ssid = "Iotic (" + device_tag + ")";

        while (!WiFi.softAP(ssid.c_str(), "123321123")) {
            Serial.println("AP creation failed! Retrying in 2 seconds...");
            delay(2000);
        }

        Serial.println("AP created succesfully!");

        server = new WiFiServer(80);
        server->begin();
        Serial.printf("Web server started, open %s in a web browser\n",
                      WiFi.localIP().toString().c_str());
    }

    String prepare_resp() {
        String resp;
        resp.reserve(1024);  // prevent ram fragmentation
        resp =
            F("HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n"  // the connection will be closed after
                                       // completion of the response
              "\r\n"
              "{\"status\": \"ok\"}"
              "\r\n");

        return resp;
    }

    void loop() {
        WiFiClient client = server->available();

        if (client) {
            Serial.println("\n[Client connected]");
            while (client.connected()) {
                // read line by line what the client (web browser) is requesting
                if (client.available()) {
                    String line = client.readStringUntil('\r');
                    Serial.print(line);
                    // wait for end of client's request, that is marked with an
                    // empty line
                    if (line.length() == 1 && line[0] == '\n') {
                        client.println(prepare_resp());
                        break;
                    }
                }
            }

            string body = "";
            while (client.available()) {
                body += (char)client.read();
            }

            start_t start_info = parse_start_info(body);

            Serial.print("SSID: ");
            Serial.println(start_info.ssid);

            Serial.print("apiKey: ");
            Serial.println(start_info.apiKey);

            Serial.print("id: ");
            Serial.println(start_info.id);

            Serial.print("pass: ");
            Serial.println(start_info.pass);
            // close the connection:
            client.stop();
            Serial.println("[Client disconnected]");

            delay(2000);

            server->close();
            WiFi.softAPdisconnect(true);
            on_init(start_info);
        }
    }

   private:
    start_t parse_start_info(string start_str) {
        StaticJsonDocument<256 * 3> doc;
        deserializeJson(doc, start_str);
        start_t start;
        strcpy(start.apiKey, doc["apiKey"]);
        strcpy(start.id, doc["id"]);
        strcpy(start.ssid, doc["ssid"]);
        strcpy(start.pass, doc["pass"]);
        strcpy(start.host, doc["host"]);
        return start;
    }
};
}  // namespace iot

#endif