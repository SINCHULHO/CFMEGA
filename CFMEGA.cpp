/*
    CFMEGA2.cpp
    Created on: 2024. 6. 07.
*/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "CFMEGA.h" 

// I2c Low Speed : 5 micro Second each
#define delayforI2C 15

/*********************************************************************************************************
    CLASS II2C 3
*********************************************************************************************************/

void SoftwareI2C_3::i2c_delay() {
//    u8 i;
//    for (i = 0; i < delayforI2C; i++) _WDR();
    _WDR();
}

void SoftwareI2C_3::i2c_delaylong() {
    u8 i;
    for (i = 0; i < 4; i++) _WDR();
}

void SoftwareI2C_3::i2c_start() {
    sda_high3;    
    sda_output3;
    scl_high3;    
    scl_output3;
    sda_low3;    
    i2c_delay();
    scl_low3;    
    i2c_delay();  // start
}

void SoftwareI2C_3::endTransmission() {
    sda_output3;
    scl_output3;
    scl_high3;    
    i2c_delay();
    sda_high3;    
    i2c_delay();
}

u8 SoftwareI2C_3::i2c_ack() {
    sda_input3;
    scl_high3;
    i2c_delaylong();
    if (sda_read3) {
        return 1; // no ack
    }
    sda_low3;      
    sda_output3;    
    scl_low3;
    i2c_delay();
    return 0; // ack
}

u8 SoftwareI2C_3::write(u8 dt) {
    u8 i;
    scl_output3;    
    for (i = 0; i < 8; i++) {
        if (dt & 0x80) { 
            sda_high3;
            sda_input3;
        }
        else {
            sda_low3;
            sda_output3;
        }
        scl_high3;    
        i2c_delay();
        /*
        if ((dt & 0x80) && (i==7)) {
            PORTB |= 64; // PB6 = PORT 12 - JUST CHECK TIMING  
            PORTB &= ~64;
        }
        */
        dt <<= 1;
        scl_low3;     
    }
    return i2c_ack();
}

u8 SoftwareI2C_3::beginTransmission(u8 addr) {
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    return write(dt);
}

u8 SoftwareI2C_3::requestFrom(u8 addr)
{
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    dt |= 0x01;
    return write(dt);
}

u8 SoftwareI2C_3::read_ack()
{
    u8 i, dt;
    sda_input3;
    i2c_delaylong();    
    for (i = 0; i < 8; i++) {
        scl_high3;
        //i2c_delay();    
        dt <<= 1;
        if (sda_read3) dt |= 1;
        else dt &= 0xfe;
        scl_low3;
        i2c_delaylong();
    }
    sda_low3;    // ack
    sda_output3;
    scl_high3;
    i2c_delay();
    scl_low3;
    i2c_delay();
    return dt;
}

u8 SoftwareI2C_3::read_na()
{
    u8 i, dt;
    sda_input3;    
    i2c_delaylong();    
    for (i = 0; i < 8; i++) {
        scl_high3;
        //i2c_delay();   
        dt <<= 1;
        if (sda_read3) dt |= 1;
        else dt &= 0xfe;
        scl_low3;
        i2c_delaylong();
    }
    sda_high3;   // noack
    sda_output3;
    scl_high3;
    i2c_delay();
    scl_low3;
    i2c_delay();
    sda_low3;
    i2c_delay();
    return dt;
}

/*********************************************************************************************************
    CLASS II2C 2
*********************************************************************************************************/

void SoftwareI2C_2::i2c_delay() {
    _WDR(); 
    
}

void SoftwareI2C_2::i2c_delaylong() {
    u8 i;
    for (i = 0; i < 4; i++) _WDR();
}

void SoftwareI2C_2::i2c_start() {
    sda_high2;      
    sda_output2;
    scl_high2;    
    scl_output2;
    sda_low2;    
    i2c_delay();
    scl_low2;    
    i2c_delay();  // start
}

void SoftwareI2C_2::endTransmission() {
    sda_output2;
    scl_output2;
    scl_high2;    
    i2c_delay();
    sda_high2;    
    i2c_delay();
}

u8 SoftwareI2C_2::i2c_ack() {
    sda_input2;
    scl_high2;
    i2c_delaylong();
    if (sda_read2) {
        return 1; // no ack
    }
    sda_low2;      
    sda_output2;    
    scl_low2;
    i2c_delay();
    return 0; // ack
}

