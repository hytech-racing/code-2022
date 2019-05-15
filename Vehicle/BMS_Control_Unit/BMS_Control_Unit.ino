/*
 * HyTech 2019 BMS Control Unit
 * Init 2017-04-11
 * Configured for HV Board Rev 11
 * Monitors cell voltages and temperatures, sends BMS_OK signal to close Shutdown Circuit
 */

/*
 * Shutdown circuit notes:
 * 1. GLV control system latches shutdown circuit closed.
 * 2. AIR's close.
 * 3. High voltage is available to the motor controller, TSAL is lit.
 * 4. Any faults (IMD OKHS, BMS_OK, BSPD) will open shutdown circuit, opening AIR's.
 */

/*
 * Operation notes:
 * 1. BMS sensors can be powered at all times.
 * 2. Once Teensy gets power from external power lines, give BMS_OK signal.
 * 3. No need to check DC bus voltage, because all batteries read their true voltages at all times. (They are continuous with each other at all times, due to no relay.)
 * 4. Once temperatures go too high, current goes too high, or cell voltages go too high or too low, drive the BMS_OK signal low.
 */

/*
 * LTC6804 state / communication notes:
 * The operation of the LTC6804 is divided into two separate sections: the core circuit and the isoSPI circuit. Both sections have an independent set of operating states, as well as a shutdown timeout. See LTC6804 Datasheet Page 20.
 * When sending an ADC conversion or diagnostic command, wake up the core circuit by calling wakeup_sleep()
 * When sending any other command (such as reading or writing registers), wake up the isoSPI circuit by calling wakeup_idle().
 */

#include <ADC_SPI.h>
#include <Arduino.h>
// #include <EEPROM.h> TODO add EEPROM functionality so we can configure parameters over CAN
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <LT_SPI.h>
#include <LTC68042.h>
#include <Metro.h>

/*************************************
 * Begin general configuration
 *************************************/

/*
 * Set Board Version
 * Uncomment whichever board this code is being uploaded to
 * Used to set pins correctly and only enable features compatible with board
 */
//#define BOARD_VERSION_HYTECH_2018_HV_REV_4
#define BOARD_VERSION_HYTECH_2019_HV_REV_11

/*
 * Set Accumulator Version
 * If installing in an Accumulator, set the version here for BMS to ignore problematic sensor readings unique to each accumulator
 */
//#define ACCUMULATOR_VERSION_HYTECH_2018_ACCUMULATOR
#define ACCUMULATOR_VERSION_HYTECH_2019_ACCUMULATOR

/*
 * Set Bench Test Mode
 * Set to true to place BMS in Bench Test Mode, set to false to disable
 * When in Bench Test Mode, the BMS will automatically ignore ICs that did not respond at startup
 */
#define MODE_BENCH_TEST false

/*
 * Set Charge Override Mode
 * Set to true to place BMS in Charge Override Mode, set to false to disable
 * When the BMS is in Charge Override Mode, it will balance without checking for the presence of the Charger ECU on CAN bus or a high signal on the Shutdown Circuit
 * This mode is useful when bench testing
 */
#define MODE_CHARGE_OVERRIDE false

/*
 * Set ADC Ignore Mode
 * Set to true to place BMS in ADC Ignore Mode, set to false to disable
 * When the BMS is in ADC Ignore Mode, it will not use data received from the ADC for determining faults, or for restricting cell balancing
 */
#define MODE_ADC_IGNORE true

/*************************************
 * End general configuration
 *************************************/

/*
 * Pin definitions
 */
#ifdef BOARD_VERSION_HYTECH_2018_HV_REV_4 // 2018 HV Board rev4
#define ADC_CS 9
#define BMS_OK A8
#define LED_STATUS 7
#define LTC6820_CS 10
#define WATCHDOG A0
#endif

#ifdef BOARD_VERSION_HYTECH_2019_HV_REV_11 // 2019 HV Board rev11
#define ADC_CS 9
#define BMS_OK A1
#define LED_STATUS 7
#define LTC6820_CS 10
#define WATCHDOG A0
#endif

/*
 * Constant definitions
 */
#define TOTAL_IC 8                      // Number of ICs in the system
#define CELLS_PER_IC 9                  // Number of cells per IC
#define THERMISTORS_PER_IC 3            // Number of cell thermistors per IC
#define PCB_THERM_PER_IC 2              // Number of PCB thermistors per IC
#define IGNORE_FAULT_THRESHOLD 10       // Number of fault-worthy values to read in succession before faulting
#define CURRENT_FAULT_THRESHOLD 5       // Number of fault-worthy electrical current values to read in succession before faulting
#define SHUTDOWN_HIGH_THRESHOLD 1500    // Value returned by ADC above which the shutdown circuit is considered powered (balancing not allowed when AIRs open)
#define BALANCE_LIMIT_FACTOR 3          // Reciprocal of the cell balancing duty cycle (3 means balancing can happen during 1 out of every 3 loops, etc)
#define COULOUMB_COUNT_INTERVAL 10000   // Microseconds between current readings

/*
 * Current Sensor ADC Channel definitions
 */
#define CH_CUR_SENSE  0
#define CH_TEMP_SENSE 1
#define CH_SHUTDOWN_G 3
#define CH_5V         4
#define CH_SHUTDOWN_H 5

/*
 * Timers
 */
Metro timer_can_update_fast = Metro(100);
Metro timer_can_update_slow = Metro(1000);
Metro timer_process_cells_fast = Metro(100);
Metro timer_process_cells_slow = Metro(1000);
Metro timer_watchdog_timer = Metro(250);
Metro timer_charge_enable_limit = Metro(30000, 1); // Don't allow charger to re-enable more than once every 30 seconds
Metro timer_charge_timeout = Metro(1000);

/*
 * Interrupt timers
 */
//IntervalTimer current_timer;

/*
 * Global variables
 */
