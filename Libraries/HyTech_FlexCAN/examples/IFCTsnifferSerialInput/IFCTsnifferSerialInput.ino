#include <HyTech_FlexCAN.h>

#define CB_SIZE 16
#define DATA_LEN 12

Circular_Buffer<uint32_t, CB_SIZE> ids;
Circular_Buffer<uint32_t, CB_SIZE, DATA_LEN> storage;
Circular_Buffer<uint32_t, CB_SIZE, DATA_LEN> last_entries;

uint8_t option = 0;
uint8_t format = HEX;
uint32_t last_id = 0;
uint32_t trim_frames = 0;
uint16_t ignore_count = 0;
uint8_t window_output = 0;

void setup() {
  pinMode(2, OUTPUT); // for the transceiver enable pin
  pinMode(13, OUTPUT); // toggle led when a command is attempted
  Can0.setBaudRate(1000000);
  Can0.enableFIFO(1);
  Can0.enableFIFOInterrupt(1);
  Can0.onReceive(canSniff);
  Can0.intervalTimer(); // enable queue system and run callback in background.
}

void loop() {
  // put your main code here, to run repeatedly:
}

void canSniff(CAN_message_t &msg) { // global callback
  uint32_t frame[DATA_LEN] = { msg.id };

  if ( !storage.find(frame, DATA_LEN, 0, 0, 0) ) {
    if ( storage.size() == storage.capacity() ) {
      if ( !option ) Serial.print("Buffer full, couldn't add CAN ID to the list!");
      return;
    }
    frame[0] = msg.id;
    for ( uint8_t i = 0; i < 8; i++ ) frame[i + 1] = msg.buf[i];
    frame[9] = 1;
    frame[10] = 0;
    frame[11] = millis();
    storage.push_back(frame, DATA_LEN);
    ids.push_back(msg.id);
    ids.sort_ascending();
  }
  else {
    frame[9]++;
    for ( uint8_t i = 0; i < 8; i++ ) frame[i + 1] = msg.buf[i];
    frame[10] = millis() - frame[11];
    frame[11] = millis();
    storage.replace(frame, DATA_LEN, 0, 0, 0);
  }

  if ( !option ) main_console();
  if ( option == 1 && frame[0] == last_id ) {
    last_entries.push_front(frame, DATA_LEN);
    last_output();
  }
}


void last_output() {
  uint32_t frame[DATA_LEN] = { last_id };
  Serial.print("\n\n\n   Lastest results for CAN ID: 0x");
  Serial.print(last_id, HEX);
  Serial.print("\t  Count: ");
  ( storage.find(frame, DATA_LEN, 0, 0, 0) ) ? Serial.print(frame[9]) : Serial.print("0");

  Serial.print("\tTime: ");
  Serial.println(frame[10]);

  if ( format != BIN) Serial.print("\n\tDATA[0]\tDATA[1]\tDATA[2]\tDATA[3]\tDATA[4]\tDATA[5]\tDATA[6]\tDATA[7]\t\tASCII\n\t ");
  else Serial.print("\n\t   DATA[0]\t   DATA[1]\t   DATA[2]\t   DATA[3]\t   DATA[4]\t   DATA[5]\t   DATA[6]\t   DATA[7]\t\tASCII\n\t ");

  for ( uint32_t k = 0; k < last_entries.size(); k++ ) {
    last_entries.peek_front(frame, DATA_LEN, k);
    for ( uint8_t i = 1; i < DATA_LEN; i++ ) {
      if ( i == 9 ) continue; /* we don't print count here */
      if ( i == 10 ) continue; /* time */
      if ( i == 11 ) {
        Serial.print("\t");
        for ( uint8_t p = 1; p < 9; p++ ) if ( frame[p] != '\r' && frame[p] != '\t' && frame[p] != '\n' ) Serial.print((char)frame[p]);
        continue; /* hidden millis() timestamp */
      }
      if ( format == BIN ) {
        Serial.print("0b");
        for ( uint8_t j = 0;  j < abs((32 - __builtin_clz(frame[i])) - 8); j++ ) Serial.print("0");
        if ( frame[i] ) Serial.print(frame[i], format);
        Serial.print("\t ");
      }
      else {
        if ( format == HEX && i < 9 && i > 0 ) ( frame[i] < 0x10 ) ? Serial.print("0x0") : Serial.print("0x");
        Serial.print(frame[i], (i == 9) ? DEC : format); Serial.print("\t ");
      }


    } Serial.print("\n\t ");
  } Serial.println();
  if ( window_output ) for ( uint8_t i = 0; i < window_output + last_entries.capacity() - last_entries.size() - 2; i++ ) Serial.print('\n');
}

