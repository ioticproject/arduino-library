#ifndef IOT_DATA_CONVERTER_H
#define IOT_DATA_CONVERTER_H

#include <ArduinoJson.h>

#include <string>

using std::string;

namespace iot {

template <class T>
string detectType() {
    if (std::is_same<T, int>::value) {
        return "int";
    } else if (std::is_same<T, double>::value) {
        return "double";
    } else if (std::is_same<T, string>::value) {
        return "string";
    } else {
        return "json";
    }
}

template <class T>
string toString(T value) {
    return "undefined";
}

template <>
string toString(string value) {
    return value;
}

template <>
string toString(int value) {
    return string(String(value).c_str());
}

template <>
string toString(double value) {
    return String(value).c_str();
}

template <>
string toString(JsonObject value) {
    String s;
    serializeJson(value, s);
    return s.c_str();
}

template <class T>
T fromString(string value) {
    return NULL;
}

template <class T>
T fromString(string type, string value) {
    return NULL;
}

template <>
string fromString(string value) {
    return value;
}

template <>
int fromString(string value) {
    return String(value.c_str()).toInt();
}

template <>
double fromString(string value) {
    return String(value.c_str()).toDouble();
}

template <>
DynamicJsonDocument fromString(string value) {
    DynamicJsonDocument doc(2 * value.size());
    deserializeJson(doc, value.c_str());
    return doc;
}

}  // namespace iot

#endif