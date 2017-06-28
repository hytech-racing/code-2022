// -------------------------------------------------------------
// a simple Arduino Teensy 3.1/3.2/3.5/3.6 CAN driver
// by teachop
// dual CAN support for MK66FX1M0 and updates for MK64FX512 by Pawelsky
// Interrupt driven Rx/Tx with buffers, object oriented callbacks by Collin Kidder
// RTR related code by H4nky84
//
#include "FlexCAN.h"
#include "kinetis_flexcan.h"

#define FLEXCANb_MCR(b)                   (*(vuint32_t*)(b))
#define FLEXCANb_CTRL1(b)                 (*(vuint32_t*)(b+4))
#define FLEXCANb_RXMGMASK(b)              (*(vuint32_t*)(b+0x10))
#define FLEXCANb_IFLAG1(b)                (*(vuint32_t*)(b+0x30))
#define FLEXCANb_IMASK1(b)                (*(vuint32_t*)(b+0x28))
#define FLEXCANb_RXFGMASK(b)              (*(vuint32_t*)(b+0x48))
#define FLEXCANb_MBn_CS(b, n)             (*(vuint32_t*)(b+0x80+n*0x10))
#define FLEXCANb_MBn_ID(b, n)             (*(vuint32_t*)(b+0x84+n*0x10))
#define FLEXCANb_MBn_WORD0(b, n)          (*(vuint32_t*)(b+0x88+n*0x10))
#define FLEXCANb_MBn_WORD1(b, n)          (*(vuint32_t*)(b+0x8C+n*0x10))
#define FLEXCANb_IDFLT_TAB(b, n)          (*(vuint32_t*)(b+0xE0+(n*4)))
#define FLEXCANb_MB_MASK(b, n)            (*(vuint32_t*)(b+0x880+(n*4)))

CAN_filter_t FlexCAN::defaultMask;

//Some of these are complete guesses. Only really 8 and 16 have been validated.
//You have been warned. But, there aren't too many options for some of these
uint8_t bitTimingTable[21][3] = 
{
    //prop, seg1, seg2 (4 + prop + seg1 + seg2, seg2 must be at least 1)
    //No value can go over 7 here.
    {0,0,1}, //5
    {1,0,1}, //6
    {1,1,1}, //7
    {2,1,1}, //8
    {2,2,1}, //9
    {2,3,1}, //10
    {2,3,2}, //11
    {2,4,2}, //12
    {2,5,2}, //13
    {2,5,3}, //14
    {2,6,3}, //15
    {2,7,3}, //16
    {2,7,4}, //17
    {3,7,4}, //18
    {3,7,5}, //19
    {4,7,5}, //20
    {4,7,6}, //21
    {5,7,6}, //22
    {6,7,6}, //23
    {6,7,7}, //24
    {7,7,7}, //25
};

// -------------------------------------------------------------
FlexCAN::FlexCAN(uint8_t id)
{
  flexcanBase = FLEXCAN0_BASE;
#ifdef __MK66FX1M0__
  if(id > 0) flexcanBase = FLEXCAN1_BASE;
#endif

  // Default mask is allow everything
  defaultMask.rtr = 0;
  defaultMask.ext = 0;
  defaultMask.id = 0;
  
  rx_buffer_head = 0;
  rx_buffer_tail = 0;
  tx_buffer_head = 0;
  tx_buffer_tail = 0;
  
  for (int i = 0; i < SIZE_LISTENERS; i++) listener[i] = NULL;
}


 /* \brief Bring the hardware into freeze which drops it off the CAN bus
 *
 * \param none
 *
 * \retval none
 *
 */
void FlexCAN::end(void)
{
  // enter freeze mode
  FLEXCANb_MCR(flexcanBase) |= (FLEXCAN_MCR_HALT);
  while(!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK))
    ;
}


 /* \brief Initializes the CAN bus to the given settings
 *
 * \param baud - Set the baud rate of the bus. Only certain values are valid 50000, 100000, 125000, 250000, 500000, 1000000
 * \param mask - A default mask to use for all mailbox masks. Optional.
 * \param txAlt - 1 to enable alternate TX pin (where available)
 * \param rxAlt - 1 to enable alternate RX pin (where available)
 *
 * \retval none
 *
 */
