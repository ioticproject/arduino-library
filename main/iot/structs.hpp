#ifndef IOT_STRUCTS_H
#define IOT_STRUCTS_H
#include <cstdint>
#include <string>
#include <functional>
#define ID_SIZE 64

namespace iot {
typedef struct start_t {
    char apiKey[ID_SIZE];
    char id[ID_SIZE];   
    char host[ID_SIZE];
    char ssid[ID_SIZE];
    char pass[ID_SIZE];
} start_t;

typedef struct data_t {
    time_t time;
    std::string type;
    std::string value;
} data_t;


typedef struct subscriber_t {
    std::string topic;
    std::function<void(data_t)> callback;
} subscriber_t;

typedef struct msg_t {
    std::string topic;
    data_t data;
} msg_t;

}  // namespace iot

#endif