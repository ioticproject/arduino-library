#ifndef IOT_CLIENT_H
#define IOT_CLIENT_H

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>

#include <functional>

#include "structs.hpp"

using std::string;

namespace iot {

class Client {
   private:
    HttpClient* httpClient;
    PubSubClient *mqttClient = NULL;
    start_t start_info;
    std::function<void(msg_t)> onMsgReceived;

    void post_req(string url, string body) {
        Serial.println((body + "\n\r at url " + url).c_str());

        httpClient->beginRequest();
        httpClient->post(url.c_str());
        httpClient->sendHeader("apiKey", start_info.apiKey);
        httpClient->sendHeader("Content-Type", "application/json");
        httpClient->sendHeader("Content-Length", body.length());
        httpClient->beginBody();
        httpClient->print(body.c_str());
        httpClient->endRequest();

        Serial.println(httpClient->responseStatusCode());
        Serial.println(httpClient->responseBody());
    }

    void sync_time() {
        httpClient->beginRequest();
        httpClient->get("/iot/sync");
        httpClient->sendHeader("apiKey", start_info.apiKey);
        httpClient->endRequest();

        String body = httpClient->responseBody();
        int time = body.toInt();
        String timeStr = String(time);
        Serial.println("Response time int: " + timeStr);
        setTime(time);
    }

   public:
    Client(start_t start_info, std::function<void(msg_t)> onMsgReceived)
        : start_info(start_info), onMsgReceived(onMsgReceived) {
    }

    void loop() { mqttClient->loop(); }

    bool init() {
        Serial.print("SSID: ");
        Serial.println(start_info.ssid);

        Serial.print("pass: ");
        Serial.println(start_info.pass);

        Serial.print("host: ");
        Serial.println(start_info.host);

        Serial.print("apiKey: ");
        Serial.println(start_info.apiKey);

        Serial.print("id: ");
        Serial.println(start_info.id);
        
        Serial.println("Connecting to wifi...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(start_info.ssid, start_info.pass);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        Serial.println();
        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());

        httpClient = new HttpClient(*(new WiFiClient()), start_info.host, 5000);
        mqttClient = new PubSubClient(*(new WiFiClient()));

        mqttClient->setServer(start_info.host, 1337);
        mqttClient->setCallback(
            [&](char topic[], byte *payload, unsigned int length) {
                char body[length + 1];
                strncpy(body, (char *)payload, length);
                body[length] = 0;

                DynamicJsonDocument doc(2 * length + 512);
                deserializeJson(doc, body);

                msg_t msg;
                strtok(topic, "/");
                msg.topic = string(strtok(NULL, "/"));
                msg.data.type = doc["type"].as<char *>();

                if (msg.data.type == "string") {
                    msg.data.value = doc["value"].as<char *>();
                } else if (msg.data.type == "int") {
                    msg.data.value = toString(doc["value"].as<int>());
                } else if (msg.data.type == "double") {
                    msg.data.value = toString(doc["value"].as<double>());
                } else if (msg.data.type == "json") {
                    msg.data.value = toString(doc["value"].as<JsonObject>());
                }

                msg.data.time = doc["time"].as<unsigned int>();
                
                Serial.println("Received " + String(msg.topic.c_str()) + ": " + body);

                this->onMsgReceived(msg);
            });

        while (!mqttClient->connect(start_info.apiKey)) {
            Serial.println("Connection failed.. retrying");
            delay(500);
        }

        Serial.println("Connected sucessfully!");
        sync_time();

        return true;
    }

    bool subscribe(string topic) {
        topic = string(start_info.id) + "/" + topic;
        Serial.println("Subscribed to " + String(topic.c_str()));
        return mqttClient->subscribe(topic.c_str());
    }

    void registerDevice(string deviceJson) {
        post_req("/iot/devices", deviceJson);
    }

    void publish(string topic, data_t data) {
        topic = string(start_info.id) + "/" + topic;

        DynamicJsonDocument doc(data.value.size() + 256);
        doc["type"] = data.type.c_str();

        if (data.type == "string") {
            doc["value"] = data.value.c_str();
        } else if (data.type == "int") {
            doc["value"] = fromString<int>(data.value);
        } else if (data.type == "double") {
            doc["value"] = fromString<double>(data.value);
        } else if (data.type == "json") {
            doc["value"] = fromString<DynamicJsonDocument>(data.value);
        }

        doc["time"] = data.time;

        String msg;
        serializeJson(doc, msg);

        Serial.println("Publish data " + String(topic.c_str()) + ": " + msg);

        mqttClient->publish(topic.c_str(), msg.c_str());
    }
};

}  // namespace iot

#endif