void FlexCAN::begin(uint32_t baud, const CAN_filter_t &mask, uint8_t txAlt, uint8_t rxAlt)
{   
  // set up the pins
  if(flexcanBase == FLEXCAN0_BASE)
  {
    //Serial.println("Begin setup of CAN0");
#if defined(__MK66FX1M0__) || defined(__MK64FX512__)
    //  3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
    // 29=PTB18=CAN0_TX, 30=PTB19=CAN0_RX (alternative)
    if(txAlt == 1) CORE_PIN29_CONFIG = PORT_PCR_MUX(2); else CORE_PIN3_CONFIG = PORT_PCR_MUX(2); 
    if(rxAlt == 1) CORE_PIN30_CONFIG = PORT_PCR_MUX(2); else CORE_PIN4_CONFIG = PORT_PCR_MUX(2);// | PORT_PCR_PE | PORT_PCR_PS; 
#else   
    //  3=PTA12=CAN0_TX,  4=PTA13=CAN0_RX (default)
    // 32=PTB18=CAN0_TX, 25=PTB19=CAN0_RX (alternative)
    if(txAlt == 1) CORE_PIN32_CONFIG = PORT_PCR_MUX(2); else CORE_PIN3_CONFIG = PORT_PCR_MUX(2); 
    if(rxAlt == 1) CORE_PIN25_CONFIG = PORT_PCR_MUX(2); else CORE_PIN4_CONFIG = PORT_PCR_MUX(2);// | PORT_PCR_PE | PORT_PCR_PS;
#endif
  }
#ifdef __MK66FX1M0__
  else if(flexcanBase == FLEXCAN1_BASE)
  {
      //Serial.println("Begin setup of CAN1");
    // 33=PTE24=CAN1_TX, 34=PTE25=CAN1_RX (default)
    // NOTE: Alternative CAN1 pins are not broken out on Teensy 3.6
    CORE_PIN33_CONFIG = PORT_PCR_MUX(2);
    CORE_PIN34_CONFIG = PORT_PCR_MUX(2);// | PORT_PCR_PE | PORT_PCR_PS;
  }
#endif

  // select clock source 16MHz xtal
  OSC0_CR |= OSC_ERCLKEN;
  if(flexcanBase == FLEXCAN0_BASE) SIM_SCGC6 |=  SIM_SCGC6_FLEXCAN0;
#ifdef __MK66FX1M0__
  else if(flexcanBase == FLEXCAN1_BASE) SIM_SCGC3 |=  SIM_SCGC3_FLEXCAN1;
#endif
  FLEXCANb_CTRL1(flexcanBase) &= ~FLEXCAN_CTRL_CLK_SRC;

  // enable CAN
  FLEXCANb_MCR(flexcanBase) |=  FLEXCAN_MCR_FRZ;
  FLEXCANb_MCR(flexcanBase) &= ~FLEXCAN_MCR_MDIS;
  while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_LPM_ACK)
    ;
  // soft reset
  FLEXCANb_MCR(flexcanBase) ^=  FLEXCAN_MCR_SOFT_RST;
  while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_SOFT_RST)
    ;
  // wait for freeze ack
  while(!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK))
    ;
  // disable self-reception
  FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_SRX_DIS;

  /*
    now using a system that tries to automatically generate a viable baud setting.
    Bear these things in mind:
    - The master clock is 16Mhz
    - You can freely divide it by anything from 1 to 256
    - There is always a start bit (+1)
    - The rest (prop, seg1, seg2) are specified 1 less than their actual value (aka +1)
    - This gives the low end bit timing as 5 (1 + 1 + 2 + 1) and the high end 25 (1 + 8 + 8 + 8)
    A worked example: 16Mhz clock, divisor = 19+1, bit values add up to 16 = 16Mhz / 20 / 16 = 50k baud        
  */

  //have to find a divisor that ends up as close to the target baud as possible while keeping the end result between 5 and 25
  int divisor = 0; 
  int result = 16000000 / baud / (divisor + 1);
  int error = baud - (16000000 / (result * (divisor + 1))); 
  int bestDivisor = 0;
  int bestError = error;

  while (result > 5)
  {
        divisor++;
        result = 16000000 / baud / (divisor + 1);
        if (result <= 25)
        {
            error = baud - (16000000 / (result * (divisor + 1)));
            if (error < 0) error *= -1;
            //if this error is better than we've ever seen then use it - it's the best option
            if (error < bestError) 
            {
                bestError = error;
                bestDivisor = divisor;
            }
            //If this is equal to a previously good option then
            //switch to it but only if the bit time result was in the middle of the range
            //this biases the output to use the middle of the range all things being equal
            //Otherwise it might try to use a higher divisor and smaller values for prop, seg1, seg2
            //and that's not necessarily the best idea.
            if (error == bestError && result > 11 && result < 19)
            {
                bestError = error;
                bestDivisor = divisor;                
            }
        }        
  }
  
  divisor = bestDivisor;
  result = 16000000 / baud / (divisor + 1);
  
  if (result < 5 || result > 25 || bestError > 300) 
  {
      Serial.println("Abort in CAN begin. Couldn't find a suitable baud config!");
      return;
  }
  
  result -= 5; //the bitTimingTable is offset by 5 since there was no reason to store bit timings for invalid numbers
  int propSeg = bitTimingTable[result][0];
  int pSeg1 = bitTimingTable[result][1];
  int pSeg2 = bitTimingTable[result][2];
  
  //obviously do not uncomment these lines in production. Just for testing
  //when you need to debug what is going on with a non-standard baud rate.
  /*
  Serial.println("Bit time values:");
  Serial.print("Prop = ");
  Serial.println(propSeg + 1);
  Serial.print("Seg1 = ");
  Serial.println(pSeg1 + 1);
  Serial.print("Seg2 = ");
  Serial.println(pSeg2 + 1);
  Serial.print("Divisor = ");
  Serial.println(divisor + 1);
  */
  
  FLEXCANb_CTRL1(flexcanBase) = (FLEXCAN_CTRL_PROPSEG(propSeg) | FLEXCAN_CTRL_RJW(1)
                                | FLEXCAN_CTRL_PSEG1(pSeg1) | FLEXCAN_CTRL_PSEG2(pSeg2) | FLEXCAN_CTRL_PRESDIV(divisor));

  FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_IRMQ; //enable per-mailbox filtering
  //now have to set default mask and filter for all the RX mailboxes or they won't receive anything by default.
  CAN_filter_t defaultFilter;
  defaultFilter.ext = 0;
  defaultFilter.rtr = 0;
  defaultFilter.id = 0;
  for (int c = 0; c < NUM_MAILBOXES - numTxMailboxes; c++)
  {
     setMask(0, c);
     setFilter(defaultFilter, c);
  }
    
  // start the CAN
  FLEXCANb_MCR(flexcanBase) &= ~(FLEXCAN_MCR_HALT);
  // wait till exit of freeze mode
  while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK);

  // wait till ready
  while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_NOT_RDY);
  
  setNumTXBoxes(2);
    