u8 SoftwareI2C_2::write(u8 dt) {
    u8 i;
    scl_output2;    
    for (i = 0; i < 8; i++) {
        if (dt & 0x80) { 
            sda_high2;
            sda_input2;
        }
        else {
            sda_low2;
            sda_output2;
        }        
        scl_high2;        
        i2c_delay();
        dt <<= 1;
        scl_low2;   
        //i2c_delay();
    }
    return i2c_ack();
}

u8 SoftwareI2C_2::beginTransmission(u8 addr) {
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    write(dt);
}

u8 SoftwareI2C_2::requestFrom(u8 addr)
{
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    dt |= 0x01;
    return write(dt);
}

u8 SoftwareI2C_2::read_ack()
{
    u8 i, dt;
    sda_input2;
    i2c_delaylong();
    for (i = 0; i < 8; i++) {
        scl_high2;
        //i2c_delay();    //사장님 주석
        dt <<= 1;
        if (sda_read2) dt |= 1;
        else dt &= 0xfe;
        scl_low2;
        i2c_delaylong();
    }
    sda_low2;    // ack
    sda_output2;
    scl_high2;
    i2c_delay();
    scl_low2;
    i2c_delay();
    return dt;
}

u8 SoftwareI2C_2::read_na()
{
    u8 i, dt;
    sda_input2;
    i2c_delaylong();
    for (i = 0; i < 8; i++) {
        scl_high2;
        //i2c_delay();       
        dt <<= 1;
        if (sda_read2) dt |= 1;
        else dt &= 0xfe;
        scl_low2;
        i2c_delaylong();
    }
    sda_high2;   // noack
    sda_output2;
    scl_high2;
    i2c_delay();
    scl_low2;
    i2c_delay();
    sda_low2; 
    i2c_delay();
    return dt;
}

/*********************************************************************************************************
    CLASS II2C 1
*********************************************************************************************************/

void SoftwareI2C_1::i2c_delay() {
    u8 i;
    for (i = 0; i < delayforI2C; i++) _WDR();
    // mcp9600 delay
    if (delay_100kh) {
        for (u8 i = 0; i < 25; i++)_WDR();
    }
}

void SoftwareI2C_1::i2c_delaylong() {
    u8 i;
    for (i = 0; i < 4; i++) _WDR();
    // mcp9600 delay
    if (delay_100kh) {
        for (u8 i = 0; i < 25; i++)_WDR();
    }
}

void SoftwareI2C_1::i2c_start() {
    sda_high1;      
    sda_output1;
    scl_high1;
    scl_output1;
    sda_low1;    
    i2c_delay();
    scl_low1;    
    i2c_delay();  // start
}

void SoftwareI2C_1::endTransmission() {
    sda_output1;
    scl_output1;
    scl_high1;    
    i2c_delay();
    sda_high1;    
    i2c_delay();
}

u8 SoftwareI2C_1::i2c_ack() {
    sda_input1;
    scl_high1;
    i2c_delaylong();
    i2c_delay();
    if (sda_read1) {
        return 1; // no ack
    }
    sda_low1;      
    sda_output1;    
    scl_low1;
    i2c_delay();
    return 0; // ack
}

u8 SoftwareI2C_1::write(u8 dt) {
    u8 i;
    scl_output1;    
    for (i = 0; i < 8; i++) {
        if (dt & 0x80) { 
            sda_high1;
            sda_input1;
        }
        else {
            sda_low1;
            sda_output1;
        }
        scl_high1;        
        i2c_delay();
        dt <<= 1;
        scl_low1;   
    }
    return i2c_ack();
}

u8 SoftwareI2C_1::beginTransmission(u8 addr) {
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    write(dt);
}

u8 SoftwareI2C_1::requestFrom(u8 addr)
{
    u8 i, dt;
    i2c_start();
    dt = addr << 1;
    dt |= 0x01;
    return write(dt);
}

u8 SoftwareI2C_1::read_ack()
{
    u8 i, dt;
    sda_input1;
    i2c_delaylong();
    i2c_delay();
    for (i = 0; i < 8; i++) {
        scl_high1;
        i2c_delay();    
        dt <<= 1;
        if (sda_read1) dt |= 1;
        else dt &= 0xfe;
        scl_low1;
        i2c_delay();
        i2c_delaylong();
    }
    sda_low1;    // ack
    sda_output1;
    scl_high1;
    i2c_delay();
    scl_low1;
    i2c_delay();
    return dt;
}

