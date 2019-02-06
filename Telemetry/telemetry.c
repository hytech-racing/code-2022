#include <stdlib.h>
#include <stdio.h>
#include "../Libraries/HyTech_CAN/HyTech_CAN.h"
#include "../Libraries/XBTools/XBTools.h"
#include <MQTTClient.h>

struct {
    CAN_msg_rcu_status                      rcu_status;
    CAN_msg_fcu_status                      fcu_status;
    CAN_msg_fcu_readings                    fcu_readings;
    CAN_message_bms_voltages_t              bms_voltages;
    CAN_message_bms_detailed_voltages_t     bms_detailed_voltages;
    CAN_message_bms_temperatures_t          bms_temperatures;
    CAN_message_bms_detailed_temperatures_t bms_detailed_temperatures;
    CAN_message_bms_onboard_temperatures_t  bms_onboard_temperatures;
    CAN_message_bms_onboard_detailed_temperatures_t bms_onboard_detailed_temperatures;
    CAN_message_bms_status_t                bms_status;
    CAN_message_bms_balancing_status_t      bms_balancing_status;
    CAN_message_ccu_status_t                ccu_status;
    CAN_message_mc_temperatures_1_t         mc_temperatures_1;
    CAN_message_mc_temperatures_2_t         mc_temperatures_2;
    CAN_message_mc_temperatures_3_t         mc_temperatures_3;
    CAN_message_mc_analog_input_voltages_t  mc_analog_input_voltages;
    CAN_message_mc_digital_input_status_t   mc_digital_input_status;
    CAN_message_mc_motor_position_information_t mc_motor_position_information;
    CAN_message_mc_current_information_t    mc_current_information;
    CAN_message_mc_voltage_information_t    mc_voltage_information;
    CAN_message_mc_internal_states_t        mc_internal_states;
    CAN_message_mc_fault_codes_t            mc_fault_codes;
    CAN_message_mc_torque_timer_information_t mc_torque_timer_information;
    CAN_message_mc_modulation_index_flux_weakening_output_information_t
        mc_modulation_index_flux_weakening_output_information;
    CAN_message_mc_firmware_information_t   mc_firmware_information;
    CAN_message_mc_command_message_t        mc_command_message;
    CAN_message_mc_read_write_parameter_command_t
        mc_read_write_parameter_command;
    CAN_message_mc_read_write_parameter_response_t
        mc_read_write_parameter_response;
    CAN_message_fcu_accelerometer_values_t  fcu_accelerometer_values;
} current_status;