#if defined(__MK20DX256__)
  NVIC_SET_PRIORITY(IRQ_CAN_MESSAGE, IRQ_PRIORITY);
  NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
#elif defined(__MK64FX512__)
  NVIC_SET_PRIORITY(IRQ_CAN0_MESSAGE, IRQ_PRIORITY);
  NVIC_ENABLE_IRQ(IRQ_CAN0_MESSAGE);
#elif defined(__MK66FX1M0__)
  if(flexcanBase == FLEXCAN0_BASE) 
  {
    NVIC_SET_PRIORITY(IRQ_CAN0_MESSAGE, IRQ_PRIORITY);
    NVIC_ENABLE_IRQ(IRQ_CAN0_MESSAGE);
  }
  else
  {
    NVIC_SET_PRIORITY(IRQ_CAN1_MESSAGE, IRQ_PRIORITY);
    NVIC_ENABLE_IRQ(IRQ_CAN1_MESSAGE);      
  }
#endif

  FLEXCANb_IMASK1(flexcanBase) = 0xFFFF; //enable interrupt masks for all 16 mailboxes

  //Serial.println("CAN initialized properly");
}

/* \brief Set listen only mode on or off.
 *
 * \param mode - set listen only mode?
 *
 * \retval None.
 *
 */
void FlexCAN::setListenOnly(bool mode)
{
    if (!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK)) { //enter freeze mode if not already there
       FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_FRZ;
       FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_HALT;
       while(!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK));
    }
    
    if (mode) FLEXCANb_CTRL1(flexcanBase) |= FLEXCAN_CTRL_LOM;
    else FLEXCANb_CTRL1(flexcanBase) &= ~FLEXCAN_CTRL_LOM;
    
    //exit freeze mode and wait until it is unfrozen.
    FLEXCANb_MCR(flexcanBase) &= ~FLEXCAN_MCR_HALT;
    while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK);
}

 /* \brief Initializes mailboxes to the requested mix of RX and TX boxes
 *
 * \param txboxes How many of the 8 boxes should be used for TX
 *
 * \retval number of tx boxes set.
 *
 */
