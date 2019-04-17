#include <Streaming.h>
#include <PString.h>
#include <DueTimer.h>

#define uint8 uint8_t
#define int8 int8_t
#define int16 int16_t
#define uint16 uint16_t
#define int32 int32_t


#define NOMINAL_VAC  240
#define TRANSFORMER_COUNT 4
#define VOLTSTEPS   int(pow(3,TRANSFORMER_COUNT))
#define NUM_SAMPLES 4000
#define PERIOD      10
uint16 Vtransformer[4] = {0, 0, 0, 0};
int8 Tpolarity[4];
int16 outputVoltageLookup[VOLTSTEPS];   // in Volts * 10 ie 2350 = 235.0 volts
int8 transformerMask[VOLTSTEPS][TRANSFORMER_COUNT];
uint8 outputMaskTable[VOLTSTEPS];
char reportTable[256][100];

int32 inputTable[256];
uint16 dacTable[256];


uint8 data[NUM_SAMPLES];
uint8 currentRead;
uint8 lastRead;
uint16 frameIndex = 0;

uint8 voltSteps;

bool readyForTrigger = true;

char message[100];
PString pMess(message, sizeof(message));

#define PORTC  PIOC->PIO_PDSR 
#define SET_PORTD  PIOD->PIO_SODR

#define D0 25
#define C1 33
#define C2 34
#define C3 35
#define C4 36
#define C5 37
#define C6 38
#define C7 39
#define C8 40

#define DAC 67

#define D0 25
#define D2 27
#define LED 13

#define pinSw1 51  //C12
#define pinSw2 49  //C14

bool  frameMode = true;
bool  snapShot = false;
bool  hold = false;
bool  ledState = false;

void setup() 
{
  SerialUSB.begin(2000000);
  while (!SerialUSB) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  analogWriteResolution(12);
  analogWrite(DAC0,0);
  
  pinMode(D0,INPUT);
  pinMode(C1,INPUT);
  pinMode(C2,INPUT);
  pinMode(C3,INPUT);
  pinMode(C4,INPUT);
  pinMode(C5,INPUT);
  pinMode(C6,INPUT);
  pinMode(C7,INPUT);
  pinMode(C8,INPUT);
  
  pinMode (pinSw1,INPUT_PULLUP);
  pinMode (pinSw2,INPUT_PULLUP);
  //digitalWrite(pinSw,HIGH);

  pinMode(LED,OUTPUT);
  pinMode(D0,OUTPUT);
  pinMode(D2,OUTPUT);
  digitalWrite(LED,HIGH);
  //SerialUSB.flush();

  voltSteps = int(pow(3, TRANSFORMER_COUNT)); //voltSteps;
  setTransformerVoltage();
  fill_TransformerMask();
  fill_outputDriveTable();
  fill_outputVoltageTable();
  fill_inputTable();
  fill_dacTable();
  fill_reportTable();
  
  delay(2000);
  SerialUSB << "DUE stepInveter Simulator 01" << endl;
  delay(500);
  digitalWrite(LED,LOW);
  currentRead = readInput();
}

void loop() {
  if (snapShot && frameIndex == NUM_SAMPLES) postProcess();
  if (snapShot) return;
  
  noInterrupts();
  digitalWrite(LED,LOW);
  while(1)
  {
    currentRead = readInput();
//    if (hold)
//    {
//      ledState = !ledState;
//      digitalWrite(LED,ledState);
//      delay(1000);   //  provides processor time for re-programming
//      break;
//    }
    if (snapShot) 
    {
      digitalWrite(LED,HIGH);
      timer_sync();
      return;
    }
    if (lastRead != currentRead)
    {
      dacc_write_conversion_data(DACC_INTERFACE, dacTable[currentRead]);
    }
  }
}



