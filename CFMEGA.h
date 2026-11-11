/*
    CFMEGA2.h
    Created on: 2024. 06. 07.
*/

#ifndef _CFMEGA_H_
#define _CFMEGA_H_
#include <Arduino.h>

#define u8 unsigned char
#define u16 unsigned int

//###########  NTC_TEMP  ############//

#define RESISTANCE_VALUE     3970.0// CB502FG NTC - 온도계수 3435.0    4001
#define NTC_RESISTANCE       10000.0
int16_t ntcRead_a4(void);


//###########  CLASS II2C_1  ############//
#define scl_high1 PORTL |= 0x10
#define scl_low1 PORTL &= ~0x10
#define sda_high1 PORTL |= 0x20
#define sda_low1 PORTL &= ~0x20

#define scl_input1 DDRL &= ~0x10
#define scl_output1 DDRL |= 0x10
#define scl_read1 PINL & 0x10

#define sda_input1 DDRL &= ~0x20
#define sda_output1 DDRL |= 0x20
#define sda_read1 PINL & 0x20
//###########  CLASS II2C_2  ############//
#define scl_high2 PORTL |= 0x04
#define scl_low2 PORTL &= ~0x04
#define sda_high2 PORTL |= 0x08
#define sda_low2 PORTL &= ~0x08

#define scl_input2 DDRL &= ~0x04
#define scl_output2 DDRL |= 0x04
#define scl_read2 PINL & 0x04

#define sda_input2 DDRL &= ~0x08
#define sda_output2 DDRL |= 0x08
#define sda_read2 PINL & 0x08
//###########  CLASS II2C_3  ############//
#define scl_high3 PORTL |= 0x01
#define scl_low3 PORTL &= ~0x01
#define sda_high3 PORTL |= 0x02
#define sda_low3 PORTL &= ~0x02

#define scl_input3 DDRL &= ~0x01
#define scl_output3 DDRL |= 0x01
#define scl_read3 PINL & 0x01

#define sda_input3 DDRL &= ~0x02
#define sda_output3 DDRL |= 0x02
#define sda_read3 PINL & 0x02
//
#define _WDR() asm volatile("wdr")


/*********************************************************************************************************
    CLASS II2C 3
*********************************************************************************************************/
class SoftwareI2C_3 {
private:
public:
    void i2c_delay();
    void i2c_delaylong();
    void i2c_start();
    void i2c_stop();
    u8 i2c_ack();
    u8 beginTransmission(u8 addr);
    u8 requestFrom(u8 addr);
    void endTransmission();
    
    u8 write(u8 dt);
    u8 read_na();     //1����Ʈ 
    u8 read_ack();    //2����Ʈ 
};
/*********************************************************************************************************
    CLASS II2C 2
*********************************************************************************************************/
class SoftwareI2C_2 {
private:
    
public:

    void i2c_delay();
    void i2c_delaylong();
    void i2c_start();
    void i2c_stop();
    u8 i2c_ack();
    u8 beginTransmission(u8 addr);
    u8 requestFrom(u8 addr);
    void endTransmission();

    u8 write(u8 dt);
    u8 read_na();     //1����Ʈ 
    u8 read_ack();    //2����Ʈ 
};

/*********************************************************************************************************
    CLASS II2C 1
*********************************************************************************************************/
class SoftwareI2C_1 {
private:
public:
    bool delay_100kh;
    void i2c_delay();
    void i2c_delaylong();
    void i2c_start();
    void i2c_stop();
    u8 i2c_ack();
    u8 beginTransmission(u8 addr);
    u8 requestFrom(u8 addr);
    void endTransmission();
    u8 write(u8 dt);
    u8 read_na();     //1����Ʈ
    u8 read_ack();    //2����Ʈ
};

/*********************************************************************************************************
    CLASS_I2C_3 CF_DO16N
*********************************************************************************************************/
#define II2C_1    0X01
#define II2C_2    0X02
#define II2C_3    0X03

//module_ID
#define CFDO16N_ID    0X20
#define CFDI16B_ID    0X20
#define CFADCA4L_ID   0X48
#define CFDAC2V_ID    0X58
#define CFRTC_ID      0X68
#define CFTC_ID       0X60

#define u8 unsigned char
#define u16 unsigned int

#define _SEC_       0x00
#define _MIN_       0x01
#define _HOUR_      0x02
#define _DAYWEEK_   0x03
#define _DATA_      0x04
#define _MONTH_     0x05
#define _YEAR_      0x06

#define HOT_JUNC_TEMP     0x00      //
#define DELTA_JUNC_TEMP   0x01
#define COLD_JUNC_TEMP    0x02
#define RAW_ADC           0x03
#define SENSOR_STATUS     0x04      //
#define THERMO_SENSOR_CONFIG  0x05  //
#define DEVICE_CONFIG         0x06  //
#define DEVICE_ID             0x20  //

#define UPDATA_FLAG         0x40  //
#define DEV_RESOLUTION      0.0625//

#define _18BIT_             0b00
#define _16BIT_             0b01
#define _14BIT_             0b10    //
#define _12BIT_             0b11

#define _K_             0b000
#define _J_             0b001
#define _T_             0b010
#define _N_             0b011
#define _S_             0b100
#define _E_             0b101
#define _B_             0b110
#define _R_             0b111



class CFNET :private SoftwareI2C_3, private SoftwareI2C_2, private SoftwareI2C_1
{
private:
    SoftwareI2C_1 I2C_1;
    SoftwareI2C_2 I2C_2;
    SoftwareI2C_3 I2C_3;

    u8 cf_data[5]; u16 out_data[8];
public:
    CFNET() {
        init_CF_DO16N();
        dacInit();
    }
    
private:
    void init_delay(u16 delay);
    u8 init_CF_DO16N(void);
    void dacInit(void);

    u8 byte_write_3(u8* module_val, u8 addr, u8 len);
    u8 byte_write_2(u8* module_val, u8 addr, u8 len);
    u8 byte_write_1(u8* module_val, u8 addr, u8 len);
    
    u16 CFADC_A4L_Read(u8 address, u8 channel);
    u16 DI16B_Read(u8 address);
    
    static u8 bcd2bin(u8 value);
    static u8 decToBcd(u8 value);

    void Tstrech();

public:
    void digitalWrite(u8 address, u16 data);
    void digitalWrite(u8 address, u8 pin, u8 on_off);

    //u8 digitalRead(u8 address);
    u16  digitalRead(u8 address);
    bool digitalRead(u8 address, u16 pin);
    
    u16 analogRead(u8 address, u8 channel);
    void analogWrite(u8 address, u8 channel, u16 data);

    void rtcWrite(u8 dateValue, u8 data);
    u8 rtcRead(u8 dateValue);

    u8 tcSetInit(u8 addr, u8 sensor);
    float tcTempRead(u8 addr);
  
};

#endif



/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