uint16_t voltage_cutoff_low = 29800; // 2.9800V
uint16_t voltage_cutoff_high = 42000; // 4.2000V
uint16_t total_voltage_cutoff = 30000; // 300.00V
uint16_t discharge_current_constant_high = 22000; // 220.00A
int16_t charge_current_constant_high = -11000; // 110.00A
uint16_t charge_temp_cell_critical_high = 4400; // 44.00C
uint16_t discharge_temp_cell_critical_high = 6000; // 60.00C
uint16_t onboard_temp_balance_disable = 6000;  // 60.00C
uint16_t onboard_temp_balance_reenable = 5000; // 50.00C
uint16_t onboard_temp_critical_high = 7000; // 70.00C
uint16_t temp_critical_low = 0; // 0C
uint16_t voltage_difference_threshold = 150; // 0.0150V

uint8_t total_count_cells = CELLS_PER_IC * TOTAL_IC; // Number of non-ignored cells (used for calculating averages)
uint8_t total_count_cell_thermistors = THERMISTORS_PER_IC * TOTAL_IC; // Number of non-ignored cell thermistors (used for calculating averages)
uint8_t total_count_pcb_thermistors = PCB_THERM_PER_IC * TOTAL_IC; // Number of non-ignored PCB thermistors (used for calculating averages)

uint8_t error_flags_history = 0; // Persistently stores fault flags until ECU restart
uint8_t consecutive_faults_overvoltage = 0; // Keeps track of how many consecutive faults occured
uint8_t consecutive_faults_undervoltage = 0;
uint8_t consecutive_faults_total_voltage_high = 0;
uint8_t consecutive_faults_thermistor = 0;
uint8_t consecutive_faults_current = 0;

volatile uint32_t total_charge = 0; // Total incoming coulombs since ECU powered // TODO make these better names
volatile uint32_t total_discharge = 0; // Total outgoing coulombs since ECU powered
uint32_t total_charge_copy;
uint32_t total_discharge_copy;

uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Numbers are stored in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages for each IC in this order: [Cell Term 1] [Cell Therm 2] [Cell Therm 3] [Onboard Therm 1] [Onboard Therm 2] [Voltage reference]

int8_t ignore_cell[TOTAL_IC][CELLS_PER_IC]; // Cells to be ignored for under/overvoltage and balancing
int8_t ignore_pcb_therm[TOTAL_IC][PCB_THERM_PER_IC]; // PCB thermistors to be ignored
int8_t ignore_cell_therm[TOTAL_IC][THERMISTORS_PER_IC]; // Cell thermistors to be ignored

/*!<
  The tx_cfg[][6] store the LTC6804 configuration data that is going to be written
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:

 |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

*/
uint8_t tx_cfg[TOTAL_IC][6]; // data defining how data will be written to daisy chain ICs.

/*!<
  the rx_cfg[][8] array stores the data that is read back from a LTC6804
  The configuration data for each IC is stored in blocks of 8 bytes. Below is an table illustrating the array organization:

|rx_config[0][0]|rx_config[0][1]|rx_config[0][2]|rx_config[0][3]|rx_config[0][4]|rx_config[0][5]|rx_config[0][6]  |rx_config[0][7] |rx_config[1][0]|rx_config[1][1]|  .....    |
|---------------|---------------|---------------|---------------|---------------|---------------|-----------------|----------------|---------------|---------------|-----------|
|IC1 CFGR0      |IC1 CFGR1      |IC1 CFGR2      |IC1 CFGR3      |IC1 CFGR4      |IC1 CFGR5      |IC1 PEC High     |IC1 PEC Low     |IC2 CFGR0      |IC2 CFGR1      |  .....    |
*/
uint8_t rx_cfg[TOTAL_IC][8];

/**
 * CAN Variables
 */
FlexCAN CAN(500000);
const CAN_filter_t can_filter_ccu_status = {0, 0, ID_CCU_STATUS}; // Note: If this is passed into CAN.begin() it will be treated as a mask. Instead, pass it into CAN.setFilter(), making sure to fill all slots 0-7 with duplicate filters as necessary
static CAN_message_t rx_msg;
static CAN_message_t tx_msg;

/**
 * ADC Declaration
 */
ADC_SPI ADC(ADC_CS);

/**
 * BMS State Variables
 */
BMS_detailed_voltages bms_detailed_voltages[TOTAL_IC][3];
BMS_status bms_status;
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[TOTAL_IC];
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[TOTAL_IC];
BMS_voltages bms_voltages;
BMS_balancing_status bms_balancing_status[(TOTAL_IC + 3) / 4]; // Round up TOTAL_IC / 4 since data from 4 ICs can fit in a single message
BMS_coulomb_counts bms_coulomb_counts;
bool fh_watchdog_test = false; // Initialize test mode to false - if set to true the BMS stops sending pulse to the watchdog timer in order to test its functionality
bool watchdog_high = true; // Initialize watchdog signal - this alternates every loop
uint8_t balance_offcycle = 0; // Tracks which loops balancing will be disabled on
bool charge_mode_entered = false; // Used to enter charge mode immediately at startup instead of waiting for timer