void timer_sync()
{
  bool isLow = false;
  Timer3.stop();
  for (int32 i = 320000; i > 0; i--)
  {
    uint8 p= readInput();
    uint8 b0 = p & 1;
    if (b0==0)
    {
      isLow = true;
      break;
    }
  }
  if (!isLow) return;
  for (int32 i = 320000; i > 0; i--)
  {
    uint8 p= readInput();
    uint8 b0 = p & 1;
    if (b0==1)
    {
      readyForTrigger=true;
      frameIndex=0;
      interrupts();
      Timer3.attachInterrupt(timer_Handler).setPeriod(PERIOD);
      Timer3.start();
      break;
    }
  }
}

void timer_Handler()
{
  static bool triggered = false;
  static bool primed = false;
  
  static uint16 dacValue = 0;
  static bool writeDAC = false;
  if (!frameMode && !snapShot) return;
  REG_PIOD_SODR |= 0x1 ;
  //gpio_write_bit(GPIOA, 4, HIGH);
  currentRead = readInput();
  
  if (currentRead != lastRead)
  {
    
    lastRead = currentRead;
    if (currentRead == 0)
    {
      REG_PIOD_SODR |= 0x4 ;
    }
    else
    {
      REG_PIOD_CODR |= 0x4 ;
    }
  }
  if (triggered)
  {
    //currentRead = readInput2();
    if (frameIndex > NUM_SAMPLES-1)
    {
      REG_PIOD_CODR |= 0x1 ;
      triggered=false;
      primed=false;
      if (snapShot)
      {
        //postProcess();
      }
      else
      {
        frameIndex=0;
      }
      //frameIndex=0;
      
    }
    else
    {
      data[frameIndex] = currentRead;
      frameIndex ++;
    }
  }
  else
  {
    if (readyForTrigger)
    {
      if (!primed)
      {
        if (currentRead & 0b10000000)
        {
          primed = true;
        }
      }
      else
      {
        if (currentRead == 0)
        {
          triggered = true;
          primed = false;
          readyForTrigger = false;
        }
      }
    }
  }
  REG_PIOD_CODR |= 0x1 ;
  //gpio_write_bit(GPIOA, 4, LOW);
}

uint8 readInput()
{
  int32 b32 = PORTC;
  int32 junk = b32;
  int32 b31 = b32 >> 1;
  uint B ;
  B = (uint8)(b31 & 0xff);
  hold = false;
  if (!(junk & 0x4000)) hold = true;    // C14
  if (!(junk & 0x1000)) snapShot = true;  // C12
  return (B);
}

void  postProcess()
{
  Timer3.stop();
  frameIndex = NUM_SAMPLES+1;
  for (int i = 0; i < NUM_SAMPLES; i++)
  {
    SerialUSB.flush();
    if (SerialUSB.availableForWrite())
    {
      char strIndex[5];
      sprintf(strIndex,"%04u,",i);
      char *b = &reportTable[data[i]][0];
      SerialUSB << strIndex <<  b  ;
    }
  }
  delay(3000);
  
  currentRead = readInput();
  snapShot = false;
  frameIndex=0;
  if (frameMode) timer_sync();
}




void fill_reportTable()
{
  char buff[10];
  for (int i=0;i<256;i++)
  {
    Bin2Char(buff,i);
    pTransformerPolarity(i);
    pMess.begin();
    pMess << buff;
    for (int j = TRANSFORMER_COUNT-1; j >=0; j--)
      {
        sprintf(buff,",%2i",Tpolarity[j]);
        pMess << buff ;
      }
    sprintf(buff,"%5i",inputTable[i]);
    pMess << "," << buff << endl;
    char *b = &reportTable[i][0];
    memcpy(b,message,strlen(message)+1);
  }
}

void fill_inputTable()
{
  for (int i=0;i<256;i++)
  {
    int32 v = Voltage(i);
    inputTable[i] = v;
  }
}

void fill_dacTable()
{
  for (int i=0;i<256;i++)
  {
    dacTable[i] = 2048 + (inputTable[i]/2); 
  }
}