int FlexCAN::setNumTXBoxes(int txboxes) {
    int c;

    if (txboxes > 15) txboxes = 15;
    if (txboxes < 1) txboxes = 1;
    numTxMailboxes = txboxes;

    //Inialize RX boxen
    for (c = 0; c < NUM_MAILBOXES - numTxMailboxes; c++) {
        FLEXCANb_MBn_CS(flexcanBase, c) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_RX_EMPTY);
    }

    //Initialize TX boxen
    for (c = NUM_MAILBOXES - numTxMailboxes; c < NUM_MAILBOXES; c++) {
        FLEXCANb_MBn_CS(flexcanBase, c) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
    }
    
    return (numTxMailboxes);
}

 /* \brief Sets a per-mailbox filter. Sets both the storage and the actual mailbox.
 *
 * \param filter is a filled out filter structure, n is the mailbox to update
 *
 * \retval Nothing
 *
 */
void FlexCAN::setFilter(const CAN_filter_t &filter, uint8_t n)
{
   if (n < NUM_MAILBOXES - numTxMailboxes) 
   {
       MBFilters[n] = filter;
       if (filter.ext) 
       {
          FLEXCANb_MBn_ID(flexcanBase, n) = (filter.id & FLEXCAN_MB_ID_EXT_MASK);
          FLEXCANb_MBn_CS(flexcanBase, n) |= FLEXCAN_MB_CS_IDE;
       } else {
          FLEXCANb_MBn_ID(flexcanBase, n) = FLEXCAN_MB_ID_IDSTD(filter.id);
          FLEXCANb_MBn_CS(flexcanBase, n) &= ~FLEXCAN_MB_CS_IDE;
       }
   }
}

/*
 * Per mailbox masks can only be set in freeze mode so have to enter that mode if not already there.
 */
void FlexCAN::setMask(uint32_t mask, uint8_t n)
{
    if (n >= NUM_MAILBOXES - numTxMailboxes) return;
    
    if (!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK)) { //enter freeze mode if not already there
       FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_FRZ;
       FLEXCANb_MCR(flexcanBase) |= FLEXCAN_MCR_HALT;
       while(!(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK));
    }
    
    FLEXCANb_MB_MASK(flexcanBase, n) = mask;
    
    //exit freeze mode and wait until it is unfrozen.
    FLEXCANb_MCR(flexcanBase) &= ~FLEXCAN_MCR_HALT;
    while(FLEXCANb_MCR(flexcanBase) & FLEXCAN_MCR_FRZ_ACK);
}


// -------------------------------------------------------------
int FlexCAN::available(void)
{
    int val;
    if (rx_buffer_head != rx_buffer_tail) 
    {
        val = rx_buffer_head - rx_buffer_tail;
        //Now, because this is a cyclic buffer it is possible that the ordering was reversed
        //So, handle that case
        if (val < 0) val += SIZE_RX_BUFFER;
    }
    else return 0;
    
    return val;
}


/**
 * \brief Retrieve a frame from the RX buffer
 *
 * \param buffer Reference to the frame structure to fill out
 *
 * \retval 0 no frames waiting to be received, 1 if a frame was returned
 */