static void process_message(uint64_t timestamp, CAN_message_t *msg)
{
    // Do logging stuff.

    switch (msg->msg_id) {
        case ID_RCU_STATUS:
        {
            CAN_msg_rcu_status *data = &msg->contents.rcu_status;
            printf("RCU STATE: %hhu\n"
                   "RCU FLAGS: 0x%hhX\n"
                   "GLV BATT VOLTAGE: %f V\n"
                   "RCU BMS FAULT: %hhu\n"
                   "RCU IMD FAULT: %hhu\n",
                   data->state,
                   data->flags,
                   data->glv_battery_voltage / 100.0,
                   (char)(!(data->flags & 1)),
                   (char)(!(data->flags & 2)));
            current_status.rcu_status = *data;
            break;
        }
        case ID_FCU_STATUS:
        {
            CAN_msg_fcu_status *data = &msg->contents.fcu_status;
            printf("FCU STATE: %hhu\n"
                   "FCU FLAGS: %hhX\n"
                   "FCU START BUTTON ID: %hhu\n"
                   "FCU BRAKE ACT: %hhu\n"
                   "FCU IMPLAUS ACCEL: %hhu\n"
                   "FCU IMPLAUSE BRAKE: %hhu\n",
                   data->state,
                   data->flags,
                   data->start_button_press_id,
                   (char)((data->flags & 8) >> 3),
                   (char)(data->flags & 1),
                   (char)((data->flags & 4) >> 2));
            current_status.fcu_status = *data;
            break;
        }
        case ID_FCU_READINGS:
        {
            CAN_msg_fcu_readings *data = &msg->contents.fcu_readings;
            printf("FCU PEDAL ACCEL 1: %hu\n"
                   "FCU PEDAL ACCEL 2: %hu\n"
                   "FCU PEDAL BRAKE: %hu\n",
                   data->accelerator_pedal_raw_1,
                   data->accelerator_pedal_raw_2,
                   data->brake_pedal_raw);
            current_status.fcu_readings = *data;
            break;
        }
        case ID_FCU_ACCELEROMETER:
        {
            break;  // TODO
        }
        case ID_RCU_RESTART_MC:
        {
            break;  // TODO
        }
        case ID_BMS_ONBOARD_TEMPERATURES:
        case ID_BMS_ONBOARD_DETAILED_TEMPERATURES:
        case ID_BMS_VOLTAGES:
        {
            CAN_message_bms_voltages_t *data = &msg->contents.bms_voltages;
            printf("BMS VOLTAGE AVERAGE: %f V\n"
                   "BMS VOLTAGE LOW: %f V\n"
                   "BMS VOLTAGE HIGH: %f V\n"
                   "BMS VOLTAGE TOTAL: %f V\n",
                   data->average_voltage / 1000.0,
                   data->low_voltage / 1000.0,
                   data->high_voltage / 1000.0,
                   data->total_voltage / 100.0);
            current_status.bms_voltages = *data;
            break;
        }
        case ID_BMS_DETAILED_VOLTAGES:
        {
            break;  // TODO need to check docs
        }
        case ID_BMS_TEMPERATURES:
        {
            CAN_message_bms_temperatures_t *data = &msg->contents.bms_temperatures;
            printf("BMS AVERAGE TEMPERATURE: %f C\n"
                   "BMS LOW TEMPERATURE: %f C\n"
                   "BMS HIGH TEMPERATURE: %f C\n",
                   data->average_temperature / 100.0,
                   data->low_temperature / 100.0,
                   data->high_temperature / 100.0);
            current_status.bms_temperatures = *data;
            break;
        }
        case ID_BMS_DETAILED_TEMPERATURES:
        {
            break;  // TODO need to check docs
        }
        case ID_BMS_STATUS:
        {
            CAN_message_bms_status_t *data = &msg->contents.bms_status;
            printf("BMS STATE: %hhu\n"
                   "BMS ERROR FLAGS: 0x%hX\n"
                   "BMS CURRENT: %f A\n",
                   data->state,
                   data->error_flags,
                   data->current / 100.0);
            current_status.bms_status = *data;
            break;
        }
        case ID_BMS_BALANCING_STATUS:
            break;
        case ID_FH_WATCHDOG_TEST:
            break;
        case ID_CCU_STATUS:
            break;
        case ID_MC_TEMPERATURES_1: {
            CAN_message_mc_temperatures_1_t *data = &msg->contents.mc_temperatures_1;
            printf("[%llu] MODULE A TEMP: %f C\n"
                   "MODULE B TEMP: %f C\n"
                   "MODULE C TEMP: %f C\n"
                   "GATE DRIVER BOARD TEMP: %f C\n",
                    timestamp,
                    data->module_a_temperature / 10.,
                    data->module_b_temperature / 10.,
                    data->module_c_temperature / 10.,
                    data->gate_driver_board_temperature / 10.);
            current_status.mc_temperatures_1 = *data;
            break;
        }
        case ID_MC_TEMPERATURES_2: {
            CAN_message_mc_temperatures_2_t *data = &msg->contents.mc_temperatures_2;
            printf("CONTROL BOARD TEMP: %f C\n"
                   "RTD 1 TEMP: %f C\n"
                   "RTD 2 TEMP: %f C\n"
                   "RTD 3 TEMP: %f C\n",
                    data->control_board_temperature / 10.,
                    data->rtd_1_temperature / 10.,
                    data->rtd_2_temperature / 10.,
                    data->rtd_3_temperature / 10.);
            current_status.mc_temperatures_2 = *data;
            break;
        }
        case ID_MC_TEMPERATURES_3: {
            CAN_message_mc_temperatures_3_t *data = &msg->contents.mc_temperatures_3;
            printf("RTD 4 TEMP: %f C\n"
                   "RTD 5 TEMP: %f C\n"
                   "MOTOR TEMP: %f C\n"
                   "TORQUE SHUDDER: %f Nm\n",
                    data->rtd_4_temperature / 10.,
                    data->rtd_5_temperature / 10.,
                    data->motor_temperature / 10.,
                    data->torque_shudder / 10.);
            current_status.mc_temperatures_3 = *data;
            break;
        }
        case ID_MC_ANALOG_INPUTS_VOLTAGES:
        {
            break;
        }
        case ID_MC_DIGITAL_INPUT_STATUS:
            break;
        case ID_MC_MOTOR_POSITION_INFORMATION:
        {
            CAN_message_mc_motor_position_information_t *data =
                &msg->contents.mc_motor_position_information;
            printf("MOTOR ANGLE: %f\n"
                   "MOTOR SPEED: %hd RPM\n"
                   "ELEC OUTPUT FREQ: %f\n"
                   "DELTA RESOLVER FILT: %hd\n",
                   data->motor_angle / 10.,
                   data->motor_speed,
                   data->electrical_output_frequency / 10.,
                   data->delta_resolver_filtered);
            current_status.mc_motor_position_information = *data;
            break;
        }
        case ID_MC_CURRENT_INFORMATION:
        {
            CAN_message_mc_current_information_t *data = &msg->contents.mc_current_information;
            printf("PHASE A CURRENT: %f A\n"
                   "PHASE B CURRENT: %f A\n"
                   "PHASE C CURRENT: %f A\n"
                   "DC BUS CURRENT: %f A\n",
                   data->phase_a_current / 10.,
                   data->phase_b_current / 10.,
                   data->phase_c_current / 10.,
                   data->dc_bus_current / 10.);
            current_status.mc_current_information = *data;
            break;
        }
        case ID_MC_VOLTAGE_INFORMATION:
        {
            CAN_message_mc_voltage_information_t *data = &msg->contents.mc_voltage_information;
            printf("DC BUS VOLTAGE: %f V\n"
                   "OUTPUT VOLTAGE: %f V\n"
                   "PHASE AB VOLTAGE: %f V\n"
                   "PHASE BC VOLTAGE: %f V\n",
                   data->dc_bus_voltage / 10.,
                   data->output_voltage / 10.,
                   data->phase_ab_voltage / 10.,
                   data->phase_bc_voltage / 10.);
            current_status.mc_voltage_information = *data;
            break;
        }
        case ID_MC_FLUX_INFORMATION:
            break;
        case ID_MC_INTERNAL_VOLTAGES:
            break;
        case ID_MC_INTERNAL_STATES:
        {
            CAN_message_mc_internal_states_t *data = &msg->contents.mc_internal_states;
            printf("VSM STATE: %hu\n"
                   "INVERTER STATE: %hhu\n"
                   "INVERTER RUN MODE: %hhu\n"
                   "INVERTER ACTIVE DISCHARGE STATE: %hhu\n"
                   "INVERTER COMMAND MODE: %hhu\n"
                   "INVERTER ENABLE: %hhu\n"
                   "INVERTER LOCKOUT: %hhu\n"
                   "DIRECTION COMMAND: %hhu\n",
                   data->vsm_state,
                   data->inverter_state,
                   (char)(data->inverter_run_mode_discharge_state & 1),
                   (char)((data->inverter_run_mode_discharge_state & 0xE0) >> 5),
                   data->inverter_command_mode,
                   (char)(data->inverter_enable & 1),
                   (char)((data->inverter_enable & 0x80) >> 7),
                   data->direction_command);
            current_status.mc_internal_states = *data;
        }
        case ID_MC_FAULT_CODES:
        {
            CAN_message_mc_fault_codes_t *data = &msg->contents.mc_fault_codes;
            printf("POST FAULT LO: 0x%hX\n"
                   "POST FAULT HI: 0x%hX\n"
                   "RUN FAULT LO: 0x%hX\n"
                   "RUN FAULT HI: 0x%hx\n",
                   data->post_fault_lo,
                   data->post_fault_hi,
                   data->run_fault_lo,
                   data->run_fault_hi);
            current_status.mc_fault_codes = *data;
            break;
        }
        case ID_MC_TORQUE_TIMER_INFORMATION:
        {
            CAN_message_mc_torque_timer_information_t *data =
                &msg->contents.mc_torque_timer_information;
            printf("COMMANDED TORQUE: %f Nm\n"
                   "TORQUE FEEDBACK: %f Nm\n"
                   "RMS UPTIME: %f s",
                   data->commanded_torque / 10.,
                   data->torque_feedback / 10.,
                   data->power_on_timer * 0.003);
            current_status.mc_torque_timer_information = *data;
            break;
        }
        case ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION:
            break;
        case ID_MC_FIRMWARE_INFORMATION:
            break;
        case ID_MC_DIAGNOSTIC_DATA:
            break;
        case ID_MC_COMMAND_MESSAGE: {
            CAN_message_mc_command_message_t *data = &msg->contents.mc_command_message;
            // TODO add the other members of this struct??
            printf("FCU REQUESTED TORQUE: %f\n",
                   data->torque_command / 10.);
            current_status.mc_command_message = *data;
        }
        case ID_MC_READ_WRITE_PARAMETER_COMMAND:
            break;
        case ID_MC_READ_WRITE_PARAMETER_RESPONSE:
            break;
        default:
            fprintf(stderr, "Error: unknown message type\n");
    }
}

