/*
 *  cfSimpleModbusRtu
 *  Support only 57600 baudrate
 *  
 *  Created on: 2019. 4. 12.  Edited by Charles 
 *  2024. 1. 10 Timer인터럽트 제거, 수신후 모드버스 체크하는 방식으로 변경
 *  바이트 저장방식 변경 : 기존 1바이트에 8비트정보 저장--> 1바이트에 1비트 정보변경으로
 *  2024. 8. 13 Change to zero base (HMI에서 40001번지를 지정하면 링크배열의 1번지가 억세스됨)
 */

#ifndef CFSIMPLEMODBUSRTU
#define CFSIMPLEMODBUSRTU

#define u8 unsigned char
#define u16 unsigned short
#define XTAL 16000000

#include "Arduino.h"
#include <avr/io.h>  //get header file
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define __ring_buffer_max 40
#define __tx_ring_buffer_max 40

u8 __ring_buffer[__ring_buffer_max];
u16 __save_pointer=0;
u16 __action_pointer=__ring_buffer_max-1;

u8 __tx_ring_buffer[__tx_ring_buffer_max];
u16 __tx_save_pointer=0;
u16 __tx_action_pointer=__tx_ring_buffer_max-1;

u8 __modbusSwitch=0;       // 메인스위치
u8 modbusBlankCheck = 0;  // 통신 Garbage 체크용, 유저 메인 루프안에서 1씩 증가시켜주어야함. 한박자 쉬고 들어오는 데이터가 있다면, 그 이전 데이터는 싹다 클리어

u8 __modbusSlaveAdr=0;
u16 * __modbusWordBuffer;
u8 * __modbusBitBuffer;
u8 __modbusBroadMode = 0;
u16 __modbusIntervalCounter=0;;
u8 uchCRCHi, uchCRCLo;
u16 uINDEX;

//static HardwareSerial *port; //< Pointer to Serial class object

/*******************************************************************************

  Function Name  : comFlush
  
*******************************************************************************/

void comFlush(void)
{
  __action_pointer = __ring_buffer_max-1;
  __save_pointer = 0;
}

void tx_comFlush(void)
{
  __tx_action_pointer = __tx_ring_buffer_max-1;
  __tx_save_pointer = 0;
}


/*******************************************************************************

  Function Name  : comLen
  
*******************************************************************************/

u16 comLen(void)
{
  u16 i;
  if (__save_pointer>(__action_pointer+1)) {
    i=__save_pointer - __action_pointer-1;
    }
  else if (__save_pointer<(__action_pointer+1)) {
    i=__ring_buffer_max - __action_pointer + __save_pointer-1;
    }
  else {
    i=0;
    }
  return i;
}

u16 tx_comLen(void)
{
  u16 i;
  if (__tx_save_pointer>(__tx_action_pointer+1)) {
    i=__tx_save_pointer - __tx_action_pointer-1;
    }
  else if (__tx_save_pointer<(__tx_action_pointer+1)) {
    i=__tx_ring_buffer_max - __tx_action_pointer + __tx_save_pointer-1;
    }
  else {
    i=0;
    }
  return i;
}

/*******************************************************************************

  Function Name  : push_ringbuffer
  
*******************************************************************************/

void modbusFrameSearch(void);

void push_ringbuffer(u8 comdt)
{
  if (__modbusSwitch) {
    if (modbusBlankCheck > 200) {  
        comFlush(); // RTU스펙에 보면, 좀 쉬었다 들어오는 데이터를 시작으로 본다. 그렇다면 그 이전에 쌓인 데이터는 전부 비운다.
        }
    modbusBlankCheck = 0;
    }
  
  if(__save_pointer != __action_pointer) { 
    __ring_buffer[__save_pointer]=comdt;
    __save_pointer++;
    if(__save_pointer>=__ring_buffer_max) {
      __save_pointer=0;
      }
  }
  if (__modbusSwitch) modbusFrameSearch(); //수신직후 모아둔 데이터가 모드버스 프레임이 맞는지 체크 체크 시작
}

// 기존 simpleModbus에는 없는 기능 : 송신도 버퍼를 이용해서 송출한다.

void tx_push_ringbuffer(u8 comdt)
{
  if(__tx_save_pointer != __tx_action_pointer) {  
    __tx_ring_buffer[__tx_save_pointer]=comdt;
    __tx_save_pointer++;
    if(__tx_save_pointer>=__tx_ring_buffer_max) {
      __tx_save_pointer=0;
      }
  }
}

/*******************************************************************************

  Function Name  : read_ringbuffer
  
*******************************************************************************/

