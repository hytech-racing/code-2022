#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <chrono>

#include "can_lib.h"
#include "../../../Libraries/HyTech17_Library/HyTech17.h"

void log_to_file(canframe_t *frame);

std::ofstream log_mc;
std::string base_log_dir = "/home/pi/logs/";
char timestr[20];

int main() {
    log_mc.open(base_log_dir + "logs_mc.txt", std::ios::out | std::ios::app);
    CAN green_bus;
    canframe_t *msg = (canframe_t*) malloc(sizeof(canframe_t));

    while (1) {
        if (green_bus.read(msg) > 0) {
            std::cout << "Error reading message or no message to read" << std::endl;
        } else {
            msg->can_id = msg->can_id & 0xFF;
            log_to_file(msg);
        }
    }

    free(msg);
    log_mc.close();
    return 0;
}

void log_to_file(canframe_t *msg) {
    if (msg->can_id == ID_MC_TORQUE_TIMER_INFORMATION) {
        std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (std::strftime(timestr, sizeof(timestr), "%T",
                    std::localtime(&timestamp))) {
            log_mc << timestr << ": ";
        }

        log_mc << std::hex << std::setfill('0') << std::setw(2) << (int) msg->can_id << ": ";
        for (int i = 0; i < msg->can_dlc; i++) {
            log_mc << std::hex << std::setfill('0') << std::setw(2) << (int) msg->data[i] << " ";
        }
        log_mc << std::endl;
    }
}