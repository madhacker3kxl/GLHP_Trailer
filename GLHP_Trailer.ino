const int    Base_Pin           = A0;   // DC voltage from Base heater
const int    SCR_Pin            = A1;   // DC voltage from SCR heater
const int    Pump_Volts_Pin     = A2;   // Pump power voltage
const int    Pump_Amps_Pin      = A3;   // Pump power amps
const int    SCR_Out_Pin        = 9;    // Output for SCR
const int    Q_Set              = 3810; // Q constant variable
const double R_Static           = 16.1; // Resistance of base heater
const double R_SCR              = 12.4; // Resistance of variable heater
const int numReadings           = 30;   //For avg values of the inputs
int start                       = 1;    // For initial value stuff
int SCR_Out_Set                 = 80;
double Base_Avg                 = 0;
double SCR_Avg                  = 0;
double Pump_Volts_Avg           = 0;
double Pump_Amps_Avg            = 0;
double Base_DC_Voltage, SCR_DC_Voltage, SCR_Out_Voltage;  // Raw to Voltage
double Pump_DC_Voltage, Pump_DC_Amps, Pump_Volts, Pump_Amps;
double Q_Base, Q_Pump, Q_SCR, Q_Total;

//******Initializes the communication and clears the LCD screen.
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  //Clear LCD Screen and back light on
  Serial.write(12);
  Serial.write(17);
}

//******Main Driver********************************************
void loop() {
  if (start == 1){
    //Set initial SCR Out Voltage to 0
    analogWrite(SCR_Out_Pin, SCR_Out_Set);
    start = 0;
  }
  Read_Input(1000);
  Convert_Data();
  Calculate_Data(); 
  Output_to_SCR();
  Print_Stuff();         
}

//******Reads the analog DC voltage*****************************
void Read_Input(int Read_Delay) {
  
  // Setup the variables
  int Base_Read[numReadings]; 
  int SCR_Read[numReadings];
  int Pump_Volts_Read[numReadings];
  int Pump_Amps_Read[numReadings];
  int Base_Total = 0;
  int SCR_Total = 0;
  int Pump_Volts_Total = 0;
  int Pump_Amps_Total = 0;

  // Clear all array contents to '0'
  for (int ClrArray = 0; ClrArray < numReadings; ClrArray++){
    Base_Read[ClrArray] = 0;
    SCR_Read[ClrArray] = 0;
    Pump_Volts_Read[ClrArray] = 0;
    Pump_Amps_Read[ClrArray]  = 0;
  }
  // Start reading
  for(int index = 0; index < numReadings; index++){

    // read from the sensor:  
    Base_Read[index] = analogRead(Base_Pin);
    delay(5);
    SCR_Read[index] = analogRead(SCR_Pin);
    delay(5);
    Pump_Volts_Read[index] = analogRead(Pump_Volts_Pin);
    delay(5);
    Pump_Amps_Read[index] = analogRead(Pump_Amps_Pin);

    // add the reading to the total:
    Base_Total += Base_Read[index];
    SCR_Total += SCR_Read[index];
    Pump_Volts_Total += Pump_Volts_Read[index];
    Pump_Amps_Total += Pump_Amps_Read[index];
    
    delay(Read_Delay);
  }
  // calculate the average:
  Base_Avg = Base_Total / numReadings; 
  SCR_Avg = SCR_Total / numReadings;
  Pump_Volts_Avg = Pump_Volts_Total / numReadings;
  Pump_Amps_Avg = Pump_Amps_Total / numReadings;
}

//******Converts bits to AC voltage************************
void Convert_Data(){

  //Convert raw average bit data to DC Voltage
  Base_DC_Voltage = Base_Avg * (5.0 / 1023);
  SCR_DC_Voltage = SCR_Avg * (5.0 /1023);
  Pump_DC_Voltage = Pump_Volts_Avg * (5.0 / 1023);
  Pump_DC_Amps = Pump_Amps_Avg * (5.0 / 1023);

  //Convert DC To AC Voltage
  Pump_Volts = (Pump_DC_Voltage / 5.0) * 210;
  Pump_Amps = Pump_DC_Amps;
}

void Calculate_Data(){
  //Reset zeros
  Q_Pump = 0;
  Q_Base = 0;
  Q_SCR  = 0;

  //Pump heat
  Q_Pump = 0.9158 * (Pump_Volts * Pump_Amps);
  
  // Base heater
  Q_Base = (Base_DC_Voltage / 10.0) * 5000; 
  
  //SCR heater
  Q_SCR = (SCR_DC_Voltage / 10.0) * (20000/5);
  
  //Total heat input
  Q_Total = Q_Pump + Q_Base + Q_SCR;
  
  //Feed back mechanism
  if (Q_Total < (Q_Set - 25) && Q_Total >= (Q_Set - 100)) {
    SCR_Out_Set += 1;
  } else if (Q_Total > (Q_Set + 25) && Q_Total <= (Q_Set + 100)) {
    SCR_Out_Set -= 1;
  } else if (Q_Total > (Q_Set + 100)) {
    SCR_Out_Set -= 2;
  } else if (Q_Total < (Q_Set - 100)) {
    SCR_Out_Set += 2;
  } 
}


//******Writes analog output D-A to SCR********
void Output_to_SCR(){
  analogWrite(SCR_Out_Pin, SCR_Out_Set);
}

//******Prints to the serial monitor***********
void Print_Stuff() {
  Serial.write(12);
  Serial.write(17);
  delay(5);
  Serial.print("Q.Pump: ");
  Serial.print(Q_Pump);
  Serial.write(13);
  Serial.print("Q.Base: ");
  Serial.print(Q_Base);
  Serial.write(13);
  Serial.print("Q.SCR:");
  Serial.print(Q_SCR);
  Serial.write(13);
  Serial.print("Q.Tot: ");
  Serial.print(Q_Total);
 
  //Correlated SCR Output voltage
  SCR_Out_Voltage = (1.78e-4 * pow(SCR_Out_Set,2)) - (6.92e-3 * SCR_Out_Set) + 3.62e-1;
  
  delay(5000);
  
  Serial.write(12);
  Serial.write(17);
  delay(5);
  Serial.print("SCR Out (V): ");
  Serial.print(SCR_Out_Voltage);
  Serial.write(13);
  Serial.print("SCR Out Set: ");
  Serial.print(SCR_Out_Set);
  Serial.write(13);
  Serial.print("Base VDC: ");
  Serial.print(Base_DC_Voltage);
  Serial.write(13);
  Serial.print("SCR VDC: ");
  Serial.print(SCR_DC_Voltage);

}
