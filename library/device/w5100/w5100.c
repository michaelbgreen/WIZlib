/**
 * @file		w5100/w5100.c
 * @brief		W5100 HAL Source File.
 * This is used by socket.c
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 * 
 * Adapted from w5200.c by Michael Green, 2017/11/13.
 */

//#define FILE_LOG_SILENCE
#include "common/common.h"
//#include "device/w5100/w5100.h"

uint16 SMASK[TOTAL_SOCK_NUM] = { 0x7ff, 0x7ff, 0x7ff, 0x7ff }; //< Variable for Tx buffer MASK in each channel */
uint16 RMASK[TOTAL_SOCK_NUM] = { 0x7ff, 0x7ff, 0x7ff, 0x7ff }; //< Variable for Rx buffer MASK in each channel */
uint16 SSIZE[TOTAL_SOCK_NUM] = { 0x800, 0x800, 0x800, 0x800 }; //< Max Tx buffer size by each channel */
uint16 RSIZE[TOTAL_SOCK_NUM] = { 0x800, 0x800, 0x800, 0x800 }; //< Max Rx buffer size by each channel */
uint16 SBUFBASEADDRESS[TOTAL_SOCK_NUM] = { 0x4000, 0x4800, 0x5000, 0x5800 }; //< Tx buffer base address by each channel */
uint16 RBUFBASEADDRESS[TOTAL_SOCK_NUM] = { 0x6000, 0x6800, 0x7000, 0x7800 }; //< Rx buffer base address by each channel */

uint16 SOFFSET[TOTAL_SOCK_NUM] = { 0, 0, 0, 0 }; //< Current write offset into Tx buffer by each channel */
uint8 windowfull_retry_cnt[TOTAL_SOCK_NUM];

uint8 incr_windowfull_retry_cnt(uint8 s)
{
	return ++windowfull_retry_cnt[s];
}

void init_windowfull_retry_cnt(uint8 s)
{
	windowfull_retry_cnt[s] = 0;
}

uint16 getIINCHIP_RxMAX(uint8 s)
{
	return RSIZE[s];
}
uint16 getIINCHIP_TxMAX(uint8 s)
{
	return SSIZE[s];
}
uint16 getIINCHIP_RxMASK(uint8 s)
{
	return RMASK[s];
}
uint16 getIINCHIP_TxMASK(uint8 s)
{
	return SMASK[s];
}
uint16 getIINCHIP_RxBASE(uint8 s)
{
	return RBUFBASEADDRESS[s];
}
uint16 getIINCHIP_TxBASE(uint8 s)
{
	return SBUFBASEADDRESS[s];
}
void IINCHIP_CSoff(void)
{
	wizspi_cs(WIZ_SPI1, VAL_LOW);
}
void IINCHIP_CSon(void)
{
	wizspi_cs(WIZ_SPI1, VAL_HIGH);
}
uint8  IINCHIP_SpiSendData(uint8 dat)
{
	return(wizspi_byte(WIZ_SPI1, dat));
}


 /*
@brief  This function writes the data into W5100 registers.
*/
uint8 IINCHIP_WRITE(uint16 addr,uint8 data)
{
  IINCHIP_ISR_DISABLE();                      // Interrupt Service Routine Disable

  //SPI MODE I/F
  IINCHIP_CSoff();                            // CS=0, SPI start

  IINCHIP_SpiSendData(0xf0);                  // Write command
  IINCHIP_SpiSendData(addr >> 8);             // Address byte 1
  IINCHIP_SpiSendData(addr);                  // Address byte 2
  IINCHIP_SpiSendData(data);                  // Data

  IINCHIP_CSon();                             // CS=1,  SPI end

  IINCHIP_ISR_ENABLE();                       // Interrupt Service Routine Enable
  return 1;
}
 
/*
@brief  This function reads the value from W5100 registers.
*/
uint8 IINCHIP_READ(uint16 addr)
{
  uint8 data;
        
  IINCHIP_ISR_DISABLE();                       // Interrupt Service Routine Disable
  
  IINCHIP_CSoff();                             // CS=0, SPI start
  
  IINCHIP_SpiSendData(0x0f);                   // Read command
  IINCHIP_SpiSendData(addr >> 8);              // Address byte 1
  IINCHIP_SpiSendData(addr);                   // Address byte 2
  data = IINCHIP_SpiSendData(0x00);            // Data
  
  IINCHIP_CSon();                              // CS=1,  SPI end
  
  IINCHIP_ISR_ENABLE();                        // Interrupt Service Routine Enable
  return data;
}

