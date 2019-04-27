

/*-------------------------------------------------------------------------------
 * 
 *          USER DEFINED VALUES
 *          
 *          TIMER_OVERFLOW : Should either be 
 *                           OVERFLOW_50HZ or OVERFLOW_60Hz
 *           
 *          Almost all user settings have been moved to  stm32_WarpVerter_PrintTables          
 * 
 ---------------------------------------------------------------------------------*/


#define OVERFLOW_50HZ  1406
#define OVERFLOW_60HZ  1172
#define TIMER_OVERFLOW OVERFLOW_50HZ

/*--------------------------------------------------------------------------------------------
 * 
 *   USE_FIXED_BATTERY_VOLTAGE : 
 *                               -  Default would be 0
 *                               -  If not zero, the Warpverter will take this value to be the  
 *                                  battery voltage and ignore the ADC.
 * 
 ---------------------------------------------------------------------------------------------*/
#define USE_FIXED_BATTERY_VOLTAGE 0
