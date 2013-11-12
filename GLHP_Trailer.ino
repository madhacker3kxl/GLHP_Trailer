const int    Voltage_Read = A0;   // DC voltage from Transformer
const int    SCR_Out      = 9;    // Output for SCR
const int    Q_Set        = 3810; // Q constant variable
const double R_Static     = 16.1; // Resistance of base heater
const double R_SCR        = 12.4; // Resistance of variable heater
const int numReadings     = 30;   //For avg values of the inputs
int start                 = 1;    // For initial value stuff
int SCR_Out_Set           = 0;
int DC_Avg                = 0;    //Average RAW Values
double DC_Voltage, AC_Voltage, SCR_Out_Voltage;  // Raw to Voltage

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
    analogWrite(SCR_Out, SCR_Out_Set);
    start = 0;
  }
  Read_Input(1000);
  Convert_Data();
  Calculate_Data();  
  Print_Stuff(); 
  Output_to_SCR();      
}

//******Reads the analog DC voltage*****************************
void Read_Input(int Read_Delay) {
  // read the analog in raw value and average them
  int DC_Voltage_Read[numReadings]; 
  int DC_Total = 0;

  // Clear all array contents to '0'
  for (int ClrArray = 0; ClrArray < numReadings; ClrArray++){
    DC_Voltage_Read[ClrArray] = 0;
  }
  // Start reading
  for(int index = 0; index < numReadings; index++){

    // read from the sensor:  
    DC_Voltage_Read[index] = analogRead(Voltage_Read);

    // add the reading to the total:
    DC_Total = DC_Total + DC_Voltage_Read[index];
    
    Serial.write(12);
    Serial.print("Reading......");
    Serial.write(13);
    Serial.print("Curr. Read: ");
    Serial.print(DC_Voltage_Read[index]);
    Serial.write(13);
    Serial.print("Tot. Read: ");
    Serial.print(DC_Total);
    Serial.write(13);
    delay(Read_Delay);
  }
  // calculate the average:
  DC_Avg = DC_Total / numReadings;  
}

//******Converts bits to AC voltage************************
void Convert_Data(){

  //Convert Raw data to Voltage
  DC_Voltage = DC_Avg * (5.0 / 1023);
  SCR_Out_Voltage = (SCR_Out_Set * (5.0 / 255)) - 0.6364;

  //Convert DC To AC Voltage
  AC_Voltage = (8.70e-3 * pow(DC_Voltage,2) + 4.538e1 *
                  DC_Voltage + 2.379e1);
}

void Calculate_Data(){
  
  // Base heater calculation
  double Q_Base = (pow(AC_Voltage,2)/R_Static); 
  //SCR calculations
  double Q_SCR = (Q_Set - Q_Base);
  
  SCR_Out_Set = int(-5.63e-6 * pow(Q_SCR,2) + 5.0e-2 *
                Q_SCR + 1.23e1);
}
//******Writes analog output D-A to SCR********
void Output_to_SCR(){
  analogWrite(SCR_Out, SCR_Out_Set);
}

//******Prints to the serial monitor***********
void Print_Stuff() {
  Serial.write(12);
  Serial.write(17);
  delay(5);
  Serial.print("DC voltage: ");
  Serial.print(DC_Voltage);
  Serial.write(13);
  Serial.print("AC Voltage: ");
  Serial.print(AC_Voltage);
  Serial.write(13);
  Serial.print("SCR Out (V): ");
  Serial.print(SCR_Out_Voltage);
  Serial.write(13);
  Serial.print("SCR Out Set: ");
  Serial.print(SCR_Out_Set);
  delay(3000);
}
