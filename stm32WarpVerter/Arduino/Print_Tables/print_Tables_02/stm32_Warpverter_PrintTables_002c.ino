#include <Streaming.h>
#include <math.h>


#define STEPS 1024
#define PRINT_TABLES 1


uint8 adc_temperatureTable[4096];
int temperature_adcTable[100];


void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
  Serial << "stm32_Warpverter_Print_Tables" << endl;
  Serial.flush();
  delay(3000);
  
  setTransformerVoltage();

  Serial << "//-----------  Copy after This Line ----------" << endl << endl;
  fill_adc_temperatureTable();
  fill_temperature_adcTable();
  //calc_Xfactor();
  doPrintOuts();
  

  
}

void loop() {
  // put your main code here, to run repeatedly:

}