u8 SoftwareI2C_1::read_na()
{
    u8 i, dt;
    sda_input1;
    i2c_delaylong();
    for (i = 0; i < 8; i++) {
        scl_high1;
        i2c_delay();  
        dt <<= 1;
        if (sda_read1) dt |= 1;
        else dt &= 0xfe;
        scl_low1;
        i2c_delay();
        i2c_delaylong();
    }
    sda_high1;   // noack
    sda_output1;
    scl_high1;
    i2c_delay();
    scl_low1;
    i2c_delay();
    sda_low1;
    i2c_delay();
    return dt;
}
/*********************************************************************************************************
    initialization : [_I2C_3 CFDO16N_], [_I2C_1 CFDAC_2V]
*********************************************************************************************************/
void CFNET::init_delay(u16 delay) {
    
    for (u16 k = 0; k < delay; k++) {
        for (u16 i = 0; i < 0xfff0; i++) _WDR();
    }
}

u8  CFNET::init_CF_DO16N(void)
{
    for (u8 i = 0; i < 8; i++) {
        if (I2C_3.beginTransmission(CFDO16N_ID + i));
        else {
            I2C_3.write(0x00); // IODIRA-0x00 ��������  IODIRB-0x01 ��������
            I2C_3.write(0x00); // 0x00 - out / 0x01 - input
            I2C_3.write(0x00); // auto increment address
            I2C_3.endTransmission();
            digitalWrite(i, 0x0000);//  INIT OUTPUT ALL LOW
        }
    }init_delay(35); // DELAY
}

void CFNET::dacInit(void)
{
    for (u8 i = 0; i < 8; i++) {
        if (I2C_1.beginTransmission(CFDAC2V_ID + i));  //0x58));
        else {
            I2C_1.write(0x01);                       // mode commend(voltage range)
            I2C_1.write(0x11);                       // 0x00 = 0-5V, 0x11 = 0-10V                        
            I2C_1.endTransmission();
            analogWrite( i, 0, 0);      // DOUT CH0 = 0v RESET;
            analogWrite( i, 1, 0);      // DOUT CH0 = 0v RESET;
        }
    }
}

/*********************************************************************************************************
    digitalWrite : [_I2C_3 CFDO16N_] 
*********************************************************************************************************/

u8 CFNET::byte_write_3(u8* module_val, u8 addr, u8 len) {

    if (I2C_3.beginTransmission(addr)) return 0;
    for (u8 i = 0; i < len; i++) { I2C_3.write(module_val[i]); }
    I2C_3.endTransmission();
}
//* WRITE PIN(1BIT) *//
void CFNET::digitalWrite(u8 address, u8 pin, u8 on_off){

        u8 ii2c_adrr = CFDO16N_ID + address;
        out_data[address] = (on_off == 0) ? out_data[address] &= ~(0x1 << pin) : out_data[address] |= (0x1 << pin);

        cf_data[0] = 0x12;                              //GPIOA_REGISTOR
        cf_data[1] = out_data[address];
        cf_data[2] = (u8)(out_data[address] >> 8);
        byte_write_3(cf_data, ii2c_adrr, 3);
}
//* WRITE PORT(1~2BYTE) *// 
void CFNET::digitalWrite(u8 address, u16 data){
        out_data[address] = data;
        
        u8 ii2c_adrr = CFDO16N_ID + address;
        cf_data[0] = 0x12;                                  //GPIOA_REGISTOR
        cf_data[1] = out_data[address];
        cf_data[2] = (u8)(out_data[address] >> 8);
        byte_write_3(cf_data, ii2c_adrr, 3);
}

/*********************************************************************************************************
    digitalRead  :  [_I2C_2 CFDI16B_]
*********************************************************************************************************/
u8 CFNET::byte_write_2(u8* module_val, u8 addr, u8 len) {

    if (I2C_2.beginTransmission(addr)) return 0;
    for (u8 i = 0; i < len; i++) { I2C_2.write(module_val[i]); }
    I2C_2.endTransmission();
}

u16 CFNET::DI16B_Read(u8 address) {

    if (I2C_2.requestFrom(address)) return ;
    u8 byte0 = I2C_2.read_ack();
    u8 byte1 = I2C_2.read_na();
    I2C_2.endTransmission();

    u16 InData = byte1 << 8;
        InData |= byte0;
   return InData;
}

