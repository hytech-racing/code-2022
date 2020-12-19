#include "HyTech_CAN.h"
#include "AutoParse.h"

#include <stdio.h>

void parse_bms_balancing_status (const char* const timestamp, const char* const prefix, BMS_balancing_status& data) {
	int group = data.get_group_id() << 2;
	for (int ic = 0; ic < 4; ++ic) {
		for (int cell = 0; cell < 9; ++cell)
			fprintf(outfile, "%s,%s" "BAL_IC_%d_CELL_%d,%d,\n", timestamp, prefix, group + ic, cell, data.get_cell_balancing(ic, cell));
	}
}

void parse_detailed_voltages (const char* const timestamp, const char* const prefix, BMS_detailed_voltages& data) {
	int group = data.get_group_id() * 3;
	for (int i = 0; i < 3; ++i)
		fprintf(outfile, "%s,%s" "CELL_%d,%f,V\n", timestamp, prefix, group + i, data.get_voltage(i) / (double) 10000);
}

void print_shutdown_status (const char* const timestamp, const char* const prefix, MCU_status& data) {
	char shutdown [8];
	int idx = 0;
	if (data.get_shutdown_b_above_threshold()) shutdown[idx++] = 'B';
	if (data.get_shutdown_c_above_threshold()) shutdown[idx++] = 'C';
	if (data.get_shutdown_d_above_threshold()) shutdown[idx++] = 'D';
	if (data.get_shutdown_e_above_threshold()) shutdown[idx++] = 'E';
	if (data.get_shutdown_f_above_threshold()) shutdown[idx++] = 'F';
	shutdown[idx++] = '\0';
	fprintf(outfile, "%s,%s" "shutdown_status,%s\n", timestamp, prefix, shutdown);
}