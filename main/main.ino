#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SimpleDHT.h>
#include "iot/Agent.hpp"

using namespace iot;

int tempValue = 0;
int ledValue = 0;

Sensor<int> temp("temp");
Actor<int> led("led");
Device device("temperature and led", {&temp, &led});

void setup() {
    device.begin();
}

void loop() {
    device.loop();    
    temp << tempValue;

    if(led.available()){
      led >> ledValue;
    }

    //use tempValue and ledValue accordingly
}

