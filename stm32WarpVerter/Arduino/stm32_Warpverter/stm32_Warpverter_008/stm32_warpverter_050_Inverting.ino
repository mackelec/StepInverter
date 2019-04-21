

/*-----------------------------------------
 * 
 *    timer_Handler
 *    
 *    -  is called by the Timer Interrupt
 *    -  is called 1024 times per 50Hz cycle
 * 
 ------------------------------------------*/


void timer_Handler()
{
  static bool do_avgV=false;
  static bool do_avgI=false;
  
  static bool junk;
   //---   Toggle pin PC14, check timer on Scope
  gpio_write_bit(GPIOB, 3, junk);
  junk=!junk;

   //---   Additional Trap to see if Code is too slow
  static bool imReady=true;
  if (!imReady)
  {
    ;
  }
  imReady = false;

   //---   Write the DRIVE OUTPUT (8 bits)
   //      -   fyi, Port B is a 16 bit port
   //      -   Writing to upper Byte, PB8 to PB15

   //      -   global Var outputDrive is already set
   //          we just need to write it.
  uint16 mask=0;
  static int8 cnt=0;
  
  byte portBlow = GPIOB->regs->ODR & 0x00ff;

  if (!inverterRunning)
  {
    outputDrive = 0;
    cycleIndex = 0;
  }
  
  mask = outputDrive<<8 | portBlow;
  //--- write PORTB 
  GPIOB->regs->ODR = mask;
 
    //--- now work out the next outputDrive value
  
  if (inverterRunning) 
  {
    proc_nextOutputValue();
    if (cycleIndex == 0) return;
  }

    //---   Calculating expotential Moving Average values 
    //            for ADC values of voltage and current.
    //      -   Using 32bit int and multiplying the value by 100
    //            to increase the resolution of integer math.
    //      -   In an attempt to spread the processor work
    //            avgADC_V is calc'ed one cycle after reading ADC and
    //            avgADC_I is calc'ed one cycle after avgADC_V is calc'ed

  setPA15;
  if (do_avgI)
  {
    avgADC_I = avgADC_I + (adcValueI*100 - avgADC_I)/16;
    do_avgI = false;
  }
  if (do_avgV)
  {
    avgADC_V = avgADC_V + (adcValueV*100 - avgADC_V)/16;
    do_avgV = false;
    do_avgI = true;
  }

   //---  Read ADC values
   //     -   do not read faster than 6400Hz, 
   //         or no more than say one in three cycles.

   //     - A count of 64 cycles resolves to 16 reads per 50Hz cycle
  cnt++;
  if (cnt>63)
  {
    cnt=0;
    adcRead();
    readControl();
    do_avgV=true;
  }
  imReady = true;
  clrPA15;
}

void adcRead()
{
  static uint16 lastADC = 0;
  //-- read adc 32bit data register
  uint32 data = ADC1->regs->DR;
  adcValueV = data >> 16;
}

/*----------------------------
 * 
 *
 * 
 -----------------------------*/

void readControl()
{
  if (!inverterRunning)
  {
    if (!gpio_read_bit(GPIOC, 14) )
    {
      onPressed ++;
      if (onPressed > 512)
      {
        onPressed = 0;
        digitalWrite(LED,LOW);
        inverterRunning = true;
      }
    }
    else
    {
      if (onPressed > 0) onPressed --;
    }
    
  }
  if (!gpio_read_bit(GPIOC, 15))
  {
    inverterRunning = false;
    onPressed = 0;
    digitalWrite(LED,HIGH);
  }
}

/*------------------------------------------
 * 
 *    This is called by the timer_Handler after
 *      writing outputDrive to the output Port.
 *      
 *    -  Keeps track of posiion with the 50Hz cycle (cycleIndex)
 *    -  Calls the new cycle procedure.
 *    -  Gets the desired Voltage at current step (cycleIndex)
 *         from the Sinewave table (sineLookup) then finds the 
 *         appropriate Transformer output to match (outputVoltageLookup), 
 *         then outputDrive comes from corresponding table (outputMaskTable) 
 * 
 -------------------------------------------*/


void proc_nextOutputValue()
{
  
  //
  // mains sync here:
  // instead of incrementing cycleIndex by one, need to have a fractional math increment.
  // we might need to increment it by 0.995, say, for a slightly slower output frequency 
  // or by 1.002 for a little faster than default 50Hz
  // pcount_delta = 1024 when running at unaltered frequency.
  // It gets modified in ext. interrupt handler as needed to obtain and maintain phase lock
  //
  //cycleIndex ++;
  //
  
  pcount_acc += pcount_delta; 
  cycleIndex = (pcount_acc >> 10); 
  //pcount_acc = 0;
  
  if (cycleIndex >= STEPS) 
  {
    // mains sync
    if (int_f == 1)
    {
      // no ext int signal triggered handler
      // set freq to default
      pcount_delta = 1024;
      // and do stuff for fact there is no sync
      //
    }
    int_f = 1;            // permit an ext interrupt
    pcount_acc = 0;           // and the fractional math part of the counter
    //--------------
    set50Hz;
    setPB4;
    //avgADC_V = 368640;   // comment out when activating ADC
    cycleIndex = 0;
    proc_NewCycle(); 
    outputDrive = 0;
    clrPB4;
    clrPB5;
    
    return;
  }
  setPB5;
  int32 desiredVolts = SINE_LOOKUP_TABLE[cycleIndex];
  desiredVolts = (int32) (desiredVolts * inputFactor );
  desiredVolts = (int32) (desiredVolts /10000 );
  findMask(desiredVolts);
  clr50Hz;
  clrPB5;
  

  
}


