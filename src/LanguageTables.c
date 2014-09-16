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
"HZ\nIMPERIAL\nINCLUDED\nINSTRUMENT\nJOB NUMBER\nJOB PEAK RESULTS\nJOB VECTOR SUM RESULTS\nLANGUAGE\nLAST SETUP\n"\
"LCD CONTRAST\nLIGHT\nLIGHTER\nLINEAR\nLIST OF SUMMARIES\nLOCATION\nLOW\nMETRIC\nMIC A\nMIC B\nMINUTE\nMINUTES\nMMPS/DIV\n"\
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
"CHAN R&V SWAPPED\n\0"
};

char italianLanguageTable[] = {
"PONDERAZIONE A\nOPZ PONDERAZIONE A\nCONTROLLO A/D OK\nCAMPO DI ATTIVITA\nTEMPO DI ATTIVAZIONE PERIODO\nDOPO OGNI 24 ORE\n"\
"DOPO OGNI 48 ORE\nDOPO OGNI 72 ORE\nAEREO\nSCALA CANALE AIR\nALLARME 1\nALLARME 1 O.S.A.\nALLARME 1 SISMICO\nALLARME 1 TEMPO\n"\
"ALLARME 2\nALLARME 2 O.S.A.\nALLARME 2 SISMICO\nALLARME 2 TEMPO\nALLARME IN USCITA\nTUTTI I DIRITTI SONO RISERVATI\n"\
"IMPOSTAZIONE CAL COMPLETATA?\nTENTATIVO DI RISOLVERE ERRORE EVENTO PARZIALE.\nAUTOCALIBRAZIONE\nAUTOMONITORAGGIO\n"\
"MONITOR AUTOM DOPO\nGRAFICO A BARRE\nDISTANZA TRA BARRE\nSTAMPA BARRE\nGRAF BARRE ANNUL\nMODO GRAFICO BARRE\nBAR\n"\
"CARICA BATTERIA\nBATTERIA\nTENSIONE DI BATTERIA\nENTRAMBE\nDATA CALIBRAZIONE MEMORIZZATA.\nIMPOSTAZIONE CAL\nIN CALIBRAZIONE\n"\
"CALIBRAZIONE\nTEST CALIBRAZIONE\nDATA CALIBRAZIONE\nDATA CALIBRAZIONE NON IMPOSTATA.\nGRAFICO DI CALIBRAZIONE\n"\
"IMPULSO DI CALIBRAZ\nCANCELL. MODO TIMER?\nCLIENTE\nCOMBO\nMODALITA COMBI\nMODALITA COMBI NON ATTIVA.\nCOMMENTI\n"\
"SOCIETA'\nCONFIG. & OPZIONI\nMENU CONFIG/OPZIONI\nCONFERMA\nCOPIE\nCOPIA\nNON IMPLEMENTATO.\nGIORNALIERO\n"\
"1 VOLTA SETTIMANA\nSCURO\nPI' SCURO\nDATA\nDATA / ORA\nGIORNO-MESE-ANNO\nSTANDARD\nSTD (BAR)\nSTD (COMBI)\n"\
"STD (AUTO TRG)\nDISABILITATO\nDISATTIVAZIONE MODO TIMER.\nNON TROVATO COLLEGAMENTO AD EVENTO.\nPERCORSO SISMICO\n"\
"NON SPEGNERE SINO AL COMPLETAMENTO DELL�OPERAZIONE.\nVUOI ATTIVARE IL TRIGGER MANUALE?\n"\
"VUOI USCIRE DALLA MODALITA IMPOSTAZIONE CAL?\nVUOI USCIRE DALLA MODALITA MONITORAGGIO?\n"\
"VUOI SALVARE LA DATA DI CALIBRAZIONE?\nMODIFICA\nVUOTO\nABILITATO\nELIMINAZIONE COMPLETATA\nCANCELLA GLI EVENTI\n"\
"CANCELLA MEMORIA\nELIMINAZIONE IN CORSO.\nIMPOSTAZ CANCELLAZ\nERRORE\nDIGITA ESC\nEVENTO\n"\
"AZZERAMENTO DELL�EVENTO COMPLETATO\nSOMMARIO EVENTI\nIMPOSTAZIONE DI FABBRICA COMPLETATA\n"\
"IMPOSTAZIONI DI FABBRICA NON DISPONIBILI\nNON RIUSCITO\nPIEDI\nFREQUENZA\nGRAFICO FREQUENZA\nFREQ\nPIENA\n"\
"REGISTRAZIONE GRAFICA\nMENU AIUTO\nINFO DI AIUTO\nMENU DI AIUTO\nORA\nORE\nH\nHZ\nANGLOSASSONE\nINCLUSO\nSTRUMENTO\n"\
"COMMESSA NUMERO\nPICCO NELLA SESSIONE\nVETTORE SOMMA NELLA SESSIONE\nLINGUA\nULTIMA SETUP\nCONTRASTO DISPLAY\nCHIARO\n"\
"PIU' CHIARO\nLINEARE\nSOMMARIO\nPOSIZIONE\nBASSA\nMETRICA\nMIC A\nMIC B\nMINUTO\nMINUTI\nMMPS/DIV\nMONITORAGGIO\n"\
"GRAFICO A BARRE\nMONITORAGGIO\nMENSILE\nNOME\nMEMORIZZAZIONE PROSSIMO EVENTO MEMORIA NON SATURA.\nNO\nNO AUTOCALIBRAZ.\n"\
"NO AUTO MONITOR\nNESSUN EVENTO REGISTRATO.\nMENU DI BASE\nNON INCLUSO\nNOTE\nSPENTO\nACCESO\nUNA VOLTA\nOPERATORE\n"\
"SOVRASCRITTURA\nRISULTATI PARZIALI\nRIUSCITO\nPICCO\nPICCO O.S.A.\nPER FAVORE ATTENDI.\n"
"CANCELLA LA MEMORIA PER RISOLVERE IL PROBLEMA.\nPREGO SPEGNERE L'UNITA'.\nPREGO PREMERE INVIO.\nGRAFICO STANDARD\nIN SPEGNIMENTO.\n"\
"STAMPA GRAFICI\nSTAMPANTE\nSTAMPANTE\nSTAMPA BLOCCATA\nSTAMPA\nPROCEDI SENZA IMPOSTARE DATA ED ORARIO?\n"\
"IN ESECUZIONE\nRAD\nRADIALE\nELENCO EVENTI IN RAM NON INIZIALIZZATO.\nTEMPO DI REG.\nREPORT SPOSTAMENTI\nRISULTATI\n"\
"FREQ CAMPIONAMENTO\nSALVA MODIFICHE CHIARO\nSALVA IMPOSTAZ.\nIMPOSTAZ. SALVATE\nRICERCA EVENTI ATTUALI. IN MEMORIA \nS\nSECONDO\n"\
"SECONDI\nTRIG SISM\nSITO DELLA MISURA\nSISMICO\nTRIGGER SISMICO\nSELEZIONA\nFORMA D'ONDA\nSENSORE A\nSENSORE B\n"\
"PROVA SENSORE\nSENSORE GUADAGNO/TIPO\nGUADAGNO/TIPO SENSORE NON IMPOSTATO.\nTIPO DI SENSORE\nNUMERO DI SERIE\n"\
"NUMERO DI SERIE NON IMPOSTATO.\nLE IMPOSTAZIONI NON SARANNO CARICATE.\nVER. SOFTWARE\nAEREO\nTRIG AIR\nTRIGGER AEREO\n"\
"SPAGNOLO\nDATA DI INIZIO\nORA DI INIZIO\nSTATO\nDATA DI FINE\nTERMINA LA STAMPA\nORA DI FINE\nCOMPLETATO\n"\
"RIASSUNTO EVENTI\nINTERVALLO MISURA\nRISULTATI DELL�INTERVALLO NEL SOMMARIO\nIN CONTROLLO\n"\
"QUESTA FUNZIONE NON E ATTUALMENTE DISPONIBILE.\nORA\nFREQUENZA TIMER\nMODALITA' TIMER\nMODALITA TIMER DISATTIVATA.\n"\
"TIMER ATTIVATO.\nMODALITA TIMER NON COMPLETATA.\nIMPOSTAZ NON VALIDA\nTRAS\nTRASVERSALE\n"\
"L�UNITA E IN MODALITA TIMER.\nUNITA' DI MISURA\nRAPPORTO U.S.B.M./O.S.M.R.E.\nVETTORE SOMMA\nVERIFICHI\nVERT\nVERTICALE\n"\
"VOLT\nATTENZIONE\nMODO FORME D'ONDA\nSETTIMANALE\nCARICA COOPERANTE\nSI\nDARE LE IMPOSTAZIONI DI FABBRICA.\n"\
"AZZERAM NUM EVENTI\nCOMPENSAZ SENSORI\nOK\nSTOP\nMAX\nCARATTERI\nRANGE\nINGLESE\nITALIANO\nFRANCESE\nINGLESE\n"\
"DIN 4150\nUS BOM\nX1 (508 mm/s)\nX2 (254 mm/s)\nX4 (127 mm/s)\nX8 (63 mm/s)\nBARRE\nAUTO TRG\nFINE\n"\
"CONFERMI CANCELLAZIONE?\nIMPOSTAZ. MODEM\nINIZIALIZ. MODEM\nNUMERO DA CHIAMARE\nRESET MODEM \nSENSIBILITA'\nALTA\nCODICE DI SBLOCCO\n"\
"ACC (793L)\nTEDESCO\nBAUD RATE\nPOWER UNIT OFF EARLY?\nNAME ALREADY USED\nDELETE SAVED SETUP?\n"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\nSCALA DELLE BARRE\nREPORT MILLIBARS\nTIMEOUT DISPLAY\nIMPULSE RESULTS\nAGGRIORNA PICCO\n"\
"STAMPA IL LOG\nLOG REGISTRAZIONI\nEVENTI REGISTRATI\nNUM DELL'EVENTO\nCANCEL ALL PRINT JOBS?\nCAL SUMMARY\nRICHIAMATE N.\n"\
"TEMPO DI RICHIAMATA\nMEMORIA CIRCOLARE\nSTATISTICA MEMORIA\nINFO DI AUTOCHIMATA\nPEAK DISPLACEMENT\nAUTOCAL FORMA D'ONDA\n"\
"START FORMA D'ONDA\nCALIBRATED ON\nGUARDA IL LOG\nSTAMPA A VIDEO\nRISULTATI DEL LOG\nRISULTATI DEL LOG\nPRINT MONITOR LOG ENTRY?\n"\
"PRINTER DISABLED\nPRINTING\nREPORT GRAFICO BARRE\nFORMATO NORMALE\nFORMATO RIDOTTO\nREPORT PEAK ACC\nPEAK ACCELERATION\n"\
"NUM DELL'EVENTO\nINIZIO DEL LOG\nFINI DEL LOG\nUSATI\nLIBERI\nDATI EVENTI\nCIRCOLARE\nSTATISTICA MEM.\n"\
"SPACE REMAINING (CURR. SETTINGS)\nPRIMA DI SOVRA. (PARAM. ATTUALI)\nFORMA D'ONDA\nORE GRAF.A BARRE\nULT EVT SCARIC\nULT EVT REG\n"\
"ULT CONNES.\nINFO DI AUTO CHIMATA\nUNITS OF AIR\nDECIBEL\nMILLIBAR\n115200 BAUD RATE\nBIT ACCURACY\nBIT\nHOURLY\nRECALIBRATE\n"\
"ON TEMP. CHANGE\nPRETRIGGER SIZE\nQUARTER SECOND\nHALF SECOND\nFULL SECOND\nPOWER SAVINGS\nNONE\nMINIMUM\nMOST\n"\
"ANALOG CHAN CONFIG\0CHAN R&V SCHEMATIC\nCHAN R&V SWAPPED\n\0"
};

