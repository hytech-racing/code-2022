#include "UserDefined.h"

CUSTOMFUNC(parse_bms_balancing_status, BMS_balancing_status& data, BMS_balancing_status& prev) {
	int group = data.get_group_id() << 2;
	for (int ic = 0; ic < 4; ++ic) {
		for (int cell = 0; cell < 9; ++cell) {
			bool bal = data.get_cell_balancing(ic, cell);
			if (verbose || bal != prev.get_cell_balancing(ic, cell))
				show("BAL_IC_%d_CELL_%d,%d\n", group + ic, cell, bal);
		}
	}
}

CUSTOMFUNC(parse_detailed_voltages, BMS_detailed_voltages& data, BMS_detailed_voltages& prev) {
	int group = data.get_group_id() * 3;
	for (int i = 0; i < 3; ++i) {
		if (verbose || data.get_voltage(i) != prev.get_voltage(i))
			show("CELL_%d,%.4f,V\n", group + i, data.get_voltage(i) / (double) 10000);
	}
}

CUSTOMFUNC(print_shutdown_status, MCU_status& data, MCU_status& prev) {
    if (verbose || data.get_bms_ok_high() != prev.get_bms_ok_high())
	show("bms_fault,%d\n", !data.get_bms_ok_high());
    if (verbose || data.get_imd_okhs_high() != prev.get_imd_okhs_high())
	show("imd_fault,%d\n", !data.get_imd_okhs_high());
    if (verbose || data.get_inverter_powered() != prev.get_inverter_powered())
	show("inverter_power,%d\n", data.get_inverter_powered());

	static char prev_shutdown [8] = "init";
	char shutdown [8];
	int idx = 0;
	if (data.get_shutdown_b_above_threshold()) shutdown[idx++] = 'B';
	if (data.get_shutdown_c_above_threshold()) shutdown[idx++] = 'C';
	if (data.get_shutdown_d_above_threshold()) shutdown[idx++] = 'D';
	if (data.get_shutdown_e_above_threshold()) shutdown[idx++] = 'E';
	if (data.get_shutdown_f_above_threshold()) shutdown[idx++] = 'F';
	shutdown[idx++] = '\0';

	if (verbose || strcmp(shutdown, prev_shutdown)) {
		show("shutdown_above_thresh,%s\n", shutdown);
		strcpy(prev_shutdown, shutdown);
	}
}