//* READ PIN(1BIT) *//
bool CFNET::digitalRead(u8 address, u16 pin) {
    u16 data; u8 ii2c_adrr;

    ii2c_adrr = CFDI16B_ID + (address & 0x0f);
    data = DI16B_Read(ii2c_adrr);
    if (data & (0x0001 << pin)) { return false; }
    else return true;
}
//* READ PORT(1~2BYTE) *//
u16 CFNET::digitalRead(u8 address){
    u16 data; u8 ii2c_adrr;

    ii2c_adrr = CFDI16B_ID + (address & 0x0f);
    data = DI16B_Read(ii2c_adrr);
    return ~data;
}
/*
u8 CFNET::digitalRead(u8 address) {
    u16 data; u8 ii2c_adrr;

    ii2c_adrr = CFDI16B_ID + (address & 0x0f);
    data = DI16B_Read(ii2c_adrr);
    return ~data;
}
*/

/*********************************************************************************************************
    Thermocouple TEMP  :  [_I2C_1 CFTC-1L_]
*********************************************************************************************************/

void CFNET::Tstrech() {
    sda_output2;    I2C_1.i2c_delay();
    sda_high2;      I2C_1.i2c_delay(); I2C_1.i2c_delaylong(); I2C_1.i2c_delaylong();
}
u8 CFNET::tcSetInit(u8 addr, u8 sensorType)
{
    u8 readResolution, writeResolution;
    u8 readSensorType, writeSensorType;

    I2C_1.delay_100kh = true;
    do{
        //WRITE RESOLUTION _14BIT_
        cf_data[0] = DEVICE_CONFIG;
        writeResolution = 0x80 | (_14BIT_ << 5);
        cf_data[1] = writeResolution;
        byte_write_1(cf_data, CFTC_ID + addr, 2);

        //READ RESOLUTION    
        if (I2C_1.requestFrom(CFTC_ID + addr)) return 0;
        Tstrech();
        readResolution = I2C_1.read_na();
        I2C_1.endTransmission();

    } while (readResolution != writeResolution);
    
    do {
        //WRITE SENSORTYPE _DEFAULT K_TYPE_
        cf_data[0] = THERMO_SENSOR_CONFIG;
        writeSensorType = (sensorType << 4) | 0b100; //sensor type and filter
        cf_data[1] = writeSensorType;
        byte_write_1(cf_data, CFTC_ID + addr, 2);

        //READ SENSORTYPE    
        if (I2C_1.requestFrom(CFTC_ID + addr)) return 0;
        Tstrech();
        readSensorType = I2C_1.read_na();
        I2C_1.endTransmission();

    } while (readSensorType != writeSensorType);
    I2C_1.delay_100kh = false;
}

float CFNET::tcTempRead(u8 addr){
    u8 status;
    u16   errorCnt=0; 
    
    I2C_1.delay_100kh = true;
    do {
        //READ STATUS    check error
        cf_data[0] = SENSOR_STATUS;
        byte_write_1(cf_data, CFTC_ID + addr, 1);
        
        if (I2C_1.requestFrom(CFTC_ID + addr)) return 0;
        Tstrech();
        status = I2C_1.read_na();
        I2C_1.endTransmission();
        errorCnt++;
        if (errorCnt > 40) { errorCnt = 0; return -1111; }

    } while ( (status & UPDATA_FLAG) != UPDATA_FLAG );

    errorCnt = 0;

    //READ TC TEMP    
    cf_data[0] = HOT_JUNC_TEMP;
    byte_write_1(cf_data, CFTC_ID + addr, 1);

    if (I2C_1.requestFrom(CFTC_ID + addr)) return 0;
    Tstrech();
    int16_t tcDATA  = I2C_1.read_ack()<<8;
    Tstrech();
    tcDATA |= I2C_1.read_na();
    I2C_1.endTransmission();

    //RESET FLAG UPDATA
    //READ
    cf_data[0] = SENSOR_STATUS;
    byte_write_1(cf_data, CFTC_ID + addr, 1);

    if (I2C_1.requestFrom(CFTC_ID + addr)) return 0;
    Tstrech();
    status = I2C_1.read_na();
    I2C_1.endTransmission();
    
    //WRITE
    cf_data[0] = SENSOR_STATUS;
    cf_data[1] = 0X0F;
    byte_write_1(cf_data, CFTC_ID + addr, 2);

    float Temperature = ((float)tcDATA * DEV_RESOLUTION);
    //Temperature =  ( ( (float)Temperature * 1.8f) + 32) ;
    return Temperature;
    
}

/*********************************************************************************************************
   analogRead :[_I2C_1 CFADC-A4L_]
*********************************************************************************************************/
u8 CFNET::byte_write_1(u8* module_val, u8 addr, u8 len) {

    if (I2C_1.beginTransmission(addr)) return 0;
    for (u8 i = 0; i < len; i++) { I2C_1.write(module_val[i]);}
    I2C_1.endTransmission();
}