char frenchLanguageTable[] = {
"FILTRE A\nOPTION FILTRE A\nCALIBRAGE A/D CORRECT\nPER. DATES ACTIVES\nPER. HEURES ACTIVES\nAPR. TS LES 24 HR\n"\
"APR. TS LES 48 HR\nAPR. TS LES 72 HR\nSON\nECHELLE SUPRESSION\nALARME 1\nNIV. SON - ALARME 1\nNIV. VIB - ALARME 1\n"\
"HEURE - ALARME 1\nALARME 2\nNIV. SON - ALARME 2\nNIV. VIB - ALARME 2\nHEURE - ALARME 2\nMODE SORTIE ALARME\n"\
"TOUS DROITS RESERVES\nAVEZ-VOUS EFFECTUE LE REGLAGE DE CALIBRATION?\nTENTATIVE DE RECUPERATION D'ERREUR EN COURS\n"\
"AUTO CALIBRAGE\nAUTODECLENCHEMENT\nAUTODECLENCH. APRES\nDIAGRAMME BARRES\nINTERVALLE DE BARRE\nIMP. EN MODE BARRES\n"\
"DIAGRAMME BARRES ANNULE\nMODE DIAG. BARRES\nBARRES\nVOLTAGE\nBATTERIE\nVOLTAGE\nLES DEUX\nCAL DATE SAUVEE\nREGLAGE CAL.\n"\
"CAILBRAGE EN COURS\nCALIBRAGE\nCONTROLE DE CALIBRAGE\nDATE DE CALIBRAGE\nDATE DE CALIBRAGE NON REGLEE\n"\
"GRAPHE DE CALIBRAGE\nSIGNAL DE CALIBRAGE\nANNULER LE MODE HORLOGE?\nCLIENT\nCOMBO\nMODE COMBO\nMODE COMBO NON INSTALLE\n"\
"COMMENTAIRES\nCOMPAGNIE\nCONFIG & OPTIONS\nMENU CONFIG/OPTIONS\nCONFIRMER\nCOPIES\nCOPIER\nNON INSTALLE ACTUELLEMENT\n"\
"JOURNALIER (TOUS)\nJOURNALIER (OUVRE)\nSOMBRE\nPLUS SOMBRE\nDATE\nDATE/HEURE\nJOUR-MOIS-ANNEE\nDEFAUT\nDEFAUT (BARRES.)\n"\
"DEFAUT (COMBO.)\nDEFAUT (AUTO DECL.)\nDESACTIVE\nDESACTIVATION DU MODE HORLOGE\n"\
"DECOUVERTE D'UN ACCES INTERROMPU A UNE MESURE\nDISTANCE SOURCE\nNE PAS ETEINDRE AVANT QUE L'OPERATION SOIT ACHEVEE\n"\
"VOULEZ-VOUS PASSER EN MODE MANUAL?\nVOULEZ-VOUS QUITTER LE MODE REGLAGE?\nVOULEZ-VOUS QUITTER LE MODE ENREGISTREMENT?\n"\
"VOULEZ-VOUS SAUVER LA DATE DE CALIBRAGE?\nEDITER\nVIDE\nAUTORISE\nEFFACEMENT ACHEVE\nEFFACE LES MESURES\n"\
"EFFACE LA MEMOIRE\nEFFACEMENT EN COURS\nEFFACE LES REGLAGES\nERREUR\nTAPER ESC\nMESURE\n"\
"REMISE A ZERO DU NOMBRE DE MESURES ACHEVEE \nRESUME DE MESURE\nREGLAGE USINE ACHEVE\n"\
"LES DONNEES DE REGLAGE USINE SONT INTROUVABLES\nECHEC\nPIED\nFREQUENCE\nGRAPHE FREQUENCE\nFRQ\nPLEIN\n"\
"ENREGISTREMENT GRAPHIQUE\nMENU INFO AIDE\nINFORMATION\nMENU AIDE\nHEURE\nHEURES\nH\nHZ\nIMPERIAL\nINCLUS\nINSTRUMENT\n"\
"NUMERO D'ACTION\nACTION CALCUL DE PIC\nACTION CALCUL DE RESULTANTE\nLANGAGE\nDERNIER REGLAGE\nCONTRASTE DU LCD\nCLAIR\n"\
"PLUS CLAIR\nLINEAIRE\nLISTE  DES RESUMES\nLIEU\nBAS\nMETRIQUE\nMICRO A\nMICRO B\nMINUTE\nMINUTES\nMM/S/DIV\nENREGISTRER\n"\
"MODE BARRE\nENR. EN COURS\nMENSUEL\nNOM\nLA ZONE MEMOIRE SUIVANTE N'EST PAS VIDE\nNON\nPAS D'AUTOCALIBRAG\n"\
"PAS D'ENREG. AUTO.\nAUCUNE MESURE RETROUVEE\nMENU PPAL NOMIS\nNON INCLUS\nNOTES\nETEINT\nALLUME\nUNE FOIS\nOPERATEUR\n"\
"ECRASER REGLAGES\nRESULTATS PARTIELS\nREUSSI\nPIC\nSURPRESSION EN PIC\nATTENDEZ SVP\n"\
"ESSAYER D'EFFACER MEMOIRE POUR RESOUDRE LE PROBLEME\nETEINDRE L'APPAREIL SVP\nAPPUYER SUR ENTER SVP\nGRAPHE STANDARD\n"\
"ETEINDRE MAINTENANT\nIMPRIMER LE GRAPHE\nIMPRIMANTE\nIMPRIMANTE ACTIVE\nIMPRESSION ARRETEE\nIMPRESSION\n"\
"CONTINUER SANS REGLER L'HEURE ET LA DATE ?\nTRAITEMENT\nRADIAL\nRADIAL\nLA RAM N'EST PAS FORMATTEE\nDUREE DE MESURE\n"\
"RAPPORT DEPLACEMENT\nRESULTATS\nVITESSE D'ECHANT.\nENREGISTRE MODIF.\nENREGISTRE REGLAGE\nREGLAGES SAUVES\n"\
"RECHERCHE DES MESURES VALIDES\nSEC\nSECONDES\nSECONDES\nSEUIL VIB\nEMPLACEMENT VIB.\nSISMIQUE\nSEUIL SISMIQUE\nSELECTION\n"\
"AUTODECLENCHEMENT\nCAPTEUR A\nCAPTEUR B\nCONTROLE CAPTEUR\nGAIN/TYPE CAPTEUR\nTYPE/GAIN DU CAPTEUR NON REGLE\n"\
"TYPE DE CAPTEUR\nNUMERO DE SERIE\nNUMERO DE SERIE NON REGLE\nLES REGLAGES NE SERONT PAS CHARGES\nVERSION LOGICIEL\nSON\n"\
"SEUIL SON\nSEUIL SURPRESSION\nESPAGNOL\nDATE DE DEBUT\nHEURE DE DEBUT\nSTATUT\nDATE D'ARRET\nARRET D'IMPRESSION\n"\
"HEURE D'ARRET\nREUSSITE\nRESUME DES EVENEMENTS\nRESUME / INTERVALLE\nRESULTAT RESUME PAR INTERVALLE\nTEST EN COURS\n"\
"CETTE OPTION N'EST PAS DISPONIBLE\nHEURE\nFREQUENCE D'HORLOGE\nMODE HORLOGE\nMODE HORLOGE DESACTIVE\nMODE HORLOGE ACTIF\n"\
"REGLAGE HORLOGE INCOMPLET\nREGLAGE HORLOGE INCORRECT\nTRAN.\nTRANSVERSAL\nUNITES EN MODE TIMER\nUNITES DE MESURE\n"\
"RAPPORT USBM/OSMRE\nRESULTANTE\nVERIFIER\nVERT\nVERTICAL\nVOLTS\nATTENTION\nMODE SIGNL SISMIQUE\nHEBDOMADAIRE\n"\
"CHARGE PAR RETARD\nOUI\nVOUS ENTREZ DANS LE MODE REGLAGE USINE\nRAZ NBRE EVENEMENTS\nRAZ DES CAPTEURS\nOK\nANNULER\nMAX\n"\
"CAR.\nGAMME\nANGLAIS\nITALIEN\nFRANCAIS\nANGLAIS\nDIN 4150-3-TAB. 1\nUSBM\nX1 (508 MM/S)\nX2 (254 MM/S)\nX4 (127 MM/S)\n"\
"X8 (63 MM/S)\nBARRE\nAUTODECL\nFIN\nETES-VOUS SUR DE VOULOIR EFFACER TOUTE LA MEMOIRE?\nCONFIGURATION MODEM\n"\
"INITIALISATIO MODEM\nCOMMUNICATION MODEM\nRAZ DU MODEM\nSENSIBILITE\nHAUT\nCODE DEVEROUILLAGE\nACC (793L)\nALLEMAND\n"\
"BAUD RATE\nPOWER UNIT OFF EARLY?\nNAME ALREADY USED\nDELETE SAVED SETUP?\nNAME MUST HAVE AT LEAST ONE CHARACTER\n"\
"BAR SCALE\nREPORT MILLIBARS\nLCD TIMEOUT\nIMPULSE RESULTS\nLCD IMPULSE TIME\nPRINT MONITOR LOG\nMONITOR LOG\n"\
"EVENTS RECORDED\nEVENT NUMBERS\nCANCEL ALL PRINT JOBS?\nCAL SUMMARY\nMODEM RETRY\nMODEM RETRY TIME\nFLASH WRAPPING\n"\
"FLASH STATS\nAUTO DIAL INFO\nPEAK DISPLACEMENT\nWAVEFORM AUTO CAL\nSTART OF WAVEFORM\nCALIBRATED ON\nVIEW MONITOR LOG\n"\
"PRINT IN MONITOR\nLOG RESULTS\nMONITOR LOG RESULTS\nPRINT MONITOR LOG ENTRY?\nPRINTER DISABLED\nPRINTING\n"\
"BARGRAPH REPORT\nNORMAL FORMAT\nSHORT FORMAT\nREPORT PEAK ACC\nPEAK ACCELERATION\nEVENT #s\nSTART OF LOG\nEND OF LOG\n"\
"USED\nFREE\nEVENT DATA\nWRAPPED\nFLASH USAGE STATS\nSPACE REMAINING (CURR. SETTINGS)\nBEFORE OVERWRITE (CURR. SETTINGS)\n"\
"WAVEFORMS\nBAR HOURS\nLAST DL EVT\nLAST REC\nLAST CONNECT\nAUTO DIALOUT INFO\nUNITS OF AIR\nDECIBEL\nMILLIBAR\n115200 BAUD RATE\n"\
"BIT ACCURACY\nBIT\nHOURLY\nRECALIBRATE\nON TEMP. CHANGE\nPRETRIGGER SIZE\nQUARTER SECOND\nHALF SECOND\nFULL SECOND\nPOWER SAVINGS\n"\
"NONE\nMINIMUM\nMOST\nANALOG CHAN CONFIG\0CHAN R&V SCHEMATIC\nCHAN R&V SWAPPED\n\0"
};

