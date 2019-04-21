
#include <Streaming.h>
#include <PString.h>
#include <STM32ADC.h>

STM32ADC adcV(ADC2);
STM32ADC adc_2(ADC1);

uint8 adcPinV = PB0;
uint8 adcPinI = PB1;

#define PIN_SYNCINP PA1
#define pinSwOff    PC15
#define pinSwON     PC14
#define pinFAN1     PA3
#define pinFAN2     PA2
#define pinRelay    PA8

#define setRelay gpio_write_bit(GPIOA, 8, HIGH)
#define clrRelay gpio_write_bit(GPIOA, 8, LOW)

#define setFAN1 gpio_write_bit(GPIOA, 3, HIGH)
#define clrFAN1 gpio_write_bit(GPIOA, 3, LOW)

#define setFAN2 gpio_write_bit(GPIOA, 2, HIGH)
#define clrFAN2 gpio_write_bit(GPIOA, 2, LOW)

#define pin50Hz PA0
#define set50Hz gpio_write_bit(GPIOA, 0, LOW)
#define clr50Hz gpio_write_bit(GPIOA, 0, HIGH)

#define clrPB5 gpio_write_bit(GPIOB, 5, LOW)
#define setPB5 gpio_write_bit(GPIOB, 5, HIGH)

#define clrPB4 gpio_write_bit(GPIOB, 4, LOW)
#define setPB4 gpio_write_bit(GPIOB, 4, HIGH)

#define clrPA15 gpio_write_bit(GPIOA, 15, LOW)
#define setPA15 gpio_write_bit(GPIOA, 15, HIGH)

volatile uint16 adcValueV;
volatile uint16 adcValueI;

volatile int32 avgADC_V;
//volatile int32 avgADC_I;

byte  outputDrive = 0;    // Output Driver : High Byte of PORT B 
uint16 cycleIndex;           // position within the cycle : between 0 to 1023 , 
int32 inputFactor = 10000;

bool inverterRunning = false;
int16 onPressed;
bool fan1ON = false;
bool fan2ON = false;

HardwareTimer inverterTimer(3);//4

#define PIN_THERM1 PA5
#define PIN_THERM2 PA4
#define PIN_VAC    PA7
#define PIN_IAC    PA6



uint8 analogInputs[] = {PIN_THERM1,PIN_THERM2,adcPinI,PIN_VAC,PIN_IAC};
uint16_t adcData[5];

#define STEPS 1024
#define VOLTSTEPS   int(pow(3,TRANSFORMER_COUNT))
#define VOLT_MASK_TABLE flash_voltMaskTable
#define SINE_LOOKUP_TABLE flash_sineLookup

#define adcTherm1  adcData[0]
#define adcTherm2  adcData[1]
#define avgADC_I   adcData[2]

char message[256];
PString pMessage(message,sizeof(message));

#define LED      PC13


//
// mains sync
//
long pcount_acc;
int pcount_delta, phase_error, old_phase_error;
int int_f,p_lock;






void loop() {
  // put your main code here, to run repeatedly:

}
