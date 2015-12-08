/*Test code for AD5420*/

#include <SPI.h>

/***********************************************************
**                                                        **
**      Port Definition                                   **
**                                                        **
***********************************************************/

//AD5420 LATCH signal
#define AD5420_LATCH  10

/***********************************************************
**                                                        **
**      Definitions for DS                                **
**                                                        **
***********************************************************/

//Upper temperature limit for calculation in degree C
#define DS_Upper_Temp_Limit  60
//Lower temperature limit for calculation in degree C
#define DS_Lower_Temp_Limit  10

/***********************************************************
**                                                        **
**      Definitions for AD5420                            **
**                                                        **
***********************************************************/

//Read registers
/*! \brief Internal address of DAC STATUS register.*/
#define AD5420_STATUS_REG        0x00
/*! \brief Internal address of DAC DATA register.*/
#define AD5420_DATA_REG          0x01
/*! \brief Internal address of DAC CONTROL register.*/
#define AD5420_CONTROL_REG       0x02

//Write Registers
/*! \brief Internal address of DAC CONTROL register.*/
#define AD5420_CONTROL_REG_ADDR  0x55
/*! \brief Internal address of DAC DATA register.*/
#define AD5420_DATA_REG_ADDR     0x01
/*! \brief Internal address of DAC RESET register.*/
#define AD5420_RESET_REG_ADDR    0x56
/*! \brief Internal address of NO Operation register.*/
#define AD5420_NOP           0x00

/*! \brief Internal flag of using REXT as reference resistance. 
 * Please set this bit before or during 
   changing #AD5420_OUTEN bit.*/
#define AD5420_REXT              0x2000
/*! \brief Internal flag of using RINT as reference resistance.*/
#define AD5420_RINT              0x0000
/*! \brief Internal flag of switching DAC on.*/
#define AD5420_OUTEN             0x1000
/*! \brief Internal flag of switching slew rate control on.*/
#define AD5420_SREN              0x0010
/*! \brief Internal flag of setting daisy-chain mode of SPI bus.*/
#define AD5420_DCEN              0x0008
/*! \brief Internal flag of setting 0-5 V output range.*/
#define AD5420_0_5V_RANGE        0x0000
/*! \brief Internal flag of setting 0-10 V output range.*/
#define AD5420_0_10V_RANGE       0x0001
/*! \brief Internal flag of setting 4-20 mA output range.*/
#define AD5420_4_20I_RANGE        0x0005
/*! \brief Internal flag of setting 0-20 mA output range.*/
#define AD5420_0_20I_RANGE        0x0006
/*! \brief Internal flag of setting 0-24 mA output range.*/
#define AD5420_0_24I_RANGE        0x0007

/*! \brief Internal flag of output current fault in status register.*/
#define AD5420_IOUT_FAULT        0x0004
/*! \brief Internal flag of slew rate being active in status register.*/
#define AD5420_SR_ACTIVE         0x0002
/*! \brief Internal flag of overheat fault of DAC in status register.*/
#define AD5420_OVRHEAT_FAULT     0x0001

/***********************************************************
**                                                        **
**      AD5420 Functions                                  **
**                                                        **
***********************************************************/
void ad5420_init() {
  //Set the Latch pin as output
  pinMode(AD5420_LATCH, OUTPUT);
  //Set Latch Pin as Low just in case
  digitalWrite(AD5420_LATCH, LOW);

  //Reset DAC1
  SPI.transfer(AD5420_RESET_REG_ADDR);
  SPI.transfer16(0x0001); //Write to reset Register
  ad5420_latch_trig();

  //Setup DAC1
  SPI.transfer(AD5420_CONTROL_REG_ADDR);
  SPI.transfer16(AD5420_DCEN); //Enable Daisy Chain
  ad5420_latch_trig();

  //Reset DAC2
  SPI.transfer(AD5420_RESET_REG_ADDR);
  SPI.transfer16(0x0001); //Write to reset Register
  SPI.transfer(AD5420_NOP); //No Operation for DAC1
  SPI.transfer16(0x0000);   //Send Blank
  ad5420_latch_trig();

  //Setup DAC1 and DAC2
  //DAC2 as V out
  SPI.transfer(AD5420_CONTROL_REG_ADDR);
  SPI.transfer16(AD5420_OUTEN | AD5420_RINT | AD5420_0_10V_RANGE);
  //DAC1 as 4 to 20mA
  SPI.transfer(AD5420_CONTROL_REG_ADDR);
  SPI.transfer16(AD5420_OUTEN | AD5420_RINT | AD5420_DCEN | AD5420_0_20I_RANGE);
  ad5420_latch_trig();
}

