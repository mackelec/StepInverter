

void setup() 
{
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);
  Serial.begin(9600);
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
  pinMode(pin50Hz,OUTPUT);
  pinMode(PB3,OUTPUT);      // test pin  : toggles on timer
  pinMode(PB5,OUTPUT);      // test pin
  pinMode(PB4,OUTPUT);      // test pin 
  pinMode(PA15,OUTPUT);      // test pin 

  pinMode(adcPinI,INPUT_ANALOG);
  pinMode(PIN_THERM1,INPUT_ANALOG);
  pinMode(PIN_THERM2,INPUT_ANALOG);
  pinMode(PIN_VAC,INPUT_ANALOG);
  pinMode(PIN_IAC,INPUT_ANALOG);
  
  
//  pinMode(PA4,OUTPUT);
//  pinMode(PC14,OUTPUT);
//  pinMode(PC15,OUTPUT);
//  pinMode(PB7,OUTPUT);
//  pinMode(PA15,OUTPUT);
  pinMode(LED,OUTPUT);
  //slow way
  pinMode(PB8,OUTPUT);
  pinMode(PB9,OUTPUT);
  pinMode(PB10,OUTPUT);
  pinMode(PB11,OUTPUT);
  pinMode(PB12,OUTPUT);
  pinMode(PB13,OUTPUT);
  pinMode(PB14,OUTPUT);
  pinMode(PB15,OUTPUT);

  pinMode(pinSwOff,INPUT_PULLUP);
  pinMode(pinSwON,INPUT_PULLUP);

  pinMode(pinFAN1,PWM);
  pinMode(pinFAN2,PWM);

  bool tablesCheck = checkTables();
  
  //---   BluePill onboard LED is turned by writing LOW 
  //        opposite of most arduino boards  
  digitalWrite(LED,LOW);
  delay(1000);
  Serial.flush();
  // quick way, the upper 8 bits all set to output, pushpull
  GPIOB->regs->CRH = 0x33333333;
  GPIOB->regs->ODR = 0;

  

/*-----------------------------------------------
 *  
 *      ADC setup
 *      
 *      -  STM32F103 has dual ADCs 
 *           ADC2 reads Battery Voltage
 *           ADC1 is used in scan mode, multiplexed, 
 *             reads Thermistor 1 & 2, Battery Current, AC voltage and Current 
 *      -  ADC 1 & 2 are set in continuous conversion modeso we don't waste time servicing it.
 *      -  ADC1 is using DMA into a 5 element buffer.
 *      -  Using the the same scan rate as in the powerScope so it is faster than 6400Hz
 *      
 *  
 ------------------------------------------------*/

  adcV.calibrate();
  adcV.setSampleRate(ADC_SMPR_239_5);
  adcV.setPins(&adcPinV, 1);
  adcV.setContinuous();

  adc_2.calibrate();
  adc_2.setSampleRate(ADC_SMPR_239_5);//set the Sample Rate
  adc_2.setScanMode();              
  adc_2.setPins(analogInputs, 5);   //set how many and which pins to convert.
  adc_2.setContinuous();            //set the ADC in continuous mode.

//set the DMA transfer for the ADC. 
//in this case we want to increment the memory side and run it in circular mode
//By doing this, we can read the last value sampled from the channels by reading the dataPoints array
  adc_2.setDMA(adcData, 5, (DMA_MINC_MODE | DMA_CIRC_MODE), NULL);

  adcV.startConversion();
  adc_2.startConversion();
  
  
  //*******  Timer setup  ( 19.52777777777777777778 microseconds, 51209.10384068278805121 Hz)
  //---  output Frquency = 50.00889046941678520626 Hz
  //---  inverterTimer frequency = 51200Hz (50Hz x 1024 steps)
  //---  inverterTimer period = 19.53125 uSec
  //---  overflow = 72MHz / 51200Hz = 1406.25
  
  inverterTimer.setMode(TIMER_CH4, TIMER_OUTPUT_COMPARE);
  inverterTimer.pause();
  inverterTimer.setPrescaleFactor(1);//1
  inverterTimer.setCount(0);
  inverterTimer.setOverflow(TIMER_OVERFLOW);//1406
  inverterTimer.setCompare(TIMER_CH4, 1);  
  inverterTimer.attachCompare4Interrupt(timer_Handler);
  inverterTimer.refresh();
  
  delay(1000);
  Serial << "stm32-StepInverter  Setup done" << endl;
  Serial.flush();

   //--- Starting with Inverter running for testing purposes.
   //---   I prefer Inverter NOT  running on start-up in real use.
  inverterRunning = true;    // 
  //digitalWrite(LED,HIGH);  // uncomment if startup with inverter off
  
  
  inverterTimer.resume();

}


/*------------------------------------------------------------
 * 
 *      CheckTables : Checks the size of the in - flash arrays
 *      
 *                    -  View the report on Serial Monitor
 *                       after pasting the array data.
 * 
 ------------------------------------------------------------*/

bool checkTables()
{
  int Vsize = sizeof(flash_voltMaskTable);
  int sineSize = sizeof(flash_sineLookup)/2;
  int sizeIF = sizeof(flash_inputFactorTable);
  bool result = true;
  Serial << endl;
  Serial.flush();
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
   delay(1000);
   Serial << "voltMaskTable size = " << Vsize;
   if (Vsize == 0x2000) {Serial << " -- Good " << endl; } else {Serial << "  -- Fail" << endl;result = false;}
   Serial.flush();
   Serial << "sineLookup size = " << sineSize;
   if (sineSize == STEPS) {Serial << " -- Good " << endl; } else {Serial << "  -- Fail" << endl;result = false;}
   Serial.flush();
   Serial << "inputFactorTable size = " << sizeIF;
   if (sizeIF == 0x2000) {Serial << " -- Good " << endl; } else {Serial << "  -- Fail" << endl;result = false;}
   Serial.flush();
   delay(1000);
   return (result);
   
}
