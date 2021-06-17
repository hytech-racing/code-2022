#pragma once

#include "drivers.h"

#ifdef DRIVER
    #if DRIVER == DAVID
        #define BRAKE_ACTIVE 800               // Threshold for brake pedal active  

        #define MIN_ACCELERATOR_PEDAL_1 1900    // Low accelerator implausibility threshold
        #define START_ACCELERATOR_PEDAL_1 2047  // Position to start acceleration
        #define END_ACCELERATOR_PEDAL_1 2445    // Position to max out acceleration
        #define MAX_ACCELERATOR_PEDAL_1 2600    // High accelerator implausibility threshold

        #define MIN_ACCELERATOR_PEDAL_2 2200    // Low accelerator implausibility threshold
        #define START_ACCELERATOR_PEDAL_2 2047  // Position to start acceleration
        #define END_ACCELERATOR_PEDAL_2 1650    // Position to max out acceleration
        #define MAX_ACCELERATOR_PEDAL_2 1500    // High accelerator implausibility threshold
    #else
        #error "Bad driver definition"
    #endif
#else
    #error "Driver required"
#endif

#define HALF_ACCELERATOR_PEDAL_1 ((START_ACCELERATOR_PEDAL_1 + END_ACCELERATOR_PEDAL_1)/2)
#define HALF_ACCELERATOR_PEDAL_2 ((START_ACCELERATOR_PEDAL_2 + END_ACCELERATOR_PEDAL_2)/2)