void setup() {
    ADC.read_adc(0); // TODO isoSPI doesn't work until some other SPI gets called. This is a placeholder until we fix the problem
    pinMode(BMS_OK, OUTPUT);
    pinMode(LED_STATUS, OUTPUT);
    pinMode(LTC6820_CS, OUTPUT);
    pinMode(WATCHDOG, OUTPUT);
    digitalWrite(BMS_OK, HIGH);
    digitalWrite(WATCHDOG, watchdog_high);

    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication
    for (int i = 0; i < 8; i++) { // Fill all filter slots with Charger Control Unit message filter (CAN controller requires filling all slots)
        CAN.setFilter(can_filter_ccu_status, i);
    }

    /* Configure CAN rx interrupt */
    /*interrupts();
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
    attachInterruptVector(IRQ_CAN_MESSAGE,parse_can_message);
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;*/
    /* Configure CAN rx interrupt */

    delay(100);
    Serial.println("CAN system and serial communication initialized");

    bms_status.set_state(BMS_STATE_DISCHARGING);
    if (MODE_CHARGE_OVERRIDE) { // Configure Charge Override Mode if enabled
        bms_status.set_state(BMS_STATE_CHARGING);
        digitalWrite(LED_STATUS, HIGH);
    }

    /*// Set up current-measuring timer
    current_timer.priority(255); // Priority range 0-255, 128 as default
    total_charge = 0;
    total_discharge = 0;
    current_timer.begin(integrate_current, COULOUMB_COUNT_INTERVAL);*/

    /* Initialize the ic/group IDs for detailed voltage, temperature, and balancing CAN messages */
    for (int i = 0; i < TOTAL_IC; i++) {
        for (int j = 0; j < 3; j++) {
            bms_detailed_voltages[i][j].set_ic_id(i);
            bms_detailed_voltages[i][j].set_group_id(j);
        }
        bms_detailed_temperatures[i].set_ic_id(i);
    }
    for (int i = 0; i < ((TOTAL_IC + 3) / 4); i++) {
        bms_balancing_status[i].set_group_id(i);
    }

    /* Ignore cells or thermistors for bench testing */
    // DEBUG Code for testing cell packs | Example:
    // for (int i=0; i<4; i++) {
    //     for (int j=0; j<9; j++) {
    //         ignore_cell[i][j] = true; // Ignore ICs 0-3
    //         total_count_cells--; // Decrement cell count (used for calculating averages)
    //     }
    // }
    // DEBUG insert cell thermistors to ignore here | Example:
    // ignore_cell_therm[5][1] = true; // Ignore IC 5 cell thermistor 1
    // total_count_cell_thermistors--; // Decrement cell thermistor count (used for calculating averages)
    // DEBUG insert PCB thermistors to ignore here | Example:
    // ignore_pcb_therm[2][0] = true; // Ignore IC 2 pcb thermistor 0
    // total_count_pcb_thermistors--; // Decrement pcb thermistor count (used for calculating averages)

    /* Ignore cells or thermistors in 2018 accumulator */
    #ifdef ACCUMULATOR_VERSION_HYTECH_2018_ACCUMULATOR
    ignore_cell_therm[6][2] = true; // Ignore IC 6 cell thermistor 2 due to faulty connector
    total_count_cell_thermistors -= 1;
    #endif

    /* Ignore cells or thermistors in 2019 accumulator */
    #ifdef ACCUMULATOR_VERSION_HYTECH_2019_ACCUMULATOR
    #endif

    /* Set up isoSPI */
    initialize(); // Call our modified initialize function instead of the default Linear function
    init_cfg(); // Initialize and write configuration registers to LTC6804 chips

    /* Bench test mode: check which ICs are online at startup and ignore cells from disconnected ICs */
    if (MODE_BENCH_TEST) {
        Serial.println("\nBench Test Mode: Ignoring all ICs which do not respond at startup");
        LTC6804_rdcfg(TOTAL_IC, rx_cfg); // Read back configuration registers that we just initialized
        for (int i=0; i < TOTAL_IC; i++) { // Check whether checksum is valid
            int calculated_pec = pec15_calc(6, &rx_cfg[i][0]);
            int received_pec = (rx_cfg[i][6] << 8) | rx_cfg[i][7];
            if (calculated_pec != received_pec) { // IC did not respond properly - ignore cells and thermistors
                Serial.print("Ignoring IC ");
                Serial.println(i);
                for (int j = 0; j < CELLS_PER_IC; j++) {
                    ignore_cell[i][j] = true;
                }
                total_count_cells -= CELLS_PER_IC; // Adjust cell count (used for calculating averages)
                for (int j = 0; j < THERMISTORS_PER_IC; j++) {
                    ignore_cell_therm[i][j] = true;
                }
                total_count_cell_thermistors -= THERMISTORS_PER_IC; // Adjust cell thermistor count (used for calculating averages)
                for (int j = 0; j < PCB_THERM_PER_IC; j++) {
                    ignore_pcb_therm[i][j] = true;
                }
                total_count_pcb_thermistors -= PCB_THERM_PER_IC; // Adjust cell pcb thermistor count (used for calculating averages)
            }
        }
        Serial.println();
    }
    
    Serial.println("Setup Complete!");
}

// TODO Implement Coulomb counting to track state of charge of battery.

/*
 * Main BMS Control Loop
 */
