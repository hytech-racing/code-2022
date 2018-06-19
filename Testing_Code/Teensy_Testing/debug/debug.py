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

    screen.addstr(3,55,'BATTERY MANAGEMENT SYSTEM')
    screen.addstr(4,55,'BMS AVERAGE TEMPERATURE: ')
    screen.addstr(5,55,'BMS LOW TEMPERATURE: ')
    screen.addstr(6,55,'BMS HIGH TEMPERATURE: ')
    screen.addstr(7,55,'BMS STATE: ')
    screen.addstr(8,55,'BMS ERROR FLAGS: ')
    screen.addstr(9,55,'BMS CURRENT: ')
    screen.addstr(10,55,'BMS VOLTAGE AVERAGE: ')
    screen.addstr(11,55,'BMS VOLTAGE LOW: ')
    screen.addstr(12,55,'BMS VOLTAGE HIGH: ')
    screen.addstr(13,55,'BMS VOLTAGE TOTAL: ')

    screen.addstr(15,55,'FRONT CONTROL UNIT')
    screen.addstr(16,55,'FCU UPTIME: ')
    screen.addstr(17,55,'FCU STATE: ')
    screen.addstr(18,55,'START BUTTON ID: ')
    screen.addstr(19,55,'FCU RAW TORQUE: ')
    screen.addstr(20,55,'FCU REQUESTED TORQUE: ')
    screen.addstr(21,55,'FCU PEDAL ACCEL 1: ')
    screen.addstr(22,55,'FCU PEDAL ACCEL 2: ')
    screen.addstr(23,55,'FCU PEDAL BRAKE: ')
    screen.addstr(24,55,'FCU BRAKE ACT: ')
    screen.addstr(25,55,'FCU IMPLAUS ACCEL: ')
    screen.addstr(26,55,'FCU IMPLAUS BRAKE: ')

    screen.addstr(28,55,'REAR CONTROL UNIT')
    screen.addstr(29,55,'RCU UPTIME: ')
    screen.addstr(30,55,'RCU STATE: ')
    screen.addstr(31,55,'RCU FLAGS: ')
    screen.addstr(32,55,'GLV BATT VOLTAGE: ')
    screen.addstr(33,55,'RCU BMS FAULT: ')
    screen.addstr(34,55,'RCU IMD FAULT: ')

    screen.addstr(3,105,'BATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES')
    screen.addstr(4,105,'IC 0 CELL 0: ')
    screen.addstr(5,105,'IC 0 CELL 1: ')
    screen.addstr(6,105,'IC 0 CELL 2: ')
    screen.addstr(7,105,'IC 0 CELL 3: ')
    screen.addstr(8,105,'IC 0 CELL 4: ')
    screen.addstr(9,105,'IC 0 CELL 5: ')
    screen.addstr(10,105,'IC 0 CELL 6: ')
    screen.addstr(11,105,'IC 0 CELL 7: ')
    screen.addstr(12,105,'IC 0 CELL 8: ')

    screen.addstr(14,105,'IC 1 CELL 0: ')
    screen.addstr(15,105,'IC 1 CELL 1: ')
    screen.addstr(16,105,'IC 1 CELL 2: ')
    screen.addstr(17,105,'IC 1 CELL 3: ')
    screen.addstr(18,105,'IC 1 CELL 4: ')
    screen.addstr(19,105,'IC 1 CELL 5: ')
    screen.addstr(20,105,'IC 1 CELL 6: ')
    screen.addstr(21,105,'IC 1 CELL 7: ')
    screen.addstr(22,105,'IC 1 CELL 8: ')

    screen.addstr(24,105,'IC 2 CELL 0: ')
    screen.addstr(25,105,'IC 2 CELL 1: ')
    screen.addstr(26,105,'IC 2 CELL 2: ')
    screen.addstr(27,105,'IC 2 CELL 3: ')
    screen.addstr(28,105,'IC 2 CELL 4: ')
    screen.addstr(29,105,'IC 2 CELL 5: ')
    screen.addstr(30,105,'IC 2 CELL 6: ')
    screen.addstr(31,105,'IC 2 CELL 7: ')
    screen.addstr(32,105,'IC 2 CELL 8: ')

    screen.addstr(34,105,'IC 3 CELL 0: ')
    screen.addstr(35,105,'IC 3 CELL 1: ')
    screen.addstr(36,105,'IC 3 CELL 2: ')
    screen.addstr(37,105,'IC 3 CELL 3: ')
    screen.addstr(38,105,'IC 3 CELL 4: ')
    screen.addstr(39,105,'IC 3 CELL 5: ')
    screen.addstr(40,105,'IC 3 CELL 6: ')
    screen.addstr(41,105,'IC 3 CELL 7: ')
    screen.addstr(42,105,'IC 3 CELL 8: ')

    screen.addstr(4,130,'IC 4 CELL 0: ')
    screen.addstr(5,130,'IC 4 CELL 1: ')
    screen.addstr(6,130,'IC 4 CELL 2: ')
    screen.addstr(7,130,'IC 4 CELL 3: ')
    screen.addstr(8,130,'IC 4 CELL 4: ')
    screen.addstr(9,130,'IC 4 CELL 5: ')
    screen.addstr(10,130,'IC 4 CELL 6: ')
    screen.addstr(11,130,'IC 4 CELL 7: ')
    screen.addstr(12,130,'IC 4 CELL 8: ')

    screen.addstr(14,130,'IC 5 CELL 0: ')
    screen.addstr(15,130,'IC 5 CELL 1: ')
    screen.addstr(16,130,'IC 5 CELL 2: ')
    screen.addstr(17,130,'IC 5 CELL 3: ')
    screen.addstr(18,130,'IC 5 CELL 4: ')
    screen.addstr(19,130,'IC 5 CELL 5: ')
    screen.addstr(20,130,'IC 5 CELL 6: ')
    screen.addstr(21,130,'IC 5 CELL 7: ')
    screen.addstr(22,130,'IC 5 CELL 8: ')

    screen.addstr(24,130,'IC 6 CELL 0: ')
    screen.addstr(25,130,'IC 6 CELL 1: ')
    screen.addstr(26,130,'IC 6 CELL 2: ')
    screen.addstr(27,130,'IC 6 CELL 3: ')
    screen.addstr(28,130,'IC 6 CELL 4: ')
    screen.addstr(29,130,'IC 6 CELL 5: ')
    screen.addstr(30,130,'IC 6 CELL 6: ')
    screen.addstr(31,130,'IC 6 CELL 7: ')
    screen.addstr(32,130,'IC 6 CELL 8: ')

    screen.addstr(34,130,'IC 7 CELL 0: ')
    screen.addstr(35,130,'IC 7 CELL 1: ')
    screen.addstr(36,130,'IC 7 CELL 2: ')
    screen.addstr(37,130,'IC 7 CELL 3: ')
    screen.addstr(38,130,'IC 7 CELL 4: ')
    screen.addstr(39,130,'IC 7 CELL 5: ')
    screen.addstr(40,130,'IC 7 CELL 6: ')
    screen.addstr(41,130,'IC 7 CELL 7: ')
    screen.addstr(42,130,'IC 7 CELL 8: ')

    screen.addstr(3,155,'BATTERY MANAGEMENT SYSTEM DETAILED TEMPERATURES')
    screen.addstr(4,155,'IC 0 THERM 0: ')
    screen.addstr(5,155,'IC 0 THERM 1: ')
    screen.addstr(6,155,'IC 0 THERM 2: ')

    screen.addstr(8,155,'IC 1 THERM 0: ')
    screen.addstr(9,155,'IC 1 THERM 1: ')
    screen.addstr(10,155,'IC 1 THERM 2: ')

    screen.addstr(12,155,'IC 2 THERM 0: ')
    screen.addstr(13,155,'IC 2 THERM 1: ')
    screen.addstr(14,155,'IC 2 THERM 2: ')

    screen.addstr(16,155,'IC 3 THERM 0: ')
    screen.addstr(17,155,'IC 3 THERM 1: ')
    screen.addstr(18,155,'IC 3 THERM 2: ')

    screen.addstr(4,180,'IC 4 THERM 0: ')
    screen.addstr(5,180,'IC 4 THERM 1: ')
    screen.addstr(6,180,'IC 4 THERM 2: ')

    screen.addstr(8,180,'IC 5 THERM 0: ')
    screen.addstr(9,180,'IC 5 THERM 1: ')
    screen.addstr(10,180,'IC 5 THERM 2: ')

    screen.addstr(12,180,'IC 6 THERM 0: ')
    screen.addstr(13,180,'IC 6 THERM 1: ')
    screen.addstr(14,180,'IC 6 THERM 2: ')

    screen.addstr(16,180,'IC 7 THERM 0: ')
    screen.addstr(17,180,'IC 7 THERM 1: ')
    screen.addstr(18,180,'IC 7 THERM 2: ')
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
                line = incomingLine[0:incomingLine.find('\n')]
                with open(filename, "a") as f:
                    f.write(str(datetime.datetime.now()) + ' ' + line + '\n')
                updateScreen(screen, line)
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
    if ('COMMANDED TORQUE:' in incomingLine):
        clearLine(36,5)
        screen.addstr(36,5,incomingLine)
    if ('TORQUE FEEDBACK' in incomingLine):
        clearLine(37,5)
        screen.addstr(37,5,incomingLine)
    if ('RMS UPTIME' in incomingLine):
        clearLine(38,5)
        screen.addstr(38,5,incomingLine)
    if ('BMS AVERAGE TEMPERATURE' in incomingLine):
        clearLine(4,55)
        screen.addstr(4,55,incomingLine)
    if ('BMS LOW TEMPERATURE' in incomingLine):
        clearLine(5,55)
        screen.addstr(5,55,incomingLine)
    if ('BMS HIGH TEMPERATURE' in incomingLine):
        clearLine(6,55)
        screen.addstr(6,55,incomingLine)
    if ('BMS STATE' in incomingLine):
        clearLine(7,55)
        screen.addstr(7,55,incomingLine)
    if ('BMS ERROR FLAGS' in incomingLine):
        clearLine(8,55)
        screen.addstr(8,55,incomingLine)
    if ('BMS CURRENT' in incomingLine):
        clearLine(9,55)
        screen.addstr(9,55,incomingLine)
    if ('BMS VOLTAGE AVERAGE' in incomingLine):
        clearLine(10,55)
        screen.addstr(10,55,incomingLine)
    if ('BMS VOLTAGE LOW' in incomingLine):
        clearLine(11,55)
        screen.addstr(11,55,incomingLine)
    if ('BMS VOLTAGE HIGH' in incomingLine):
        clearLine(12,55)
        screen.addstr(12,55,incomingLine)
    if ('BMS VOLTAGE TOTAL' in incomingLine):
        clearLine(13,55)
        screen.addstr(13,55,incomingLine)
    if ('FCU UPTIME' in incomingLine):
        clearLine(16,55)
        screen.addstr(16,55,incomingLine)
    if ('FCU STATE' in incomingLine):
        clearLine(17,55)
        screen.addstr(17,55,incomingLine)
    if ('FCU START BUTTON ID' in incomingLine):
        clearLine(18,55)
        screen.addstr(18,55,incomingLine)
    if ('FCU RAW TORQUE' in incomingLine):
        clearLine(19,55)
        screen.addstr(19,55,incomingLine)
    if ('FCU REQUESTED TORQUE' in incomingLine):
        clearLine(20,55)
        screen.addstr(20,55,incomingLine)
    if ('FCU PEDAL ACCEL 1' in incomingLine):
        clearLine(21,55)
        screen.addstr(21,55,incomingLine)
    if ('FCU PEDAL ACCEL 2' in incomingLine):
        clearLine(22,55)
        screen.addstr(22,55,incomingLine)
    if ('FCU PEDAL BRAKE' in incomingLine):
        clearLine(23,55)
        screen.addstr(23,55,incomingLine)
    if ('FCU BRAKE ACT' in incomingLine):
        clearLine(24,55)
        screen.addstr(24,55,incomingLine)
    if ('FCU IMPLAUS ACCEL' in incomingLine):
        clearLine(25,55)
        screen.addstr(25,55,incomingLine)
    if ('FCU IMPLAUS BRAKE' in incomingLine):
        clearLine(26,55)
        screen.addstr(26,55,incomingLine)
    if ('RCU UPTIME' in incomingLine):
        clearLine(29,55)
        screen.addstr(29,55,incomingLine)
    if ('RCU STATE' in incomingLine):
        clearLine(30,55)
        screen.addstr(30,55,incomingLine)
    if ('RCU FLAGS' in incomingLine):
        clearLine(31,55)
        screen.addstr(31,55,incomingLine)
    if ('GLV BATT VOLTAGE' in incomingLine):
        clearLine(32,55)
        screen.addstr(32,55,incomingLine)
    if ('RCU BMS FAULT' in incomingLine):
        clearLine(33,55)
        screen.addstr(33,55,incomingLine)
    if ('RCU IMD FAULT' in incomingLine):
        clearLine(34,55)
        screen.addstr(34,55,incomingLine)
    if ('IC 0 C' in incomingLine):
        row = 4 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 1 C' in incomingLine):
        row = 14 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 2 C' in incomingLine):
        row = 24 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 3 C' in incomingLine):
        row = 34 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 4 C' in incomingLine):
        row = 4 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 5 C' in incomingLine):
        row = 14 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 6 C' in incomingLine):
        row = 24 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 7 C' in incomingLine):
        row = 34 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 0 T' in incomingLine):
        row = 4 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 1 T' in incomingLine):
        row = 8 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 2 T' in incomingLine):
        row = 12 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 3 T' in incomingLine):
        row = 16 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 4 T' in incomingLine):
        row = 4 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 5 T' in incomingLine):
        row = 8 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 6 T' in incomingLine):
        row = 12 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 7 T' in incomingLine):
        row = 16 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    screen.refresh()

def clearLine(y, x):
    blanks = '                                                  '
    screen.addstr(y,x,blanks)

def clearLineShort(y, x):
    blanks = '                      '
    screen.addstr(y,x,blanks)

main()
