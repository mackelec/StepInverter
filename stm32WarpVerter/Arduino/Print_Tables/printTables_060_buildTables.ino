
#define VOLTSTEPS   int(pow(3,TRANSFORMER_COUNT))
uint16 Vtransformer[TRANSFORMER_COUNT];   // in  "Volts * 10"  ie 2350 = 235.0 volts  (deci-Volts ??)

int16 sineLookup[STEPS];   
int16 outputVoltageLookup[VOLTSTEPS];   // in Volts * 10 ie 2350 = 235.0 volts   (deciVolts)
int8 transformerMask[VOLTSTEPS][TRANSFORMER_COUNT];
uint8 outputMaskTable[VOLTSTEPS];


void calc_Xfactor()
{
  int32 x = 40960000 / BATTERY_VOLTAGE_AT_MAX_ADC;
  int32 Xfactor = (int32) (NOMINAL_BATTERY_VOLTAGE * x);
  Serial << endl;
  Serial << "#define XFACTOR      " << Xfactor << endl << endl;
  Serial.flush();
   delay(500);
}

void calc_lowVoltCutoff()
{
   float x =   (BATTERY_LOW_CUTOFF  * 4096.0);
   x = x / BATTERY_VOLTAGE_AT_MAX_ADC;
   int adcCutoff = int(x);
   Serial << endl;
   Serial << "#define LOWVOLT_CUTOFF_ADC      " << adcCutoff << endl << endl;
   Serial.flush();
   delay(500);
}

void setTransformerVoltage()
{
  
  float Vpeak =  NOMINAL_VAC * sqrt(2) * 10;
  int B=0;

  if (TRANSFORMER_LARGE_SECVOLT != 0)
  {
    Vtransformer[0] = TRANSFORMER_TINY_SECVOLT;
    Vtransformer[1] = TRANSFORMER_SMALL_SECVOLT;
    Vtransformer[2] = TRANSFORMER_MEDIUM_SECVOLT;
    Vtransformer[3] = TRANSFORMER_LARGE_SECVOLT;
  }
  else
  {
    for (int i=0;i<TRANSFORMER_COUNT;i++)
    {
      B += int(pow(3,i));
    }
    for (int i=0;i<TRANSFORMER_COUNT;i++)
    {
      int x = TRANSFORMER_COUNT - i - 1;
      float F =  pow(3,x) / B ;
      Vtransformer[x] = (uint16) (Vpeak * F);
    }
  }
  
  
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
  for (int i=0;i<TRANSFORMER_COUNT;i++)
  {
    Serial << "Tx " << i << " : " << Vtransformer[i] << endl;
  }
  while (!Serial) 
   {
    ; // wait for serial port to connect. Needed for native USB port only
   }
}

/*--------------------------------------------------------------------------------
 * 
 *  Fills  array transformeMask with an incremented ternary (base 3) mask
 *  
 *  in relation to the transformer  0 = -1, 1 = 0, 2 = +1 
 * 
 --------------------------------------------------------------------------------*/

void fill_TransformerMask()
{
  for (int i=0;i<TRANSFORMER_COUNT;i++)
  {
    transformerMask[0][i] = 0;
  }
  for (int i=1;i<VOLTSTEPS;i++)
  {
    for (int j=0;j<TRANSFORMER_COUNT;j++)
    {
      if (j==0)
      {
        transformerMask[i][j] = 0;
        if (transformerMask[i-1][j] < 2)  transformerMask[i][j] = transformerMask[i-1][j] +1;
      }
      if (j>0) transformerMask[i][j] = transformerMask[i-1][j];
      if (j>0 && transformerMask[i-1][j-1] > transformerMask[i][j-1])
      {
        transformerMask[i][j] = 0;
        if (transformerMask[i-1][j] < 2) transformerMask[i][j] = transformerMask[i-1][j] +1;
      }
    }
  }
}

