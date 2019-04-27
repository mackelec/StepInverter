
#define VOLTSTEPS   int(pow(3,TRANSFORMER_COUNT))
uint16 Vtransformer[TRANSFORMER_COUNT];   // in  "Volts * 10"  ie 2350 = 235.0 volts  (deci-Volts ??)

int16 sineLookup[STEPS];   
int16 outputVoltageLookup[VOLTSTEPS];   // in Volts * 10 ie 2350 = 235.0 volts   (deciVolts)
int8 transformerMask[VOLTSTEPS][TRANSFORMER_COUNT];
uint8 outputMaskTable[VOLTSTEPS];

void doPrintOuts()
{
  print_Header();
  //return;
  print_lowVoltCutoff();
  printCurrentStops();
  print_TemperatureDeclares();
  
  fill_TransformerMask();
  fill_outputDriveTable();
  fill_outputVoltageTable();
  
  print_sineLookup();
  print_volt_maskTable();
  print_pwmFan1Table();
  print_pwmFan2Table();
  #ifdef THEFACTORTABLE 
    print_TheFactor();
  #else
    print_InputFactorTable();
    print_currentFactor();
  #endif
  
}


void print_Header()
{
  Serial << "/*-----------------------------------------------------------" << endl;
  Serial << " *" << endl;
  Serial << " *            Title:     " << INVERTER_TITLE << endl;
  Serial << " *" << endl;
  Serial << " *        Generated:     " << F(__TIME__) << "    " << F(__DATE__) << endl;
  Serial << " *" << endl;
  Serial << " *               by:     "  << F(__FILE__)  << endl;
  Serial << " *" << endl;
  Serial << " *-----------------------------------------------------------*/" << endl;
  Serial <<  endl;
  Serial.flush();
  delay(500);
}