void main_console() {
  uint32_t frame[DATA_LEN];
  uint16_t trim_counter = 0, ignore_counter = 0;
  if ( format != BIN) Serial.print("\n\n\n\tCAN ID\tDATA[0]\tDATA[1]\tDATA[2]\tDATA[3]\tDATA[4]\tDATA[5]\tDATA[6]\tDATA[7]\tCOUNT\t  TIME\t\tASCII\n    ");
  else Serial.print("\n\tCAN ID\t   DATA[0]\t   DATA[1]\t   DATA[2]\t   DATA[3]\t   DATA[4]\t   DATA[5]\t   DATA[6]\t   DATA[7]\tCOUNT\t  TIME\t\tASCII\n    ");

  for ( uint32_t k = 0; k < storage.size(); k++ ) {
    frame[0] = ids.peek(k);
    storage.find(frame, DATA_LEN, 0, 0, 0);

    /* TRIM MODE */
    if ( trim_frames && frame[10] > trim_frames ) {
      trim_counter++;
      continue;
    }
    /* IGNORE SET COUNT */
    if ( ignore_count && frame[9] > ignore_count ) {
      ignore_counter++;
      continue;
    }

    for ( uint8_t i = 0; i < DATA_LEN; i++ ) {
      if ( i == 11 ) {
        Serial.print("\t");
        for ( uint8_t p = 1; p < 9; p++ ) if ( frame[p] != '\r' && frame[p] != '\t' && frame[p] != '\n' ) Serial.print((char)frame[p]);
        continue; /* hidden millis() timestamp */
      }
      if ( !i ) {
        char padded[12];
        sprintf(padded, "%08lX", frame[i]);
        Serial.print("0x");
        Serial.print(padded); Serial.print("\t ");
        continue;
      }
      if ( format == BIN ) {
        if ( i < 9 ) {
          Serial.print("0b");
          for ( uint8_t j = 0;  j < abs((32 - __builtin_clz(frame[i])) - 8); j++ ) Serial.print("0");
        }
        if ( frame[i] ) Serial.print(frame[i], (i >= 9) ? DEC : format);
        Serial.print("\t ");
      }
      else {
        if ( format == HEX && i < 9 && i > 0 ) ( frame[i] < 0x10 ) ? Serial.print("0x0") : Serial.print("0x");
        Serial.print(frame[i], (i >= 9 ) ? DEC : format); Serial.print("\t ");
      }
    } Serial.print("\n    ");
  } Serial.println();
  if ( window_output ) for ( uint8_t i = 0; i < window_output + storage.capacity() - storage.size() + trim_counter + ignore_counter; i++ ) Serial.print('\n');
}







void serialEvent() {

  char serial_buffer[50];
  uint8_t terminate = Serial.readBytesUntil('\n', serial_buffer, sizeof(serial_buffer) - 1);
  if ( serial_buffer[terminate] != '\0' ) serial_buffer[terminate] = '\0';
  char delimiters[] = "!:, ";
  char* _pos = strtok(serial_buffer, delimiters);
  switch ( _pos[0] ) {
    case 'O': {
        _pos = strtok(NULL, delimiters);

        uint32_t frame[DATA_LEN] = { ( !atoi(_pos) ) ? strtoul(_pos, NULL, 16) : atoi(_pos) };
        last_id = frame[0];

        if ( storage.find(frame, DATA_LEN, 0, 0, 0) ) {
          last_entries.flush();
          last_entries.push_front(frame, DATA_LEN);
          option = 1;
        }
        else option = 0;
        break;
      }
    case 'D': { // DEC output
        format = DEC;
        break;
      }
    case 'B': { // BIN output
        format = BIN;
        break;
      }
    case 'H': { // HEX output
        format = HEX;
        break;
      }
    case 'I': { // TRIM output
        _pos = strtok(NULL, delimiters);
        ignore_count = strtoul(_pos, NULL, 10);
        break;
      }
    case 'W': { // Window output
        _pos = strtok(NULL, delimiters);
        window_output = strtoul(_pos, NULL, 10);
        break;
      }
    case 'T': { // TRIM output
        _pos = strtok(NULL, delimiters);
        trim_frames = strtoul(_pos, NULL, 10);
        break;
      }
    case 'M': { // main output
        option = 0;
        break;
      }
    case 'R': { // reset
        format = HEX;
        option = trim_frames = ignore_count = window_output = 0;
        __disable_irq();
        storage.flush();
        ids.flush();
        __enable_irq();
        break;
      }
      if ( !option ) main_console();
      if ( option == 1 ) last_output();
  }
  digitalWrite(13, !digitalRead(13));
}


