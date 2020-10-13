/*
 * Dashboard_status.cpp - CAN message parser: Dashboard status message
 * Created Sept 2020.
 */

#include "HyTech_CAN.h"

/*  Blank Constructor for Dashboard_status
 *
 * Used to initialize instance of Dashboard_status with no data
 */

Dashboard_status::Dashboard_status() {
    message = {};
}

/* Constructor for Dashboard_status using a buffer
 *
 * Used to initialize instance of Dashboard_status with data
 * that's in an 8xbyte array (typically msg.buf)
 *
 * Param - Pass in buffer you are trying to initialize data from
 */

Dashboard_status::Dashboard_status(uint8_t buf[8]) {
    load(buf);
}

/* Constructor for Dashboard_status
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param (bool) - Board
 *     - Mark value
 * Param (bool) - Mark
 *     - Mode value
 * Param (bool) - MC_cycle
 *     - MC Cycle value
 * Param (bool) - Start
 *     - Start value
 * Param (bool) - Extra
 *     - Extra value
 */

Dashboard_status::Dashboard_status(bool mark, bool mode, bool mc_cycle, bool start, bool extra) {
    set_mark(mark);
    set_mode(mode);
    set_mc_cycle(mc_cycle);
    set_start(start);
    set_extra(extra);
}

/* Load from buffer & write to variable instance
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param - Pass in buffer you are trying to read from
 * Example: curDashboard_status.load(msg.buf);
 */

void Dashboard_status::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.mark), &buf[0], sizeof(bool));
    memcpy(&(message.mode), &buf[1], sizeof(bool));
    memcpy(&(message.mc_cycle), &buf[2], sizeof(bool));
    memcpy(&(message.start), &buf[3], sizeof(bool));
    memcpy(&(message.extra), &buf[4], sizeof(bool));
}

/* Write to buffer
 *
 * Used to copy data from instance of this class
 * to msg variable in microcontroller code
 *
 * Param - Pass in buffer you are trying to mod3ify
 * Example: curDashboard_status.write(msg.buf);
 */

void Dashboard_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.mark), sizeof(bool));
    memcpy(&buf[1], &(message.mode), sizeof(bool));
    memcpy(&buf[2], &(message.mc_cycle), sizeof(bool));
    memcpy(&buf[3], &(message.start), sizeof(bool));
    memcpy(&buf[4], &(message.extra), sizeof(bool));
}

/* Get functions
 *
 */

bool Dashboard_status::get_mark() {
    return message.mark;
}
bool Dashboard_status::get_mode() {
    return message.mode;
}
bool Dashboard_status::get_mc_cycle() {
    return message.mc_cycle;
}
bool Dashboard_status::get_start() {
    return message.start;
}
bool Dashboard_status::get_extra() {
    return message.extra;
}

/* Set functions
 *
 * Param (bool) - Variable to replace old data
 */

void Dashboard_status::set_mark(bool mark) {
    message.mark = mark;
}
void Dashboard_status::set_mode(bool mode) {
    message.mode = mode;
}
void Dashboard_status::set_mc_cycle(bool mc_cycle) {
    message.mc_cycle = mc_cycle;
}
void Dashboard_status::set_start(bool start) {
    message.start = start;
}
void Dashboard_status::set_extra(bool extra) {
    message.extra = extra;
}

/* Toggle Functions
 *
 */
void Dashboard_status::toggle_mark() {
    message.mark = !message.mark;
}
void Dashboard_status::toggle_mode() {
    message.mode = !message.mode;
}
void Dashboard_status::toggle_mc_cycle() {
    message.mc_cycle = !message.mc_cycle;
}
void Dashboard_status::toggle_start() {
    message.start = !message.start;
}
void Dashboard_status::toggle_extra() {
    message.extra = !message.extra;
}