#define ADDRESS     "tcp://hytech-telemetry.ryangallaway.me:1883"
#define CLIENTID    "HyTechCANReceiver"
#define TOPIC       "hytech_car/telemetry"
#define QOS         1
#define TIMEOUT     10000L

static void connection_lost(void *context, char *cause)
{
    printf("Connection lost: %s\n", cause);
}

static void msg_delivered(void *context, MQTTClient_deliveryToken dt)
{
    // Do nothing for now.
}

static int msg_arrived(void *context, char *topic_name, int topic_len,
        MQTTClient_message *msg)
{
    char *payload_str = msg->payload;
    int payload_len = msg->payloadlen;
    for (int i = 0; payload_str[i] != 0; i++) {
        if (payload_str[i] == ',') {
            payload_str[i] = 0;
            uint64_t timestamp = atoll(payload_str);
            CAN_message_t payload;
            cobs_decode((uint8_t *)&payload_str[i + 1], 32, (uint8_t *)&payload);
            uint16_t checksum_calc = fletcher16((uint8_t *)&payload, sizeof(payload));
            if (payload.checksum != checksum_calc) {
                fprintf(stderr, "Error: checksum mismatch: "
                        "calculated: %hu received: %hu\n",
                        checksum_calc, payload.checksum);
                goto cleanup;
            }
            process_message(timestamp, &payload);
            goto cleanup;
        }
    }
    fprintf(stderr, "Message formatted improperly\n");

cleanup:
    MQTTClient_freeMessage(&msg);
    MQTTClient_free(topic_name);
    return 1;
}

int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connection_lost,
                            msg_arrived, msg_delivered);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
    while (1) if (getchar() == 'q') break;
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
