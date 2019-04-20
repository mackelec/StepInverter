
###  Arduino Software

The software for the stm32_Warpverter is separated into two projects.
* # Print_Tables project.  
  *  The objective of this software is to take the User Settings and create "Tables" in the form of arrays.  
  *  The USER_Settings tab holds various setting that may be adjusted.  
  *  Upload and run the program on the BluePill.  
  *  The BluePill creates the required "Table" arrays for the stm32_Warpverter and exports the array data to the Serial port.  
  *  From the Serial Monitor the array data is copied and pasted into the stm32_Warpverter project.
  
* # stm32_Warpverter project.
  *  This software drives stm32_Warpverter PCB.
  *  Make sure tables are created by the Print_Tables Project.  
  *  The tables are pasted into `LookupTables` tab.  
  *  On first execution of the software - Use the Serial monitor to ensure the tables are the right size.