int FlexCAN::read(CAN_message_t &msg)
{
    if (rx_buffer_head == rx_buffer_tail) return 0;
    msg.id = rx_frame_buff[rx_buffer_tail].id;
    msg.ext = rx_frame_buff[rx_buffer_tail].ext;
    msg.len = rx_frame_buff[rx_buffer_tail].len;
    msg.rtr = rx_frame_buff[rx_buffer_tail].rtr;
    for (int c = 0; c < 8; c++) msg.buf[c] = rx_frame_buff[rx_buffer_tail].buf[c];
    rx_buffer_tail = (rx_buffer_tail + 1) % SIZE_RX_BUFFER;

    return 1;
}

/**
 * \brief Send a frame out of this canbus port
 *
 * \param txFrame The filled out frame structure to use for sending
 *
 * \note Will do one of two things - 1. Send the given frame out of the first available mailbox
 * or 2. queue the frame for sending later via interrupt. Automatically turns on TX interrupt
 * if necessary.
 * 
 * Returns whether sending/queueing succeeded. Will not smash the queue if it gets full.
 */    
int FlexCAN::write(const CAN_message_t &msg)
{
  // find an available buffer
  int buffer = -1;
  for (int index = NUM_MAILBOXES - numTxMailboxes - 1; index < NUM_MAILBOXES; index++) {
    if ((FLEXCANb_MBn_CS(flexcanBase, index) & FLEXCAN_MB_CS_CODE_MASK) == FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE)) {
      buffer = index;
      break;// found one
    }    
  }

  if (buffer > -1)
  {
     //Serial.println("Writing a frame directly.");
     writeTxRegisters(msg, buffer);
     return 1;
  }
  else //no mailboxes available. Try to buffer it
  {
    uint8_t temp;
    //Serial.println("Buffered TX");
    temp = (tx_buffer_tail + 1) % SIZE_TX_BUFFER;
    if (temp == tx_buffer_head) return 0;
    tx_frame_buff[tx_buffer_tail].id = msg.id;
    tx_frame_buff[tx_buffer_tail].ext = msg.ext;
    tx_frame_buff[tx_buffer_tail].rtr = msg.rtr;
    tx_frame_buff[tx_buffer_tail].len  = msg.len;
    for (int c = 0; c < 8; c++) tx_frame_buff[tx_buffer_tail].buf[c] = msg.buf[c];
    tx_buffer_tail = temp;
    return 1;
      
  }

  return 0; //could not send the frame!
}