int32 Voltage(uint8 reading)
{
  pTransformerPolarity(reading);
  int32 v = 0;
  for (int j = 0; j < TRANSFORMER_COUNT; j++)
  {
    v += Vtransformer[j] * Tpolarity[j];
  }
  return (v);
}

void pTransformerPolarity(uint8 txdata)
{
  int16 v;
  uint8 w;
  v = txdata & 0b00000011;
  Tpolarity[0] = v;
  if (v == 2) Tpolarity[0] = (int8) -1;
  if (v == 3) Tpolarity[0] = (int8) 0;

  v = txdata & 0b00001100;
  v = v >> 2;
  Tpolarity[1] = v;
  if (v == 2) Tpolarity[1] = (int8) -1;
  if (v == 3) Tpolarity[1] = (int8) 0;

  v = txdata & 0b00110000;
  v = v >> 4;
  Tpolarity[2] = v;
  if (v == 2) Tpolarity[2] = (int8) -1;
  if (v == 3) Tpolarity[2] = (int8) 0;

  v = txdata & 0b11000000;
  v = v >> 6;
  Tpolarity[3] = v;
  if (v == 2) Tpolarity[3] = (int8) -1;
  if (v == 3) Tpolarity[3] = (int8) 0;
}

void setTransformerVoltage()
{

  float Vpeak =  NOMINAL_VAC * sqrt(2) * 10;
  int B = 0;
  for (int i = 0; i < TRANSFORMER_COUNT; i++)
  {
    B += int(pow(3, i));
  }
  for (int i = 0; i < TRANSFORMER_COUNT; i++)
  {
    int x = TRANSFORMER_COUNT - i - 1;
    float F =  pow(3, x) / B ;
    Vtransformer[x] = (uint16) (Vpeak * F);
  }

  for (int i = 0; i < TRANSFORMER_COUNT; i++)
  {
    delay(100);
    SerialUSB << "Tx " << i << " : " << Vtransformer[i] << endl;
    SerialUSB.flush();
    delay(50);
  }
}

/*--------------------------------------------------------------------------------

    Fills  array transformeMask with an incremented ternary (base 3) mask

    in relation to the transformer  0 = -1, 1 = 0, 2 = +1

  --------------------------------------------------------------------------------*/

void fill_TransformerMask()
{
  for (int i = 0; i < TRANSFORMER_COUNT; i++)
  {
    transformerMask[0][i] = 0;
  }
  for (int i = 1; i < voltSteps; i++)
  {
    for (int j = 0; j < TRANSFORMER_COUNT; j++)
    {
      if (j == 0)
      {
        transformerMask[i][j] = 0;
        if (transformerMask[i - 1][j] < 2)  transformerMask[i][j] = transformerMask[i - 1][j] + 1;
      }
      if (j > 0) transformerMask[i][j] = transformerMask[i - 1][j];
      if (j > 0 && transformerMask[i - 1][j - 1] > transformerMask[i][j - 1])
      {
        transformerMask[i][j] = 0;
        if (transformerMask[i - 1][j] < 2) transformerMask[i][j] = transformerMask[i - 1][j] + 1;
      }
    }
  }
}

void fill_outputDriveTable()
{
  for (int i = 0; i < voltSteps; i++)
  {
    outputMaskTable[i] = 0 ;
    for (int j = 0; j < TRANSFORMER_COUNT; j++)
    {
      int mult = int(pow(4, j));
      int state = (int) transformerMask[i][j] - 1;
      if (state < 0 )state = 2;
      outputMaskTable[i] += state * mult;
    }
  }


}

void fill_outputVoltageTable()
{
  for (int i = 0; i < voltSteps; i++)
  {
    int32 sumVolts = 0;
    for (int j = 0; j < TRANSFORMER_COUNT; j++)
    {
      int8 p = (int8) transformerMask[i][j] - 1;
      int32 v = (int32) Vtransformer[j];
      sumVolts += p * v;
    }
    outputVoltageLookup[i] = sumVolts;
  }
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