/*--------------------
 * 
 *     Is called at the beginning of every 50Hz cycle
 *     -  Looks up the inputFactor.
 *          InputFactor is the ratio of average Battery Voltage 
 *          to Nominal Battery Voltage.
 *     -  InputFactor is applied to desiredVoltage to adjust 
 *          the output Voltage.
 *     
 *     This is also a conveniant place to Control based on:
 *         -  Thermisor 1 and 2, controls Fan 1 and 2.
 *         -  Over Temperature        - shut down
 *         -  Under Voltage (Battery) - shut down.
 *     
 * 
 ---------------------*/

void proc_NewCycle()
{
  int ptr = int(avgADC_V/100 + 50);
  inputFactor = flash_inputFactorTable[ptr];

  if (avgADC_V < LOWVOLT_CUTOFF_ADC)
  {
    inverterRunning = false;
    onPressed = 0;
    digitalWrite(LED,HIGH);
  }
    
  if (! FAN1_ADC_TURNON)
  {
    uint8 pwm = flash_pwmFan1Table[adcData[0]];
    analogWrite(pinFAN1,pwm);
  }
  else
  {
    if (fan1ON)
    {
      if (adcData[0] < FAN1_ADC_TURNOFF) fan1ON = false;
    }
    if (adcData[0] > FAN1_ADC_TURNON) fan1ON = true;
    if (fan1ON) {setFAN1;} else {clrFAN1;}
  }
  
  if (! FAN2_ADC_TURNON)
  {
    uint8 pwm = flash_pwmFan2Table[adcData[1]];
    analogWrite(pinFAN2,pwm);
  }
  else
  {
    if (fan2ON)
    {
      if (adcData[1] < FAN2_ADC_TURNOFF) fan2ON = false;
    }
    if (adcData[1] > FAN2_ADC_TURNON) fan2ON = true;
    if (fan2ON) {setFAN2;} else {clrFAN2;}
  }

  if (adcData[0] > TEMPERATURE_1_ADC_SHUTDOWN) 
  {
    inverterRunning = false;
    onPressed = 0;
    digitalWrite(LED,HIGH);
  }
  if (adcData[1] > TEMPERATURE_2_ADC_SHUTDOWN) 
  {
    inverterRunning = false;
    onPressed = 0;
    digitalWrite(LED,HIGH);
  }
}


/*------------------------------------------------------------
 * 
 *     findMask  :  sets the outputDrive var by 
 *                  looking up flash_voltMaskTable
 *                  
 *     flash_voltMaskTable : Positive Voltages 0      - 0x0fff
 *                           Negative Voltages 0x1000 - 0x1fff
 * 
 -------------------------------------------------------------*/

void findMask(int32 matchVolts)
{
  int ptr;
  if (matchVolts < 0)
  {
    ptr = abs(matchVolts) + 0x1000;
    if (ptr > 0x2000-1) ptr = 0x1fff;
    if (ptr < 0x1000) ptr = 0x1000;
    outputDrive = flash_voltMaskTable[ptr];
  }
  else
  {
    ptr = matchVolts;
    if (ptr > 0x1000-1) ptr = 0x0fff;
    if (ptr < 0) ptr = 0;
    outputDrive = flash_voltMaskTable[ptr];
  }
  
}

/*----------------------------------------------------------------
 * 
 *    mains_sync  -  Thanks Poida, www.thebackshed.com
 * 
 * 
 -----------------------------------------------------------------*/

void mains_sync()
  {
  int pcint, new_delta; 

  
  if  (int_f == 0) 
    {  
    return;  // only once per 50 Hz cycle. Noise! much noise...
    }
 
  int_f = 0;
  pcint = cycleIndex -  512;    // get position in 50Hz waveform, -512 to 512 counts
                                // pcint is position in output waveform when this interrupt occurs
  phase_error = (pcint - 4);    // subtract setpoint, 0 for my setup, in counts
                                // mains sync sample will have some low pass filter in effect, this gives a phase delay.
                                // setpoint = 0, will be different, effected by RC LP filter delay at least, other influences?              
  new_delta =   (phase_error/4 + (phase_error - old_phase_error)); // PID,
  //the divide by 4 is compiled as 2 x looped arithmetic shifts
   
  if (new_delta > 80) 
    new_delta = 80;       // Keeps things safe. When 80 or higher is used, my inverter spasms. Safe thanks to current limited power supply
  if (new_delta < -80) 
    new_delta = -80;     // even this small a range of changes still lets it sync to mains is less than 1/2 second.    

  pcount_delta = pcount_delta - new_delta;  // apply PID output to the process, i.e. modify AC output frequency 
  if(pcount_delta < 900) pcount_delta = 900;  // clamp change to output frequency to reasonable limits
  if(pcount_delta > 1100) pcount_delta = 1100;  // this range is approx 40 - 58 Hz, with 1024 = 50Hz mid point
  old_phase_error = phase_error;
  if (abs(phase_error) < 5 && abs(new_delta) < 5)
    {
      p_lock = 1;
      setRelay;              // tell world I think I'm in sync. drive relay, light LED, whatever
    }
  else
    {
      p_lock=0;
      clrRelay;
    }
  }

uint16 MAKE_WORD( const byte Byte_hi, const byte Byte_lo)
{
     return   (( Byte_hi << 8  ) | Byte_lo & 0x00FF );
}


void Bin2Char(char *buff, uint8 value)
{
    buff[8] = 0;
    int i=0;
    for (uint8 mask = 0x80; mask; mask >>= 1)
    {
        buff[i] = ((mask & value) ? '1' : '0');
        i++;
        buff[i] = 0;
    }
    
}

int32 smoothX(int32 avg, uint16 adc, int weight)
{
  int32 result = (int32) (avg + (adc * 100 - avg) / weight);
  return (result);
}