void FlexCAN::writeTxRegisters(const CAN_message_t &msg, uint8_t buffer)
{
  // transmit the frame
  FLEXCANb_MBn_CS(flexcanBase, buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_INACTIVE);
  if(msg.ext) {
    FLEXCANb_MBn_ID(flexcanBase, buffer) = (msg.id & FLEXCAN_MB_ID_EXT_MASK);
  } else {
    FLEXCANb_MBn_ID(flexcanBase, buffer) = FLEXCAN_MB_ID_IDSTD(msg.id);
  }
  FLEXCANb_MBn_WORD0(flexcanBase, buffer) = (msg.buf[0]<<24)|(msg.buf[1]<<16)|(msg.buf[2]<<8)|msg.buf[3];
  FLEXCANb_MBn_WORD1(flexcanBase, buffer) = (msg.buf[4]<<24)|(msg.buf[5]<<16)|(msg.buf[6]<<8)|msg.buf[7];

  if(msg.ext) {
    if(msg.rtr) {
        FLEXCANb_MBn_CS(flexcanBase, buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                                         | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE | FLEXCAN_MB_CS_RTR;
    } else {
        FLEXCANb_MBn_CS(flexcanBase, buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                                         | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_SRR | FLEXCAN_MB_CS_IDE;
    }
  } else {
    if(msg.rtr) {
        FLEXCANb_MBn_CS(flexcanBase, buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                                         | FLEXCAN_MB_CS_LENGTH(msg.len) | FLEXCAN_MB_CS_RTR;
    } else {
        FLEXCANb_MBn_CS(flexcanBase, buffer) = FLEXCAN_MB_CS_CODE(FLEXCAN_MB_CODE_TX_ONCE)
                                         | FLEXCAN_MB_CS_LENGTH(msg.len);
    }
  }    
}

void FlexCAN::readRxRegisters(CAN_message_t& msg, uint8_t buffer)
{
  // get identifier and dlc
  msg.len = FLEXCAN_get_length(FLEXCANb_MBn_CS(flexcanBase, buffer));
  msg.ext = (FLEXCANb_MBn_CS(flexcanBase, buffer) & FLEXCAN_MB_CS_IDE)? 1:0;
  msg.rtr = (FLEXCANb_MBn_CS(flexcanBase, buffer) & FLEXCAN_MB_CS_RTR)? 1:0;
  msg.id  = (FLEXCANb_MBn_ID(flexcanBase, buffer) & FLEXCAN_MB_ID_EXT_MASK);
  if(!msg.ext) {
    msg.id >>= FLEXCAN_MB_ID_STD_BIT_NO;
  }

  // copy out message
  uint32_t dataIn = FLEXCANb_MBn_WORD0(flexcanBase, buffer);
  msg.buf[3] = dataIn;
  dataIn >>=8;
  msg.buf[2] = dataIn;
  dataIn >>=8;
  msg.buf[1] = dataIn;
  dataIn >>=8;
  msg.buf[0] = dataIn;
  if ( 4 < msg.len ) {
    dataIn = FLEXCANb_MBn_WORD1(flexcanBase, buffer);
    msg.buf[7] = dataIn;
    dataIn >>=8;
    msg.buf[6] = dataIn;
    dataIn >>=8;
    msg.buf[5] = dataIn;
    dataIn >>=8;
    msg.buf[4] = dataIn;
  }
  for( int loop=msg.len; loop<8; ++loop ) {
    msg.buf[loop] = 0;
  }
}


//a message either came in or was freshly sent. Figure out which and act accordingly.
void FlexCAN::message_isr(void) 
{
    uint8_t temp;
    uint32_t status = FLEXCANb_IFLAG1(flexcanBase);
    CAN_message_t readMesg;
    bool caughtFrame;
    CANListener *thisListener;
    
    for (int i = 0; i < 16; i++) if (status & (1 << i)) //has this mailbox triggered an interrupt?
    {        
        uint32_t code = FLEXCAN_get_code(FLEXCANb_MBn_CS(flexcanBase, i));
        switch (code)
        {
        /* //these codes exist but aren't useful here as far as I know. Just kept for reference and in case they're needed some day.
        case 0: //inactive Receive box. Must be a false alarm!?
            break;
        case 1: //mailbox is busy. Don't touch it.
            break;
        case 4: //rx empty already. Why did it interrupt then?            
            break;                                 
        case 9: //TX being aborted.
            break;
        case 0xA: //remote request response. Remote request is deprecated and I don't care about it. Be gone!
            break; 
        case 0xC: //TX mailbox is full and will be sent as soon as possible
            break;
        case 0xE: //remote request junk again. Go away.
            break;
        */    
        case 2: //rx full, that's more like it. Copy the frame to RX buffer
        case 6: //rx overrun. We didn't get there in time and a second frame tried to enter the MB. Whoops... Can probably still grab the frame though.
            readRxRegisters(readMesg, i);            
            caughtFrame = false;
            //First, try to send a callback. If no callback registered then buffer the frame.
            for (int listenerPos = 0; listenerPos < SIZE_LISTENERS; listenerPos++)
            {
                thisListener = listener[listenerPos];
                if (thisListener != NULL)
                {
                    if (thisListener->callbacksActive & (1 << i)) 
                    {
                        caughtFrame = true;
                        thisListener->gotFrame(readMesg, i);
                    }
                    else if (thisListener->callbacksActive & (1 << 31)) 
                    {
                        caughtFrame = true;
                        thisListener->gotFrame(readMesg, -1);
                    }
                }
            }
            
            if (!caughtFrame) //if no objects caught this frame then queue it in the buffer
            {
                temp = (rx_buffer_head + 1) % SIZE_RX_BUFFER;
                if (temp != rx_buffer_tail) 
                {
                    memcpy((void *) &rx_frame_buff[rx_buffer_head], &readMesg, sizeof(CAN_message_t));                    
                    rx_buffer_head = temp;
                }                
            }            
                        
            //it seems filtering works by matching against the ID stored in the mailbox
            //so after a frame comes in we've got to refresh the ID field to be the filter ID and not the ID
            //that just came in.
            if(MBFilters[i].ext) {
                FLEXCANb_MBn_ID(flexcanBase, i) = (MBFilters[i].id & FLEXCAN_MB_ID_EXT_MASK);
            } else {
                FLEXCANb_MBn_ID(flexcanBase, i) = FLEXCAN_MB_ID_IDSTD(MBFilters[i].id);
            }
            break;
        case 8: //TX inactive. Just chillin' waiting for a message to send. Let's see if we've got one.
            if (tx_buffer_head != tx_buffer_tail) 
            { //if there is a frame in the queue to send
                writeTxRegisters((CAN_message_t &)tx_frame_buff[tx_buffer_head], i);                
                tx_buffer_head = (tx_buffer_head + 1) % SIZE_TX_BUFFER;
            }
            break;
        }
    }
    
    FLEXCANb_IFLAG1(flexcanBase) = status; //writing its value back to itself clears all flags
}

boolean FlexCAN::attachObj(CANListener *listener)
{
    for (int i = 0; i < SIZE_LISTENERS; i++)
    {
        if (this->listener[i] == NULL)
        {
            this->listener[i] = listener;
            listener->callbacksActive = 0;
            return true;            
        }
    }
    return false;
}

boolean FlexCAN::detachObj(CANListener *listener)
{
    for (int i = 0; i < SIZE_LISTENERS; i++)
    {
        if (this->listener[i] == listener)
        {
            this->listener[i] = NULL;           
            return true;            
        }
    }
    return false;  
}

void FlexCAN::bus_off_isr(void)
{
    
}

void FlexCAN::error_isr(void)
{
    
}

void FlexCAN::tx_warn_isr(void)
{
    
}

void FlexCAN::rx_warn_isr(void)
{
    
}

void FlexCAN::wakeup_isr(void)
{
    
}

void can0_message_isr(void) {
    Can0.message_isr();
}

void can0_bus_off_isr(void) {
    Can0.bus_off_isr();
}

void can0_error_isr(void) {
    Can0.error_isr();
}

void can0_tx_warn_isr(void) {
    Can0.tx_warn_isr();
}

void can0_rx_warn_isr(void) {
    Can0.rx_warn_isr();
}

void can0_wakeup_isr(void) {
    Can0.wakeup_isr();
}

#ifdef __MK66FX1M0__
void can1_message_isr(void) {
    Can1.message_isr();
}

void can1_bus_off_isr(void) {
    Can1.bus_off_isr();
}

void can1_error_isr(void) {
    Can1.error_isr();
}

void can1_tx_warn_isr(void) {
    Can1.tx_warn_isr();
}

void can1_rx_warn_isr(void) {
    Can1.rx_warn_isr();
}

void can1_wakeup_isr(void) {
    Can1.wakeup_isr();
}
#endif

CANListener::CANListener()
{
    callbacksActive = 0; //none. Bitfield were bits 0-15 are the mailboxes and bit 31 is the general callback
}

//an empty version so that the linker doesn't complain that no implementation exists.
void CANListener::gotFrame(CAN_message_t &frame, int mailbox)
{
  
}

void CANListener::attachMBHandler(uint8_t mailBox)
{
    if (mailBox >= 0 && mailBox < NUM_MAILBOXES)
    {
        callbacksActive |= (1L << mailBox);
    }
}

void CANListener::detachMBHandler(uint8_t mailBox)
{
    if (mailBox >= 0 && mailBox < NUM_MAILBOXES)
    {
        callbacksActive &= ~(1L << mailBox);
    }  
}

void CANListener::attachGeneralHandler()
{
    callbacksActive |= (1L << 31);
}

void CANListener::detachGeneralHandler()
{
    callbacksActive &= ~(1L << 31);
}

FlexCAN Can0(0);
#ifdef __MK66FX1M0__
FlexCAN Can1(1);
#endif