u8 read_ringbuffer(void)
{
  __action_pointer++;
  if(__action_pointer >= __ring_buffer_max){
    __action_pointer=0;
    }
  return(__ring_buffer[__action_pointer]);
}

u8 tx_read_ringbuffer(void)
{
  __tx_action_pointer++;
  if(__tx_action_pointer >= __tx_ring_buffer_max){
    __tx_action_pointer=0;
    }
  return(__tx_ring_buffer[__tx_action_pointer]);
}

/*******************************************************************************

 Interrupt Service Routine

*******************************************************************************/

ISR(USART2_RX_vect)
{
	char data = UDR2;
  push_ringbuffer(data);  
}

/*******************************************************************************

 Interrupt Service Routine

*******************************************************************************/

// Uart 2

ISR(USART2_UDRE_vect)
{
  //PORTB |= 64; // PB6 = PORT 12 - JUST CHECK TIMING
  UDR2 = tx_read_ringbuffer();
  if (tx_comLen()==0) UCSR2B &= ~(1<<UDRIE2); 
  //PORTB &= ~64;

}

void tx2_start_interrupt(void)
{
   UCSR2B |= (1<<UDRIE2);  // Usart data register empty interrupt enable
}

/*******************************************************************************

  Simple Modbus Functions

*******************************************************************************/

void mb_crc_compute(u8 rch);

u8 mb_get_byte_rtu(void)
{
  return read_ringbuffer();
}

u16 mb_get_word_rtu(void)
{
  u8 wh,wl;
  wh = read_ringbuffer();
  wl = read_ringbuffer();
  return (wh<<8)+wl;
}

void mb_inc_pointer(void)
{
  (void)read_ringbuffer();
}

void mb_put_byte_rtu(u8 dt)
{
  tx_push_ringbuffer(dt);
  mb_crc_compute(dt);
}

//
//  Bit Read RTU (FC 0,1)
//
u8 mb_ncdtb[] = {1,2,4,8,16,32,64,128};

u8 mb_collectbits(u16 adr)
{
  u8 i,j,m=0;               // 보내기 위한 데이터를 조립
  
  for (i=0; i<8; i++) {
    j = __modbusBitBuffer[adr];
    if (j & 1) m |= mb_ncdtb[i];
    adr++;
  }
  return m;
}

void mbcmd_read_bit_rtu(u8 cmd)
{
  u8 i,j,bln;
  u16 stadr,ln;
  stadr = mb_get_word_rtu(); // Start Address
  stadr++; // HMI의 어드레스와 맞추기 위해 시작어드레스를 +1로 
  ln = mb_get_word_rtu(); // Length
  mb_inc_pointer();
  mb_inc_pointer(); // CRC 이미 체크했으므로 Pass
  // Trasmit Start
  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(cmd);         // Command
  bln = ln >> 3;
  if (ln & 7) bln++;          // 나머지가 있으면 +1
  mb_put_byte_rtu(bln);           // Byte Count
  for (i=0; i<bln; i++) {
    j=mb_collectbits(stadr);
    mb_put_byte_rtu(j);
    stadr += 8;
    }
  i = uchCRCLo; // 세이브
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(i);
}

//
//  1 Bit Write (FC 5)
//

void mbcmd_write_bit_single_rtu()
{
  u8 i;
  u16 stadr,val;
  stadr = mb_get_word_rtu(); // Start Address
  stadr++; // HMI의 어드레스와 맞추기 위해 시작어드레스를 +1로 
  val = mb_get_word_rtu(); // Write Value
  mb_inc_pointer(); // CRC pass
  mb_inc_pointer();

  if (val) __modbusBitBuffer[stadr] = 1;
  else __modbusBitBuffer[stadr] = 0;
  stadr--;  // 어드레스를 다시 원래대로
  // 
  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(5);           // Command
  mb_put_byte_rtu(stadr >> 8);
  mb_put_byte_rtu(stadr);
  mb_put_byte_rtu(val >> 8);
  mb_put_byte_rtu(val);
  i = uchCRCLo;
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(i);
}

//
//  MODBUS RTU Word Read  (FC 3, 4)
//

void mbcmd_read_word_rtu(u8 cmd)
{
  u8 i;
  u16 stadr,ln,li,dt;
  stadr = mb_get_word_rtu(); // Start Address
  stadr++; // HMI와 맞추기위해
  ln = mb_get_word_rtu(); // Length
  mb_inc_pointer();
  mb_inc_pointer();   // CRC pass
  //
  //  Send Start
  //
  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(cmd);         // Command
  i = ln << 1;
  mb_put_byte_rtu(i);           // Byte Count

  for (li=0; li<ln; li++) {
    dt = __modbusWordBuffer[stadr++];
    mb_put_byte_rtu(dt >> 8); // High
    mb_put_byte_rtu(dt); // Low
    }
  i = uchCRCLo;
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(i);
}

