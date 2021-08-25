


namespace LTC6811_2 {

class Reg_Group_Config {
public:
    Reg_Group_Config() = default;
    Reg_Group_Config(uint8_t gpio_, bool refon_, bool dten_, bool adcpot_, uint16_t undervoltage_, uint16_t overvoltage_, uint16_t discharge_, uint8_t dcto_) : 
        gpio(gpio_),
        refon(refon_),
        dten(dten_),
        adcpot(adcpot_),
        undervoltage(undervoltage_),
        overvoltage(overvoltage_),
        discharge(discharge_),
        dcto(dcto_) { };

    uint8_t get_gpio() { return gpio; };
    bool get_refon() { return refon; };
    bool get_dten() { return dten; };
    bool get_adcpot() { return adcpot; };
    uint16_t get_undervoltage() { return undervoltage; };
    uint16_t get_overvoltage() { return overvoltage; };
    uint16_t get_discharge() { return discharge; };
    uint8_t get_dcto() { return dcto; };

private:
    uint8_t gpio; //use bitmasking with GPIO_CONFIG enum
    bool refon;
    bool dten;
    bool adcopt;
    uint16_t undervotage;
    uint16_t overvoltage;
    uint16_t discharge; //use bitmasking with the CELL_DISCHARGE enum
    uint8_t dcto;
};


class Reg_Group_Cell_A {
public:
    Reg_Group_Cell_A() = default;
    Reg_Group_Cell_A(uint16_t c1, uint16_t c2, uint16_t c3) :
        cell1_voltage(c1),
        cell2_voltage(c2),
        cell3_voltage(c3) { };
    uint16_t get_cell1_voltage() { return cell1_voltage; };
    uint16_t get_cell2_voltage() { return cell2_voltage; };
    uint16_t get_cell3_voltage() { return cell3_voltage; };
private:
    uint16_t cell1_voltage;
    uint16_t cell2_voltage;
    uint16_t cell3_voltage;
};


class Reg_Group_Cell_B {
public:
    Reg_Group_Cell_B() = default;
    Reg_Group_Cell_B(uint16_t c4, uint16_t c5, uint16_t c6) :
        cell4_voltage(c4),
        cell5_voltage(c5),
        cell6_voltage(c6) { };
    uint16_t get_cell4_voltage() { return cell4_voltage; };
    uint16_t get_cell5_voltage() { return cell5_voltage; };
    uint16_t get_cell6_voltage() { return cell6_voltage; };
private:
    uint16_t cell4_voltage;
    uint16_t cell5_voltage;
    uint16_t cell6_voltage;
};


class Reg_Group_Cell_C {
public:
    Reg_Group_Cell_C() = default;
    Reg_Group_Cell_C(uint16_t c7, uint16_t c8, uint16_t c9) :
        cell1_voltage(c7),
        cell2_voltage(c8),
        cell3_voltage(c9) { };
    uint16_t get_cell7_voltage() { return cell7_voltage; };
    uint16_t get_cell8_voltage() { return cell8_voltage; };
    uint16_t get_cell9_voltage() { return cell9_voltage; };
private:
    uint16_t cell7_voltage;
    uint16_t cell8_voltage;
    uint16_t cell9_voltage;
};


class Reg_Group_Cell_D {
public:
    Reg_Group_Cell_D() = default;
    Reg_Group_Cell_D(uint16_t c10, uint16_t c11, uint16_t c12) :
        cell10_voltage(c10),
        cell11_voltage(c11),
        cell12_voltage(c12) { };
    uint16_t get_cell10_voltage() { return cell10_voltage; };
    uint16_t get_cell11_voltage() { return cell11_voltage; };
    uint16_t get_cell12_voltage() { return cell12_voltage; };
private:
    uint16_t cell10_voltage;
    uint16_t cell11_voltage;
    uint16_t cell12_voltage;
};


class Reg_Group_Aux_A {
public:
    Reg_Group_Aux_A() = default;
    Reg_Group_Aux_A(uint16_t g1, uint16_t g2, uint16_t g3) :
        gpio1_voltage(g1),
        gpio2_voltage(g2),
        gpoi3_voltage(g3) { };
    uint16_t get_gpio1_voltage() { return gpio1_voltage; };
    uint16_t get_gpio2_voltage() { return gpio2_voltage; };
    uint16_t get_gpio3_voltage() { return gpio3_voltage; };
private:
    uint16_t gpio1_voltage;
    uint16_t gpio2_voltage;
    uint16_t gpio3_voltage;
};

class Reg_Group_Aux_B {
public:
    Reg_Group_Aux_B() = default;
    Reg_Group_Aux_B(uint16_t g4, uint16_t g5, uint16_t g6) :
        gpio1_voltage(g4),
        gpio2_voltage(g5),
        gpoi3_voltage(g6) { };
    uint16_t get_gpio4_voltage() { return gpio4_voltage; };
    uint16_t get_gpio5_voltage() { return gpio5_voltage; };
    uint16_t get_gpio6_voltage() { return gpio6_voltage; };
private:
    uint16_t gpio4_voltage;
    uint16_t gpio5_voltage;
    uint16_t gpio6_voltage;
};


class Reg_Group_Status_A {
public:
    Reg_Group_Status_A() = default;
    Reg_Group_Status_A(uint16_t sc, uint16_t itmp, uint16_t va) :
        sum_of_all_cells(sc),
        internal_die_temp(itmp),
        analog_supply_voltage(va) { };
    uint16_t get_sum_of_all_cells() { return sum_of_all_cells; };
    uint16_t get_internal_die_temp() { return internal_die_temp; };
    uint16_t get_analog_supply_voltage() { return analog_supply_voltage; };
private:
    uint16_t sum_of_all_cells;
    uint16_t internal_die_temp;
    uint16_t analog_supply_voltage;
};

class Reg_Group_Status_B {
public:
    Reg_Group_Status_B() = default;
    Reg_Group_Status_B(uint16_t vd, uint16_t cov, uint16_t cuv, uint8_t rev, bool muxfail, bool thsd) :
        digital_supply_voltage(vd),
        cell_overvoltage_flags(cov),
        cell_undervoltage_flags(cuv),
        revision_code(rev),
        mux_self_test_result(muxfail),
        thermal_shutdown_status(thsd) { };

