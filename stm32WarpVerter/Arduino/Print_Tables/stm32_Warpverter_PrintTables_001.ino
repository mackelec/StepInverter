#include <Streaming.h>
#include <math.h>


#define STEPS 1024
#define PRINT_TABLES 1
//#define VOLT_MASK_TABLE flash_voltMaskTable
//#define SINE_LOOKUP_TABLE flash_sineLookup

void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
  Serial << "stm32_Warpverter_Print_Tables" << endl;
  Serial.flush();
  delay(3000);
  
  setTransformerVoltage();

  Serial << "-----------  Copy after This Line ----------" << endl << endl;
  calc_Xfactor();
  calc_lowVoltCutoff();
  fill_TransformerMask();
  fill_outputDriveTable();
  fill_outputVoltageTable();
  fill_sineLookup();
  fill_volt_maskTable();
  fill_InputFactorTable();
  fill_pwmFan1Table();
  fill_pwmFan2Table();
}

void loop() {
  // put your main code here, to run repeatedly:

}
