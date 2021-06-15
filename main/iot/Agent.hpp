#ifndef IOT_AGENT_H
#define IOT_AGENT_H
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <deque>
#include <string>
#include <type_traits>

#include "DataConverter.hpp"
#include "Device.hpp"
#include "structs.hpp"

using std::string;

namespace iot {

template <class T>
class Agent : public AgentI {
   protected:
    const string tag;
    const string dataSpec;
    const uint8_t type;
    const std::vector<T> buffer;

   public:
    Agent(const uint8_t type, const string tag, const string dataSpec) : type(type), tag(tag), dataSpec(dataSpec) {}
    Agent(const uint8_t type, const string tag) : type(type), tag(tag), dataSpec("null") {}

    string getTag() override { return this->tag; }
    uint8_t getType() override { return this->type; }
    string getDataType() override { return detectType<T>(); }
    string getDataSpec() override { return this->dataSpec; }
};

template <class T>
class Actor : public Agent<T> {
   private:
    Actor(){};

    std::deque<data_t> buffer;

    void onDataReceived(data_t data) {
        buffer.push_front(data);
    }

   protected:
    Device* device = NULL;
    void connect(Device* device) override {
        this->device = device;
        device->subscribe(this->tag, [&](data_t data){
            onDataReceived(data);
        });
    }

   public:
    Actor(string tag) : Agent<T>(AGENT_ACTOR, tag){};
    Actor(string tag, string dataSpec) : Agent<T>(AGENT_ACTOR, tag, dataSpec){};

    bool available() { return !buffer.empty(); }

    Actor<T>& operator>>(T& value) {
        while (buffer.empty()) {
            delay(1000);
            device->loop();
        }

        value = fromString<T>(buffer.back().value);
        buffer.pop_back();
        return *this;
    }
};

template <class T>
class Sensor : public Agent<T> {
   private:
    Sensor(){};

   protected:
    Device* device = NULL;
    void connect(Device* device) override { this->device = device; }

   public:
    Sensor(string tag) : Agent<T>(AGENT_SENSOR, tag){};
    Sensor(string tag, string dataSpec) : Agent<T>(AGENT_SENSOR, tag, dataSpec){};

    Sensor<T>& operator<<(const T& value) {
        data_t data;
        data.time = now();
        data.type = detectType<T>();
        data.value = toString(value);
        this->device->publish(this->tag, data);
        return *this;
    }
};

}  // namespace iot
#endif