#pragma once

#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass @Custom(parse_dashboard_leds)
class Dashboard_status {
public:
    Dashboard_status() = default;
    Dashboard_status(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8])  { memcpy(this,buf,sizeof(*this)); }
    inline void write(uint8_t buf[8]) const { memcpy(buf,this,sizeof(*this)); }

    /* Dashboard status */
    
    inline uint8_t get_dashboard_states()        const { return (dashboard_states); }
    inline bool get_start_btn()                  const { return (dashboard_states & 0x01); }
    inline bool get_buzzer_active()              const { return (dashboard_states & 0x02); }
    inline bool get_ssok_above_threshold()       const { return (dashboard_states & 0x04); }
    inline bool get_shutdown_h_above_threshold() const { return (dashboard_states & 0x08); }

    inline void set_dashboard_states(uint8_t states)       { dashboard_states = states; }
    inline void set_start_btn(bool pressed)                { dashboard_states = (dashboard_states & 0xFE) | (pressed); }
    inline void set_buzzer_active(bool active)             { dashboard_states = (dashboard_states & 0xFD) | (active  << 1); }
    inline void set_ssok_above_threshold(bool above)       { dashboard_states = (dashboard_states & 0xFB) | (above   << 2); }
    inline void set_shutdown_h_above_threshold(bool above) { dashboard_states = (dashboard_states & 0xF7) | (above   << 3); }
    inline void toggle_start_btn()                         { dashboard_states ^= 0x01; }

    // mark button: makes a note in the telemetry log
    // mode button: changes torque mode
    // mc cycle button: restarts the inverter
    // start button: starts car
    // launch_ctrl button: toggles launch control state

    /* Dashboard buttons */

    inline uint8_t get_button_flags() const { return (button_flags); }
    inline bool get_mark_btn()        const { return (button_flags & 0x01); }
    inline bool get_mode_btn()        const { return (button_flags & 0x02); }
    inline bool get_mc_cycle_btn()    const { return (button_flags & 0x04); }
    inline bool get_launch_ctrl_btn() const { return (button_flags & 0x08); }

    inline void set_button_flags (uint8_t flags)           { button_flags = flags; }
    inline void set_mark_btn (bool mark_btn)               { button_flags = (button_flags & 0xFE) | (mark_btn); }
    inline void set_mode_btn (bool mode_btn)               { button_flags = (button_flags & 0xFD) | (mode_btn        << 1); }
    inline void set_mc_cycle_btn (bool mc_cycle_btn)       { button_flags = (button_flags & 0xFB) | (mc_cycle_btn    << 2); }
    inline void set_launch_ctrl_btn (bool launch_ctrl_btn) { button_flags = (button_flags & 0xF7) | (launch_ctrl_btn << 3); }

    inline void toggle_mark_btn()        { button_flags ^= 0x01; }
    inline void toggle_mode_btn()        { button_flags ^= 0x02; }
    inline void toggle_mc_cycle_btn()    { button_flags ^= 0x04; }
    inline void toggle_launch_ctrl_btn() { button_flags ^= 0x08; }

    /* Dashboard leds */

    inline uint16_t get_led_flags()      const { return (led_flags); }
    inline uint8_t get_ams_led()         const { return (led_flags & 0x0003); }
    inline uint8_t get_imd_led()         const { return (led_flags & 0x000C) >> 2;  }
    inline uint8_t get_mode_led()        const { return (led_flags & 0x0030) >> 4;  }
    inline uint8_t get_mc_error_led()    const { return (led_flags & 0x00C0) >> 6;  }
    inline uint8_t get_start_led()       const { return (led_flags & 0x0300) >> 8;  }
    inline uint8_t get_launch_ctrl_led() const { return (led_flags & 0x0C00) >> 10; }

    inline void set_led_flags(uint16_t flags)                { led_flags = flags; }
    inline void set_ams_led(uint8_t ams_led)                 { led_flags = (led_flags & 0xFFFC) | ((ams_led      & 0x3)     );  }
    inline void set_imd_led(uint8_t imd_led)                 { led_flags = (led_flags & 0xFFF3) | ((imd_led      & 0x3) << 2);  }
    inline void set_mode_led(uint8_t mode_led)               { led_flags = (led_flags & 0xFFCF) | ((mode_led     & 0x3) << 4);  }
    inline void set_mc_error_led(uint8_t mc_error_led)       { led_flags = (led_flags & 0xFF3F) | ((mc_error_led & 0x3) << 6);  }
    inline void set_start_led(uint8_t start_led)             { led_flags = (led_flags & 0xFCFF) | ((start_led    & 0x3) << 8);  }
    inline void set_launch_ctrl_led(uint8_t launch_ctrl_led) { led_flags = (led_flags & 0xF3FF) | ((led_flags    & 0x3) << 10); }
private:
    // (4 bits) unused
    /* @Parse @Flaglist(
         start_btn,
         buzzer_active,
         ssok_above_threshold,
         shutdown_h_above_threshold
        ) */
    uint8_t dashboard_states;

    // (4 bits) unused
    /* @Parse @Flaglist(
         mark_btn,
         mode_btn,
         mc_cycle_btn,
         launch_ctrl_btn
        ) */
    uint8_t button_flags;

    /*
     * ams (2 bit)
     * imd (2 bis)
     * mode (2 bits)
     * mc_error (2 bits)
     * start (2 bits)
     * launch control (2 bits)
     * (4 bits unused)
     */
    uint16_t led_flags;
};

#pragma pack(pop)