void loop() {
    parse_can_message();

    if (timer_charge_timeout.check() && bms_status.get_state() > BMS_STATE_DISCHARGING && !MODE_CHARGE_OVERRIDE) { // 1 second timeout - if timeout is reached, disable charging
        Serial.println("Disabling charge mode - CCU timeout");
        bms_status.set_state(BMS_STATE_DISCHARGING);
        digitalWrite(LED_STATUS, LOW);
    }

    if (timer_process_cells_fast.check()) {}

    if (timer_process_cells_slow.check()) {
        process_voltages(); // Poll controllers, process values, populate bms_voltages
        #ifndef BOARD_VERSION_HYTECH_2018_HV_REV_4 // Don't try to balance cells or use onboard ADC values on 2018 HV board
        balance_cells(); // Check local cell voltage data and balance individual cells as necessary
        process_adc(); // Poll ADC, process values, populate bms_status
        #endif
        process_temps(); // Poll controllers, process values, populate populate bms_temperatures, bms_detailed_temperatures, bms_onboard_temperatures, and bms_onboard_detailed_temperatures
        print_temps(); // Print cell and pcb temperatures to serial
        print_cells(); // Print the cell voltages and balancing status to serial
        print_current(); // Print measured current sensor value
        //process_coulombs(); // Process new coulomb counts, sending over CAN and printing to Serial
        print_uptime(); // Print the BMS uptime to serial

        Serial.print("State: ");
        if (bms_status.get_state() == BMS_STATE_DISCHARGING) {Serial.println("DISCHARGING");}
        if (bms_status.get_state() == BMS_STATE_CHARGING) {Serial.println("CHARGING");}
        if (bms_status.get_state() == BMS_STATE_BALANCING) {Serial.println("BALANCING");}
        if (bms_status.get_state() == BMS_STATE_BALANCING_OVERHEATED) {Serial.println("BALANCING_OVERHEATED");}

        if (bms_status.get_error_flags()) { // BMS error - drive BMS_OK signal low
            error_flags_history |= bms_status.get_error_flags();
            digitalWrite(BMS_OK, LOW);
            Serial.print("---------- STATUS NOT GOOD * Error Code 0x");
            Serial.print(bms_status.get_error_flags(), HEX);
            Serial.println(" ----------");
        } else {
            digitalWrite(BMS_OK, HIGH);
            Serial.println("---------- STATUS GOOD ----------");
            if (error_flags_history) {
                Serial.println("An Error Occured But Has Been Cleared");
                Serial.print("Error code: 0x");
                Serial.println(error_flags_history, HEX);
            }
        }
    }

    if (timer_can_update_fast.check()) {

        tx_msg.timeout = 4; // Use blocking mode, wait up to 4ms to send each message instead of immediately failing (keep in mind this is slower)

        bms_status.write(tx_msg.buf);
        tx_msg.id = ID_BMS_STATUS;
        tx_msg.len = sizeof(CAN_message_bms_status_t);
        CAN.write(tx_msg);

        tx_msg.timeout = 0;

    }

    if (timer_can_update_slow.check()) {

        tx_msg.timeout = 4; // Use blocking mode, wait up to 4ms to send each message instead of immediately failing (keep in mind this is slower)

        bms_voltages.write(tx_msg.buf);
        tx_msg.id = ID_BMS_VOLTAGES;
        tx_msg.len = sizeof(CAN_message_bms_voltages_t);
        CAN.write(tx_msg);

        bms_temperatures.write(tx_msg.buf);
        tx_msg.id = ID_BMS_TEMPERATURES;
        tx_msg.len = sizeof(CAN_message_bms_temperatures_t);
        CAN.write(tx_msg);

        bms_onboard_temperatures.write(tx_msg.buf);
        tx_msg.id = ID_BMS_ONBOARD_TEMPERATURES;
        tx_msg.len = sizeof(CAN_message_bms_onboard_temperatures_t);
        CAN.write(tx_msg);

        tx_msg.id = ID_BMS_DETAILED_VOLTAGES;
        tx_msg.len = sizeof(CAN_message_bms_detailed_voltages_t);
        for (int i = 0; i < TOTAL_IC; i++) {
            for (int j = 0; j < 3; j++) {
                bms_detailed_voltages[i][j].write(tx_msg.buf);
                CAN.write(tx_msg);
            }
        }

        tx_msg.id = ID_BMS_DETAILED_TEMPERATURES;
        tx_msg.len = sizeof(CAN_message_bms_detailed_temperatures_t);
        for (int i = 0; i < TOTAL_IC; i++) {
            bms_detailed_temperatures[i].write(tx_msg.buf);
            CAN.write(tx_msg);
        }

        tx_msg.id = ID_BMS_ONBOARD_DETAILED_TEMPERATURES;
        tx_msg.len = sizeof(CAN_message_bms_onboard_detailed_temperatures_t);
        for (int i = 0; i < TOTAL_IC; i++) {
            bms_onboard_detailed_temperatures[i].write(tx_msg.buf);
            CAN.write(tx_msg);
        }
        
        tx_msg.id = ID_BMS_BALANCING_STATUS;
        tx_msg.len = sizeof(CAN_message_bms_balancing_status_t);
        for (int i = 0; i < (TOTAL_IC + 3) / 4; i++) {
            bms_balancing_status[i].write(tx_msg.buf);
            CAN.write(tx_msg);
        }

        tx_msg.timeout = 0;

    }

    if (timer_watchdog_timer.check() && !fh_watchdog_test) { // Send alternating keepalive signal to watchdog timer
        watchdog_high = !watchdog_high;
        digitalWrite(WATCHDOG, watchdog_high);
    }
}

/*
 * Initialize communication with LTC6804 chips. Based off of LTC6804_initialize()
 * Changes: Doesn't call quikeval_SPI_connect(), Sets ADC mode to MD_FILTERED
 */
void initialize() {
    spi_enable(SPI_CLOCK_DIV16); // Sets 1MHz Clock
    set_adc(MD_FILTERED,DCP_DISABLED,CELL_CH_ALL,AUX_CH_ALL); // Sets global variables used in ADC commands // TODO Change CELL_CH_ALL and AUX_CH_ALL so we don't read all GPIOs and cells
}

/*
 * Initialize the configuration array and write configuration to ICs
 * See LTC6804 Datasheet Page 51 for tables of register definitions
 */
void init_cfg() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][0] = 0xFE; // 11111110 - All GPIOs enabled, Reference Remains Powered Up Until Watchdog Timeout, ADCOPT 0 allows us to use filtered ADC mode // TODO maybe we can speed things up by disabling some GPIOs
        tx_cfg[i][1] = 0x00;
        tx_cfg[i][2] = 0x00;
        tx_cfg[i][3] = 0x00;
        tx_cfg[i][4] = 0x00;
        tx_cfg[i][5] = 0x00;
    }
    cfg_set_overvoltage_comparison_voltage(voltage_cutoff_high * 10); // Calculate overvoltage comparison register values
    cfg_set_undervoltage_comparison_voltage(voltage_cutoff_low * 10); // Calculate undervoltage comparison register values
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54 // TODO is this needed?
    LTC6804_wrcfg(TOTAL_IC, tx_cfg); // Write configuration to ICs
}

void modify_discharge_config(int ic, int cell, bool setDischarge) { // TODO unify language about "balancing" vs "discharging"
    if (ic < TOTAL_IC && cell < CELLS_PER_IC) {
        bms_balancing_status[ic / 4].set_cell_balancing(ic % 4, cell, setDischarge);
        if (cell > 4) {
            cell++; // Increment cell, skipping the disconnected C5. This abstracts the missing cell from the rest of the program.
        }
        if (cell < 8) {
            if(setDischarge) {
                tx_cfg[ic][4] = tx_cfg[ic][4] | (0b1 << cell);
            } else {
                tx_cfg[ic][4] = tx_cfg[ic][4] & ~(0b1 << cell );
            }
        } else {
            if (setDischarge) {
                tx_cfg[ic][5] = tx_cfg[ic][5] | (0b1 << (cell - 8));
            } else {
                tx_cfg[ic][5] = tx_cfg[ic][5] & ~(0b1 << (cell - 8));
            }
        }
    }
}

void discharge_cell(int ic, int cell) {
    discharge_cell(ic, cell, true);
}