//Function to trigger the latch
void ad5420_latch_trig() {
  digitalWrite(AD5420_LATCH, HIGH);
  _delay_us(10);
  digitalWrite(AD5420_LATCH, LOW);
}

void send_dac2_data(float *dac2_data) {
  uint16_t data;
  //Send data to DAC2
  switch (AD5420_0_10V_RANGE)
  {
    case AD5420_0_5V_RANGE: {
        //Vout=(V_RF_IN)*(Data/2^N)
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_10V_RANGE: {
        //Vout=(V_RF_IN)*(Data/2^N)
        data = (uint16_t)(65536 / 5) * ((*dac2_data - DS_Lower_Temp_Limit) /
                         (DS_Upper_Temp_Limit - DS_Lower_Temp_Limit) * 10);
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_4_20I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_20I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_24I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
  } //End of switch


  //Send NO OP to DAC1
  SPI.transfer(AD5420_NOP);
  SPI.transfer16(0x0000); //Send blank
  ad5420_latch_trig();
}

void send_dac1_data(float *dac1_data) {
  uint16_t data;

  //Send NO OP to DAC2
  SPI.transfer(AD5420_NOP);
  SPI.transfer16(0x0000); //Send blank

  //Send Data to DAC1
  switch (AD5420_4_20I_RANGE)
  {
    case AD5420_0_5V_RANGE: {
        //Vout=(V_RF_IN)*(Data/2^N)
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_10V_RANGE: {
        //Vout=(V_RF_IN)*(Data/2^N)
        data = (uint16_t)(65536 / 5) * ((*dac2_data - DS_Lower_Temp_Limit) /
                         (DS_Upper_Temp_Limit - DS_Lower_Temp_Limit) * 10);
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_4_20I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_20I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
    case AD5420_0_24I_RANGE: {
        SPI.transfer(AD5420_DATA_REG_ADDR);
        SPI.transfer16(data);
        break;
      }
  } //End of switch

  ad5420_latch_trig(); //Trigger the latch
}

/***********************************************************
**                                                        **
**      Definitions for MUX                               **
**                                                        **
***********************************************************/

//MUX Pins A and B
#define Mux_A  9
#define Mux_B  8

/***********************************************************
**                                                        **
**      MUX Functions                                     **
**                                                        **
***********************************************************/
//Setup the Mux pins and set it as LOW
void mux_init() {
  pinMode(Mux_A, OUTPUT);
  pinMode(Mux_B, OUTPUT);
  digitalWrite(Mux_A, LOW);
  digitalWrite(Mux_B, LOW);
}

void mux_lcd() {
  digitalWrite(Mux_A, LOW);
  digitalWrite(Mux_B, LOW);
}

void mux_423() {
  digitalWrite(Mux_A, HIGH);
  digitalWrite(Mux_B, LOW);
}


/***********************************************************
**                                                        **
**      Main Code                                         **
**                                                        **
***********************************************************/

void setup() {
  //Setup Serial Port
  Serial.begin(9600);

  //Start SPI Module
  SPI.begin();

  //Initialize DAC
  ad5420_init();

  //  uint16_t data = 0xFFFF;
  //  send_dac_data(&data,&data);
}


void loop() {
  // put your main code here, to run repeatedly:

}