void fill_outputDriveTable()
{
  uint8 _mask[VOLTSTEPS];
  for (int i=0;i<VOLTSTEPS;i++)
  {
    _mask[i] = 0 ;
    //outputMaskTable[i] = 0 ;
    for (int j=0;j<TRANSFORMER_COUNT;j++)
    {
      int mult =int(pow(4,j));
      int state = (int) transformerMask[i][j]-1;
      if (state < 0 )state = 2;
      _mask[i] += state * mult;
    }
  }
  for (int i=0;i<VOLTSTEPS;i++)
  {
    uint8 mask = 0;
    if (_mask[i] & 0x01) mask += 2;
    if (_mask[i] & 0x02) mask += 1;
    if (_mask[i] & 0x04) mask += 4;
    if (_mask[i] & 0x08) mask += 8;
    
    if (_mask[i] & 0x10) mask += 128;
    if (_mask[i] & 0x20) mask += 64;
    if (_mask[i] & 0x40) mask += 32;
    if (_mask[i] & 0x80) mask += 16;
    outputMaskTable[i] =  mask;
  }
}

void fill_outputDriveTable_0416()
{
  for (int i=0;i<VOLTSTEPS;i++)
  {
    outputMaskTable[i] = 0 ;
    for (int j=0;j<TRANSFORMER_COUNT;j++)
    {
      int mult =int(pow(4,j));
      int state = (int) transformerMask[i][j]-1;
      if (state < 0 )state = 2;
      outputMaskTable[i] += state * mult;
    }
  }
}

void fill_outputVoltageTable()
{
  for (int i=0;i<VOLTSTEPS;i++)
  {
    int32 sumVolts = 0;
    for (int j=0;j<TRANSFORMER_COUNT;j++)
    {
      int8 p = (int8) transformerMask[i][j]-1;
      int32 v = (int32) Vtransformer[j];
      sumVolts += p * v;
    }
    outputVoltageLookup[i] = sumVolts;
  }
}