void discharge_cell(int ic, int cell, bool setDischarge) {
    modify_discharge_config(ic, cell, setDischarge);
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void discharge_all() {
    for (int i = 0; i < TOTAL_IC; i++) {
        bms_balancing_status[i / 4].set_ic_balancing(i % 4, ~0x0);
        tx_cfg[i][4] = 0b11111111;
        tx_cfg[i][5] = tx_cfg[i][5] | 0b00001111;
    }
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void stop_discharge_cell(int ic, int cell) {
    discharge_cell(ic, cell, false);
}

void stop_discharge_all(bool skip_clearing_status) {
    for (int i = 0; i < TOTAL_IC; i++) {
        if (!skip_clearing_status) { // Optionally leave bms_balancing_status alone - useful if only temporarily disabling balancing
            bms_balancing_status[i / 4].set_ic_balancing(i % 4, 0x0);
        }
        tx_cfg[i][4] = 0b0;
        tx_cfg[i][5] = 0b0;
    }
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void stop_discharge_all() {
    stop_discharge_all(false);
}

void balance_cells() {
    balance_offcycle = (balance_offcycle + 1) % BALANCE_LIMIT_FACTOR; // Only allow balancing on 1/BALANCE_LIMIT_FACTOR cycles
    bool cells_balancing = false; // This gets set to true later if it turns out we are balancing any cells this loop
    if (bms_voltages.get_low() > voltage_cutoff_low // TODO technically this could keep a widely spread out pack from ever balancing
        && !bms_status.get_error_flags()
        && bms_status.get_state() >= BMS_STATE_CHARGING
        && bms_status.get_state() <= BMS_STATE_BALANCING
        && (bms_status.get_shutdown_h_above_threshold() || MODE_CHARGE_OVERRIDE || MODE_ADC_IGNORE)) { // Don't check shutdown circuit if in Charge Override Mode or ADC Ignore Mode
        if (balance_offcycle) { // One last check - are we in an off-cycle? If so, we want to disable balancing but preserve bmc_balancing_status
            stop_discharge_all(true); // Stop all cells from discharging, but don't clear bms_balancing_status - this way we can view what will shortly be balancing again
        } else { // Proceed with balancing
            for (int ic = 0; ic < TOTAL_IC; ic++) { // Loop through ICs
                for (int cell = 0; cell < CELLS_PER_IC; cell++) { // Loop through cells
                    if (!ignore_cell[ic][cell]) { // Ignore any cells specified in ignore_cell
                        uint16_t cell_voltage = cell_voltages[ic][cell]; // current cell voltage in mV
                        if (cell_voltage < bms_voltages.get_low() + voltage_difference_threshold) {
                            modify_discharge_config(ic, cell, false); // Modify our local version of the discharge configuration
                        } else if (cell_voltage > bms_voltages.get_low() + voltage_difference_threshold) {
                            modify_discharge_config(ic, cell, true); // Modify our local version of the discharge configuration
                            cells_balancing = true;
                        }
                    }
                }
            }
            if (cells_balancing) { // Cells currently balancing
                bms_status.set_state(BMS_STATE_BALANCING);
            } else { // Balancing allowed, but no cells currently balancing
                if (timer_charge_enable_limit.check()) {
                    bms_status.set_state(BMS_STATE_CHARGING);
                }
            }
            wakeup_idle();
            LTC6804_wrcfg(TOTAL_IC, tx_cfg); // Write the new discharge configuration to the LTC6804s
        }
    } else {
        stop_discharge_all(); // Make sure none of the cells are discharging
    }
}

void poll_cell_voltages() {
    wakeup_sleep(); // Wake up LTC6804 ADC core
    LTC6804_adcv(); // Start cell ADC conversion
    delay(202); // Need to wait at least 201.317ms for conversion to finish, due to filtered sampling mode (26Hz) - See LTC6804 Datasheet Table 5
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    uint8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_voltages); // Reads voltages from ADC registers and stores in cell_voltages.
    if (error == -1) {
        Serial.println("A PEC error was detected in cell voltage data");
    }
    // Move C7-C10 down by one in the array, skipping C6. This abstracts the missing cell from the rest of the program.
    for (int i=0; i<TOTAL_IC; i++) { // Loop through ICs
        for (int j=6; j<10; j++) { // Loop through C7-C10
            cell_voltages[i][j-1] = cell_voltages[i][j];
        }
    }
}

void process_voltages() {
    poll_cell_voltages(); // Poll controller and store data in cell_voltages[] array
    uint32_t totalVolts = 0; // stored in 10 mV units
    uint16_t maxVolt = 0; // stored in 0.1 mV units
    uint16_t minVolt = 65535; // stored in 0.1 mV units
    uint16_t avgVolt = 0; // stored in 0.1 mV units
    int maxIC = 0;
    int maxCell = 0;
    int minIC = 0;
    int minCell = 0;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            bms_detailed_voltages[ic][cell / 3].set_voltage(cell % 3, cell_voltages[ic][cell]); // Populate CAN message struct
            if (!ignore_cell[ic][cell]) {
                uint16_t currentCell = cell_voltages[ic][cell];
                if (currentCell > maxVolt) {
                    maxVolt = currentCell;
                    maxIC = ic;
                    maxCell = cell;
                }
                if (currentCell < minVolt) {
                    minVolt = currentCell;
                    minIC = ic;
                    minCell = cell;
                }
                totalVolts += currentCell;
            }
        }
    }
    avgVolt = totalVolts / total_count_cells; // stored in 0.1 mV units
    totalVolts /= 100; // convert 0.1mV units down to 10mV units
    bms_voltages.set_average(avgVolt);
    bms_voltages.set_low(minVolt);
    bms_voltages.set_high(maxVolt);
    bms_voltages.set_total(totalVolts);

    bms_status.set_overvoltage(false); // RESET these values, then check below if they should be set again
    bms_status.set_undervoltage(false);
    bms_status.set_total_voltage_high(false);

    if (bms_voltages.get_high() > voltage_cutoff_high) {
        if (consecutive_faults_overvoltage >= IGNORE_FAULT_THRESHOLD) {
            bms_status.set_overvoltage(true);
        } else {
            consecutive_faults_overvoltage += 1;
        }
        Serial.println("VOLTAGE FAULT too high!!!!!!!!!!!!!!!!!!!");
        Serial.print("max IC: "); Serial.println(maxIC);
        Serial.print("max Cell: "); Serial.println(maxCell); Serial.println();
    } else {
        consecutive_faults_overvoltage = 0;
    }

    if (bms_voltages.get_low() < voltage_cutoff_low) {
        if (consecutive_faults_undervoltage >= IGNORE_FAULT_THRESHOLD) {
            bms_status.set_undervoltage(true);
        } else {
            consecutive_faults_undervoltage += 1;
        }
        Serial.println("VOLTAGE FAULT too low!!!!!!!!!!!!!!!!!!!");
        Serial.print("min IC: "); Serial.println(minIC);
        Serial.print("min Cell: "); Serial.println(minCell); Serial.println();
    } else {
        consecutive_faults_undervoltage = 0;
    }

    if (bms_voltages.get_total() > total_voltage_cutoff) {
        if (consecutive_faults_total_voltage_high >= IGNORE_FAULT_THRESHOLD) {
            bms_status.set_total_voltage_high(true);
        } else {
            consecutive_faults_total_voltage_high += 1;
        }
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
    }else {
        consecutive_faults_total_voltage_high = 0;
    }
}

