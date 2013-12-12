/*
    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Deviation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "a7105.h"

#define CS_PIN 10

#define CS_HI() digitalWrite(CS_PIN, HIGH);
#define CS_LO() digitalWrite(CS_PIN, LOW);

void A7105_WriteReg(u8 address, u8 data)
{
    CS_LO();
    SPI.transfer(address); // spi_xfer(SPI2, address);
    SPI.transfer(data);    // spi_xfer(SPI2, data); 
    CS_HI();
}


void A7105_Setup() {
  pinMode(CS_PIN, OUTPUT);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
//  SPI.setClockDivider(10);
  SPI.setBitOrder(MSBFIRST);
  // set gpio1 to SDO (MISO) by writing to reg GIO1S
//  A7105_WriteReg(0x0b,0x06); // 0b0110
}

u8 A7105_ReadReg(u8 address)
{
    u8 data;
    int i;
    CS_LO();
    // Bits A7-A0 make up the first u8.
    // Bit A7 = 1 == Strobe.  A7 = 0 == access register.
    // Bit a6 = 1 == read.  a6 = 0 == write. 
    // bits 0-5 = address.  Assuming address < 64 the below says we want to read an address.
    SPI.transfer(0x40 | address); //spi_xfer(SPI2, 0x40 | address);
    data = SPI.transfer(0);
    CS_HI();
//    Serial.print(address); Serial.print(" "); Serial.println(data);
    return data;
// Not necessary because we expect SPI class to handle this?
//    /* Wait for tx completion before spi shutdown */
//    while(!(SPI_SR(SPI2) & SPI_SR_TXE))
//        ;
//    while((SPI_SR(SPI2) & SPI_SR_BSY))
//        ;

//    spi_disable(SPI2);
//    spi_set_bidirectional_receive_only_mode(SPI2);
//    */
//    /* Force read from SPI_DR to ensure RXNE is clear (probably not needed) */
//    volatile u8 x = SPI_DR(SPI2);
//    (void)x;
//    spi_enable(SPI2);  //This starts the data read
//    for(i = 0; i < 20; i++)  //Wait > 1 SPI clock (but less than 8).  clock is 4.5MHz
//        asm volatile ("nop");
//    ;
//    spi_disable(SPI2); //This ends the read window
//    data = spi_read(SPI2);
//    CS_HI();
//    spi_set_unidirectional_mode(SPI2);
//    spi_enable(SPI2);
 //   return data;
}
  
void A7105_WriteData(u8 *dpbuffer, u8 len, u8 channel)
{
    int i;
    CS_LO();
    SPI.transfer(A7105_RST_WRPTR);    //reset write FIFO PTR
    SPI.transfer(0x05); // FIFO DATA register - about to send data to put into FIFO
    for (i = 0; i < len; i++)
        SPI.transfer(dpbuffer[i]); // send some data
    CS_HI();

    // set the channel
    A7105_WriteReg(0x0F, channel);

    CS_LO();
    SPI.transfer(A7105_TX); // strobe command to actually transmit the daat
    CS_HI();
}

void A7105_ReadData(u8 *dpbuffer, u8 len)
{
    A7105_Strobe(A7105_RST_RDPTR); //A7105_RST_RDPTR
    for(int i = 0; i < len; i++) {
        dpbuffer[i] = A7105_ReadReg(0x05);
    }
    
    return;
}

void A7105_Reset()
{
    A7105_WriteReg(0x00, 0x00);
    delayMicroseconds(1000);
//    Serial.println(A7105_ReadReg(0x2));
}

void A7105_WriteID(unsigned long id)
{
    CS_LO();
    SPI.transfer(0x06);
    SPI.transfer((id >> 24) & 0xFF);
    SPI.transfer((id >> 16) & 0xFF);
    SPI.transfer((id >> 8) & 0xFF);
    SPI.transfer((id >> 0) & 0xFF);
    CS_HI();
}

void A7105_SetPower(int power)
{
    /*
    Power amp is ~+16dBm so:
    TXPOWER_100uW  = -23dBm == PAC=0 TBG=0
    TXPOWER_300uW  = -20dBm == PAC=0 TBG=1
    TXPOWER_1mW    = -16dBm == PAC=0 TBG=2
    TXPOWER_3mW    = -11dBm == PAC=0 TBG=4
    TXPOWER_10mW   = -6dBm  == PAC=1 TBG=5
    TXPOWER_30mW   = 0dBm   == PAC=2 TBG=7
    TXPOWER_100mW  = 1dBm   == PAC=3 TBG=7
    TXPOWER_150mW  = 1dBm   == PAC=3 TBG=7
    */
    u8 pac, tbg;
    switch(power) {
        case 0: pac = 0; tbg = 0; break;
        case 1: pac = 0; tbg = 1; break;
        case 2: pac = 0; tbg = 2; break;
        case 3: pac = 0; tbg = 4; break;
        case 4: pac = 1; tbg = 5; break;
        case 5: pac = 2; tbg = 7; break;
        case 6: pac = 3; tbg = 7; break;
        case 7: pac = 3; tbg = 7; break;
        default: pac = 0; tbg = 0; break;
    };
    A7105_WriteReg(0x28, (pac << 3) | tbg);
}

void A7105_Strobe(enum A7105_State state)
{
    CS_LO();
    SPI.transfer(state);
    CS_HI();
}