//
//    1 Word Write (FC 6)
//

void mbcmd_write_word_single_rtu()
{
  u8 i;
  u16 stadr,val;
  stadr = mb_get_word_rtu(); // Start Address
  stadr++; // HMI와 맞추기위해
  val = mb_get_word_rtu(); // Write Value
  mb_inc_pointer();
  mb_inc_pointer();   // CRC pass
  __modbusWordBuffer[stadr] = val;
  stadr--; // 원래대로

  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(6);           // Command
  mb_put_byte_rtu(stadr >> 8);
  mb_put_byte_rtu(stadr);
  mb_put_byte_rtu(val >> 8);
  mb_put_byte_rtu(val);
  i = uchCRCLo; // Save
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(i);
}


//
//  연속으로 Bit Write (FC 15)
//

void mb_writebits(u16 adr, u8 val, u8 ln)
{
  u8 i,m;
  if (ln > 7) m=8;
    else m=ln;   // 15인경우도 8개씩 나누어서 처리
  for (i=0;i<m;i++) { // 8번 (마지막은 8번 미만) 돌면서, buffer의 내용을 수정한다.

    if (val & mb_ncdtb[i]) 
      __modbusBitBuffer[adr] = 1; // 하위 1비트만 쓴다.
    else
      __modbusBitBuffer[adr] = 0; 

    adr++;
    }
}

void mbcmd_write_bit_multiple_rtu()
{
  u8 i,j,cnt;
  u16 stadr,ln,lnsave,stadr_save;
  stadr_save = stadr = mb_get_word_rtu(); // Start Address
  lnsave = ln = mb_get_word_rtu(); // Length
  cnt = mb_get_byte_rtu(); // byte count
  stadr++; // HMI와 맞추기위해
  for (i=0; i<cnt; i++) {
    j = mb_get_byte_rtu(); // value
    mb_writebits(stadr, j, ln);
    stadr += 8;
    ln -= 8;
    }
  mb_inc_pointer();
  mb_inc_pointer();     // CRC  pass
  //
  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(15);         // Command
  mb_put_byte_rtu(stadr_save >> 8);
  mb_put_byte_rtu(stadr_save);
  mb_put_byte_rtu(lnsave >> 8);
  mb_put_byte_rtu(lnsave);
  i = uchCRCLo; // Save
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(i);
}

//
//    Multiple Worde Write (FC 16)
//

void mbcmd_write_word_multiple_rtu()
{
  u8 j;
  u16 stadr,realadr,val,ln,i;
  realadr = stadr = mb_get_word_rtu(); // Start Address
  ln = mb_get_word_rtu(); // length
  mb_inc_pointer(); // byte count
  realadr++; // HMI와 맞추기 위해
  for (i=0;i<ln;i++) {
    val = mb_get_word_rtu(); // Write Value
    __modbusWordBuffer[realadr++] = val;      
    }
  mb_inc_pointer();
  mb_inc_pointer(); 
  // 
  if (__modbusBroadMode) return;    // Brodecast Mode
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  mb_put_byte_rtu(__modbusSlaveAdr);       // Slave Address
  mb_put_byte_rtu(16);           // Command
  mb_put_byte_rtu(stadr >> 8);
  mb_put_byte_rtu(stadr);
  mb_put_byte_rtu(ln >> 8);
  mb_put_byte_rtu(ln);
  j = uchCRCLo; 
  mb_put_byte_rtu(uchCRCHi); // CRC
  mb_put_byte_rtu(j);
}


/*******************************************************************************

  CRC16

*******************************************************************************/

const u8 auchCRCH[]  =  {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40};

const u8 auchCRCL[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,0x40};

void mb_crc_compute(u8 rch)
{
  uINDEX = uchCRCHi ^ rch;
  uchCRCHi = uchCRCLo ^ auchCRCH[uINDEX];
  uchCRCLo = auchCRCL[uINDEX];
}

//
//  CRC16 라이브러리 
//  유저 소스에서 호출할 경우를 대비
//
/*
u16 getCrc16_byte(u8 * targetArray, u16 datalength)			
{
  u16 d;
  uchCRCHi = uchCRCLo = 0xff;   // CRC Compute start
  for (d=0; d<datalength; d++) {
    mb_crc_compute(targetArray[d]);
    }
  d = (uchCRCHi << 8) + uchCRCLo;
  return d;
}

u16 getCrc16_word(u16 * targetArray, u16 datalength)		// integer 배열을 위한 getCrc
{
  u16 d;
  uchCRCHi = uchCRCLo = 0xff;   
  for (d=0; d<datalength; d++) {
    mb_crc_compute(targetArray[d]);
    }
  d = (uchCRCHi << 8) + uchCRCLo;
  return d;
}
*/
/*******************************************************************************

  Modbus Main Process
  
*******************************************************************************/