void poll_aux_voltages() {
    wakeup_sleep();
    //delayMicroseconds(200) // TODO try this if we are still having intermittent 6.5535 issues, maybe the last ADC isn't being given enough time to wake up
    LTC6804_adax(); // Start GPIO ADC conversion
    delay(202); // Need to wait at least 201.317ms for conversion to finish, due to filtered sampling mode (26Hz) - See LTC6804 Datasheet Table 5
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    uint8_t error = LTC6804_rdaux(0, TOTAL_IC, aux_voltages);
    if (error == -1) {
        Serial.println("A PEC error was detected in auxiliary voltage data");
    }
    // print_aux();
    delay(200);
}

void process_temps() {
    poll_aux_voltages(); // Poll controllers and store data in aux_voltages[] array
    process_cell_temps(); // Process values, populate populate bms_temperatures and bms_detailed_temperatures
    process_onboard_temps(); // Process values, populate bms_onboard_temperatures and bms_onboard_detailed_temperatures
}

void process_cell_temps() { // Note: For up-to-date information you must poll the LTC6820s with poll_aux_voltages() before calling this function
    double avgTemp, lowTemp, highTemp, totalTemp, thermTemp;
    totalTemp = 0;
    lowTemp = 9999; // Alternatively use first value from array, but that could be an ignored thermistor
    highTemp = -9999;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int j = 0; j < THERMISTORS_PER_IC; j++) {
            if (!ignore_cell_therm[ic][j]) {
                thermTemp = calculate_cell_temp(aux_voltages[ic][j], aux_voltages[ic][5]); // aux_voltages[ic][5] stores the reference voltage

                if (thermTemp < lowTemp) {
                    lowTemp = thermTemp;
                }

                if (thermTemp > highTemp) {
                    highTemp = thermTemp;
                }

                bms_detailed_temperatures[ic].set_temperature(j, thermTemp); // Populate CAN message struct
                totalTemp += thermTemp;
            }
        }
    }
    avgTemp = (int16_t) (totalTemp / total_count_cell_thermistors);
    bms_temperatures.set_low_temperature((int16_t) lowTemp);
    bms_temperatures.set_high_temperature((int16_t) highTemp);
    bms_temperatures.set_average_temperature((int16_t) avgTemp);

    bms_status.set_discharge_overtemp(false); // RESET these values, then check below if they should be set again
    bms_status.set_charge_overtemp(false);

    if (bms_status.get_state() == BMS_STATE_DISCHARGING) { // Check temperatures if discharging
        if (bms_temperatures.get_high_temperature() > discharge_temp_cell_critical_high) {
            if (consecutive_faults_thermistor >= IGNORE_FAULT_THRESHOLD) {
                bms_status.set_discharge_overtemp(true);
                Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
            } else {
                consecutive_faults_thermistor++;
            }
        }
    } else if (bms_status.get_state() >= BMS_STATE_CHARGING) { // Check temperatures if charging
        if (bms_temperatures.get_high_temperature() > charge_temp_cell_critical_high) {
            if (consecutive_faults_thermistor >= IGNORE_FAULT_THRESHOLD) {
                bms_status.set_charge_overtemp(true);
                Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
            } else {
                consecutive_faults_thermistor++;
            }
        }
    } else {
        consecutive_faults_thermistor = 0;
    }
}

double calculate_cell_temp(double aux_voltage, double v_ref) {
    /* aux_voltage = (R/(10k+R))*v_ref
     * R = 10k * aux_voltage / (v_ref - aux_voltage)
     */
    aux_voltage /= 10000;
    v_ref /= 10000;
    double thermistor_resistance = 1e4 * aux_voltage / (v_ref - aux_voltage);

    /*
     * Temperature equation (in Kelvin) based on resistance is the following:
     * 1/T = 1/T0 + (1/B) * ln(R/R0)      (R = thermistor resistance)
     * T = 1/(1/T0 + (1/B) * ln(R/R0))
     */
    double T0 = 298.15; // 25C in Kelvin
    double b = 3984;    // B constant of the thermistor
    double R0 = 10000;  // Resistance of thermistor at 25C
    double temperature = 1 / ((1 / T0) + (1 / b) * log(thermistor_resistance / R0)) - (double) 273.15;

    return (int16_t)(temperature * 100);
}