/*
@brief  This function writes into W5100 memory(Buffer)
*/ 
uint16 IINCHIP_WRITE_BLOCK(uint16 addr,uint8* buf,uint16 len)
{
  uint16 idx = 0;

  // W5100 does not support block writes.
  for(idx = 0; idx < len; idx++)
  {   
    IINCHIP_WRITE(addr+idx, buf[idx]);
  }
  
  return len;
}

/*
@brief  This function reads into W5200 memory(Buffer)
*/ 
uint16 IINCHIP_READ_BLOCK(uint16 addr, uint8* buf,uint16 len)
{
  uint16 idx = 0;
        
  // W5100 does not support block reads.
  for(idx = 0; idx < len; idx++)
  {
    buf[idx] = IINCHIP_READ(addr+idx);
  }
        
  return len;
}

/*
@brief  This function sets up gateway IP address.
@param addr a pointer to a 4 -byte array responsible to set the GW address.
*/ 
void setGAR(uint8 *addr)
{
  IINCHIP_WRITE_BLOCK(WIZC_GAR0,addr,4);
}

/*
@brief  It sets up SubnetMask address
@param addr a pointer to a 4 -byte array responsible to set the SubnetMask address
*/ 
void setSUBR(uint8 *addr)
{
  IINCHIP_WRITE_BLOCK(WIZC_SUBR0,addr,4);
}

/*
@brief  This function sets up MAC address.
@param addr a pointer to a 6 -byte array responsible to set the MAC address. 
*/ 
void setSHAR(uint8 *addr)
{
  IINCHIP_WRITE_BLOCK(WIZC_SHAR0,addr,6);
}

/*
@brief  This function sets up Source IP address.
@param addr a pointer to a 4 -byte array responsible to set the Source IP address.
*/
void setSIPR(uint8 *addr)
{
  IINCHIP_WRITE_BLOCK(WIZC_SIPR0,addr,4);
}

/*
@brief  This function sets up Source IP address.
*/
void getGAR(uint8 *addr)
{
  IINCHIP_READ_BLOCK(WIZC_GAR0,addr,4);
}
void getSUBR(uint8 *addr)
{
  IINCHIP_READ_BLOCK(WIZC_SUBR0,addr,4);
}
void getSHAR(uint8 *addr)
{
  IINCHIP_READ_BLOCK(WIZC_SHAR0,addr,6);
}
void getSIPR(uint8 *addr)
{
  IINCHIP_READ_BLOCK(WIZC_SIPR0,addr,4);
}
void getDIPR(uint8 s, uint8 *addr)
{
  IINCHIP_READ_BLOCK(WIZ_SOCK_REG(s, WIZS_DIPR0),addr,4);
}
void getDPORT(uint8 s, uint16 *port)
{
  *port = ((uint16)IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT0)) << 8) +
    IINCHIP_READ(WIZ_SOCK_REG(s, WIZS_DPORT1));
}

void setMR(uint8 val)
{
  IINCHIP_WRITE(WIZC_MR,val);
}

/*
@brief  This function gets Interrupt register in common register.
 */
uint8 getIR(void)
{
   return IINCHIP_READ(WIZC_IR);
}


/*
 Retransmittion 
 */
 
/*
@brief  This function sets up Retransmission time.

If there is no response from the peer or delay in response then retransmission 
will be there as per RTR (Retry Time-value Register)setting
*/
void setRTR(uint16 timeout)
{
  IINCHIP_WRITE(WIZC_RTR0, (uint8)(timeout >> 8));
  IINCHIP_WRITE(WIZC_RTR1, (uint8)timeout);
}

/*
@brief  This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time 
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint8 retry)
{
  IINCHIP_WRITE(WIZC_RCR,retry);
}




/*
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void setIMR(uint8 mask)
{
  IINCHIP_WRITE(WIZC_IMR,mask); // must be setted 0x10.
}

/*
@brief  This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(uint8 s, uint16 Sn_MSSR0)
{
  IINCHIP_WRITE(Sn_MSSR0(s),(uint8)(Sn_MSSR0 >> 8));
  IINCHIP_WRITE((Sn_MSSR0(s) + 1),(uint8)Sn_MSSR0);
}

void setSn_TTL(uint8 s, uint8 ttl)
{
   IINCHIP_WRITE(Sn_TTL(s), ttl);
}


/*
@brief  These below function is used to setup the Protocol Field of IP Header when
    executing the IP Layer RAW mode.
*/
void setSn_PROTO(uint8 s, uint8 proto)
{
  IINCHIP_WRITE(Sn_PROTO(s),proto);
}


