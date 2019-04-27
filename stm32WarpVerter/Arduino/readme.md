 ![Inverter View](https://github.com/mackelec/StepInverter/blob/master/stm32WarpVerter/images/stm32_warpverter_scope.jpg)

###  Arduino Software

The software for the stm32_Warpverter is separated into two projects.

* # stm32_Warpverter project.  
  * This is the software which runs the stm32_Warpverter.  
  * The latest version is [stm32_warpverter_009](https://github.com/mackelec/StepInverter/tree/master/stm32WarpVerter/Arduino/stm32_Warpverter/stm32_Warpverter_009)
  *  This software runs on stm32F103C8 "Blue Pill" and the [stm32_Warpverter PCB](https://github.com/mackelec/StepInverter/tree/master/stm32WarpVerter/PCB).
  *  The STM32 BluePill has a generous Flash Rom capacity - notional 64k but most seem to have 128k. This software makes extensive use of pre-calculated lookup tables making full use of available Flash Rom..  
  *  Tables are produced by the "Print_Tables" arduino software.  See Next Section.  
  *  Comprises all the features you need in a functional Inverter.
  *  Most features are configurable - mostly in the Print_Tables software.

* # Print_Tables project.  
  *  The latest version is [print_Tables_2c](https://github.com/mackelec/StepInverter/tree/master/stm32WarpVerter/Arduino/Print_Tables/print_Tables_02).
  *  The objective of this software is to take the User Settings and create "Tables" in the form of arrays.  
  *  The USER_Settings tab holds various setting that may be adjusted.  
  *  Upload and run the program on the BluePill.  
  *  The BluePill creates the required "Table" arrays for the stm32_Warpverter and exports the array data to the Serial port.  
  *  From the Serial Monitor the array data is copied and pasted into the stm32_Warpverter project.
  

## Features

* This software provides the functionality to acheive Tony's "WarpSpeed" Warpverter; a low distortion, four transformer, very low frequency, pure sine wave inverter.
* Open loop control of output voltage based on input voltage and input current.  
* Two 10k thermister inputs and corresponding PWM FAN outputs.  Fully configurable temperatures and control method: modulating PWM or Bang-Bang.
* Seperate ON and OFF inputs.  Off input can be used as external Fault input.
* Bi-Colour LED for ON-OFF status.
* InputCurrent - Fan function.  Two independant Input Current thresholds - when met will operate the two corresponding FANS. 
* Under and Over Input Voltage shut downs (configurable).
* Over Current shut down .
* 50Hz / 60Hz configurable. 
* External Syncronising input.  Using Poida's mains sync routine.  Should sync with other warpVerters accurately.
* 50Hz (60Hz) pulse output (approx 20uSec) will ensure accurate syncronising with other warpVerters.