void process_onboard_temps() { // Note: For up-to-date information you must poll the LTC6820s with poll_aux_voltages() before calling this function
    double avgTemp, lowTemp, highTemp, totalTemp, thermTemp;
    totalTemp = 0;
    lowTemp = 9999; // Alternatively use first value from array, but that could be an ignored thermistor
    highTemp = -9999;

    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int j = 0; j < PCB_THERM_PER_IC; j++) {
            if (!ignore_pcb_therm[ic][j]) {
                thermTemp = calculate_onboard_temp(aux_voltages[ic][j+3], aux_voltages[ic][5]);
                if (thermTemp < lowTemp) {
                    lowTemp = thermTemp;
                }

                if (thermTemp > highTemp) {
                    highTemp = thermTemp;
                }

                bms_onboard_detailed_temperatures[ic].set_temperature(j, thermTemp); // Populate CAN message struct
                totalTemp += thermTemp;
            }
        }
    }
    avgTemp = (int16_t) (totalTemp / total_count_pcb_thermistors);
    bms_onboard_temperatures.set_low_temperature((int16_t) lowTemp);
    bms_onboard_temperatures.set_high_temperature((int16_t) highTemp);
    bms_onboard_temperatures.set_average_temperature((int16_t) avgTemp);

    bms_status.set_onboard_overtemp(false); // RESET this value, then check below if they should be set

    if (bms_onboard_temperatures.get_high_temperature() > onboard_temp_critical_high) {
        if (consecutive_faults_thermistor >= IGNORE_FAULT_THRESHOLD) {
            bms_status.set_onboard_overtemp(true);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        } else {
            consecutive_faults_thermistor++;
        }
    } else {
        consecutive_faults_thermistor = 0;
    }

    if (bms_status.get_state() == BMS_STATE_BALANCING) {
        if (bms_onboard_temperatures.get_high_temperature() >= onboard_temp_balance_disable) {
            bms_status.set_state(BMS_STATE_BALANCING_OVERHEATED);
            Serial.println("WARNING: Onboard temperature too high; disabling balancing");
        }
    } else if (bms_status.get_state() == BMS_STATE_BALANCING_OVERHEATED) {
        if (bms_onboard_temperatures.get_high_temperature() < onboard_temp_balance_reenable) {
            bms_status.set_state(BMS_STATE_BALANCING);
            Serial.println("CLEARED: Onboard temperature OK; reenabling balancing");
        }
    }
}

double calculate_onboard_temp(double aux_voltage, double v_ref) {
    /* aux_voltage = (R/(10k+R))*v_ref
     * R = 10k * aux_voltage / (v_ref - aux_voltage)
     */
    aux_voltage /= 10000;
    v_ref /= 10000;
    double thermistor_resistance = 1e4 * aux_voltage / (v_ref - aux_voltage);

    /*
     * Temperature equation (in Kelvin) based on resistance is the following:
     * 1/T = 1/T0 + (1/B) * ln(R/R0)      (R = thermistor resistance)
     * T = 1/(1/T0 + (1/B) * ln(R/R0))
     */
    double T0 = 298.15; // 25C in Kelvin
    double b = 3380;    // B constant of the thermistor
    double R0 = 10000;  // Resistance of thermistor at 25C
    double temperature = 1 / ((1 / T0) + (1 / b) * log(thermistor_resistance / R0)) - (double) 273.15;
    
    return (int16_t) (temperature * 100);
}

void process_adc() {
    //Process shutdown circuit measurements
    bms_status.set_shutdown_g_above_threshold(read_adc(CH_SHUTDOWN_G) > SHUTDOWN_HIGH_THRESHOLD);
    bms_status.set_shutdown_h_above_threshold(read_adc(CH_SHUTDOWN_H) > SHUTDOWN_HIGH_THRESHOLD);

    // Process current measurement
    int current = get_current();
    bms_status.set_current(current);
    bms_status.set_charge_overcurrent(false); // RESET these values, then check below if they should be set again
    bms_status.set_discharge_overcurrent(false);
    if (bms_status.get_current() < charge_current_constant_high && !MODE_ADC_IGNORE) {
        if (consecutive_faults_current >= CURRENT_FAULT_THRESHOLD) {
            bms_status.set_charge_overcurrent(true);
            Serial.println("CHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        } else {
            consecutive_faults_current++;
        }
    } else if (bms_status.get_current() > discharge_current_constant_high && !MODE_ADC_IGNORE) {
        if (consecutive_faults_current >= CURRENT_FAULT_THRESHOLD) {
            bms_status.set_discharge_overcurrent(true);
            Serial.println("DISCHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        } else {
            consecutive_faults_current++;
        }
    } else {
        consecutive_faults_current = 0;
    }
}

/*
 * Update maximum and minimum allowed voltage, current, temperature, etc.
 */
int update_constraints(uint8_t address, uint16_t value) { // TODO EEPROM + CAN programming feature
    switch(address) {
        case 0:
            voltage_cutoff_low = value;
            break;
        case 1:
            voltage_cutoff_high = value;
            break;
        case 2:
            total_voltage_cutoff = value;
            break;
        case 3:
            discharge_current_constant_high = value;
            break;
        case 4:
            charge_current_constant_high = value;
            break;
        case 5:
            charge_temp_cell_critical_high = value;
            break;
        case 6:
            discharge_temp_cell_critical_high = value;
            break;
        case 7:
            voltage_difference_threshold = value;
            break;
        default:
            return -1;
    }
    return 0;
}

void print_temps() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\tCell Temperatures\t\t\t\t\t\t\t\t\t   PCB Temperatures");
    Serial.println("\tTHERM 0\t\tTHERM 1\t\tTHERM 2\t\t\t\t\t\t\t\tTHERM 0\t\tTHERM 1");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int therm = 0; therm < THERMISTORS_PER_IC; therm++) {
            if (!ignore_cell_therm[ic][therm]) {
                double temp = ((double) bms_detailed_temperatures[ic].get_temperature(therm)) / 100;
                Serial.print(temp, 2);
                Serial.print(" ºC");
            } else {
                Serial.print("IGN\t");
            }
            Serial.print("\t");
        }
        Serial.print("\t\t\t\t\t\t");
        for (int therm = 0; therm < PCB_THERM_PER_IC; therm++) {
            if (!ignore_pcb_therm[ic][therm]) {
                double temp = ((double) bms_onboard_detailed_temperatures[ic].get_temperature(therm)) / 100;
                Serial.print(temp, 2);
                Serial.print(" ºC");
            } else {
                Serial.print("IGN\t");
            }
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.print("\nCell temperature statistics\t\t Average: ");
    Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Min: ");
    Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Max: ");
    Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" ºC\n");
    Serial.print("PCB temperature statistics\t\t Average: ");
    Serial.print(bms_onboard_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Min: ");
    Serial.print(bms_onboard_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Max: ");
    Serial.print(bms_onboard_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" ºC\n");
}

void print_cells() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            int ignored = ignore_cell[ic][cell];
            if (!MODE_BENCH_TEST || !ignored) { // Don't clutter test bench UI with ignored cells
                double voltage = cell_voltages[ic][cell] * 0.0001;
                Serial.print(voltage, 4); Serial.print("V\t");
            } else {
                Serial.print("\t");
            }
        }
        Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            int balancing = bms_balancing_status[ic / 4].get_cell_balancing(ic % 4, cell);
            int ignored = ignore_cell[ic][cell];
            if (balancing) {
                Serial.print("BAL");
            } else if (ignored) {
                Serial.print("IGN");
            }
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.println();
    Serial.println("\t\t\t\tDelta from Min Cell");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            if (!ignore_cell[ic][cell]) {
                double voltage = (cell_voltages[ic][cell]-bms_voltages.get_low()) * 0.0001;
                Serial.print(voltage, 4);
                Serial.print("V");
            }
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.println();
    Serial.print("Cell voltage statistics\t\tTotal: "); Serial.print(bms_voltages.get_total() / (double) 1e2, 4); Serial.print("V\t\t");
    Serial.print("Average: "); Serial.print(bms_voltages.get_average() / (double) 1e4, 4); Serial.print("V\t\t");
    Serial.print("Min: "); Serial.print(bms_voltages.get_low() / (double) 1e4, 4); Serial.print("V\t\t");
    Serial.print("Max: "); Serial.print(bms_voltages.get_high() / (double) 1e4, 4); Serial.println("V");
}