void fill_volt_maskTable()
{
  uint8 volt_maskTable[8192];
  for (int i=0;i<0x2000;i++)
  {
    int32  volt = i & 0xfff;
    uint8 mask = 0;
    if (i & 0x1000) volt=-volt;
    volt_maskTable[i]=0;
    for (int j=1;j<VOLTSTEPS;j++)
    {
      volt_maskTable[i] = outputMaskTable[j-1];
      if (volt < (outputVoltageLookup[j] + outputVoltageLookup[j-1])/2)
      {
        break;
      }
      volt_maskTable[i] = outputMaskTable[j];
    }
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "uint8 flash_voltMaskTable[]  __FLASH__ = { " << volt_maskTable[0];
    for (int i=1;i< 0x2000; i++)
    {
      j++;
      if (j>16)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << volt_maskTable[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
}

void fill_sineLookup()
{
  double Vpeak =  NOMINAL_VAC * sqrt(2) ;
  for (int i=0;i< STEPS; i++)
  {
    double rad = 2 * PI / STEPS * i;
    double volts = sin(rad) * Vpeak;
    sineLookup[i] = int(volts * 10 + 0.5);
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "int16 flash_sineLookup[]  __FLASH__ = { " << sineLookup[0];
    for (int i=1;i< STEPS; i++)
    {
      j++;
      if (j>9)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << sineLookup[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
}



double Thermistor(int RawADC, long *_resistance) {
 // Inputs ADC Value from Thermistor and outputs Temperature in Celsius
 //  requires: include <math.h>
 // Utilizes the Steinhart-Hart Thermistor Equation:
 //    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}
 //    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
   long Resistance;  
   double Temp;  // Dual-Purpose variable to save space.
   //Resistance=10000.0*((1024.0/RawADC) - 1);  // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (1024 /ADC -1) * BalanceResistor
   Resistance=6800.0*(6206.0/RawADC) - 6800.0 - 3300.0;
   * _resistance = Resistance;
   Temp = log(Resistance); // Saving the Log(resistance) so not to calculate it 4 times later. // "Temp" means "Temporary" on this line.
   Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));   // Now it means both "Temporary" and "Temperature"
   Temp = Temp - 273.15;  // Convert Kelvin to Celsius                                         // Now it only means "Temperature"

   // Uncomment this line for the function to return Fahrenheit instead.
   //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert to Fahrenheit
   return Temp;  // Return the Temperature
}

void fill_pwmFan1Table()
{
  uint8 pwmFan1Table[4096];
  long resistance;
  int16 turnon=0,turnoff=0;
  int16 adcTemp1Off=0,adcTemp2Off=0;
  
  int bandlow = FAN1_ON_TEMP_START*100;
  int bandhigh = FAN1_ON_TEMP_FULL*100;
  
  for (int i=0;i<4096;i++)
  {
    uint pwm =0;
    double t = Thermistor(i,&resistance);
    int intTemp = int(t*100.0);

    if (t < TEMPERATURE_1_CUTOFF) adcTemp1Off = i;
    if (t < TEMPERATURE_2_CUTOFF) adcTemp2Off = i;
    
    if (FAN1_ON_TEMP_FULL > FAN1_ON_TEMP_START)
    {
      if (t > FAN1_ON_TEMP_START && t < FAN1_ON_TEMP_FULL)
      {
        pwm = map(intTemp,bandlow,bandhigh,0,255);
      }
      if (t <= FAN1_ON_TEMP_START) pwm = 0;
      if (t >= FAN1_ON_TEMP_FULL) pwm = 255;
    }
    else
    {
      pwm = 255;
      if (t <= FAN1_ON_TEMP_START) pwm = 0;
      if (turnon == 0 && intTemp > bandlow) turnon = i;
      if (intTemp < bandhigh) turnoff = i;
    }
    pwmFan1Table[i] = pwm;
    
    //analogWrite(PA3,255);
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "#define TEMPERATURE_1_ADC_SHUTDOWN    " << adcTemp1Off << endl;
    Serial << "#define TEMPERATURE_2_ADC_SHUTDOWN    " << adcTemp2Off << endl;
    Serial << endl;
    Serial << "#define FAN1_ADC_TURNON    " << turnon << endl;
    Serial << "#define FAN1_ADC_TURNOFF   " << turnoff << endl;
    Serial << endl;
    Serial << "uint8 flash_pwmFan1Table[]  __FLASH__ = { " << pwmFan1Table[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>16)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << pwmFan1Table[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
}


void fill_pwmFan2Table()
{
  uint8 pwmFan2Table[4096];
  long resistance;
  int bandlow = FAN2_ON_TEMP_START*100;
  int bandhigh = FAN2_ON_TEMP_FULL*100;
  int16 turnon=0,turnoff=0;
  for (int i=0;i<4096;i++)
  {
    uint pwm =0;
    double t = Thermistor(i,&resistance);
    int intTemp = int(t*100.0);
    
    if (FAN2_ON_TEMP_FULL > FAN2_ON_TEMP_START)
    {
      if (t > FAN2_ON_TEMP_START && t < FAN2_ON_TEMP_FULL)
      {
        pwm = map(intTemp,bandlow,bandhigh,0,255);
      }
      if (t <= FAN2_ON_TEMP_START) pwm = 0;
      if (t >= FAN2_ON_TEMP_FULL) pwm = 255;
    }
    else
    {
      pwm = 255;
      if (t <= FAN2_ON_TEMP_START) pwm = 0;
      if (turnon == 0 && intTemp > bandlow) turnon = i;
      if (intTemp < bandhigh) turnoff = i;
    }
    pwmFan2Table[i] = pwm;
    //analogWrite(PA3,255);
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "#define FAN2_ADC_TURNON    " << turnon << endl;
    Serial << "#define FAN2_ADC_TURNOFF   " << turnoff << endl;
    Serial << endl;
    Serial << "uint8 flash_pwmFan2Table[]  __FLASH__ = { " << pwmFan2Table[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>16)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << pwmFan2Table[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
}


void fill_InputFactorTable()
{
  uint16 inputFactor[4096];
  inputFactor[0]=10000;
  for (int i=1;i<4096;i++)
  {
    //double x = (NOMINAL_BATTERY_VOLTAGE * 409600.0)/(i*BATTERY_VOLTAGE_AT_MAX_ADC
    double a = (double) (BATTERY_LOW_CUTOFF * 4096.0) /(double) BATTERY_VOLTAGE_AT_MAX_ADC ;
    int A = int (a);
    double x = (double) NOMINAL_BATTERY_VOLTAGE / BATTERY_VOLTAGE_AT_MAX_ADC;
    double y = 40960000.0 / i;
    double z = x * y + 0.5;
    inputFactor[i] = int(z);
    if (inputFactor[i] > 10000) inputFactor[i] = 10000;
    if (i < A) inputFactor[i] = 10000;
    
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "uint16 flash_inputFactorTable[]  __FLASH__ = { " << inputFactor[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>16)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << inputFactor[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
  
}