char germanLanguageTable[] = {
"EINGEBEN\nOPTION EINGEBEN\nA/D KALIBRIERUNG OK\nZEITSPANNE DATUM\nZEITSPANNE UHRZEIT\nALLE 24 STUNDEN\nALLE 48 STUNDEN\n"\
"ALLE 72 STUNDEN\nLARM\nSKALA SCHALLMESSUNG\nALARM 1\nALARM 1 SCHALL\nGEO ALARMSCHWELLE 1\nALARM 1 ZEIT\nALARM 2\n"\
"ALARM 2 SCHALL\nGEO ALARMSCHWELLE 2\nALARM 2 ZEIT\nALARM AUSGABEMODUS\nALLE RECHTE VORBEHALTEN\nCAL SETUP BEENDEN ?\n"\
"VERSUCHE, PARTIELLEN EREIGNISFEHLER ZU BEHEBEN\nAUTO KALIBRIERUNG\nAUTO MONITOR\nAUTO MONITOR NACH\nSAULENDIAGRAMM\n"\
"SAULEN-INTERVALL\nBALKENDIAGR. DRUCK\nSAULENDIAGRAMM ABGEBROCHEN\nMODUS S�ULENDIAGR.\nSAULEN\nBATT.-SPANNUNG\nBATTERIE\n"\
"BATTERIESPANNUNG\nBEIDE\nKALIBRIERUNGSDATUM GESPEICHERT\nKALIBRIERUNG- SETUP\nKALIBRIERE\nKALIBRIERUNG\n"\
"KALIBRIERUNGS - CHECK\nKALIBRIERUNGSDATUM\nKEIN KALIBRIERUNGSDATUM EINGEGEBEN\nKALIBRIERUNGSKURVE\nKALIBRIERUNGSIMPULS\n"\
"TIMER - MODUS ABBRECHEN?\nKUNDE\nKOMBIN.\nKOMBINATIONSMODUS\nKOMBINATIONSMODUS NICHT EINGESTELLT\nKOMMENTAR:\nUNTERNEHMEN:\n"\
"KONFIGURATION:\nKONFIGURATIONS-MENU\nBESTATIGEN\nKOPIEN\nKOPIEREN\nMOMENTAN NICHT EINGESTELLT\nTAGLICH-JEDEN TAG\n"\
"TAGLICH-WOCHENTAGE\nDUNKEL\nDUNKLER\nDATUM\nDATUM / ZEIT\nTAG - MONAT - JAHR\nVORGABE\nVORGABE (SAULEN)\n"\
"VORGABE (KOMBINATION)\nVORGABE (SELF TRG)\nAUSGESCHALTET\nTIMER - MODUS AUSGESCHALTET\n"\
"ABGEBROCHENE EREIGNISVERBINDUNG FESTGESTELLT\nABSTAND Z SPRENGORT\nNICHT ABSCHALTEN BEVOR DER VORGANG NICHT BEENDET IST\n"\
"IN DEN MANUALLEN - TRIGGER - MODUS EINSTEIGEN ?\nKAL - SETUP MODUS VERLASSEN?\nAUFZEICHNUNGS - MODUS VERLASSEN?\n"\
"KALIBRIERUNGSDATUM SICHERN?\nBEARBEITEN\nLEER\nAKTIV\nALLES LOSCHEN\nEREIGNISSE LOSCHEN\nSPEICHER LOSCHEN\n"\
"LAUFENDEN VORGANG LOSCHEN\nEINSTELLUNG LOSCHEN\nFEHLER\nHIT ABBRECHEN\nEREIGNIS LOSCHEN\nEREIGNISNUMMER NULLSETZEN FERTIG\n"\
"ALLE EREIGNISSE ZEIGE\nHERSTELLER - SETUP FERTIG\nHERSTELLER - SETUPDATEN NICHT GEFUNDEN\nNICHT ERFOLGREICH\nFEET\n"\
"FREQUENZ\nFREQUENZ - AUSDRUCK\nFRQ\nVOLL\nGRAPHISCHE AUFNAHME\nHILFE - MENU\nHILFE\nHILFE - MENU\nSTUNDE\nSTUNDEN\nSTD.\n"\
"HZ.\nIMPERIAL\nINBEGRIFFEN\nINSTRUMENT\nAUFTRAGSNUMMER\nMAX - WERTE AUFTRAG\nVEKTORSUMME AUFTRAG - RESULTATE\nSPRACHE\n"\
"LETZTES SETUP\nLCD KONTRAST\nHELL/LICHT\nHELLER\nLINEAR\nLISTE D. ERGEBNISSE\nORT\nSCHWACH\nMETRISCH\nMIKROFON A\n"\
"MIKROFON B\nMINUTE\nMINUTEN\nMMPS/DIV\nMONITOR\nMONITOR - DIAGRAMM\nBEOBACHTUNG\nMONATLICH\nNAME\n"\
"NACHSTER EREIGNIS - SPEICHERPLATZ NICHT LEER\nNEIN\nKEINE AUTOKALIBR.\nKEINE AUTO MESSUNG\n"\
"KEINE GESPEICHERTEN EREIGNISSE GEFUNDEN\nNOMIS HAUPTMENU\nNICHT INBEGRIFFEN\nNOTIZEN\nAUS\nEIN\nEINMAL\nBEDIENER\n"\
"EINSTELL. SPEICHERN\nTEILWEISE RESULTATE\nGENEHMIGT\nPEAK\nPEAK AIR\nBITTE EINEN AUGENBLICK GEDULD\n"\
"UM DAS PROBLEM ZU BEHEBEN BITTE FLASH LOSCHEN\nBITTE GERAT AUSSCHALTEN\nENTER DRUCKEN\nPLOT - STANDARD\n"\
"GERAT WIRD HERUNTERGEFAHREN\nDIAGRAMM DRUCKEN\nDRUCKER\nDRUCKER EIN\nDRUCK ABGEBROCHEN\nAUSDRUCK\n"\
"WEITER OHNE DATUMS - UND ZEITEINSTELLUNG\nBEARBEITUNG\nRAD\nRADIAL\nRAM ZUSAMMENSTELLUNGS - TABELLE NICHT INITIALISIERT\n"\
"AUFNAHMEZEIT\nREPORT - ABSTAND\nRESULTATE\nSAMPLING - RATE\nANDERUNGEN SICHERN\nSETUP SPEICHERN\nGESPEICH. EINSTELL\n"\
"SPEICHER WIRD AUF EREIGNISSE UNTERSUCHT\nS\nSEKUNDE\nSEKUNDEN\nSEIS TRIG\nSEIS. STANDORT\nSEISMISCH\nSEISMISCHER TRIGGER\n"\
"AUSWAHLEN\nSELF TRIGGER\nSENSOR A\nSENSOR B\nSENSOR CHECK\nSENSOR VERSTARKUNG\nSENSOR VERSTARKUNG / TYPE NICHT EINGESTELLT\n"\
"SENSOR TYPE\nSERIENNUMMER\nSERIENNUMMER NICHT EINGEGEBEN\nEINSTELLUNGEN WERDEN NICHT GELADEN\nSOFTWARE VERSION\nSCHALL\n"\
"SCHALL TRIG\nSCHALL TRIGGER\nSPANISCH\nSTARTDATUM\nSTARTZEIT\nSTATUS\nENDE DES MESSDATUMS\nDRUCK ABBRECHEN\n"\
"ENDE DER MESSZEIT\nERFOLG\nUBERSICHT EREIGNISSE\nUBERSICHT INTERVALL\nUBERSICHT INTERVALL - RESULTATE\nTESTING\n"\
"DIESE FUNKTION IST MOMENTAN NICHT VERFUGBAR\nZEIT\nTIMER - FREQUENZ\nTIMER - MODUS\nTIMER - MODUS DEAKTIVIERT\n"\
"TIMER - MODUS AKTIV\nTIMER - MODUS - SETUP NICHT ABGESCHLOSSEN\nTIMER - EINSTELLUNGEN UNBRAUCHBAR\nTRAN\nTRANSVERSAL\n"\
"GERAT LAUFT IM TIMER - MODUS\nMABEINHEITEN\nUSBM/OSMRE REPORT\nVEKTORSUMME\nVERIFIZIEREN\nVERT\nVERTIKAL\nVOLT\nWARNUNG\n"\
"WAVEFORM - MODUS\nWOCHENTLICH\nGEWICHT PRO ZZSTUFE\nJA\nSIE BEFINDEN SICH JETZT IM HERSTELLER - SETUP\nEREIGNISNR. NULLEN\n"\
"NULLUNG DER SENSOREN\nOK\nABBRECHEN\nMAX.\nZEICHEN\nBEREICH\nENGLISCH\nITALIENISCH\nFRANZOSICH\nBRITISCH\n"\
"DIN 4150-3-TAB. 1\nUS BOM\nX1 (508 MM/S)\nX2 (254 MM/S)\nX4 (127 MM/S)\nX8 (63 MM/S)\nBALKEN\nSELF TRG\nENDE\n"\
"WIRKLICH ALLE EREIGNISSE LOSCHEN?\nMODEM SETUP\nMODEM INIT\nMODEM DIAL\nMODEM RESET\nSENSITIVITY\nHIGH\nUNLOCK CODE\n"\
"ACC (793L)\nDEUTSCH\nBAUD RATE\nPOWER UNIT OFF EARLY?\nNAME ALREADY USED\nDELETE SAVED SETUP?\n"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\nBAR SCALE\nREPORT MILLIBARS\nLCD TIMEOUT\nIMPULSE RESULTS\nLCD IMPULSE TIME\n"\
"PRINT MONITOR LOG\nMONITOR LOG\nEVENTS RECORDED\nEVENT NUMBERS\nCANCEL ALL PRINT JOBS?\nCAL SUMMARY\nMODEM RETRY\n"\
"MODEM RETRY TIME\nFLASH WRAPPING\nFLASH STATS\nAUTO DIAL INFO\nPEAK DISPLACEMENT\nWAVEFORM AUTO CAL\nSTART OF WAVEFORM\n"\
"CALIBRATED ON\nVIEW MONITOR LOG\nPRINT IN MONITOR\nLOG RESULTS\nMONITOR LOG RESULTS\nPRINT MONITOR LOG ENTRY?\n"\
"PRINTER DISABLED\nPRINTING\nBARGRAPH REPORT\nNORMAL FORMAT\nSHORT FORMAT\nREPORT PEAK ACC\nPEAK ACCELERATION\n"\
"EVENT #s\nSTART OF LOG\nEND OF LOG\nUSED\nFREE\nEVENT DATA\nWRAPPED\nFLASH USAGE STATS\nSPACE REMAINING (CURR. SETTINGS)\n"\
"BEFORE OVERWRITE (CURR. SETTINGS)\nWAVEFORMS\nBAR HOURS\nLAST DL EVT\nLAST REC\nLAST CONNECT\nAUTO DIALOUT INFO\n"\
"UNITS OF AIR\nDECIBEL\nMILLIBAR\n115200 BAUD RATE\nBIT ACCURACY\nBIT\nHOURLY\nRECALIBRATE\nON TEMP. CHANGE\nPRETRIGGER SIZE\n"\
"QUARTER SECOND\nHALF SECOND\nFULL SECOND\nPOWER SAVINGS\nNONE\nMINIMUM\nMOST\nANALOG CHAN CONFIG\0CHAN R&V SCHEMATIC\n"\
"CHAN R&V SWAPPED\n\0"
};

char spanishLanguageTable[] = {
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
"HZ\nIMPERIAL\nINCLUDED\nINSTRUMENT\nJOB NUMBER\nJOB PEAK RESULTS\nJOB VECTOR SUM RESULTS\nLANGUAGE\nLAST SETUP\n"\
"LCD CONTRAST\nLIGHT\nLIGHTER\nLINEAR\nLIST OF SUMMARIES\nLOCATION\nLOW\nMETRIC\nMIC A\nMIC B\nMINUTE\nMINUTES\nMMPS/DIV\n"\
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
"CHAN R&V SWAPPED\n\0"
};
