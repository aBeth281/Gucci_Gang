#include <SoftwareSerial.h>                    // use the software serial library
   SoftwareSerial mySerial(2,5);               // receive data at pin 2; transmit data at pin 5
   
// Section for Tempurature Values setup
int valT;
int deviationT = 26;
float setTemp = 25;
int setAnalT = (setTemp + 42.105)/.1332;
int LCLT = setAnalT - deviationT; 
int UCLT = setAnalT + deviationT;
float LCLTemp = (0.1332*LCLT) - 42.105; //Equation for finding the Lower Control Limit
float UCLTemp = (0.1332*UCLT) - 42.105; // Equation for finding the Upper Control Limit
//--------------------------------------------------------------------------------------

//Section for Salinity Values setup
int valS;
int deviationS = 8;
float setSalin = .100;
int setAnalS = (859.81*pow(setSalin, 0.1468));
int LCLS = setAnalS - deviationS;
int UCLS = setAnalS + deviationS;
float LCLSalin = (1.0254*pow(10, -20)*pow(LCLS, 6.8111));
float UCLSalin = (1.0254*pow(10, -20)*pow(UCLS, 6.8111));
//--------------------------------------------------------------------------------------

long Time_elapsed;

void setup() {
  pinMode(3, OUTPUT); // Pin 3 is used for controlling Salinity
  pinMode(4, OUTPUT); // Pin 4 is used for turning on the 20 ohm resistance heating element
  pinMode(6, OUTPUT); // Pin 6 is used for turning on the saltyBoi solenoid valve
  pinMode(7, OUTPUT); // Pin 7 is used for turning on the waterBoi solenoid valve
  
  mySerial.begin(9600); 
  Serial.begin(9600);
  delay(1000);           // set data rate to 9600 baud; wait for bootup
  Serial.println("Hello World!");
  Serial.println("----------------------------------------------------------------------");

  Serial.print("Analog Temp LCL: "); Serial.print(LCLT); 
  Serial.print("  Analog Temp Set Point: "); Serial.print(setAnalT);
  Serial.print("  Analog Temp UCL: "); Serial.println(UCLT);
  Serial.print("Temp LCL: "); Serial.print(LCLTemp);
  Serial.print("       Temp Set Point: "); Serial.print(setTemp);
  Serial.print("       Temp UCL: "); Serial.println(UCLTemp);
  
  Serial.print("Analog Salinity LCL: "); Serial.print(LCLS);
  Serial.print("  Analog Salinity Set Point: "); Serial.print(setAnalS);
  Serial.print("  Analong Salinnity UCL: "); Serial.println(UCLS);
  Serial.print("Salinity LCL: "); Serial.print(LCLSalin);
  Serial.print("        Salinity Set Point: ");Serial.print(setSalin);
  Serial.print("        Salinity UCL: "); Serial.println(UCLSalin);
  Serial.println("----------------------------------------------------------------------");
  
/** This section of code creates the static display on the LCD screen **/
   mySerial.write(254);  mySerial.write(1);    
   mySerial.write(254);  mySerial.write(132);  
   mySerial.write("LCL    SP   UCL");           
   mySerial.write(254);  mySerial.write(192);   
   mySerial.write("S:"); 
   mySerial.write(254); mySerial.write(195);
   mySerial.print(LCLSalin, 3);  
   mySerial.write(254); mySerial.write(201);
   mySerial.print(setSalin, 3);     
   mySerial.write(254); mySerial.write(207);
   mySerial.print(UCLSalin, 3);                
   mySerial.write(254);  mySerial.write(148);
   mySerial.write("T:"); 
   mySerial.write(254); mySerial.write(152);
   mySerial.print(LCLTemp, 1);  
   mySerial.write(254); mySerial.write(158);
   mySerial.print(setTemp, 1);     
   mySerial.write(254); mySerial.write(164);
   mySerial.print(UCLTemp, 1);
   mySerial.write(254);  mySerial.write(212);
   mySerial.write("S=");
   mySerial.write(254);  mySerial.write(220);
   mySerial.write("T=");
   mySerial.write(254);  mySerial.write(227);    
   mySerial.write("H=");  
} 
void loop() {
  
  Time_elapsed = millis();
  Serial.print("Time: ");
  Serial.print(Time_elapsed/1000);
  Serial.println(" sec");
  
  /** ---------------------------------------------------------------------------------------------------------
  / This section of code reads the temperature with a thermistor and changes it with a 20 ohm resistance heater
  --------------------------------------------------------------------------------------------------------- **/
  valT = analogRead(0); //current value reading
  float temp = (0.1332*valT) - 42.105; //The equation for determining the temperature with respect to analog value
  mySerial.write(254);  mySerial.write(222);
  mySerial.print(temp, 1);  
  Serial.print("  Current Temperature: ");
  Serial.print(temp);
  if ( valT < LCLT){
    warmup();
  }
  else if (valT > LCLT){
    cooldown();
  }
  /** -------------------------------------------------------------------------------------------------------**/
  
  /** ---------------------------------------------------------------------------------------------------------
  / This section of code reads the temperature with a conductivity sensor and changes via solenoid valves
  --------------------------------------------------------------------------------------------------------- **/
  digitalWrite(3,HIGH);          // apply 5V to the conductivity sensor
  delay(100);                    // hold the voltage at pin 8 for 0.1s
  int valS=analogRead(1);        // read voltage on + side of 10kohm resistor
  float salin = (2*pow(10, -20)*pow(valS, 6.7449));
  mySerial.write(254); mySerial.write(214);
  mySerial.print(salin, 3);
  Serial.print("  Current Salinity: ");
  Serial.println(salin, 3);
  digitalWrite(3,LOW);           // turn off power to the conductivity sensor
  if (valS < LCLS && Time_elapsed > 10000) {
    saltyBoi();
  }
  else if (valS > UCLS && Time_elapsed > 10000) {
    waterBoi();
  }                  
  /** -------------------------------------------------------------------------------------------------------**/
  
  delay(1000);    // delay 1 second between measurements
} 
void warmup() { //Turns on the heater
    digitalWrite(4, HIGH);
    Serial.println(" -- WARMING UP!");
    mySerial.write(254); mySerial.write(229);
    mySerial.write("ON "); //Prints out on the LCD that the Heater is on 
}

void cooldown() { // Turns off the Heater
    digitalWrite(4, LOW);
    Serial.println(" -- COOLING DOWN!");
    mySerial.write(254); mySerial.write(229);
    mySerial.write("OFF"); //Prints out on the LCD that the Heater is off
}

void saltyBoi() { //Turns on the Saline Solution solenoid valve
    Serial.println(" -- Turing on the Salty Boi -- ");
    digitalWrite(6, HIGH);
    delay(500);
    digitalWrite(6, LOW);
    Time_elapsed = 0;
}

void waterBoi() { //Turns on the DeIonized water solenoid valve
    Serial.println(" -- Turing on the Water Boi -- ");
    digitalWrite(7, HIGH);
    delay(500);
    digitalWrite(7, LOW);
    Time_elapsed = 0;
}
