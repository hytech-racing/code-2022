import curses
import csv
import sys

from console_config import CONSOLE_CONFIG

class Console:
    def process(self):
        reader = csv.reader(sys.stdin)
        HEADER = next(reader)

        ID_COL = HEADER.index('id')
        LABEL_COL = HEADER.index('label')
        VALUE_COL = HEADER.index('value')
        UNIT_COL = HEADER.index('unit')

        self.screen.addstr(0,59,' - RECEIVED')

        for record in reader:
            key = record[LABEL_COL].replace('_', ' ').upper()
            line = record[VALUE_COL] + ((" " + record[UNIT_COL]) if len(record) > UNIT_COL else "")
            can_id = int(record[ID_COL], 16)

            if can_id == 0xDE: # ID_BMS_BALANCING_STATUS
                row = 22 + int(key[7])          # BAL_IC_?_CELL_?
                col = 160 + 5 * int(key[14])    # 0123456789ABCDE
                if record[VALUE_COL][0] == '1':
                    self.screen.addstr(row, col, "BAL")
                else:
                    self.clearLineBal(row, col)
            else:
                position = CONSOLE_CONFIG.get(key)
                if (position is None):
                    continue

                [ row, col ] = position
                if can_id == 0xD8 or can_id == 0xDA: # ID_BMS_DETAILED_VOLTAGES, ID_BMS_DETAILED_TEMPERATURES
                    self.clearLineShort(row, col)
                else:
                    self.clearLine(row, col)
                self.screen.addstr(row, col, line)

    def setupScreen(self, initScreen):
        self.screen = initScreen

        self.screen.border('#', '#', '#', '#', 0, 0, 0, 0)
        self.screen.addstr(0,5,'HYTECH RACING 2020 VEHICLE SERIAL DEBUGGER')

        self.screen.addstr(3,5,'RMS INVERTER')
        self.screen.addstr(40,5,'GLV CURRENT READINGS')
        self.screen.addstr(3,55,'BATTERY MANAGEMENT SYSTEM')
        self.screen.addstr(3,105,'BATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES')
        self.screen.addstr(3,155,'BATTERY MANAGEMENT SYSTEM DETAILED TEMPERATURES')

        self.screen.addstr(17,55,'MAIN CONTROL UNIT')

        for key, pos in CONSOLE_CONFIG.items():
            self.screen.addstr(pos[0], pos[1], key + ":")

        self.screen.addstr(20,155,'BATTERY MANAGEMENT SYSTEM BALANCING STATUS')
        for i in range(8):
            self.screen.addstr(21, 160 + 5 * i, 'C' + str(i))
        for i in range(7):
            self.screen.addstr(22 + i, 155, 'IC' + str(i))

        self.screen.nodelay(True)
        self.screen.refresh()

        try:
            self.process()
        except KeyboardInterrupt:
            curses.endwin()
            sys.exit(0)

    def clearLine(self, y, x):
        blanks = '                                                  '
        self.screen.addstr(y,x,blanks)

    def clearLineShort(self, y, x):
        blanks = '                      '
        self.screen.addstr(y,x,blanks)

    def clearLineBal(self, y, x):
        blanks = '   '
        self.screen.addstr(y,x,blanks)

curses.wrapper(Console().setupScreen)