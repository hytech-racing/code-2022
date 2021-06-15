#include <time.h>

#include "gtest/gtest.h"
#include "MockLTC.h"
#include "LTC68042.h"
#include "Simulator.h"

TEST(UNIT, MockLTC) {
	srand(time(0));
	MockLTC mock_ltc(10, 1);

	mock_ltc.begin();
	Simulator::begin();

	pinMode(10, OUTPUT);
	set_adc(MD_FILTERED,DCP_DISABLED,CELL_CH_ALL,AUX_CH_ALL);

	uint16_t mock_cell_voltages[12];
	uint16_t mock_aux_voltages[6];
	uint8_t mock_config[6];

	uint16_t cell_voltages[1][12];
	uint16_t aux_voltages[1][6];
	uint8_t rx_config[1][8];
	uint8_t tx_config[1][6];

	for (int i = 0; i < 6; ++i) mock_config[i] = rand() & 0xFF;

	mock_ltc.setConfig(0, mock_config);
	LTC6804_rdcfg(1, rx_config);
	ASSERT_EQ(0, memcmp(rx_config[0], mock_config, 6 * sizeof(uint8_t)));

	for (int i = 0; i < 6; ++i) tx_config[0][i] = rand() & 0xFF;
	LTC6804_wrcfg(1, tx_config);
	mock_ltc.getConfig(0, mock_config);
	ASSERT_EQ(0, memcmp(tx_config[0], mock_config, 6 * sizeof(uint8_t)));

	for (int i = 0; i < 12; ++i)
		mock_ltc.setCellVoltage(0, i, mock_cell_voltages[i] = rand() & 0xFFFF);
	LTC6804_adcv();
	delay(202);
	ASSERT_NE(-1, LTC6804_rdcv(0, 1, cell_voltages));
	ASSERT_EQ(0, memcmp(cell_voltages[0], mock_cell_voltages, 12 * sizeof(uint16_t)));

	for (int i = 0; i < 6; ++i)
		mock_ltc.setAuxVoltage(0, i, mock_aux_voltages[i] = rand() & 0xFFFF);
	LTC6804_adax();
	delay(202);
	LTC6804_rdaux(0, 1, aux_voltages);
	ASSERT_EQ(0, memcmp(aux_voltages[0], mock_aux_voltages, 6 * sizeof(uint16_t)));

	mock_ltc.teardown();
	Simulator::teardown();
}
