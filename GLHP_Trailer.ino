#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>


/******************************************************************************
**                                                                           **
**      Definition                                                  **
**                                                                           **
******************************************************************************/

//Number of readings to take during analog read
#define numReading 5

/******************************************************************************
**                                                                           **
**      Port Definition                                              **
**                                                                           **
******************************************************************************/

//AD5420 LATCH signal
#define AD5420_LATCH 10

//MUX Pins A and B
#define Mux_A  9
#define Mux_B  8

//One Wire Bus port
#define ONE_WIRE_BUS 7

//Fixed heater solid state relay switch
#define SSR_SW 6

//Flow Meter
#define Flow_Meter    A0
#define max_flow_gpm  100

float Flow_Rate = 0;
/******************************************************************************
**                                                                           **
**      Definitions for DS                                             **
**                                                                           **
******************************************************************************/

// Change the precision to 12 bit
#define TEMPERATURE_PRECISION 12
//Upper temperature limit for calculation in degree C
#define DS_Upper_Temp_Limit  60
//Lower temperature limit for calculation in degree C
#define DS_Lower_Temp_Limit  10
//Define toggle pin for datalogger which temp reading status
#define Which_Temp 5

//Data to send to datalogger
bool Toggle = true;
//How long before changing temp status
#define DS_Interval 5000 
unsigned long PreviousMillis_DS = 0;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress
//Temp sensor that coming into the pump
IN_temp,
//Temp sensor that coming out the pump
OUT_temp;
//DeviceA = {0x28, 0x20, 0x62, 0x28, 0x07, 0x00, 0x00, 0x2C},
//DeviceB = {0x28, 0x19, 0x97, 0x26, 0x07, 0x00, 0x00, 0x36},
//DeviceC = {0x28, 0x29, 0xAF, 0x27, 0x07, 0x00, 0x00, 0xEE},
//DeviceD = {0x28, 0xA2, 0xAD, 0x28, 0x07, 0x00, 0x00, 0xE5};
  
//Global variable for temperature values
float IN_temp_value = 0, OUT_temp_value = 0;

/******************************************************************************
**                                                                           **
**      Definitions for AD5420                                           **
**                                                                           **
******************************************************************************/

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

/*! \brief Internal flag of using REXT as reference resistance. Please set this
bit before or during changing #AD5420_OUTEN bit.*/
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