u16 CFNET::CFADC_A4L_Read(u8 address, u8 channel) {
    const u8 CONVERION = 0x00;
    const u8 CONFIG = 0x01;
    u8 busy;

    u16 config = (u16)
        (
            (1 << 15)                 // Start conversion  
            | (1 << 14)               // non-differential
            | (channel << 12)         // channel  
            | (0b000 << 9)            // 6.144V
            | (1 << 8)                // Single-shot  
            | (0b101 << 5)            // 128SPS   0b100
            | (0 << 4)
            | (0 << 3)
            | (0 << 2)
            | (0b11)                  // Disable comparator11
        );
    cf_data[0] = CONFIG;
    cf_data[1] = (u8)(config >> 8);
    cf_data[2] = (u8)config;
    byte_write_1(cf_data, address, 3);
    
    // Wait for conversion to complete
    do { 
    if (I2C_1.requestFrom(address))return 0;
    busy = I2C_1.read_ack();             
    I2C_1.endTransmission();
    busy = busy & 0b10000000;
    } while (busy == 0);
    
    cf_data[0] = CONVERION;
    byte_write_1(cf_data, address,1);

    if (I2C_1.requestFrom(address))return 0;
    u8 byte0 = I2C_1.read_ack();                  
    u8 byte1 = I2C_1.read_ack();                   
    I2C_1.endTransmission();
    
    u16 adData = (u16)byte0 << 8 | byte1;
    return adData;
}

u16 CFNET::analogRead(u8 address, u8 channel) {
    u8 ii2c_adrr;  u16 data;
    
    ii2c_adrr = CFADCA4L_ID + address;
    data = CFADC_A4L_Read(ii2c_adrr, channel);
    if(data & 0x8000)data = 0;             // -volte = 0
    else if (data > 26665)data = 26666;    // MAX 5V VALUE
    return data;
    
}

/*********************************************************************************************************
   analogWrite :[ _I2C_1 CFDAC-2V_ ]
*********************************************************************************************************/

void CFNET::analogWrite(u8 address, u8 channel, u16 data) {
    u8 ch = 0;  u8 ii2c_adrr;
    
        if (data > 4094) { data = 4095; }
        u16 vout_data0 = data << 4;
        u16 vout_data1 = data >> 4;
        if (channel == 0) { ch = 0x02; }                             //채널 0 = 0x02, 1 = 0x04
        else { ch = 0x04; }

        ii2c_adrr = CFDAC2V_ID + address ;
        cf_data[0] = ch;                                            //체널 0=0X02  1= 0X04
        cf_data[1] = vout_data0;
        cf_data[2] = vout_data1;
        byte_write_1(cf_data, ii2c_adrr, 3);
 }



/*********************************************************************************************************
    RTC FRTC [ _I2C_1 CFDAC-2V_ ]
*********************************************************************************************************/
static u8 CFNET::bcd2bin(u8 value) { return value - 6 * (value >> 4); }
static u8 CFNET::decToBcd(u8 value) { return ((value / 10 * 16) + (value % 10)); }

void CFNET::rtcWrite(u8 dateValue, u8 data) {

    cf_data[0] = dateValue;
    cf_data[1] = decToBcd(data);
    byte_write_1(cf_data, CFRTC_ID, 2);
}

u8 CFNET::rtcRead(u8 dateValue) {

    cf_data[0] = dateValue;
    byte_write_1(cf_data, CFRTC_ID, 1);

    if (I2C_1.requestFrom(CFRTC_ID))return 0;
    u16 byte0 = I2C_1.read_na();
    byte0 = bcd2bin(byte0);
    I2C_1.endTransmission();

    return byte0;
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/

int16_t ntcRead_a4(void)
{

    uint16_t reading = analogRead(A4);
    int16_t temp = 0;

    float sensorV = 5.0 - (reading * 5.0 / 1024);
    //float sensorV = reading*5.0/1024.0; 

    float sensorR = ((5.0 * NTC_RESISTANCE) / sensorV) - NTC_RESISTANCE;
    float kT = 1.0 / ((1.0 / (273.15 + 25.0)) + (1.0 / RESISTANCE_VALUE) * log(sensorR / NTC_RESISTANCE));
    float cT = kT - 273.15;
    cT = cT * 10;
    temp = (int16_t)cT;
    u16 value_temp = cT;

    return temp;

}
