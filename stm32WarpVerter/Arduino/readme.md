 ![Inverter View](https://github.com/mackelec/StepInverter/blob/master/stm32WarpVerter/images/stm32_warpverter_scope.jpg)

###  Arduino Software

The software for the stm32_Warpverter is separated into two projects.

* # stm32_Warpverter project.  
  * This is the software which runs the stm32_Warpverter.  
  * The latest version is [stm32_warpverter_009](https://github.com/mackelec/StepInverter/tree/master/stm32WarpVerter/Arduino/stm32_Warpverter/stm32_Warpverter_009)
  *  This software runs on stm32F103C8 "Blue Pill" and the [stm32_Warpverter PCB](https://github.com/mackelec/StepInverter/tree/master/stm32WarpVerter/PCB).
  *  Make sure tables are created by the Print_Tables Project.  
  *  The tables are pasted into `LookupTables` tab.  
  *  On first execution of the software - Use the Serial monitor to ensure the tables are the right size.

* # Print_Tables project.  
  *  The objective of this software is to take the User Settings and create "Tables" in the form of arrays.  
  *  The USER_Settings tab holds various setting that may be adjusted.  
  *  Upload and run the program on the BluePill.  
  *  The BluePill creates the required "Table" arrays for the stm32_Warpverter and exports the array data to the Serial port.  
  *  From the Serial Monitor the array data is copied and pasted into the stm32_Warpverter project.
  