/*
@brief  get socket interrupt status

These below functions are used to read the Interrupt & Soket Status register
*/
uint8 getSn_IR(uint8 s)
{
   return IINCHIP_READ(Sn_IR(s));
}


/*
@brief   get socket status
*/
uint8 getSn_SR(uint8 s)
{
   return IINCHIP_READ(Sn_SR(s));
}


/*
@brief  get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16 getSn_TX_FSR(uint8 s)
{
    // Evidence suggests that reading the TX_FSR register always yields the
    // size of the entire buffer, not size remaining.  Also, reading TX_WR
    // always yields the offset of the start of the entire packet, NOT the
    // next available address to write.
    return ((uint16)IINCHIP_READ(Sn_TX_FSR0(s)) << 8)
      + IINCHIP_READ(Sn_TX_FSR0(s) + 1)
      - SOFFSET[s];
}


/*
@brief   get socket RX recv buf size

This gives size of received data in receive buffer. 
*/
uint16 getSn_RX_RSR(uint8 s)
{
  uint16 val=0,val1=0;
  do
  {
    val1 = IINCHIP_READ(Sn_RX_RSR0(s));
    val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
      if(val1 != 0)
    {
        val = IINCHIP_READ(Sn_RX_RSR0(s));
        val = (val << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
    }
  } while (val != val1);
   return val;
}


/*
@brief  This function is being called by send() and sendto() function also. for copy the data form application buffer to Transmite buffer of the chip.

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
And this function is being used for copy the data form application buffer to Transmite
buffer of the chip. It calculate the actual physical address where one has to write
the data in transmite buffer. Here also take care of the condition while it exceed
the Tx memory uper-bound of socket.

*/
void send_data_processing(uint8 s, uint8 *data, uint16 len)
{
  uint16 ptr;
  uint16 size;
  uint16 dst_mask;
  uint16 dst_addr;

  ptr = IINCHIP_READ(Sn_TX_WR0(s));
  ptr = (ptr << 8) + IINCHIP_READ(Sn_TX_WR0(s) + 1);
  ptr += SOFFSET[s];

  dst_mask = ptr & getIINCHIP_TxMASK(s);
  dst_addr = getIINCHIP_TxBASE(s) + dst_mask;
  if (dst_mask + len > getIINCHIP_TxMAX(s)) 
  {
    size = getIINCHIP_TxMAX(s) - dst_mask;
    IINCHIP_WRITE_BLOCK(dst_addr, data, size);
    data += size;
    size = len - size;
    dst_addr = getIINCHIP_TxBASE(s);
    IINCHIP_WRITE_BLOCK(dst_addr, data, size);
  } 
  else
  {
    IINCHIP_WRITE_BLOCK(dst_addr, data, len);
  }

  ptr += len;
  SOFFSET[s] += len;

  IINCHIP_WRITE(Sn_TX_WR0(s),(uint8)(ptr >> 8));
  IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8)ptr);
}


/*
@brief  This function is being called by recv() also. This function is being used for copy the data form Receive buffer of the chip to application buffer.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
It calculate the actual physical address where one has to read
the data from Receive buffer. Here also take care of the condition while it exceed
the Rx memory uper-bound of socket.
*/
void recv_data_processing(uint8 s, uint8 *data, uint16 len)
{
  uint16 ptr;
  uint16 size;
  uint16 src_mask;
  uint16 src_addr;

  ptr = IINCHIP_READ(Sn_RX_RD0(s));
  ptr = (ptr << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
  
  DBGA("ISR_RX: rd_ptr : %.4x", ptr);

  src_mask = ptr & getIINCHIP_RxMASK(s);
  src_addr = getIINCHIP_RxBASE(s) + src_mask;
  
  if( (src_mask + len) > getIINCHIP_RxMAX(s) ) 
  {
    size = getIINCHIP_RxMAX(s) - src_mask;
    IINCHIP_READ_BLOCK(src_addr,data,size);
    data += size;
    size = len - size;
    src_addr = getIINCHIP_RxBASE(s);
    IINCHIP_READ_BLOCK(src_addr,data,size);
  } 
  else
  {
    IINCHIP_READ_BLOCK(src_addr,data,len);
  }
    
  ptr += len;
  IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)(ptr >> 8));
  IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)ptr);
}

void recv_data_ignore(uint8 s, uint16 len)
{
  uint16 ptr;

  ptr = IINCHIP_READ(Sn_RX_RD0(s));
  ptr = (ptr << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
  ptr += len;
  IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)(ptr >> 8));
  IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)ptr);
}
