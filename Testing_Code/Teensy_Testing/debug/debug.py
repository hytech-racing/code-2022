import curses
import serial
import sys
import datetime

def main():
    if len(sys.argv) != 2:
        print('Usage:')
        print('debug.py <serial device>')
        quit()
    global screen
    screen = curses.initscr()
    screen.border('#', '#', '#', '#', 0, 0, 0, 0)
    screen.addstr(0,5,'HYTECH RACING 2018 VEHICLE SERIAL DEBUGGER')
    screen.addstr(3,5,'RMS INVERTER')
    screen.addstr(4,5,'MODULE A TEMP: ')
    screen.addstr(5,5,'MODULE B TEMP: ')
    screen.addstr(6,5,'MODULE C TEMP: ')
    screen.addstr(7,5,'GATE DRIVER BOARD TEMP: ')
    screen.addstr(8,5,'RTD 4 TEMP: ')
    screen.addstr(9,5,'RTD 5 TEMP: ')
    screen.addstr(10,5,'MOTOR TEMP: ')
    screen.addstr(11,5,'TORQUE SHUDDER: ')
    screen.addstr(12,5,'MOTOR ANGLE: ')
    screen.addstr(13,5,'MOTOR SPEED: ')
    screen.addstr(14,5,'ELEC OUTPUT FREQ: ')
    screen.addstr(15,5,'DELTA RESOLVER FILT: ')
    screen.addstr(16,5,'PHASE A CURRENT: ')
    screen.addstr(17,5,'PHASE B CURRENT: ')
    screen.addstr(18,5,'PHASE C CURRENT: ')
    screen.addstr(19,5,'DC BUS CURRENT: ')
    screen.addstr(20,5,'DC BUS VOLTAGE: ')
    screen.addstr(21,5,'OUTPUT VOLTAGE: ')
    screen.addstr(22,5,'PHASE AB VOLTAGE: ')
    screen.addstr(23,5,'PHASE BC VOLTAGE: ')
    screen.addstr(24,5,'VSM STATE: ')
    screen.addstr(25,5,'INVERTER STATE: ')
    screen.addstr(26,5,'INVERTER RUN MODE: ')
    screen.addstr(27,5,'INVERTER ACTIVE DISCHARGE STATE: ')
    screen.addstr(28,5,'INVERTER COMMAND MODE: ')
    screen.addstr(29,5,'INVERTER ENABLE: ')
    screen.addstr(30,5,'INVERTER LOCKOUT: ')
    screen.addstr(31,5,'DIRECTION COMMAND: ')
    screen.addstr(32,5,'POST FAULT LO: ')
    screen.addstr(33,5,'POST FAULT HI: ')
    screen.addstr(34,5,'RUN FAULT LO: ')
    screen.addstr(35,5,'RUN FAULT HI: ')
    screen.addstr(36,5,'COMMANDED TORQUE: ')
    screen.addstr(37,5,'TORQUE FEEDBACK: ')
    screen.addstr(38,5,'RMS UPTIME: ')
    screen.addstr(3,55,'FRONT CONTROL UNIT')
    screen.addstr(4,55,'FCU UPTIME: ')
    screen.addstr(5,55,'FCU STATE: ')
    screen.addstr(6,55,'START BUTTON ID: ')
    screen.addstr(7,55,'FCU RAW TORQUE: ')
    screen.addstr(8,55,'FCU REQUESTED TORQUE: ')
    screen.addstr(9,55,'FCU PEDAL THROTTLE 1: ')
    screen.addstr(10,55,'FCU PEDAL THROTTLE 2: ')
    screen.addstr(11,55,'FCU PEDAL BRAKE: ')
    screen.addstr(12,55,'FCU BRAKE ACT: ')
    screen.addstr(13,55,'FCU IMPLAUS THROTTLE: ')
    screen.addstr(14,55,'FCU IMPLAUS BRAKE: ')
    screen.addstr(3,105,'REAR CONTROL UNIT')
    screen.addstr(4,105,'RCU UPTIME: ')
    screen.addstr(5,105,'RCU STATE: ')
    screen.addstr(6,105,'BMS FAULT: ')
    screen.addstr(7,105,'IMD FAULT: ')
    screen.addstr(8,105,'BMS AVERAGE TEMPERATURE: ')
    screen.addstr(9,105,'BMS LOW TEMPERATURE: ')
    screen.addstr(10,105,'BMS HIGH TEMPERATURE: ')
    screen.addstr(11,105,'BMS STATE: ')
    screen.addstr(12,105,'BMS ERROR FLAGS: ')
    screen.addstr(13,105,'BMS CURRENT: ')
    screen.addstr(14,105,'BMS VOLTAGE AVERAGE: ')
    screen.addstr(15,105,'BMS VOLTAGE LOW: ')
    screen.addstr(16,105,'BMS VOLTAGE HIGH: ')
    screen.addstr(17,105,'BMS VOLTAGE TOTAL: ')
    curses.wrapper(live)
    curses.endwin()