void usart1init(u16 baud)
{
  unsigned long i;

  i = (unsigned long)(((XTAL/baud)/16)-1); // U2X x2
  UBRR1H = (u8)(i>>8);
  UBRR1L = (u8)i;

  UCSR1B |= (1<<RXCIE1 | 1<<RXEN1 | 1<<TXEN1);
  UCSR1C |= (1 << UCSZ10) | (1 << USBS1); // 8BIT, 1STOP BIT, NO PARITY
}

void usart2init(u16 baud)
{
  unsigned long i;

  i = (unsigned long)(((XTAL/baud)/16)-1); // U2X x2
  UBRR2H = (u8)(i>>8);
  UBRR2L = (u8)i;

  UCSR2B |= (1<<RXCIE2 | 1<<RXEN2 | 1<<TXEN2);
  UCSR2C |= (1 << UCSZ20) | (1 << USBS2); // 8BIT, 1STOP BIT, NO PARITY
}

void usart3init(u16 baud)
{
  unsigned long i;

  i = (unsigned long)(((XTAL/baud)/16)-1); // U2X x2
  UBRR3H = (u8)(i>>8);
  UBRR3L = (u8)i;

  UCSR3B |= (1<<RXCIE3 | 1<<RXEN3 | 1<<TXEN3);
  UCSR3C |= (1 << UCSZ30) | (1 << USBS3); // 8BIT, 1STOP BIT, NO PARITY
}

// 보레이트 57600 고정
// 채널3개중 한개만 사용가능

void startModbusServer(u8 modslaveadr, u16 * modbusBufferRegister, u8 * modbusBufferCoil)
{
  __modbusSlaveAdr = modslaveadr;
  __modbusSwitch = 1; // On
  __modbusWordBuffer = modbusBufferRegister;
  __modbusBitBuffer = modbusBufferCoil;
  usart2init(57600); 
  sei();
}

//
//  Main Processing
//

void modbusMainProcessing(void)
{
  u8 fc,sladr;
  sladr = mb_get_byte_rtu(); // Slave Address Read
  if (sladr == 0) {
    __modbusBroadMode = 1; // Slave Address = 0 is BrodeCast Mode
    }
  else {
    if (sladr != __modbusSlaveAdr) { // 슬레이브 어드레스가 일치하지 않을경우
      comFlush();
      return;
      }
   }

  fc = mb_get_byte_rtu(); // 펑션코드
  tx_comFlush(); // 송신버퍼 클리어
  switch (fc) {
  case 1:
  case 2:
    mbcmd_read_bit_rtu(fc);
    break;
  case 3:
  case 4:
    mbcmd_read_word_rtu(fc);
    break;
  case 5:
    mbcmd_write_bit_single_rtu();
    break;
  case 6:
    mbcmd_write_word_single_rtu();
    break;
  case 15:
    mbcmd_write_bit_multiple_rtu();
    break;
  case 16:
    mbcmd_write_word_multiple_rtu();
    break;
  default:
    comFlush(); // 없는 펑션코드일 경우
    return;
    }

  tx2_start_interrupt(); 
}

//
// 모드버스 프레임이 완성되었는지 체크 (맨뒤에 CRC16까지 다 오면 한 프레임 완성)
//

void modbusFrameSearch(void)
{
  u8 uch, ucl, bh, bl;
  u16 mclen, i;
  u16 uix;
  u16 save_pointer;
  mclen = comLen(); // 총 수신된 바이트 수

  if (mclen < 8) return; // 8바이트보다 작은 프레임은 없으므로 바로 패스

  uch = ucl = 0xff;       // 들어온 데이터를 가지고 CRC계산 시작
  save_pointer = __action_pointer; // 실행지점 포인터를 보관해둠
  for (i = 0; i< mclen-2; i++) {
    uix = uch ^ mb_get_byte_rtu(); // __action_pointer를 건드리므로 나중에 복구해주어야 함
    uch = ucl ^ auchCRCH[uix];
    ucl = auchCRCL[uix];
    }
  bh = mb_get_byte_rtu(); // 이것이 바로 CRC값
  bl = mb_get_byte_rtu();
  __action_pointer = save_pointer; // 복구

  if ((bh == uch) && (bl == ucl)) {
   	modbusMainProcessing(); // 발견됨. 메인처리 시작.
  }

}

#endif /* CFSIMPLEMODBUSRTU */
/*========================Simple modbus end===============================*/
