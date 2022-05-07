#pragma once

#include "drivers.h"

#ifdef DRIVER
    #if DRIVER == DAVID
        #define BRAKE_ACTIVE 800                // Threshold for brake pedal active  

        #define MIN_ACCELERATOR_PEDAL_1 1280    // Low accelerator implausibility threshold
        #define START_ACCELERATOR_PEDAL_1 1300  // Position to start acceleration
        #define END_ACCELERATOR_PEDAL_1 1930    // Position to max out acceleration
        #define MAX_ACCELERATOR_PEDAL_1 2060    // High accelerator implausibility threshold

        #define MIN_ACCELERATOR_PEDAL_2 630    // Low accelerator implausibility threshold
        #define START_ACCELERATOR_PEDAL_2 650  // Position to start acceleration
        #define END_ACCELERATOR_PEDAL_2 960    // Position to max out acceleration
        #define MAX_ACCELERATOR_PEDAL_2 1060    // High accelerator implausibility threshold
    #else
        #error "Bad driver definition"
    #endif
#else
    #error "Driver required"
#endif

#define HALF_ACCELERATOR_PEDAL_1 ((START_ACCELERATOR_PEDAL_1 + END_ACCELERATOR_PEDAL_1)/2)
#define HALF_ACCELERATOR_PEDAL_2 ((START_ACCELERATOR_PEDAL_2 + END_ACCELERATOR_PEDAL_2)/2)