def live(screen):
    filename = str(datetime.datetime.now()) + ".txt"
    ser = serial.Serial(sys.argv[1], 115200)
    incomingLine = ''
    screen.nodelay(True)
    char = screen.getch()
    while char != ord('q') and char != ord('Q'):
        char = screen.getch()
        data = ser.read()
        if len(data) > 0:
            incomingLine += data
            if ('\n' in incomingLine):
                with open(filename, "a") as f:
                    f.write(str(datetime.datetime.now()) + ' ' + incomingLine)
                updateScreen(screen, incomingLine[0:incomingLine.find('\n')])
                incomingLine = incomingLine[incomingLine.find('\n') + 1:]

def updateScreen(screen, incomingLine):
    if ('MODULE A TEMP' in incomingLine):
        clearLine(4,5)
        screen.addstr(4,5,incomingLine)
    if ('MODULE B TEMP' in incomingLine):
        clearLine(5,5)
        screen.addstr(5,5,incomingLine)
    if ('MODULE C TEMP' in incomingLine):
        clearLine(6,5)
        screen.addstr(6,5,incomingLine)
    if ('GATE DRIVER BOARD TEMP' in incomingLine):
        clearLine(7,5)
        screen.addstr(7,5,incomingLine)
    if ('RTD 4 TEMP' in incomingLine):
        clearLine(8,5)
        screen.addstr(8,5,incomingLine)
    if ('RTD 5 TEMP' in incomingLine):
        clearLine(9,5)
        screen.addstr(9,5,incomingLine)
    if ('MOTOR TEMP' in incomingLine):
        clearLine(10,5)
        screen.addstr(10,5,incomingLine)
    if ('TORQUE SHUDDER' in incomingLine):
        clearLine(11,5)
        screen.addstr(11,5,incomingLine)
    if ('MOTOR ANGLE' in incomingLine):
        clearLine(12,5)
        screen.addstr(12,5,incomingLine)
    if ('MOTOR SPEED' in incomingLine):
        clearLine(13,5)
        screen.addstr(13,5,incomingLine)
    if ('ELEC OUTPUT FREQ' in incomingLine):
        clearLine(14,5)
        screen.addstr(14,5,incomingLine)
    if ('DELTA RESOLVER FILT' in incomingLine):
        clearLine(15,5)
        screen.addstr(15,5,incomingLine)
    if ('PHASE A CURRENT' in incomingLine):
        clearLine(16,5)
        screen.addstr(16,5,incomingLine)
    if ('PHASE B CURRENT' in incomingLine):
        clearLine(17,5)
        screen.addstr(17,5,incomingLine)
    if ('PHASE C CURRENT' in incomingLine):
        clearLine(18,5)
        screen.addstr(18,5,incomingLine)
    if ('DC BUS CURRENT' in incomingLine):
        clearLine(19,5)
        screen.addstr(19,5,incomingLine)
    if ('DC BUS VOLTAGE' in incomingLine):
        clearLine(20,5)
        screen.addstr(20,5,incomingLine)
    if ('OUTPUT VOLTAGE' in incomingLine):
        clearLine(21,5)
        screen.addstr(21,5,incomingLine)
    if ('PHASE AB VOLTAGE' in incomingLine):
        clearLine(22,5)
        screen.addstr(22,5,incomingLine)
    if ('PHASE BC VOLTAGE' in incomingLine):
        clearLine(23,5)
        screen.addstr(23,5,incomingLine)
    if ('VSM STATE' in incomingLine):
        clearLine(24,5)
        screen.addstr(24,5,incomingLine)
    if ('INVERTER STATE' in incomingLine):
        clearLine(25,5)
        screen.addstr(25,5,incomingLine)
    if ('INVERTER RUN MODE' in incomingLine):
        clearLine(26,5)
        screen.addstr(26,5,incomingLine)
    if ('INVERTER ACTIVE DISCHARGE STATE' in incomingLine):
        clearLine(27,5)
        screen.addstr(27,5,incomingLine)
    if ('INVERTER COMMAND MODE' in incomingLine):
        clearLine(28,5)
        screen.addstr(28,5,incomingLine)
    if ('INVERTER ENABLE' in incomingLine):
        clearLine(29,5)
        screen.addstr(29,5,incomingLine)
    if ('INVERTER LOCKOUT' in incomingLine):
        clearLine(30,5)
        screen.addstr(30,5,incomingLine)
    if ('DIRECTION COMMAND' in incomingLine):
        clearLine(31,5)
        screen.addstr(31,5,incomingLine)
    if ('POST FAULT LO' in incomingLine):
        clearLine(32,5)
        screen.addstr(32,5,incomingLine)
    if ('POST FAULT HI' in incomingLine):
        clearLine(33,5)
        screen.addstr(33,5,incomingLine)
    if ('RUN FAULT LO' in incomingLine):
        clearLine(34,5)
        screen.addstr(34,5,incomingLine)
    if ('RUN FAULT HI' in incomingLine):
        clearLine(35,5)
        screen.addstr(35,5,incomingLine)
    if ('COMMANDED TORQUE' in incomingLine):
        clearLine(36,5)
        screen.addstr(36,5,incomingLine)
    if ('TORQUE FEEDBACK' in incomingLine):
        clearLine(37,5)
        screen.addstr(37,5,incomingLine)
    if ('RMS UPTIME' in incomingLine):
        clearLine(38,5)
        screen.addstr(38,5,incomingLine)
    if ('FCU UPTIME' in incomingLine):
        clearLine(4,55)
        screen.addstr(4,55,incomingLine)
    if ('FCU STATE' in incomingLine):
        clearLine(5,55)
        screen.addstr(5,55,incomingLine)
    if ('FCU START BUTTON ID' in incomingLine):
        clearLine(6,55)
        screen.addstr(6,55,incomingLine)
    if ('FCU RAW TORQUE' in incomingLine):
        clearLine(7,55)
        screen.addstr(7,55,incomingLine)
    if ('FCU REQUESTED TORQUE' in incomingLine):
        clearLine(8,55)
        screen.addstr(8,55,incomingLine)
    if ('FCU PEDAL THROTTLE 1' in incomingLine):
        clearLine(9,55)
        screen.addstr(9,55,incomingLine)
    if ('FCU PEDAL THROTTLE 2' in incomingLine):
        clearLine(10,55)
        screen.addstr(10,55,incomingLine)
    if ('FCU PEDAL BRAKE' in incomingLine):
        clearLine(11,55)
        screen.addstr(11,55,incomingLine)
    if ('FCU BRAKE ACT' in incomingLine):
        clearLine(12,55)
        screen.addstr(12,55,incomingLine)
    if ('FCU IMPLAUS THROTTLE' in incomingLine):
        clearLine(13,55)
        screen.addstr(13,55,incomingLine)
    if ('FCU IMPLAUS BRAKE' in incomingLine):
        clearLine(14,55)
        screen.addstr(14,55,incomingLine)
    if ('RCU UPTIME' in incomingLine):
        clearLine(4,105)
        screen.addstr(4,105,incomingLine)
    if ('RCU STATE' in incomingLine):
        clearLine(5,105)
        screen.addstr(5,105,incomingLine)
    if ('RCU BMS FAULT' in incomingLine):
        clearLine(6,105)
        screen.addstr(6,105,incomingLine)
    if ('RCU IMD FAULT' in incomingLine):
        clearLine(7,105)
        screen.addstr(7,105,incomingLine)
    if ('BMS AVERAGE TEMPERATURE' in incomingLine):
        clearLine(8,105)
        screen.addstr(8,105,incomingLine)
    if ('BMS LOW TEMPERATURE' in incomingLine):
        clearLine(9,105)
        screen.addstr(9,105,incomingLine)
    if ('BMS HIGH TEMPERATURE' in incomingLine):
        clearLine(10,105)
        screen.addstr(10,105,incomingLine)
    if ('BMS STATE' in incomingLine):
        clearLine(11,105)
        screen.addstr(11,105,incomingLine)
    if ('BMS ERROR FLAGS' in incomingLine):
        clearLine(12,105)
        screen.addstr(12,105,incomingLine)
    if ('BMS CURRENT' in incomingLine):
        clearLine(13,105)
        screen.addstr(13,105,incomingLine)
    if ('BMS VOLTAGE AVERAGE' in incomingLine):
        clearLine(14,105)
        screen.addstr(14,105,incomingLine)
    if ('BMS VOLTAGE LOW' in incomingLine):
        clearLine(15,105)
        screen.addstr(15,105,incomingLine)
    if ('BMS VOLTAGE HIGH' in incomingLine):
        clearLine(16,105)
        screen.addstr(16,105,incomingLine)
    if ('BMS VOLTAGE TOTAL' in incomingLine):
        clearLine(17,105)
        screen.addstr(17,105,incomingLine)
    screen.refresh()

def clearLine(y, x):
    blanks = '                                                  '
    screen.addstr(y,x,blanks);

main()
