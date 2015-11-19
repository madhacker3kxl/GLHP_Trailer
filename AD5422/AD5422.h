//This is AD5422.h is based on d-wsky's code
//https://github.com/d-wsky/DAC-AD5420

#ifndef _AD5422_H
#define _AD5422_H

/*ADDRESS BYTE FUNCTIONS */

#define AD_NOP 			0x00
//Internal address of DAC DATA register
#define DATA_REGISTER 	0x01
//Internal address of DAC READBACK register
#define READBACK		0x02
//Internal address of DAC CONTROL register
#define CNTRL_REG		0x55
//
#define RSTREG			0x56

/*  CONTROL REGISTER FUNCTIONS */

//Internal flag of using REXT as reference resistance
#define AD5422_REXT              0x2000
//Internal flag of switching DAC on
#define AD5422_OUT_EN             0x1000
//Internal flag of switching slew rate control on
#define AD5422_SlewRate_EN              0x0010
//brief Internal flag of setting daisy-chain mode of SPI bus
#define AD5422_DaisyChain_EN              0x0008

/*  READ ADDRESS DECODING*/

#define RSTATUSREG 		0x00
#define RDATAREG		0x01
#define RCNTRLREG		0x02

/*  OUTPUT RANGE OPTIONS */

#define _0_5V 			0x00
#define _0_10V			0x01
#define _4_20mA			0x05
#define _0_20mA			0x06
#define _0_24mA			0x07

/*  FLAGS  */

//Internal flag of output current fault in status register
#define IOUT_FAULT      0x0004
//Internal flag of slew rate being active in status register
#define SR_ACTIVE       0x0002
//Internal flag of overheat fault of DAC in status register
#define OVRHEAT_FAULT   0x0001

#endif /* AD5420_H_ */