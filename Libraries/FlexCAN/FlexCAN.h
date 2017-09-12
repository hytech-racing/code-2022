// -------------------------------------------------------------
// a simple Arduino Teensy 3.1/3.2/3.6 CAN driver
// by teachop
// dual CAN support for MK66FX1M0 by Pawelsky
//
#ifndef __FLEXCAN_H__
#define __FLEXCAN_H__

#include <Arduino.h>

#define SIZE_RX_BUFFER  32 //RX incoming ring buffer size
#define SIZE_TX_BUFFER  16 //TX ring buffer size
#define SIZE_LISTENERS  4  //number of classes that can register as listeners on each CAN bus
#define NUM_MAILBOXES   16 //architecture specific but all Teensy 3.x boards have 16 mailboxes
#define IRQ_PRIORITY    64 //0 = highest, 255 = lowest

typedef struct CAN_message_t {
  uint32_t id; // can identifier
  uint8_t ext; // identifier is extended
  uint8_t rtr; //remote transmission request packet type
  uint8_t len; // length of data
  uint16_t timeout; // milliseconds, zero will disable waiting
  uint8_t buf[8];
} CAN_message_t;

typedef struct CAN_filter_t {
  uint8_t rtr;
  uint8_t ext;
  uint32_t id;
} CAN_filter_t;

class CANListener
{
public:
  CANListener();
    
  virtual void gotFrame(CAN_message_t &frame, int mailbox);

  void attachMBHandler(uint8_t mailBox);
  void detachMBHandler(uint8_t mailBox);
  void attachGeneralHandler();
  void detachGeneralHandler();
    
private:
  int callbacksActive; //bitfield letting the code know which callbacks to actually try to use (for object oriented callbacks only)
    
  friend class FlexCAN; //class has to have access to the the guts of this one 
};

// -------------------------------------------------------------
class FlexCAN
{
private:
  uint32_t flexcanBase;
  struct CAN_filter_t MBFilters[NUM_MAILBOXES];
  static struct CAN_filter_t defaultMask;
  volatile CAN_message_t rx_frame_buff[SIZE_RX_BUFFER];
  volatile CAN_message_t tx_frame_buff[SIZE_TX_BUFFER];
  volatile uint16_t rx_buffer_head, rx_buffer_tail;
  volatile uint16_t tx_buffer_head, tx_buffer_tail;
  void mailbox_int_handler(uint8_t mb, uint32_t ul_status);
  CANListener *listener[SIZE_LISTENERS];

  void writeTxRegisters(const CAN_message_t &msg, uint8_t buffer);
  void readRxRegisters(CAN_message_t &msg, uint8_t buffer);
protected:
  uint8_t numTxMailboxes;

public:
  FlexCAN(uint8_t id = 0);
  void begin(uint32_t baud = 250000, const CAN_filter_t &mask = defaultMask, uint8_t txAlt = 0, uint8_t rxAlt = 0);

  void setFilter(const CAN_filter_t &filter, uint8_t n);
  void setMask(uint32_t mask, uint8_t n);
  void end(void);
  int available(void);
  int write(const CAN_message_t &msg);
  int read(CAN_message_t &msg);
  
  //new functionality added to header but not yet implemented. Fix me
  void setListenOnly(bool mode); //pass true to go into listen only mode, false to be in normal mode

  boolean attachObj(CANListener *listener);
  boolean detachObj(CANListener *listener);
  
  //int watchFor(); //allow anything through
  //int watchFor(uint32_t id); //allow just this ID through (automatic determination of extended status)
  //int watchFor(uint32_t id, uint32_t mask); //allow a range of ids through
  //int watchForRange(uint32_t id1, uint32_t id2); //try to allow the range from id1 to id2 - automatically determine base ID and mask

  int setNumTXBoxes(int txboxes);
  
  void message_isr(void);
  void bus_off_isr(void);
  void error_isr(void);
  void tx_warn_isr(void);
  void rx_warn_isr(void);
  void wakeup_isr(void);
};

extern FlexCAN Can0;
#ifdef __MK66FX1M0__
extern FlexCAN Can1;
#endif

#endif // __FLEXCAN_H__
