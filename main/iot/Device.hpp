#ifndef IOT_DEVICE_H
#define IOT_DEVICE_H

#include <ArduinoJson.h>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "Client.hpp"
#include "Repo.hpp"
#include "Server.hpp"

#define AGENT_SENSOR 0
#define AGENT_ACTOR 1

using std::string;

namespace iot {

class Device;

class AgentI {
   protected:
    virtual void connect(Device *device) {}

   public:
    friend class Device;
    virtual string getTag(){};
    virtual uint8_t getType(){};
    virtual string getDataType();
    virtual string getDataSpec();
};

class Device {
   private:
    Repo repo;
    Server *server = NULL;
    Client *client = NULL;

    void onStartInfoReceived(start_t start_info) {
        repo.save_start_info(start_info);
        delete server;
    }

    void onMsgReceived(msg_t msg) {
        for (subscriber_t &subscriber : subscribers) {
            if (subscriber.topic == msg.topic) {
                subscriber.callback(msg.data);
            }
        }
    }

   public:
    const string tag;
    std::vector<AgentI *> agents;
    std::vector<subscriber_t> subscribers;

    Device(string tag, std::vector<AgentI *> agents)
        : tag(tag), agents(agents) {}

    void begin() {
        for (AgentI *agent : agents) {
            agent->connect(this);
        }

        Serial.println("Initializing device");

        if (repo.isServerState()) {
            server = new Server(tag, [&](start_t start_info) {
                this->onStartInfoReceived(start_info);
            });
        } else {
            Serial.println("Initializing client");
            client = new Client(repo.start_info,
                                [&](msg_t msg) { this->onMsgReceived(msg); });

            if (!client->init()) {
                repo.reset();
                ESP.restart();
            }

            client->registerDevice(getJson());

            for (subscriber_t &subscriber : subscribers) {
                client->subscribe(subscriber.topic);
            }
        }
    }

    void subscribe(string topic, function<void(data_t)> callback) {
        subscriber_t subscriber;
        subscriber.topic = topic;
        subscriber.callback = callback;
        subscribers.push_back(subscriber);
    }

    void publish(string topic, data_t data) { client->publish(topic, data); }

    void loop() {
        while (repo.isServerState()) {
            server->loop();
        }

        client->loop();
    }

    void reset(){
        repo.reset();
        ESP.restart();
    }

    string getJson() {
        DynamicJsonDocument doc(256 * (agents.size()));

        doc["name"] = this->tag.c_str();

        int sensorIt = 0;
        int actorIt = 0;

        for (AgentI *agent : agents) {
            String entity;
            int ind;
            if (agent->getType() == AGENT_ACTOR) {
                entity = "actors";
                ind = actorIt++;
            } else {
                entity = "sensors";
                ind = sensorIt++;
            }

            doc[entity][ind]["name"] = agent->getTag().c_str();
            doc[entity][ind]["dataType"] = agent->getDataType().c_str();
            doc[entity][ind]["type"] = agent->getDataSpec().c_str();
        }

        String json;
        serializeJson(doc, json);
        Serial.println(json);
        return json.c_str();
    }
};

};  // namespace iot

#endif
