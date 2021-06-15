#ifndef IOT_REPO_H
#define IOT_REPO_H

#include <EEPROM.h>

#include "structs.hpp"

#define SERVER_MODE 0
#define CLIENT_MODE 1

#define MODE_SIZE 1
#define PARAM_SIZE 64

#define MODE_ADDR 0
#define SSID_ADDR (MODE_ADDR + MODE_SIZE)
#define PASS_ADDR (SSID_ADDR + PARAM_SIZE)
#define ID_ADDR (PASS_ADDR + PARAM_SIZE)
#define API_KEY_ADDR (ID_ADDR + PARAM_SIZE)
#define HOST_ADDR (API_KEY_ADDR + PARAM_SIZE)
#define TOTAL_SIZE (MODE_SIZE + 5 * PARAM_SIZE)

namespace iot {
class Repo {
   public:
    int mode = 0;
    start_t start_info;

    Repo() { init(); }

    void init() {
        EEPROM.begin(TOTAL_SIZE);
        this->mode = EEPROM.read(MODE_ADDR);

        if (this->mode == CLIENT_MODE) {
            for (int i = 0; i < PARAM_SIZE; i++) {
                this->start_info.ssid[i] = EEPROM.read(SSID_ADDR + i);
                this->start_info.pass[i] = EEPROM.read(PASS_ADDR + i);
                this->start_info.id[i] = EEPROM.read(ID_ADDR + i);
                this->start_info.apiKey[i] = EEPROM.read(API_KEY_ADDR + i);
                this->start_info.host[i] = EEPROM.read(HOST_ADDR + i);
            }
        }
    }

    void save_start_info(start_t info) {
        this->mode = 1;
        this->start_info = info;

        EEPROM.write(MODE_ADDR, this->mode);
        for (int i = 0; i < PARAM_SIZE; i++) {
            EEPROM.write(SSID_ADDR + i, this->start_info.ssid[i]);
            EEPROM.write(PASS_ADDR + i, this->start_info.pass[i]);
            EEPROM.write(ID_ADDR + i, this->start_info.id[i]);
            EEPROM.write(API_KEY_ADDR + i, this->start_info.apiKey[i]);
            EEPROM.write(HOST_ADDR + i, this->start_info.host[i]);
        }

        EEPROM.commit();
        delay(500);
        ESP.restart();
    }

    bool isServerState() { return mode == SERVER_MODE; }

    bool isClientState() { return mode == CLIENT_MODE; }

    void reset() {
        for (int i = 0; i < TOTAL_SIZE; i++) {
            EEPROM.write(i, 0);
        }

        EEPROM.commit();
    }
};
}  // namespace iot

#endif