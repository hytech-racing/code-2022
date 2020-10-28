/*
 * Dashboard_status.cpp - CAN message parser: Dashboard status message
 * Created Sept 2020.
 */

#include "HyTech_CAN.h"
#include <iostream>
#include <vector>
#include <string>
#include <gtest/gtest.h>
using namespace std;

int main()
{
    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

    for (const string& word : msg)
    {
        cout << word << " ";
    }
    cout << endl;
}
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

/* Constructor for Dashboard_status w/flags
 */

Dashboard_status::Dashboard_status(uint8_t btn_flags, uint8_t led_flags) {
    set_button_flags(btn_flags);
    set_led_flags(led_flags);
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

    memcpy(&message, buf, sizeof(message));

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

    memcpy(buf, &message, sizeof(message));

}

//Button get functions

uint8_t Dashboard_status::get_button_flags() {
    return message.button_flags;
}
bool Dashboard_status::get_mark_btn() {
    return message.button_flags & 0x1;
}
bool Dashboard_status::get_mode_btn() {
    return (message.button_flags & 0x2);
}
bool Dashboard_status::get_mc_cycle_btn() {
    return (message.button_flags & 0x4);
}
bool Dashboard_status::get_start_btn() {
    return (message.button_flags & 0x8);
}
bool Dashboard_status::get_extra_btn() {
    return (message.button_flags & 0x10);
}

//Button set functions

void Dashboard_status::set_button_flags(uint8_t flags){
    message.button_flags =  flags;
}
void Dashboard_status::set_mark_btn(bool mark_btn) {
    message.button_flags = (message.button_flags & 0xFE) | (mark_btn & 0x1); 
}
void Dashboard_status::set_mode_btn(bool mode_btn) {
    message.button_flags = (message.button_flags & 0xFD) | ((mode_btn & 0x1) << 1);
}
void Dashboard_status::set_mc_cycle_btn(bool mc_cycle_btn) {
    message.button_flags = (message.button_flags & 0xFB) | ((mc_cycle_btn & 0x1) << 2);
}
void Dashboard_status::set_start_btn(bool start_btn) {
    message.button_flags = (message.button_flags & 0xF7) | ((start_btn & 0x1) << 3);
}
void Dashboard_status::set_extra_btn(bool extra_btn) {
    message.button_flags = (message.button_flags & 0xEF) | ((extra_btn & 0x1) << 4);
}

/* button toggle Functions
 *
 */
void Dashboard_status::toggle_mark_btn() {
    message.button_flags ^= 0x1;
}
void Dashboard_status::toggle_mode_btn() {
    message.button_flags ^= 0x2;
}
void Dashboard_status::toggle_mc_cycle_btn() {
    message.button_flags ^= 0x4;
}
void Dashboard_status::toggle_start_btn() {
    message.button_flags ^= 0x8;
}
void Dashboard_status::toggle_extra_btn() {
    message.button_flags ^= 0x10;
}


//LED get functions

uint8_t Dashboard_status::get_led_flags() {
    return message.led_flags;
}
bool Dashboard_status::get_ams_led() {
    return message.led_flags & 0x1;
}
bool Dashboard_status::get_imd_led() {
    return (message.led_flags & 0x2);
}
uint8_t Dashboard_status::get_mode_led() {
    return (message.led_flags & 0xC) >> 2; //2 bits required to store mode
}
bool Dashboard_status::get_mc_error_led() {
    return (message.led_flags & 0x10);
}
uint8_t Dashboard_status::get_start_led() {
    return (message.led_flags & 0x60) >> 5;//2 bits required to store start
}

//LED set functions

void Dashboard_status::set_led_flags(uint8_t flags){
    message.led_flags = flags;
}
void Dashboard_status::set_ams_led(bool ams_led) {
    message.led_flags = (message.led_flags & 0xFE) | (ams_led & 0x1); 
}
void Dashboard_status::set_imd_led(bool imd_led) {
    message.led_flags = (message.led_flags & 0xFD) | ((imd_led & 0x1) << 1);
}
void Dashboard_status::set_mode_led(uint8_t mode_led) {
    message.led_flags = (message.led_flags & 0xF3) | ((mode_led & 0x3) << 2);
}
void Dashboard_status::set_mc_error_led(bool mc_error_led) {
    message.led_flags = (message.led_flags & 0xEF) | ((mc_error_led & 0x1) << 4);
}
void Dashboard_status::set_start_led(uint8_t start_led) {
    message.led_flags = (message.led_flags & 0x9F) | ((start_led & 0x3) << 5);
}