void print_lowVoltCutoff()
{
   float x =   (BATTERY_LOW_CUTOFF  * 4096.0);
   x = x / BATTERY_VOLTAGE_AT_MAX_ADC;
   uint32 adcCutoff = int(x)  ;
   uint32 adc = (uint32) adcValueAtmilliVolt(BATTERY_HIGH_CUTOFF)  ;
   Serial << endl;
   Serial << "#define BATTERY_VOLTAGE_AT_MAX_ADC      "  << BATTERY_VOLTAGE_AT_MAX_ADC << endl;
   Serial << endl;
   Serial << "#define LOW_DC_VOLT_CUTOFF_ADC      " << adcCutoff << endl ;
   Serial << "#define HIGH_DC_VOLT_CUTOFF_ADC     " << adc << endl << endl;
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

//void fill_outputDriveTable_0416()
//{
//  for (int i=0;i<VOLTSTEPS;i++)
//  {
//    outputMaskTable[i] = 0 ;
//    for (int j=0;j<TRANSFORMER_COUNT;j++)
//    {
//      int mult =int(pow(4,j));
//      int state = (int) transformerMask[i][j]-1;
//      if (state < 0 )state = 2;
//      outputMaskTable[i] += state * mult;
//    }
//  }
//}

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

void print_volt_maskTable()
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
      if (j>15)
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

void print_sineLookup()
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
      if (j>7)
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

void print_TemperatureDeclares()
{
  Serial << endl;
  uint32 adc,adc1=0,adc2=0;
  adc = (uint32) temperature_adcTable[TEMPERATURE_1_CUTOFF]  ;
  Serial << "#define TEMPERATURE_1_ADC_SHUTDOWN    " << adc << endl;
  adc = temperature_adcTable[TEMPERATURE_2_CUTOFF] ;
  Serial << "#define TEMPERATURE_2_ADC_SHUTDOWN    " << adc << endl;
  Serial << endl;
  if (FAN1_ON_TEMP_FULL < FAN1_ON_TEMP_START)
  {
    adc1 = temperature_adcTable[FAN1_ON_TEMP_START] ;
    adc2 = temperature_adcTable[FAN1_ON_TEMP_START-FAN1_ON_TEMP_FULL] ;
  }
  
  Serial << "#define FAN1_ADC_TURNON    " << adc1 << endl;
  Serial << "#define FAN1_ADC_TURNOFF   " << adc2 << endl;

  adc1=0;adc2=0;
  if (FAN2_ON_TEMP_FULL < FAN2_ON_TEMP_START)
  {
    adc1 = temperature_adcTable[FAN2_ON_TEMP_START] ;
    adc2 = temperature_adcTable[FAN2_ON_TEMP_START-FAN2_ON_TEMP_FULL] ;
  }
  Serial << "#define FAN2_ADC_TURNON    " << adc1 << endl;
  Serial << "#define FAN2_ADC_TURNOFF   " << adc2 << endl;
}

void print_pwmFan1Table()
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
//    Serial << "#define TEMPERATURE_1_ADC_SHUTDOWN    " << adcTemp1Off << endl;
//    Serial << "#define TEMPERATURE_2_ADC_SHUTDOWN    " << adcTemp2Off << endl;
//    Serial << endl;
//    Serial << "#define FAN1_ADC_TURNON    " << turnon << endl;
//    Serial << "#define FAN1_ADC_TURNOFF   " << turnoff << endl;
//    Serial << endl;
    Serial << "uint8 flash_pwmFan1Table[]  __FLASH__ = { " << pwmFan1Table[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>15)
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


void print_pwmFan2Table()
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
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    //Serial << "#define FAN2_ADC_TURNON    " << turnon << endl;
    //Serial << "#define FAN2_ADC_TURNOFF   " << turnoff << endl;
    //Serial << endl;
    Serial << "uint8 flash_pwmFan2Table[]  __FLASH__ = { " << pwmFan2Table[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>15)
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

void print_InputFactorTable()
{
  uint8 inputFactor[4096];
  inputFactor[0]=0;
  int16 adcNomVolt = adcValueAtmilliVolt(NOMINAL_BATTERY_VOLTAGE);
  for (int i=1;i<4096;i++)
  {
    
    if (i < adcNomVolt)
    {
      inputFactor[i] = 0;
    }
    else
    {
      double r = (double) adcNomVolt / (double) i;
      r = r * 256 + 0.5;
      int k = int (r);
//      int k = map(i, adcNomVolt,4095,0,255);
      if (k<0)k=0;
      if (k>255)k=255;
      inputFactor[i] = k;
    }
  }
  
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "uint8 flash_inputFactorTable[]  __FLASH__ = { " << inputFactor[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>15)
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

//void fill_InputFactorTable_0423()
//{
//  uint16 inputFactor[4096];
//  inputFactor[0]=10000;
//  for (int i=1;i<4096;i++)
//  {
//    //double x = (NOMINAL_BATTERY_VOLTAGE * 409600.0)/(i*BATTERY_VOLTAGE_AT_MAX_ADC
//    double a = (double) (BATTERY_LOW_CUTOFF * 4096.0) /(double) BATTERY_VOLTAGE_AT_MAX_ADC ;
//    int A = int (a);
//    double x = (double) NOMINAL_BATTERY_VOLTAGE / BATTERY_VOLTAGE_AT_MAX_ADC;
//    double y = 40960000.0 / i;
//    double z = x * y + 0.5;
//    inputFactor[i] = int(z);
//    if (inputFactor[i] > 10000) inputFactor[i] = 10000;
//    if (i < A) inputFactor[i] = 10000;
//    
//  }
//  if (PRINT_TABLES)
//  {
//    int j=0;
//    Serial << endl;
//    Serial << "uint16 flash_inputFactorTable[]  __FLASH__ = { " << inputFactor[0];
//    for (int i=1;i<4096;i++)
//    {
//      j++;
//      if (j>16)
//      {
//        Serial << endl;Serial << "                   ";
//        Serial.flush();
//        j=0;
//      }
//      Serial << "," << inputFactor[i] ;
//      Serial.flush();
//    }
//    Serial << "};" << endl << endl;
//  }
//  
//}

void printCurrentStops()
{
  Serial << endl;
  uint32 adc = adcValueAtmilliAmp(DC_CURRENT_CUTOFF) ;
  Serial << "#define DC_CURRENT_CUTOFF_ADC        " << adc << endl;
  adc = adcValueAtmilliAmp(FAN1_ON_DC_CURRENT) ;
  Serial << "#define FAN1_ON_DC_CURRENT_ADC       " << adc << endl;
  adc = adcValueAtmilliAmp(FAN1_OFF_DC_CURRENT) ;
  Serial << "#define FAN1_OFF_DC_CURRENT_ADC      " << adc << endl;
  adc = adcValueAtmilliAmp(FAN2_ON_DC_CURRENT) ;
  Serial << "#define FAN2_ON_DC_CURRENT_ADC       " << adc << endl;
  adc = adcValueAtmilliAmp(FAN2_OFF_DC_CURRENT) ;
  Serial << "#define FAN2_OFF_DC_CURRENT_ADC      " << adc << endl;

}

void print_currentFactor()
{
  uint8 cf[4096];

  for (int i=0;i<4096;i++)
  {
    if (i < ADC_VALUE_ZERO_DC_CURRENT) 
    {
      cf[i]=1;
    }
    else
    {
      int32 milliAmps = map(i,ADC_VALUE_ZERO_DC_CURRENT,4095,0,DC_CURRENT_AT_MAX_ADC);
      double VoltsDrop = (double) milliAmps / (double) DC_CURRENT_PER_AC_VOLTDROP; 
      double factor = (double) NOMINAL_VAC /  ((double) NOMINAL_VAC - VoltsDrop);
      factor = factor - 1.0; // all factors will be over 1 so no need to store it
      factor = factor * 256.0;
      int f =  int(factor);
      if (f < 0 ) f = 0;
      if (f> 255) f = 255;
      cf[i] = (uint8) f;
    }
    
  }
  if (PRINT_TABLES)
  {
    int j=0;
    Serial << endl;
    Serial << "uint8 flash_currentFactorTable[]  __FLASH__ = { " << cf[0];
    for (int i=1;i<4096;i++)
    {
      j++;
      if (j>15)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << "," << cf[i] ;
      Serial.flush();
    }
    Serial << "};" << endl << endl;
  }
}

void print_TheFactor()
{
  int j;
  char a=' ';
  int Vbits = 10,Ibits = 6;
  
  int Vsize = 1 << Vbits;
  int Isize = 1 << Ibits;
  int vX = 1 << (12-Vbits);
  int iX = 1 << (12-Ibits);
  int32 vf;

  int16 adcNomVolt = adcValueAtmilliVolt(NOMINAL_BATTERY_VOLTAGE);

  Serial << endl;
  Serial << "uint8 flash_TheFactorTable[]  __FLASH__ = { " ;

  //for (int i=0;i<600;i++)
  for (int i=0;i<Vsize;i++)
  {
    
    int32 V = map(i*vX,0,4095,0,BATTERY_VOLTAGE_AT_MAX_ADC);
    //if (V < NOMINAL_BATTERY_VOLTAGE) continue;
    for (int k=0;k<Isize;k++)
    {
      int I=0;
      if (k*iX>ADC_VALUE_ZERO_DC_CURRENT)
      {
        I = map(k*iX,ADC_VALUE_ZERO_DC_CURRENT,4095,0,DC_CURRENT_AT_MAX_ADC);
      }
     
     //---- calculate Power, and voltage drop
      float vd=0;
      if (I>0)
      {
        float P = ((float)V/1000.0)*((float)I/1000.0);
         vd= P/WATT_PER_AC_VOLTDROP;
      }

     //----------
      int f=0;
      if (V >= NOMINAL_BATTERY_VOLTAGE)
      {
        double v = (double) V /1000.0;
        double idc = (double) I / 1000.0;
  
        double z = ((double) NOMINAL_BATTERY_VOLTAGE/1000)/v;
        double y = vd / (double) NOMINAL_VAC;
        y = y*z;
        double x = y+z;
        double xx = x * 256 ;
        f = int(xx);
        if (f<0) f=1;
        if (f>255) f = 0;
      }
      j++;
      if (j>15)
      {
        Serial << endl;Serial << "                   ";
        Serial.flush();
        j=0;
      }
      Serial << a << f ;
      Serial.flush();  
      a = ',';

      //double R = (double) NOMINAL_VAC /((double) NOMINAL_BATTERY_VOLTAGE/1000);
      //double ans = R * v * x - vd ;
      
//      Serial << " i= " << i;
//      Serial << " k= " << k;
//      Serial << " V= " << V ;
//      Serial << " v =" << v ;
//      Serial << " I= " << I;
//      Serial << " idc = " << idc;
//      Serial << " P= " << P;
//      Serial << " R= " << R;
//      Serial << " VD= " << vd;
//      Serial << " z= " << z;
//      Serial << " y= " << y;
//      Serial << " x= " << x;
//      Serial << " ans=" <<  ans << endl; 
 
    }
  }
  Serial << "};" << endl << endl;
  delay(1000);
}

//void fill_powerFactor_0()
//{
//  char a=' ';
//  int j=0;
//  Serial << endl;
//  Serial << "uint8 flash_powerFactorTable[]  __FLASH__ = {" ;
//  for (int i=0;i<256;i++)
//  {
//    
//    int32 V = map(i*16,0,4095,0,BATTERY_VOLTAGE_AT_MAX_ADC);
//    for (int k=0;k<256;k++)
//    {
//      int I=0;
//      if (k*16>ADC_VALUE_ZERO_DC_CURRENT)
//      {
//        I = map(k*16,ADC_VALUE_ZERO_DC_CURRENT,4095,0,DC_CURRENT_AT_MAX_ADC);
//      }
//      j++;
//      if (j>16)
//      {
//        Serial << endl;Serial << "                   ";
//        Serial.flush();
//        j=0;
//      }
//      float P = ((float)V/1000.0)*((float)I/1000.0);
//      float vd= P/WATT_PER_AC_VOLTDROP;
//      double factor = (double) NOMINAL_VAC /  ((double) NOMINAL_VAC - vd);
//      factor = factor - 1.0; // all factors will be over 1 so no need to store it
//      factor = factor * 256.0;
//      int f = int(factor);
//      if (f < 0 ) f = 0;
//      if (f> 255) f = 255;
//      uint uf = (uint8) f;
//      Serial << a << uf ;
//      a=',';
//    }
//  }
//  Serial << "};" << endl << endl;
//  delay(1000);
//}


void calc_Xfactor()
{
  int32 x = 40960000 / BATTERY_VOLTAGE_AT_MAX_ADC;
  int32 Xfactor = (int32) (NOMINAL_BATTERY_VOLTAGE * x);
  Serial << endl;
  Serial << "#define XFACTOR      " << Xfactor << endl << endl;
  Serial.flush();
   delay(500);
}

int16 adcValueAtmilliAmp(int32 milliAmp)
{
  int32 x = map(milliAmp,0,DC_CURRENT_AT_MAX_ADC,ADC_VALUE_ZERO_DC_CURRENT,4095);
  return((int16) x);
}

int16 adcValueAtmilliVolt(int32 milliVolt)
{
  int32 x = map(milliVolt,0,BATTERY_VOLTAGE_AT_MAX_ADC,0,4095);
  return((int16) x);
}

int16 adcValueAtmilliAmp2(int32 milliAmp)
{
  int16 counts = 4096 - ADC_VALUE_ZERO_DC_CURRENT;
  double x = (double) milliAmp / (double) DC_CURRENT_AT_MAX_ADC   * (double) counts;
  int16 z = int(x) + ADC_VALUE_ZERO_DC_CURRENT;
  return (z);
}

void fill_adc_temperatureTable()
{
  for (int i=1;i<4096;i++)
  {
    long resistance;
    double t = Thermistor(i,&resistance);
    //Serial << "adc = " << i << "   T= " << t;
    if (t <0) t=0;
    adc_temperatureTable[i] = (uint8) int(t );
    //Serial << "  array= " << adc_temperatureTable[i] << endl;
  }
  adc_temperatureTable[0]=adc_temperatureTable[1];
}

void fill_temperature_adcTable()
{
  for (int i =0;i < 100;i++)
  {
    temperature_adcTable[i] = 0;
  }
  for (int i=1;i<4096;i++)
  {
    uint8 t = adc_temperatureTable[i];
    if (t>0 && t < 100)
    {
      if (temperature_adcTable[t] == 0) temperature_adcTable[t] = i;
    }
  }
//  for (int i =0;i < 100;i++)
//  {
//    Serial << "Temp = " << i << "    adc= " << temperature_adcTable[i] << endl;
//  }
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
