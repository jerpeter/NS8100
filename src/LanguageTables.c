///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
char englishLanguageTable[] = {
"A WEIGHTING\nA WEIGHTING OPTION\nA/D CALIBRATED OK\nACTIVE DATE PERIOD\nACTIVE TIME PERIOD\nAFTER EVERY 24 HR\n"\
"AFTER EVERY 48 HR\nAFTER EVERY 72 HR\nAIR\nAIR CHANNEL SCALE\nALARM 1\nALARM 1 AIR LEVEL\nALARM 1 SEISMIC LVL\n"\
"ALARM 1 TIME\nALARM 2\nALARM 2 AIR LEVEL\nALARM 2 SEISMIC LVL\nALARM 2 TIME\nALARM OUTPUT MODE\nALL RIGHTS RESERVED\n"\
"ARE YOU DONE WITH CAL SETUP?\nATTEMPTING TO HANDLE PARTIAL EVENT ERROR.\nAUTO CALIBRATION\nAUTO MONITOR\n"\
"AUTO MONITOR AFTER\nBAR GRAPH\nBAR INTERVAL\nBAR DISPLAY RESULT\nBARGRAPH ABORTED\nBARGRAPH MODE\nBARS\nBATT VOLTAGE\n"\
"BATTERY\nBATTERY VOLTAGE\nBOTH\nCAL DATE STORED.\nCAL SETUP\nCALIBRATING\nCALIBRATION\nCALIBRATION CHECK\nCALIBRATION DATE\n"\
"CALIBRATION DATE NOT SET.\nCALIBRATION GRAPH\nCALIBRATION PULSE\nCANCEL TIMER MODE?\nCLIENT\nCOMBO\nCOMBO MODE\n"\
"COMBO MODE NOT IMPLEMENTED.\nCOMMENTS\nCOMPANY\nCONFIG & OPTIONS\nCONFIG/OPTIONS MENU\nCONFIRM\nCOPIES\nCOPY\n"\
"CURRENTLY NOT IMPLEMENTED.\nDAILY (EVERY DAY)\nDAILY (WEEKDAYS)\nDARK\nDARKER\nDATE\nDATE/TIME\nDAY-MONTH-YEAR\nDEFAULT\n"\
"DEFAULT (BAR)\nDEFAULT (COMBO)\nDEFAULT (SELF TRG)\nDISABLED\nDISABLING TIMER MODE.\nDISCOVERED A BROKEN EVENT LINK.\n"\
"DISTANCE TO SOURCE\nDO NOT TURN THE UNIT OFF UNTIL THE OPERATION IS COMPLETE.\nDO YOU WANT TO ENTER MANUAL TRIGGER MODE?\n"\
"DO YOU WANT TO LEAVE CAL SETUP MODE?\nDO YOU WANT TO LEAVE MONITOR MODE?\nDO YOU WANT TO SAVE THE CAL DATE?\nEDIT\nEMPTY\n"\
"ENABLED\nERASE COMPLETE.\nERASE EVENTS\nERASE MEMORY\nERASE OPERATION IN PROGRESS.\nERASE SETTINGS\nERROR\nESC HIT\nEVENT\n"\
"EVENT NUMBER ZEROING COMPLETE.\nEVENT SUMMARY\nFACTORY SETUP COMPLETE.\nFACTORY SETUP DATA COULD NOT BE FOUND.\nFAILED\n"\
"FEET\nFREQUENCY\nFREQUENCY PLOT\nFRQ\nFULL\nGRAPHICAL RECORD\nHELP INFO MENU\nHELP INFORMATION\nHELP MENU\nHOUR\nHOURS\nHR\n"\
"HZ\nINCHES\nINCLUDED\nINSTRUMENT\nJOB NUMBER\nJOB PEAK RESULTS\nJOB VECTOR SUM RESULTS\nLANGUAGE\nLAST SETUP\n"\
"LCD CONTRAST\nLIGHT\nLIGHTER\nLINEAR\nLIST OF SUMMARIES\nLOCATION\nLOW\nMILLIMETERS\nMIC A\nMIC B\nMINUTE\nMINUTES\nMMPS/DIV\n"\
"MONITOR\nMONITOR BARGRAPH\nMONITORING\nMONTHLY\nNAME\nNEXT EVENT STORAGE LOCATION NOT EMPTY.\nNO\nNO AUTO CAL\n"\
"NO AUTO MONITOR\nNO STORED EVENTS FOUND.\nNOMIS 8100 MAIN\nNOT INCLUDED\nNOTES\nOFF\nON\nONE TIME\nOPERATOR\n"\
"OVERWRITE SETTINGS\nPARTIAL RESULTS\nPASSED\nPEAK\nPEAK AIR\nPLEASE BE PATIENT.\n"\
"PLEASE CONSIDER ERASING THE FLASH TO FIX PROBLEM.\nPLEASE POWER OFF UNIT.\nPLEASE PRESS ENTER.\nPLOT STANDARD\n"\
"POWERING UNIT OFF NOW.\nPRINT GRAPH\nPRINTER\nPRINTER ON\nPRINTING STOPPED\nPRINTOUT\n"\
"PROCEED WITHOUT SETTING DATE AND TIME?\nPROCESSING\nRAD\nRADIAL\nRAM SUMMARY TABLE IS UNINITIALIZED.\n"\
"RECORD TIME\nREPORT DISPLACEMENT\nRESULTS\nSAMPLE RATE\nSAVE CHANGES\nSAVE SETUP\nSAVED SETTINGS\n"\
"SCANNING STORAGE FOR VALID EVENTS.\nSEC\nSECOND\nSECONDS\nSEIS TRIG\nSEIS. LOCATION\nSEISMIC\nSEISMIC TRIGGER\nSELECT\n"\
"SELF TRIGGER\nSENSOR A\nSENSOR B\nSENSOR CHECK\nSENSOR GAIN/TYPE\nSENSOR GAIN/TYPE NOT SET.\nSENSOR TYPE\nSERIAL NUMBER\n"\
"SERIAL NUMBER NOT SET.\nSETTINGS WILL NOT BE LOADED.\nSOFTWARE VER\nSOUND\nAIR TRIG\nAIR TRIGGER\nSPANISH\nSTART DATE\n"\
"START TIME\nSTATUS\nSTOP DATE\nSTOP PRINTING\nSTOP TIME\nSUCCESS\nSUMMARIES EVENTS\nSUMMARY INTERVAL\n"\
"SUMMARY INTERVAL RESULTS\nTESTING\nTHIS FEATURE IS NOT CURRENTLY AVAILABLE.\nTIME\nTIMER FREQUENCY\nTIMER MODE\n"\
"TIMER MODE DISABLED.\nTIMER MODE NOW ACTIVE.\nTIMER MODE SETUP NOT COMPLETED.\nTIMER SETTINGS INVALID.\nTRAN\nTRANSVERSE\n"\
"UNIT IS IN TIMER MODE.\nUNITS OF MEASURE\nUSBM/OSMRE REPORT\nVECTOR SUM\nVERIFY\nVERT\nVERTICAL\nVOLTS\nWARNING\n"\
"WAVEFORM MODE\nWEEKLY\nWEIGHT PER DELAY\nYES\nYOU HAVE ENTERED THE FACTORY SETUP.\nZERO EVENT NUMBER\n(ZEROING SENSORS)\n"\
"OK\nCANCEL\nMAX\nCHARS\nRANGE\nENGLISH\nITALIAN\nFRENCH\nBRITISH\nDIN 4150\nUS BOM\nX1 (20 IPS)\nX2 (10 IPS)\nX4 (5 IPS)\n"\
"X8 (2.5 IPS)\nBAR\nSELF TRG\nEND\nREALLY ERASE ALL EVENTS?\nMODEM SETUP\nMODEM INIT\nMODEM DIAL\nMODEM RESET\nSENSITIVITY\n"\
"HIGH\nUNLOCK CODE\nACC (793L)\nGERMAN\nBAUD RATE\nPOWER UNIT OFF EARLY?\nNAME ALREADY USED\nDELETE SAVED SETUP?\n"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\nBAR SCALE\nREPORT MILLIBARS\nLCD TIMEOUT\nIMPULSE RESULTS\nLCD IMPULSE TIME\n"\
"PRINT MONITOR LOG\nMONITOR LOG\nEVENTS RECORDED\nEVENT NUMBERS\nCANCEL ALL PRINT JOBS?\nCAL SUMMARY\nMODEM RETRY\n"\
"MODEM RETRY TIME\nFLASH WRAPPING\nFLASH STATS\nAUTO DIAL INFO\nPEAK DISPLACEMENT\nWAVEFORM AUTO CAL\nSTART OF WAVEFORM\n"\
"CALIBRATED ON\nVIEW MONITOR LOG\nPRINT IN MONITOR\nLOG RESULTS\nMONITOR LOG RESULTS\nPRINT MONITOR LOG ENTRY?\n"\
"PRINTER DISABLED\nPRINTING\nBARGRAPH REPORT\nNORMAL FORMAT\nSHORT FORMAT\nREPORT PEAK ACC\nPEAK ACCELERATION\n"\
"EVENT #s\nSTART OF LOG\nEND OF LOG\nUSED\nFREE\nEVENT DATA\nWRAPPED\nFLASH USAGE STATS\nSPACE REMAINING (CURR. SETTINGS)\n"\
"BEFORE OVERWRITE (CURR. SETTINGS)\nWAVEFORMS\nBAR HOURS\nLAST DL EVT\nLAST REC\nLAST CONNECT\nAUTO DIALOUT INFO\n"\
"UNITS OF AIR\nDECIBEL\nMILLIBAR\n115200 BAUD RATE\nBIT ACCURACY\nBIT\nHOURLY\nRECALIBRATE\nON TEMP. CHANGE\nPRETRIGGER SIZE\n"\
"QUARTER SECOND\nHALF SECOND\nFULL SECOND\nPOWER SAVINGS\nNONE\nMINIMUM\nMOST\nANALOG CHAN CONFIG\0CHAN R&V SCHEMATIC\n"\
"CHAN R&V SWAPPED\nNORMAL (DEFAULT)\nNORM AND NO USB\nNORM AND NO USB/CRAFT\nSAVE COMPRESS DATA\nYES (FASTER DL)\n"\
"FILE SYSTEM BUSY DURING\nERROR TRYING TO ACCESS\nSUMMARY LIST\nINITIALIZING SUMMARY LIST WITH STORED EVENT INFO\nFILE NOT FOUND\n"\
"FILE CORRUPT\nUNABLE TO DELETE\nREMOVING\nUNABLE TO ACCESS\nDIR\nDELETE EVENTS\nREMOVED\nEVENTS\nFAILED TO INIT SD CARD\n"\
"FAILED TO MOUNT SD CARD\nFAILED TO SELECT SD CARD DRIVE\nSD CARD IS NOT PRESENT\nCALCULATING EVENT STORAGE SPACE FREE\n"\
"INITIALIZING MONITOR LOG WITH SAVED ENTRIES\nPLEASE CHARGE BATTERY\nUSB THUMB DRIVE\nUSB DEVICE CABLE WAS REMOVED\n"\
"USB HOST OTG CABLE WAS REMOVED\nDISCONNECTED\nDISCOVERED\nUSB DOWNLOAD\nSYNC EVENTS TO USB THUMB DRIVE?\nSYNC IN PROGRESS\n"\
"SYNC SUCCESSFUL\nTOTAL EVENTS\nNEW\nEXISTING\nSYNC ENCOUNTERED AN ERROR\nPLEASE REMOVE THUMB DRIVE TO CONSERVE POWER\n"\
"USB DEVICE STATUS\nUSB TO PC CABLE WAS CONNECTED\nUSB TO PC CABLE WAS DISCONNECTED\nUSB HOST STATUS\nUSB DEVICE WAS CONNECTED\n"\
"USB OTG HOST CABLE WAS CONNECTED\nUSB STATUS\nUSB CONNECTION DISABLED FOR MONITORING\nUSB CONNECTION DISABLED FOR FILE OPERATION\n"\
"USB DEVICE WAS REMOVED\nAPP LOADER\nSTARTING APP LOADER\nFLASH MEMORY IS FULL\nWRAPPING\nUNAVAILABLE\nSTOPPED\n"\
"CLOSING MONITOR SESSION\nPERFORMING CALIBRATION\nBEING SAVED\nEVENT COMPLETE\nWAVEFORM\nCOMBO - WAVEFORM\nCOMBO - BARGRAPH\n"\
"MAY TAKE TIME\nSYNC PROGRESS\nEXISTS\nSYNCING\nACOUSTIC\nTYPE\nSTANDARD\nSMART SENSOR\nNOT FOUND\nSTART CALIBRATION DIAGNOSTICS?\n"\
"ERASE FACTORY SETUP?\nUNSUPPORTED DEVICE\nPLEASE REMOVE IMMEDIATELY BEFORE SELECTING OK\nUNIT SET FOR EXTERNAL TRIGGER\n"\
"SET TO NO TRIGGER\nPLEASE CHANGE\nSEISMIC OR AIR TRIGGER\nFOR HOURLY MODE THE HOURS AND MINUTES FIELDS ARE INDEPENDENT\n"\
"TIMER MODE HOURLY\nHOURS ARE ACTIVE HOURS, MINS ARE ACTIVE MIN RANGE EACH HOUR\nFIRMWARE\nMODEM SYNC FAILED\nCRAFT SERIAL ERROR\n"\
"SENSOR CALIBRATION\nDISPLAY SUCCESSIVE SAMPLES\nCHANNEL NOISE PERCENTAGES\nDISPLAY CALIBRATED\nDISPLAY NOT CALIBRATED\nZERO\n"\
"MIN MAX AVG\nCONNECT USB CABLE\nCOPY TO SYSTEM DIR\nREMOVE CABLE WHEN DONE\nMENU ACTIVATION\nSERIAL ACTIVATION\nINITIALIZING\n"\
"EXTERNAL TRIGGER\nRS232 POWER SAVINGS\nBOTH TRIGGERS SET TO NO AND EXTERNAL TRIGGER DISABLED\nNO TRIGGER SOURCE SELECTED\n"\
"ALARM TESTING\nCHAN VERIFICATION\n\0"
};
