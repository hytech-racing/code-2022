/*
  DCU_status.cpp - HyTech Dashboard Control Unit CAN message: Status
  Created by Karvin Dassanayake, February 12, 2017.
 */

#include "HyTech17.h"

DCU_status::DCU_status() {
    message = {};
}

DCU_status::DCU_status(uint8_t buf[8]) {
  load(buf);
}

DCU_status::DCU_status(uint8_t btn_press_id, uint8_t light_active_1, uint8_t light_active_2, uint8_t rtds_state) {
    set_btn_press_id(btn_press_id);
    set_light_active_1(light_active_1);
    set_light_active_2(light_active_2);
    set_rtds_state(rtds_state);
}

