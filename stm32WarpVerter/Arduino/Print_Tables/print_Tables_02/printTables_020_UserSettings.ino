




/*-------------------------------------------------------------------------------
 * 
 *          USER DEFINED VALUES
 *          ________________________
 *          
 *          INVERTER_TITLE          :  Useful title todescribe this setup( inside " ")
 *                              
 *          THEFACTORTABLE          :  Sets whether you use "theTable" ; Comment this line out to disable.      
 *                                     - "theTable" is 64k in size so you must have a 128k Blue Pill  (most are it seems)
 *                                     - Uses "WATT_PER_AC_VOLTDROP" 
 *                                     - If enabled, Output Regulation should be more accurate.
 *                                     - If disabled, warpverter uses "InputFactorTable" and "currentFactorTable"
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
 *           
 *                       
 *           BATTERY_VOLTAGE_AT_MAX_ADC  :  Whole integer value.
 *                                          -  Expressed in milliVolts.
 *                                          -  This is the Maximum Battery Voltage 
 *                                            value read by the ADC(analog to digital converter).
 *                                          -  Put another way.  This is the Battery Voltage 
 *                                            that when divided down and feed into the ADC is 3.3 volts.
 *                                          -  Ensure 3.3volts is not exceeded or damage may result.
 *                                          
 *           BATTERY_LOW_CUTOFF :       Whole integer value
 *                                      -  Expressed in milliVolts
 *                                      -  Inverter will turn off at this level.                               
 *                                 
 *           BATTERY_HIGH_CUTOFF         Whole integer value
 *                                      -  Expressed in milliVolts
 *                                      -  Inverter will turn off at this level.              
 *                                      
 *           ADC_VALUE_ZERO_DC_CURRENT :    Range 0 - 4095                           
 *                                          - Current transducers often have their 
 *                                           Zero reference at half way.
 *                                          -  Typically 2048
 *                                          
 *           DC_CURRENT_AT_MAX_ADC      :   Value in milliAmps at Maximum ADC range.                        
 *           
 *           DC_CURRENT_CUTOFF          :   Value in milliAmps, DC Current above this level 
 *                                          will shut down Inverter.
 *                                          
 *           DC_CURRENT_PER_AC_VOLTDROP :   Use this setting if "THEFACTORTABLE" is commented out.
 *                                          -  Value in milliAmps.
 *                                          -  Used as a proxy for Power.
 *                                          -  The amount of current which causes
 *                                             one volt RMS drop on the output.
 *                                             
 *           WATT_PER_AC_VOLTDROP        :  Use this setting if using "THEFACTORTABLE" 
 *                                          -  Value in Watts.
 *                                          -  The amount of Power which causes
 *                                             one volt RMS drop on the output.
 * 
 ---------------------------------------------------------------------------------*/


#define INVERTER_TITLE             "Tony, WarpSpeed's Inverter"

//#define THEFACTORTABLE             1          // comment this line out if Blue-Pill is 64k

#define NOMINAL_VAC                236
#define NOMINAL_BATTERY_VOLTAGE    90000      // in milliVolts
#define TRANSFORMER_COUNT          4
#define BATTERY_VOLTAGE_AT_MAX_ADC 180000     // milliVolts
#define BATTERY_LOW_CUTOFF         80000
#define BATTERY_HIGH_CUTOFF        180000

#define ADC_VALUE_ZERO_DC_CURRENT  2048      // half way to 4096 - being full scale
#define DC_CURRENT_AT_MAX_ADC      100000    // milliAmps
#define DC_CURRENT_CUTOFF          90000    // milliAmp

#define DC_CURRENT_PER_AC_VOLTDROP  5000    // milliAmp
#define WATT_PER_AC_VOLTDROP        500    //  watts

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

#define FAN1_ON_TEMP_START  45
#define FAN1_ON_TEMP_FULL   55
#define FAN2_ON_TEMP_START  55
#define FAN2_ON_TEMP_FULL   10

#define FAN1_ON_DC_CURRENT  10000
#define FAN1_OFF_DC_CURRENT  7000
#define FAN2_ON_DC_CURRENT  15000
#define FAN2_OFF_DC_CURRENT 12000
/*-----------------------------------------------------------------------
 * 
 *       TEMPERATURE_x_CUTOFF   :  Inverter will turn off at these Temperatures.
 * 
 ------------------------------------------------------------------------*/

#define TEMPERATURE_1_CUTOFF 65
#define TEMPERATURE_2_CUTOFF 75



 
