#include <iostream>
#include <type_traits>

#include "../../Libraries/HyTech_CAN/HyTech_CAN.h"

using namespace std;

int main()
{
    MCU_status mcu_status = {};
    BMS_status bms_status = {};
    cout << is_standard_layout<MCU_status>::value
     << " " << is_trivial<MCU_status>::value 
     << " " << sizeof mcu_status << endl;

     cout << is_standard_layout<BMS_status>::value
     << " " << is_trivial<BMS_status>::value 
     << " " << sizeof bms_status << endl;
}