void print_current() {
    Serial.print("\nCurrent Sensor: ");
    Serial.print(bms_status.get_current() / (double) 100, 2);
    Serial.println("A");
}

void print_aux() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic);
        for (int i = 0; i < 6; i++) {
            Serial.print("Aux-"); Serial.print(i+1); Serial.print(": ");
            float voltage = aux_voltages[current_ic][i] * 0.0001;
            Serial.println(voltage, 4);
        }
        Serial.println();
    }
}

void print_cell_temperatures() {
    Serial.print("\nAverage cell temperature: ");
    Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("Low cell temperature: ");
    Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("High cell temperature: ");
    Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" C\n");
}

void print_onboard_temperatures() {
    Serial.print("\nAverage onboard temperature: ");
    Serial.print(bms_onboard_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("Low onboard temperature: ");
    Serial.print(bms_onboard_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("High onboard temperature: ");
    Serial.print(bms_onboard_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" C\n");
}

/*
 * Print ECU uptime
 */
void print_uptime() {
    Serial.print("\nECU uptime: ");
    Serial.print(millis() / 1000);
    Serial.print(" seconds (");
    Serial.print(millis() / 1000 / 60);
    Serial.print(" minutes, ");
    Serial.print(millis() / 1000 % 60);
    Serial.println(" seconds)\n");
}

/*
 * Set VOV in configuration registers
 * Voltage is in 100 uV increments
 * See LTC6804 datasheet pages 25 and 53
 */
void cfg_set_overvoltage_comparison_voltage(uint16_t voltage) {
    voltage /= 16;
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][2] = (tx_cfg[i][2] & 0x0F) | ((voltage && 0x00F) << 8);
        tx_cfg[i][3] = (voltage && 0xFF0) >> 4;
    }
}

/*
 * Set VUV in configuration registers
 * Voltage is in 100 uV increments
 * See LTC6804 datasheet pages 25 and 53
 */
void cfg_set_undervoltage_comparison_voltage(uint16_t voltage) {
    voltage /= 16;
    voltage -= 10000;
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][1] = voltage && 0x0FF;
        tx_cfg[i][2] = (tx_cfg[i][2] & 0xF0) | ((voltage && 0xF00) >> 8);
    }
}

void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_CCU_STATUS) { // Enter charging mode if CCU status message is received
            timer_charge_timeout.reset();
            if (bms_status.get_state() == BMS_STATE_DISCHARGING && (timer_charge_enable_limit.check() || !charge_mode_entered)) {
                bms_status.set_state(BMS_STATE_CHARGING);
                digitalWrite(LED_STATUS, HIGH);
                charge_mode_entered = true;
            }
        }

        if (rx_msg.id == ID_FH_WATCHDOG_TEST) { // Stop sending pulse to watchdog timer in order to test its functionality
            fh_watchdog_test = true;
        }
    }
}

int16_t get_current() {
    /*
     * Current sensor: ISB-300-A-604
     * Maximum positive current (300A) corresponds to 4.5V signal
     * Maximum negative current (-300A) corresponds to 0.5V signal
     * 0A current corresponds to 2.5V signal
     *
     * voltage = read_adc() * 5 / 4095
     * current = (voltage - 2.5) * 300 / 2
     */
    double voltage = read_adc(CH_CUR_SENSE) / (double) 819;
    double current = (voltage - 2.5) * (double) 150;
    return (int16_t) (current * 100); // Current in Amps x 100
}

void integrate_current() {
    int delta = get_current();
    if (delta > 0) {
        total_discharge = total_discharge + delta;
    } else {
        total_charge = total_charge - delta; // Units will be 0.01A / (1 / (COULOUMB_COUNT_INTERVAL x 10^-6) s)
    }
}

void process_coulombs() {
    noInterrupts(); // Disable interrupts to ensure the values we are reading do not change while copying
    total_charge_copy = total_charge;
    total_discharge_copy = total_discharge;
    interrupts();

    Serial.print("\nCoulombs charged: ");
    Serial.println(total_charge_copy / 10000);
    Serial.print("Coulombs discharged: ");
    Serial.println(total_discharge_copy / 10000);

    bms_coulomb_counts.set_total_charge(total_charge_copy);
    bms_coulomb_counts.set_total_discharge(total_discharge_copy);
}

/*
 * Helper function reads from ADC then sets SPI settings such that isoSPI will continue to work
 */
int read_adc(int channel) {
    noInterrupts(); // Since timer interrupt triggers SPI communication, we don't want it to interrupt other SPI communication
    int retval = ADC.read_adc(channel);
    interrupts();
    spi_enable(SPI_CLOCK_DIV16); // Reconfigure 1MHz SPI clock speed after ADC reading so LTC communication is successful
    return retval;
}