    uint16_t get_digital_supply_voltage() { return digital_supply_voltage; };
    uint16_t get_cell_overvoltage_flags() { return cell_overvoltage_flags; };
    uint16_t get_cell_undervoltage_flags() { return cell_undervoltage_flags; };
    uint8_t get_revision_code() { return revision_code; };
    bool get_mux_self_test_result() { return mux_self_test_result; };
    bool get_thermal_shutdown_status() { return thermal_shutdown_status; };

private:
    uint16_t digital_supply_voltage;
    uint16_t cell_overvoltage_flags;
    uint16_t cell_undervoltage_flags;
    uint8_t revision_code;
    bool mux_self_test_result;
    bool thermal_shutdown_status;

};

Reg_Group_COMM
class Reg_Group_S_Ctrl {
public:
    Reg_Group_S_Ctrl() = default;
    Reg_Group_S_Ctrl(S_CONTROL pin1_,
                    S_CONTROL pin2_, 
                    S_CONTROL pin3_,
                    S_CONTROL pin4_,
                    S_CONTROL pin5_,
                    S_CONTROL pin6_,
                    S_CONTROL pin7_,
                    S_CONTROL pin8_,
                    S_CONTROL pin9_,
                    S_CONTROL pin10_,
                    S_CONTROL pin11_,
                    S_CONTROL pin12_) :
        pin1(pin1_),
        pin2(pin2_),
        pin3(pin3_),
        pin4(pin4_),
        pin5(pin5_),
        pin6(pin6_),
        pin7(pin7_),
        pin8(pin8_),
        pin9(pin9_),
        pin10(pin10_),
        pin11(pin11_),
        pin12(pin12_) { };

    S_CONTROL get_pin1() { return pin1; };
    S_CONTROL get_pin2() { return pin2; };
    S_CONTROL get_pin3() { return pin3; };
    S_CONTROL get_pin4() { return pin4; };
    S_CONTROL get_pin5() { return pin5; };
    S_CONTROL get_pin6() { return pin6; };
    S_CONTROL get_pin7() { return pin7; };
    S_CONTROL get_pin8() { return pin8; };
    S_CONTROL get_pin9() { return pin9; };
    S_CONTROL get_pin10() { return pin1; };
    S_CONTROL get_pin11() { return pin11; };
    S_CONTROL get_pin12() { return pin12; };

private:
    S_CONTROL pin1;
    S_CONTROL pin2;
    S_CONTROL pin3;
    S_CONTROL pin4;
    S_CONTROL pin5;
    S_CONTROL pin6;
    S_CONTROL pin7;
    S_CONTROL pin8;
    S_CONTROL pin9;
    S_CONTROL pin10;
    S_CONTROL pin11;
    S_CONTROL pin12;
};

class Reg_Group_PWM {

};

}
