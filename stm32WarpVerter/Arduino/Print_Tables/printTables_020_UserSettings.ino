


/*-------------------------------------------------------------------------------
 * 
 *          USER DEFINED VALUES
 *          
 *          TIMER_OVERFLOW : Should either be 
 *                           OVERFLOW_50HZ or OVERFLOW_60Hz
 *                              
 *          NOMINAL_VAC : Whole integer value
 *                        -  Is the output RMS AC value you expect AT the     
 *                          nominal Battery Voltage.
 *                        
 *          NOMINAL_BATTERY_VOLTAGE  :  Whole integer value
 *                                      -  Expressed in millivolts.
 *                                      -  This is the Battery Voltage for 
 *                                        which the transformers turns ratio
 *                                        results in the Nominal RMS AC voltage.
 *           
 *           TRANSFORMER_COUNT  :  Number of transformers in Inverter. (Max 4)
 *            
 *           BATTERY_LOW_CUTOFF :  Whole integer value
 *                                 -  Expressed in milliVolts
 *                                 -  Inverter will turn off at this level.
 *                       
 *           BATTERY_VOLTAGE_AT_MAX_ADC  :  Whole integer value.
 *                                          -  Expressed in milliVolts.
 *                                          -  This is the Maximum Battery Voltage 
 *                                            value read by the ADC(analog to digital converter).
 *                                          -  Put another way.  This is the Battery Voltage 
 *                                            that when divided down and feed into the ADC is 3.3 volts.
 *                                          -  Ensure 3.3volts is not exceeded or damage may result.
 * 
 ---------------------------------------------------------------------------------*/




#define NOMINAL_VAC                240
#define NOMINAL_BATTERY_VOLTAGE    26500    // in milliVolts
#define TRANSFORMER_COUNT          4
#define BATTERY_LOW_CUTOFF         24500
#define BATTERY_VOLTAGE_AT_MAX_ADC 35000   // milliVolts

/*--------------------------------------------
 *   Manual Setting of Transformer Secondaries
 *   -   If zero - automatic settings are used.
 *   -   All should be zero or All should be set.
 *   -   Measure or determine Secondary Voltage 
 *          AT Nominal Battery Voltage.
 *   -   Settings should be in Volts x 10
 *       (ie. 235 volts should be 2350)
 ---------------------------------------------*/
#define TRANSFORMER_LARGE_SECVOLT  0 
#define TRANSFORMER_MEDIUM_SECVOLT 0 
#define TRANSFORMER_SMALL_SECVOLT  0 
#define TRANSFORMER_TINY_SECVOLT   0 


/*----------------------------------------------------------
 *       Fan Controlls
 *       
 *       ALL Temperatures are in degrees Celcius.
 *       
 *       Fans can be controlled by PWM or Bang-Bang.
 *          -  For Bang-Bang control, set FANx_ON_FULL with a lower number than
 *            the FANx_ON_TEMP_START and it will use this as the Hysteresis value.
 *       
 *       FANx_ON_TEMP_START :  Temperature where PWM control will start.
 *                             - In Bang-Bang mode Fan will be turned fully on 
 *                               at this temperature.
 *                              
 *       FANx_ON_TEMP_FULL  :  Temperature at which Fan is fully on.
 *                             - In Bang-Bang mode this value is the Hsteresis value.
------------------------------------------------------------ */

#define FAN1_ON_TEMP_START  35
#define FAN1_ON_TEMP_FULL   45
#define FAN2_ON_TEMP_START  45
#define FAN2_ON_TEMP_FULL   5


/*-----------------------------------------------------------------------
 * 
 *       TEMPERATURE_x_CUTOFF   :  Inverter will turn off at these Temperatures.
 * 
 ------------------------------------------------------------------------*/

#define TEMPERATURE_1_CUTOFF 65
#define TEMPERATURE_2_CUTOFF 60



 