/******************************************************************************
**                                                                           **
**      AD5420 Functions                                             **
**                                                                           **
******************************************************************************/
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
  switch(AD5420_0_10V_RANGE)
  {
    case AD5420_0_5V_RANGE: {
      //Vout=(V_RF_IN)*(Data/2^N)
      data = (uint16_t)(65536/5)*((*dac2_data-DS_Lower_Temp_Limit)/
      (DS_Upper_Temp_Limit - DS_Lower_Temp_Limit)*5);
      SPI.transfer(AD5420_DATA_REG_ADDR);
      SPI.transfer16(data);
      break;
      }
    case AD5420_0_10V_RANGE: {
      //Vout=(V_RF_IN)*(Data/2^N)
      data = (uint16_t)(65536/5)*((*dac2_data-DS_Lower_Temp_Limit)/
              (DS_Upper_Temp_Limit - DS_Lower_Temp_Limit)*10);
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
  switch(AD5420_4_20I_RANGE)
  {
    case AD5420_0_5V_RANGE: {
      //Vout=(V_RF_IN)*(Data/2^N)
      SPI.transfer(AD5420_DATA_REG_ADDR);
      SPI.transfer16(data);
      break;
    }
    case AD5420_0_10V_RANGE: {
      //Vout=(V_RF_IN)*(Data/2^N)
      data = (uint16_t)(65536/5)*((*dac1_data-DS_Lower_Temp_Limit)/
      (DS_Upper_Temp_Limit - DS_Lower_Temp_Limit)*10);
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

/******************************************************************************
**                                                                           **
**      MUX Functions                                              **
**                                                                           **
******************************************************************************/
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

/******************************************************************************
**                                                                           **
**      Fixed heater solid state relay switch functions                      **
**                                                                           **
******************************************************************************/
void SSR_Init() {
  pinMode(SSR_SW, OUTPUT);
  digitalWrite(SSR_SW, LOW);
}

void SSR_OFF() {
  digitalWrite(SSR_SW, LOW);
}

void SSR_ON() {
  digitalWrite(SSR_SW, HIGH);
}

//Read the flow meter value numReading times then out put avg
void Read_Flow_Meter() {
  int flow[numReading], added_total = 0, dc_value = 0;
  float flowrate = 0, avg_value = 0;
  
  for(int index = 0; index < numReading; index ++) {
    flow[index] = analogRead(Flow_Meter);
    
    //Add the readings to total
    added_total += flow[index];
  }
  
  //calculate average from the total
  avg_value = (float)(added_total / numReading);
    
  //Convert to flow rate and return flowrate 
  
  Flow_Rate = (map(avg_value, 0, 1023, 0, max_flow_gpm));
}

/******************************************************************************
**                                                                           **
**      Dallas Temperature Functions                               **
**                                                                           **
******************************************************************************/

//Initialize the temp sensor
void DS_Temp_Init() {
  //Setup pin for datalogger which temp
  //Low for in_temp, high for out_temp
  pinMode(Which_Temp, OUTPUT);
  digitalWrite(Which_Temp, LOW);
  
  //Start the temp sensor library
  temp_sensors.begin();
  
  //Check to see if they are found, if not, go to loop forever
  if (!temp_sensors.getAddress(IN_temp, 0))
  {
    //Display on screen that the sensor is not found
    //--display command here
    //Halt
    while(true){};
  }
  
  if (!temp_sensors.getAddress(OUT_temp, 1))
  {
    //Display on screen that the sensor is not found
    //--display command here
    //Halt
    while(true){};
  }
  
  //If the sensors were found then set the precision to 12bit
  temp_sensors.setResolution(IN_temp, TEMPERATURE_PRECISION);
  temp_sensors.setResolution(OUT_temp, TEMPERATURE_PRECISION);
}

//Read temperature from DS sensor returns IN_Temp, OUT_temp
void Read_Temperature() {
  float DevA[numReading], DevB[numReading],
      DevAT = 0, DevBT = 0, AvgA = 0, AvgB = 0;
  
  //Read the sensors
  for (int index = 0; index < numReading; index++)
  {
    temp_sensors.requestTemperatures();
    DevA [index] = temp_sensors.getTempC(IN_temp);
    DevB [index] = temp_sensors.getTempC(OUT_temp);

    DevAT += DevA[index];
    DevBT += DevB[index];
  }
  
  //Average the values together
  AvgA = (DevAT / numReading);
  AvgB = (DevBT / numReading);
  
  //Put the values to global variable
  IN_temp_value = AvgA;
  OUT_temp_value = AvgB;
}


/******************************************************************************
**                                                                           **
**      Main Code                                                **
**                                                                           **
******************************************************************************/

void setup() {
  //Setup Serial Port
  Serial.begin(9600);
  
  //Start SPI Module
  SPI.begin();
  
  //Initialize DAC and ports
  ad5420_init();
  mux_init();
  SSR_Init();
  DS_Temp_Init();
}
  

void loop() {
  //Get current millis() value
  unsigned long currentMillis = millis();
  
  //Every 5 seconds send DAC2 with temp data and toggle pin for datalogger
  if (currentMillis - PreviousMillis_DS >= DS_Interval)
  {
    PreviousMillis_DS = currentMillis;
    //If toggle is true then send dac2 IN_Temp else send Out_temp
    if (Toggle)
    {
      digitalWrite(Which_Temp, LOW);
      send_dac2_data(&IN_temp_value);
      Toggle = false;
    }
    else
    {
      digitalWrite(Which_Temp, HIGH);
      send_dac2_data(&OUT_temp_value);
      Toggle = true;
    }
  }
  
}
