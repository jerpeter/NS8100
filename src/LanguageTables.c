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
"A WEIGHTING\0A WEIGHTING OPTION\0A/D CALIBRATED OK\0ACTIVE DATE PERIOD\0ACTIVE TIME PERIOD\0AFTER EVERY 24 HR\0"\
"AFTER EVERY 48 HR\0AFTER EVERY 72 HR\0AIR\0AIR CHANNEL SCALE\0ALARM 1\0ALARM 1 AIR LEVEL\0ALARM 1 SEISMIC LVL\0"\
"ALARM 1 TIME\0ALARM 2\0ALARM 2 AIR LEVEL\0ALARM 2 SEISMIC LVL\0ALARM 2 TIME\0ALARM OUTPUT MODE\0ALL RIGHTS RESERVED\0"\
"ARE YOU DONE WITH CAL SETUP?\0ATTEMPTING TO HANDLE PARTIAL EVENT ERROR.\0AUTO CALIBRATION\0AUTO MONITOR\0"\
"AUTO MONITOR AFTER\0BAR GRAPH\0BAR INTERVAL\0BAR DISPLAY RESULT\0BARGRAPH ABORTED\0BARGRAPH MODE\0BARS\0BATT VOLTAGE\0"\
"BATTERY\0BATTERY VOLTAGE\0BOTH\0CAL DATE STORED.\0CAL SETUP\0CALIBRATING\0CALIBRATION\0CALIBRATION CHECK\0CALIBRATION DATE\0"\
"CALIBRATION DATE NOT SET.\0CALIBRATION GRAPH\0CALIBRATION PULSE\0CANCEL TIMER MODE?\0CLIENT\0COMBO\0COMBO MODE\0"\
"COMBO MODE NOT IMPLEMENTED.\0COMMENTS\0COMPANY\0CONFIG & OPTIONS\0CONFIG/OPTIONS MENU\0CONFIRM\0COPIES\0COPY\0"\
"CURRENTLY NOT IMPLEMENTED.\0DAILY (EVERY DAY)\0DAILY (WEEKDAYS)\0DARK\0DARKER\0DATE\0DATE/TIME\0DAY-MONTH-YEAR\0DEFAULT\0"\
"DEFAULT (BAR)\0DEFAULT (COMBO)\0DEFAULT (SELF TRG)\0DISABLED\0DISABLING TIMER MODE.\0DISCOVERED A BROKEN EVENT LINK.\0"\
"DISTANCE TO SOURCE\0DO NOT TURN THE UNIT OFF UNTIL THE OPERATION IS COMPLETE.\0DO YOU WANT TO ENTER MANUAL TRIGGER MODE?\0"\
"DO YOU WANT TO LEAVE CAL SETUP MODE?\0DO YOU WANT TO LEAVE MONITOR MODE?\0DO YOU WANT TO SAVE THE CAL DATE?\0EDIT\0EMPTY\0"\
"ENABLED\0ERASE COMPLETE.\0ERASE EVENTS\0ERASE MEMORY\0ERASE OPERATION IN PROGRESS.\0ERASE SETTINGS\0ERROR\0ESC HIT\0EVENT\0"\
"EVENT NUMBER ZEROING COMPLETE.\0EVENT SUMMARY\0FACTORY SETUP COMPLETE.\0FACTORY SETUP DATA COULD NOT BE FOUND.\0FAILED\0"\
"FEET\0FREQUENCY\0FREQUENCY PLOT\0FRQ\0FULL\0GRAPHICAL RECORD\0HELP INFO MENU\0HELP INFORMATION\0HELP MENU\0HOUR\0HOURS\0HR\0"\
"HZ\0IMPERIAL\0INCLUDED\0INSTRUMENT\0JOB NUMBER\0JOB PEAK RESULTS\0JOB VECTOR SUM RESULTS\0LANGUAGE\0LAST SETUP\0"\
"LCD CONTRAST\0LIGHT\0LIGHTER\0LINEAR\0LIST OF SUMMARIES\0LOCATION\0LOW\0METRIC\0MIC A\0MIC B\0MINUTE\0MINUTES\0MMPS/DIV\0"\
"MONITOR\0MONITOR BARGRAPH\0MONITORING\0MONTHLY\0NAME\0NEXT EVENT STORAGE LOCATION NOT EMPTY.\0NO\0NO AUTO CAL\0"\
"NO AUTO MONITOR\0NO STORED EVENTS FOUND.\0NOMIS 8100 MAIN\0NOT INCLUDED\0NOTES\0OFF\0ON\0ONE TIME\0OPERATOR\0"\
"OVERWRITE SETTINGS\0PARTIAL RESULTS\0PASSED\0PEAK\0PEAK AIR\0PLEASE BE PATIENT.\0"\
"PLEASE CONSIDER ERASING THE FLASH TO FIX PROBLEM.\0PLEASE POWER OFF UNIT.\0PLEASE PRESS ENTER.\0PLOT STANDARD\0"\
"POWERING UNIT OFF NOW.\0PRINT GRAPH\0PRINTER\0PRINTER ON\0PRINTING STOPPED\0PRINTOUT\0"\
"PROCEED WITHOUT SETTING DATE AND TIME?\0PROCESSING\0RAD\0RADIAL\0RAM SUMMARY TABLE IS UNINITIALIZED.\0"\
"RECORD TIME\0REPORT DISPLACEMENT\0RESULTS\0SAMPLE RATE\0SAVE CHANGES\0SAVE SETUP\0SAVED SETTINGS\0"\
"SCANNING STORAGE FOR VALID EVENTS.\0SEC\0SECOND\0SECONDS\0SEIS TRIG\0SEIS. LOCATION\0SEISMIC\0SEISMIC TRIGGER\0SELECT\0"\
"SELF TRIGGER\0SENSOR A\0SENSOR B\0SENSOR CHECK\0SENSOR GAIN/TYPE\0SENSOR GAIN/TYPE NOT SET.\0SENSOR TYPE\0SERIAL NUMBER\0"\
"SERIAL NUMBER NOT SET.\0SETTINGS WILL NOT BE LOADED.\0SOFTWARE VER\0SOUND\0AIR TRIG\0AIR TRIGGER\0SPANISH\0START DATE\0"\
"START TIME\0STATUS\0STOP DATE\0STOP PRINTING\0STOP TIME\0SUCCESS\0SUMMARIES EVENTS\0SUMMARY INTERVAL\0"\
"SUMMARY INTERVAL RESULTS\0TESTING\0THIS FEATURE IS NOT CURRENTLY AVAILABLE.\0TIME\0TIMER FREQUENCY\0TIMER MODE\0"\
"TIMER MODE DISABLED.\0TIMER MODE NOW ACTIVE.\0TIMER MODE SETUP NOT COMPLETED.\0TIMER SETTINGS INVALID.\0TRAN\0TRANSVERSE\0"\
"UNIT IS IN TIMER MODE.\0UNITS OF MEASURE\0USBM/OSMRE REPORT\0VECTOR SUM\0VERIFY\0VERT\0VERTICAL\0VOLTS\0WARNING\0"\
"WAVEFORM MODE\0WEEKLY\0WEIGHT PER DELAY\0YES\0YOU HAVE ENTERED THE FACTORY SETUP.\0ZERO EVENT NUMBER\0(ZEROING SENSORS)\0"\
"OK\0CANCEL\0MAX\0CHARS\0RANGE\0ENGLISH\0ITALIAN\0FRENCH\0BRITISH\0DIN 4150\0US BOM\0X1 (20 IPS)\0X2 (10 IPS)\0X4 (5 IPS)\0"\
"X8 (2.5 IPS)\0BAR\0SELF TRG\0END\0REALLY ERASE ALL EVENTS?\0MODEM SETUP\0MODEM INIT\0MODEM DIAL\0MODEM RESET\0SENSITIVITY\0"\
"HIGH\0UNLOCK CODE\0ACC (793L)\0GERMAN\0BAUD RATE\0POWER UNIT OFF EARLY?\0NAME ALREADY USED\0DELETE SAVED SETUP?\0"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\0BAR SCALE\0REPORT MILLIBARS\0LCD TIMEOUT\0IMPULSE RESULTS\0LCD IMPULSE TIME\0"\
"PRINT MONITOR LOG\0MONITOR LOG\0EVENTS RECORDED\0EVENT NUMBERS\0CANCEL ALL PRINT JOBS?\0CAL SUMMARY\0MODEM RETRY\0"\
"MODEM RETRY TIME\0FLASH WRAPPING\0FLASH STATS\0AUTO DIAL INFO\0PEAK DISPLACEMENT\0WAVEFORM AUTO CAL\0START OF WAVEFORM\0"\
"CALIBRATED ON\0VIEW MONITOR LOG\0PRINT IN MONITOR\0LOG RESULTS\0MONITOR LOG RESULTS\0PRINT MONITOR LOG ENTRY?\0"\
"PRINTER DISABLED\0PRINTING\0BARGRAPH REPORT\0NORMAL FORMAT\0SHORT FORMAT\0REPORT PEAK ACC\0PEAK ACCELERATION\0"\
"EVENT #s\0START OF LOG\0END OF LOG\0USED\0FREE\0EVENT DATA\0WRAPPED\0FLASH USAGE STATS\0SPACE REMAINING (CURR. SETTINGS)\0"\
"BEFORE OVERWRITE (CURR. SETTINGS)\0WAVEFORMS\0BAR HOURS\0LAST DL EVT\0LAST REC\0LAST CONNECT\0AUTO DIALOUT INFO\0"\
"UNITS OF AIR\0DECIBEL\0MILLIBAR\0115200 BAUD RATE\0BIT ACCURACY\0BIT\0HOURLY\0RECALIBRATE\0ON TEMP. CHANGE\0PRETRIGGER SIZE\0"\
"QUARTER SECOND\0HALF SECOND\0FULL SECOND\0\0"
};

char italianLanguageTable[] = {
"PONDERAZIONE A\0OPZ PONDERAZIONE A\0CONTROLLO A/D OK\0CAMPO DI ATTIVITA\0TEMPO DI ATTIVAZIONE PERIODO\0DOPO OGNI 24 ORE\0"\
"DOPO OGNI 48 ORE\0DOPO OGNI 72 ORE\0AEREO\0SCALA CANALE AIR\0ALLARME 1\0ALLARME 1 O.S.A.\0ALLARME 1 SISMICO\0ALLARME 1 TEMPO\0"\
"ALLARME 2\0ALLARME 2 O.S.A.\0ALLARME 2 SISMICO\0ALLARME 2 TEMPO\0ALLARME IN USCITA\0TUTTI I DIRITTI SONO RISERVATI\0"\
"IMPOSTAZIONE CAL COMPLETATA?\0TENTATIVO DI RISOLVERE ERRORE EVENTO PARZIALE.\0AUTOCALIBRAZIONE\0AUTOMONITORAGGIO\0"\
"MONITOR AUTOM DOPO\0GRAFICO A BARRE\0DISTANZA TRA BARRE\0STAMPA BARRE\0GRAF BARRE ANNUL\0MODO GRAFICO BARRE\0BAR\0"\
"CARICA BATTERIA\0BATTERIA\0TENSIONE DI BATTERIA\0ENTRAMBE\0DATA CALIBRAZIONE MEMORIZZATA.\0IMPOSTAZIONE CAL\0IN CALIBRAZIONE\0"\
"CALIBRAZIONE\0TEST CALIBRAZIONE\0DATA CALIBRAZIONE\0DATA CALIBRAZIONE NON IMPOSTATA.\0GRAFICO DI CALIBRAZIONE\0"\
"IMPULSO DI CALIBRAZ\0CANCELL. MODO TIMER?\0CLIENTE\0COMBO\0MODALITA COMBI\0MODALITA COMBI NON ATTIVA.\0COMMENTI\0"\
"SOCIETA'\0CONFIG. & OPZIONI\0MENU CONFIG/OPZIONI\0CONFERMA\0COPIE\0COPIA\0NON IMPLEMENTATO.\0GIORNALIERO\0"\
"1 VOLTA SETTIMANA\0SCURO\0PI' SCURO\0DATA\0DATA / ORA\0GIORNO-MESE-ANNO\0STANDARD\0STD (BAR)\0STD (COMBI)\0"\
"STD (AUTO TRG)\0DISABILITATO\0DISATTIVAZIONE MODO TIMER.\0NON TROVATO COLLEGAMENTO AD EVENTO.\0PERCORSO SISMICO\0"\
"NON SPEGNERE SINO AL COMPLETAMENTO DELL�OPERAZIONE.\0VUOI ATTIVARE IL TRIGGER MANUALE?\0"\
"VUOI USCIRE DALLA MODALITA IMPOSTAZIONE CAL?\0VUOI USCIRE DALLA MODALITA MONITORAGGIO?\0"\
"VUOI SALVARE LA DATA DI CALIBRAZIONE?\0MODIFICA\0VUOTO\0ABILITATO\0ELIMINAZIONE COMPLETATA\0CANCELLA GLI EVENTI\0"\
"CANCELLA MEMORIA\0ELIMINAZIONE IN CORSO.\0IMPOSTAZ CANCELLAZ\0ERRORE\0DIGITA ESC\0EVENTO\0"\
"AZZERAMENTO DELL�EVENTO COMPLETATO\0SOMMARIO EVENTI\0IMPOSTAZIONE DI FABBRICA COMPLETATA\0"\
"IMPOSTAZIONI DI FABBRICA NON DISPONIBILI\0NON RIUSCITO\0PIEDI\0FREQUENZA\0GRAFICO FREQUENZA\0FREQ\0PIENA\0"\
"REGISTRAZIONE GRAFICA\0MENU AIUTO\0INFO DI AIUTO\0MENU DI AIUTO\0ORA\0ORE\0H\0HZ\0ANGLOSASSONE\0INCLUSO\0STRUMENTO\0"\
"COMMESSA NUMERO\0PICCO NELLA SESSIONE\0VETTORE SOMMA NELLA SESSIONE\0LINGUA\0ULTIMA SETUP\0CONTRASTO DISPLAY\0CHIARO\0"\
"PIU' CHIARO\0LINEARE\0SOMMARIO\0POSIZIONE\0BASSA\0METRICA\0MIC A\0MIC B\0MINUTO\0MINUTI\0MMPS/DIV\0MONITORAGGIO\0"\
"GRAFICO A BARRE\0MONITORAGGIO\0MENSILE\0NOME\0MEMORIZZAZIONE PROSSIMO EVENTO MEMORIA NON SATURA.\0NO\0NO AUTOCALIBRAZ.\0"\
"NO AUTO MONITOR\0NESSUN EVENTO REGISTRATO.\0MENU DI BASE\0NON INCLUSO\0NOTE\0SPENTO\0ACCESO\0UNA VOLTA\0OPERATORE\0"\
"SOVRASCRITTURA\0RISULTATI PARZIALI\0RIUSCITO\0PICCO\0PICCO O.S.A.\0PER FAVORE ATTENDI.\0"
"CANCELLA LA MEMORIA PER RISOLVERE IL PROBLEMA.\0PREGO SPEGNERE L'UNITA'.\0PREGO PREMERE INVIO.\0GRAFICO STANDARD\0IN SPEGNIMENTO.\0"\
"STAMPA GRAFICI\0STAMPANTE\0STAMPANTE\0STAMPA BLOCCATA\0STAMPA\0PROCEDI SENZA IMPOSTARE DATA ED ORARIO?\0"\
"IN ESECUZIONE\0RAD\0RADIALE\0ELENCO EVENTI IN RAM NON INIZIALIZZATO.\0TEMPO DI REG.\0REPORT SPOSTAMENTI\0RISULTATI\0"\
"FREQ CAMPIONAMENTO\0SALVA MODIFICHE CHIARO\0SALVA IMPOSTAZ.\0IMPOSTAZ. SALVATE\0RICERCA EVENTI ATTUALI. IN MEMORIA \0S\0SECONDO\0"\
"SECONDI\0TRIG SISM\0SITO DELLA MISURA\0SISMICO\0TRIGGER SISMICO\0SELEZIONA\0FORMA D'ONDA\0SENSORE A\0SENSORE B\0"\
"PROVA SENSORE\0SENSORE GUADAGNO/TIPO\0GUADAGNO/TIPO SENSORE NON IMPOSTATO.\0TIPO DI SENSORE\0NUMERO DI SERIE\0"\
"NUMERO DI SERIE NON IMPOSTATO.\0LE IMPOSTAZIONI NON SARANNO CARICATE.\0VER. SOFTWARE\0AEREO\0TRIG AIR\0TRIGGER AEREO\0"\
"SPAGNOLO\0DATA DI INIZIO\0ORA DI INIZIO\0STATO\0DATA DI FINE\0TERMINA LA STAMPA\0ORA DI FINE\0COMPLETATO\0"\
"RIASSUNTO EVENTI\0INTERVALLO MISURA\0RISULTATI DELL�INTERVALLO NEL SOMMARIO\0IN CONTROLLO\0"\
"QUESTA FUNZIONE NON E ATTUALMENTE DISPONIBILE.\0ORA\0FREQUENZA TIMER\0MODALITA' TIMER\0MODALITA TIMER DISATTIVATA.\0"\
"TIMER ATTIVATO.\0MODALITA TIMER NON COMPLETATA.\0IMPOSTAZ NON VALIDA\0TRAS\0TRASVERSALE\0"\
"L�UNITA E IN MODALITA TIMER.\0UNITA' DI MISURA\0RAPPORTO U.S.B.M./O.S.M.R.E.\0VETTORE SOMMA\0VERIFICHI\0VERT\0VERTICALE\0"\
"VOLT\0ATTENZIONE\0MODO FORME D'ONDA\0SETTIMANALE\0CARICA COOPERANTE\0SI\0DARE LE IMPOSTAZIONI DI FABBRICA.\0"\
"AZZERAM NUM EVENTI\0COMPENSAZ SENSORI\0OK\0STOP\0MAX\0CARATTERI\0RANGE\0INGLESE\0ITALIANO\0FRANCESE\0INGLESE\0"\
"DIN 4150\0US BOM\0X1 (508 mm/s)\0X2 (254 mm/s)\0X4 (127 mm/s)\0X8 (63 mm/s)\0BARRE\0AUTO TRG\0FINE\0"\
"CONFERMI CANCELLAZIONE?\0IMPOSTAZ. MODEM\0INIZIALIZ. MODEM\0NUMERO DA CHIAMARE\0RESET MODEM \0SENSIBILITA'\0ALTA\0CODICE DI SBLOCCO\0"\
"ACC (793L)\0TEDESCO\0BAUD RATE\0POWER UNIT OFF EARLY?\0NAME ALREADY USED\0DELETE SAVED SETUP?\0"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\0SCALA DELLE BARRE\0REPORT MILLIBARS\0TIMEOUT DISPLAY\0IMPULSE RESULTS\0AGGRIORNA PICCO\0"\
"STAMPA IL LOG\0LOG REGISTRAZIONI\0EVENTI REGISTRATI\0NUM DELL'EVENTO\0CANCEL ALL PRINT JOBS?\0CAL SUMMARY\0RICHIAMATE N.\0"\
"TEMPO DI RICHIAMATA\0MEMORIA CIRCOLARE\0STATISTICA MEMORIA\0INFO DI AUTOCHIMATA\0PEAK DISPLACEMENT\0AUTOCAL FORMA D'ONDA\0"\
"START FORMA D'ONDA\0CALIBRATED ON\0GUARDA IL LOG\0STAMPA A VIDEO\0RISULTATI DEL LOG\0RISULTATI DEL LOG\0PRINT MONITOR LOG ENTRY?\0"\
"PRINTER DISABLED\0PRINTING\0REPORT GRAFICO BARRE\0FORMATO NORMALE\0FORMATO RIDOTTO\0REPORT PEAK ACC\0PEAK ACCELERATION\0"\
"NUM DELL'EVENTO\0INIZIO DEL LOG\0FINI DEL LOG\0USATI\0LIBERI\0DATI EVENTI\0CIRCOLARE\0STATISTICA MEM.\0"\
"SPACE REMAINING (CURR. SETTINGS)\0PRIMA DI SOVRA. (PARAM. ATTUALI)\0FORMA D'ONDA\0ORE GRAF.A BARRE\0ULT EVT SCARIC\0ULT EVT REG\0"\
"ULT CONNES.\0INFO DI AUTO CHIMATA\0UNITS OF AIR\0DECIBEL\0MILLIBAR\0115200 BAUD RATE\0BIT ACCURACY\0BIT\0HOURLY\0RECALIBRATE\0"\
"ON TEMP. CHANGE\0PRETRIGGER SIZE\0QUARTER SECOND\0HALF SECOND\0FULL SECOND\0\0"
};

char frenchLanguageTable[] = {
"FILTRE A\0OPTION FILTRE A\0CALIBRAGE A/D CORRECT\0PER. DATES ACTIVES\0PER. HEURES ACTIVES\0APR. TS LES 24 HR\0"\
"APR. TS LES 48 HR\0APR. TS LES 72 HR\0SON\0ECHELLE SUPRESSION\0ALARME 1\0NIV. SON - ALARME 1\0NIV. VIB - ALARME 1\0"\
"HEURE - ALARME 1\0ALARME 2\0NIV. SON - ALARME 2\0NIV. VIB - ALARME 2\0HEURE - ALARME 2\0MODE SORTIE ALARME\0"\
"TOUS DROITS RESERVES\0AVEZ-VOUS EFFECTUE LE REGLAGE DE CALIBRATION?\0TENTATIVE DE RECUPERATION D'ERREUR EN COURS\0"\
"AUTO CALIBRAGE\0AUTODECLENCHEMENT\0AUTODECLENCH. APRES\0DIAGRAMME BARRES\0INTERVALLE DE BARRE\0IMP. EN MODE BARRES\0"\
"DIAGRAMME BARRES ANNULE\0MODE DIAG. BARRES\0BARRES\0VOLTAGE\0BATTERIE\0VOLTAGE\0LES DEUX\0CAL DATE SAUVEE\0REGLAGE CAL.\0"\
"CAILBRAGE EN COURS\0CALIBRAGE\0CONTROLE DE CALIBRAGE\0DATE DE CALIBRAGE\0DATE DE CALIBRAGE NON REGLEE\0"\
"GRAPHE DE CALIBRAGE\0SIGNAL DE CALIBRAGE\0ANNULER LE MODE HORLOGE?\0CLIENT\0COMBO\0MODE COMBO\0MODE COMBO NON INSTALLE\0"\
"COMMENTAIRES\0COMPAGNIE\0CONFIG & OPTIONS\0MENU CONFIG/OPTIONS\0CONFIRMER\0COPIES\0COPIER\0NON INSTALLE ACTUELLEMENT\0"\
"JOURNALIER (TOUS)\0JOURNALIER (OUVRE)\0SOMBRE\0PLUS SOMBRE\0DATE\0DATE/HEURE\0JOUR-MOIS-ANNEE\0DEFAUT\0DEFAUT (BARRES.)\0"\
"DEFAUT (COMBO.)\0DEFAUT (AUTO DECL.)\0DESACTIVE\0DESACTIVATION DU MODE HORLOGE\0"\
"DECOUVERTE D'UN ACCES INTERROMPU A UNE MESURE\0DISTANCE SOURCE\0NE PAS ETEINDRE AVANT QUE L'OPERATION SOIT ACHEVEE\0"\
"VOULEZ-VOUS PASSER EN MODE MANUEL?\0VOULEZ-VOUS QUITTER LE MODE REGLAGE?\0VOULEZ-VOUS QUITTER LE MODE ENREGISTREMENT?\0"\
"VOULEZ-VOUS SAUVER LA DATE DE CALIBRAGE?\0EDITER\0VIDE\0AUTORISE\0EFFACEMENT ACHEVE\0EFFACE LES MESURES\0"\
"EFFACE LA MEMOIRE\0EFFACEMENT EN COURS\0EFFACE LES REGLAGES\0ERREUR\0TAPER ESC\0MESURE\0"\
"REMISE A ZERO DU NOMBRE DE MESURES ACHEVEE \0RESUME DE MESURE\0REGLAGE USINE ACHEVE\0"\
"LES DONNEES DE REGLAGE USINE SONT INTROUVABLES\0ECHEC\0PIED\0FREQUENCE\0GRAPHE FREQUENCE\0FRQ\0PLEIN\0"\
"ENREGISTREMENT GRAPHIQUE\0MENU INFO AIDE\0INFORMATION\0MENU AIDE\0HEURE\0HEURES\0H\0HZ\0IMPERIAL\0INCLUS\0INSTRUMENT\0"\
"NUMERO D'ACTION\0ACTION CALCUL DE PIC\0ACTION CALCUL DE RESULTANTE\0LANGAGE\0DERNIER REGLAGE\0CONTRASTE DU LCD\0CLAIR\0"\
"PLUS CLAIR\0LINEAIRE\0LISTE  DES RESUMES\0LIEU\0BAS\0METRIQUE\0MICRO A\0MICRO B\0MINUTE\0MINUTES\0MM/S/DIV\0ENREGISTRER\0"\
"MODE BARRE\0ENR. EN COURS\0MENSUEL\0NOM\0LA ZONE MEMOIRE SUIVANTE N'EST PAS VIDE\0NON\0PAS D'AUTOCALIBRAG\0"\
"PAS D'ENREG. AUTO.\0AUCUNE MESURE RETROUVEE\0MENU PPAL NOMIS\0NON INCLUS\0NOTES\0ETEINT\0ALLUME\0UNE FOIS\0OPERATEUR\0"\
"ECRASER REGLAGES\0RESULTATS PARTIELS\0REUSSI\0PIC\0SURPRESSION EN PIC\0ATTENDEZ SVP\0"\
"ESSAYER D'EFFACER MEMOIRE POUR RESOUDRE LE PROBLEME\0ETEINDRE L'APPAREIL SVP\0APPUYER SUR ENTER SVP\0GRAPHE STANDARD\0"\
"ETEINDRE MAINTENANT\0IMPRIMER LE GRAPHE\0IMPRIMANTE\0IMPRIMANTE ACTIVE\0IMPRESSION ARRETEE\0IMPRESSION\0"\
"CONTINUER SANS REGLER L'HEURE ET LA DATE ?\0TRAITEMENT\0RADIAL\0RADIAL\0LA RAM N'EST PAS FORMATTEE\0DUREE DE MESURE\0"\
"RAPPORT DEPLACEMENT\0RESULTATS\0VITESSE D'ECHANT.\0ENREGISTRE MODIF.\0ENREGISTRE REGLAGE\0REGLAGES SAUVES\0"\
"RECHERCHE DES MESURES VALIDES\0SEC\0SECONDES\0SECONDES\0SEUIL VIB\0EMPLACEMENT VIB.\0SISMIQUE\0SEUIL SISMIQUE\0SELECTION\0"\
"AUTODECLENCHEMENT\0CAPTEUR A\0CAPTEUR B\0CONTROLE CAPTEUR\0GAIN/TYPE CAPTEUR\0TYPE/GAIN DU CAPTEUR NON REGLE\0"\
"TYPE DE CAPTEUR\0NUMERO DE SERIE\0NUMERO DE SERIE NON REGLE\0LES REGLAGES NE SERONT PAS CHARGES\0VERSION LOGICIEL\0SON\0"\
"SEUIL SON\0SEUIL SURPRESSION\0ESPAGNOL\0DATE DE DEBUT\0HEURE DE DEBUT\0STATUT\0DATE D'ARRET\0ARRET D'IMPRESSION\0"\
"HEURE D'ARRET\0REUSSITE\0RESUME DES EVENEMENTS\0RESUME / INTERVALLE\0RESULTAT RESUME PAR INTERVALLE\0TEST EN COURS\0"\
"CETTE OPTION N'EST PAS DISPONIBLE\0HEURE\0FREQUENCE D'HORLOGE\0MODE HORLOGE\0MODE HORLOGE DESACTIVE\0MODE HORLOGE ACTIF\0"\
"REGLAGE HORLOGE INCOMPLET\0REGLAGE HORLOGE INCORRECT\0TRAN.\0TRANSVERSAL\0UNITES EN MODE TIMER\0UNITES DE MESURE\0"\
"RAPPORT USBM/OSMRE\0RESULTANTE\0VERIFIER\0VERT\0VERTICAL\0VOLTS\0ATTENTION\0MODE SIGNL SISMIQUE\0HEBDOMADAIRE\0"\
"CHARGE PAR RETARD\0OUI\0VOUS ENTREZ DANS LE MODE REGLAGE USINE\0RAZ NBRE EVENEMENTS\0RAZ DES CAPTEURS\0OK\0ANNULER\0MAX\0"\
"CAR.\0GAMME\0ANGLAIS\0ITALIEN\0FRANCAIS\0ANGLAIS\0DIN 4150-3-TAB. 1\0USBM\0X1 (508 MM/S)\0X2 (254 MM/S)\0X4 (127 MM/S)\0"\
"X8 (63 MM/S)\0BARRE\0AUTODECL\0FIN\0ETES-VOUS SUR DE VOULOIR EFFACER TOUTE LA MEMOIRE?\0CONFIGURATION MODEM\0"\
"INITIALISATIO MODEM\0COMMUNICATION MODEM\0RAZ DU MODEM\0SENSIBILITE\0HAUT\0CODE DEVEROUILLAGE\0ACC (793L)\0ALLEMAND\0"\
"BAUD RATE\0POWER UNIT OFF EARLY?\0NAME ALREADY USED\0DELETE SAVED SETUP?\0NAME MUST HAVE AT LEAST ONE CHARACTER\0"\
"BAR SCALE\0REPORT MILLIBARS\0LCD TIMEOUT\0IMPULSE RESULTS\0LCD IMPULSE TIME\0PRINT MONITOR LOG\0MONITOR LOG\0"\
"EVENTS RECORDED\0EVENT NUMBERS\0CANCEL ALL PRINT JOBS?\0CAL SUMMARY\0MODEM RETRY\0MODEM RETRY TIME\0FLASH WRAPPING\0"\
"FLASH STATS\0AUTO DIAL INFO\0PEAK DISPLACEMENT\0WAVEFORM AUTO CAL\0START OF WAVEFORM\0CALIBRATED ON\0VIEW MONITOR LOG\0"\
"PRINT IN MONITOR\0LOG RESULTS\0MONITOR LOG RESULTS\0PRINT MONITOR LOG ENTRY?\0PRINTER DISABLED\0PRINTING\0"\
"BARGRAPH REPORT\0NORMAL FORMAT\0SHORT FORMAT\0REPORT PEAK ACC\0PEAK ACCELERATION\0EVENT #s\0START OF LOG\0END OF LOG\0"\
"USED\0FREE\0EVENT DATA\0WRAPPED\0FLASH USAGE STATS\0SPACE REMAINING (CURR. SETTINGS)\0BEFORE OVERWRITE (CURR. SETTINGS)\0"\
"WAVEFORMS\0BAR HOURS\0LAST DL EVT\0LAST REC\0LAST CONNECT\0AUTO DIALOUT INFO\0UNITS OF AIR\0DECIBEL\0MILLIBAR\0115200 BAUD RATE\0"\
"BIT ACCURACY\0BIT\0HOURLY\0RECALIBRATE\0ON TEMP. CHANGE\0PRETRIGGER SIZE\0QUARTER SECOND\0HALF SECOND\0FULL SECOND\0\0"
};

char germanLanguageTable[] = {
"EINGEBEN\0OPTION EINGEBEN\0A/D KALIBRIERUNG OK\0ZEITSPANNE DATUM\0ZEITSPANNE UHRZEIT\0ALLE 24 STUNDEN\0ALLE 48 STUNDEN\0"\
"ALLE 72 STUNDEN\0LARM\0SKALA SCHALLMESSUNG\0ALARM 1\0ALARM 1 SCHALL\0GEO ALARMSCHWELLE 1\0ALARM 1 ZEIT\0ALARM 2\0"\
"ALARM 2 SCHALL\0GEO ALARMSCHWELLE 2\0ALARM 2 ZEIT\0ALARM AUSGABEMODUS\0ALLE RECHTE VORBEHALTEN\0CAL SETUP BEENDEN ?\0"\
"VERSUCHE, PARTIELLEN EREIGNISFEHLER ZU BEHEBEN\0AUTO KALIBRIERUNG\0AUTO MONITOR\0AUTO MONITOR NACH\0SAULENDIAGRAMM\0"\
"SAULEN-INTERVALL\0BALKENDIAGR. DRUCK\0SAULENDIAGRAMM ABGEBROCHEN\0MODUS S�ULENDIAGR.\0SAULEN\0BATT.-SPANNUNG\0BATTERIE\0"\
"BATTERIESPANNUNG\0BEIDE\0KALIBRIERUNGSDATUM GESPEICHERT\0KALIBRIERUNG- SETUP\0KALIBRIERE\0KALIBRIERUNG\0"\
"KALIBRIERUNGS - CHECK\0KALIBRIERUNGSDATUM\0KEIN KALIBRIERUNGSDATUM EINGEGEBEN\0KALIBRIERUNGSKURVE\0KALIBRIERUNGSIMPULS\0"\
"TIMER - MODUS ABBRECHEN?\0KUNDE\0KOMBIN.\0KOMBINATIONSMODUS\0KOMBINATIONSMODUS NICHT EINGESTELLT\0KOMMENTAR:\0UNTERNEHMEN:\0"\
"KONFIGURATION:\0KONFIGURATIONS-MENU\0BESTATIGEN\0KOPIEN\0KOPIEREN\0MOMENTAN NICHT EINGESTELLT\0TAGLICH-JEDEN TAG\0"\
"TAGLICH-WOCHENTAGE\0DUNKEL\0DUNKLER\0DATUM\0DATUM / ZEIT\0TAG - MONAT - JAHR\0VORGABE\0VORGABE (SAULEN)\0"\
"VORGABE (KOMBINATION)\0VORGABE (SELF TRG)\0AUSGESCHALTET\0TIMER - MODUS AUSGESCHALTET\0"\
"ABGEBROCHENE EREIGNISVERBINDUNG FESTGESTELLT\0ABSTAND Z SPRENGORT\0NICHT ABSCHALTEN BEVOR DER VORGANG NICHT BEENDET IST\0"\
"IN DEN MANUELLEN - TRIGGER - MODUS EINSTEIGEN ?\0KAL - SETUP MODUS VERLASSEN?\0AUFZEICHNUNGS - MODUS VERLASSEN?\0"\
"KALIBRIERUNGSDATUM SICHERN?\0BEARBEITEN\0LEER\0AKTIV\0ALLES LOSCHEN\0EREIGNISSE LOSCHEN\0SPEICHER LOSCHEN\0"\
"LAUFENDEN VORGANG LOSCHEN\0EINSTELLUNG LOSCHEN\0FEHLER\0HIT ABBRECHEN\0EREIGNIS LOSCHEN\0EREIGNISNUMMER NULLSETZEN FERTIG\0"\
"ALLE EREIGNISSE ZEIGE\0HERSTELLER - SETUP FERTIG\0HERSTELLER - SETUPDATEN NICHT GEFUNDEN\0NICHT ERFOLGREICH\0FEET\0"\
"FREQUENZ\0FREQUENZ - AUSDRUCK\0FRQ\0VOLL\0GRAPHISCHE AUFNAHME\0HILFE - MENU\0HILFE\0HILFE - MENU\0STUNDE\0STUNDEN\0STD.\0"\
"HZ.\0IMPERIAL\0INBEGRIFFEN\0INSTRUMENT\0AUFTRAGSNUMMER\0MAX - WERTE AUFTRAG\0VEKTORSUMME AUFTRAG - RESULTATE\0SPRACHE\0"\
"LETZTES SETUP\0LCD KONTRAST\0HELL/LICHT\0HELLER\0LINEAR\0LISTE D. ERGEBNISSE\0ORT\0SCHWACH\0METRISCH\0MIKROFON A\0"\
"MIKROFON B\0MINUTE\0MINUTEN\0MMPS/DIV\0MONITOR\0MONITOR - DIAGRAMM\0BEOBACHTUNG\0MONATLICH\0NAME\0"\
"NACHSTER EREIGNIS - SPEICHERPLATZ NICHT LEER\0NEIN\0KEINE AUTOKALIBR.\0KEINE AUTO MESSUNG\0"\
"KEINE GESPEICHERTEN EREIGNISSE GEFUNDEN\0NOMIS HAUPTMENU\0NICHT INBEGRIFFEN\0NOTIZEN\0AUS\0EIN\0EINMAL\0BEDIENER\0"\
"EINSTELL. SPEICHERN\0TEILWEISE RESULTATE\0GENEHMIGT\0PEAK\0PEAK AIR\0BITTE EINEN AUGENBLICK GEDULD\0"\
"UM DAS PROBLEM ZU BEHEBEN BITTE FLASH LOSCHEN\0BITTE GERAT AUSSCHALTEN\0ENTER DRUCKEN\0PLOT - STANDARD\0"\
"GERAT WIRD HERUNTERGEFAHREN\0DIAGRAMM DRUCKEN\0DRUCKER\0DRUCKER EIN\0DRUCK ABGEBROCHEN\0AUSDRUCK\0"\
"WEITER OHNE DATUMS - UND ZEITEINSTELLUNG\0BEARBEITUNG\0RAD\0RADIAL\0RAM ZUSAMMENSTELLUNGS - TABELLE NICHT INITIALISIERT\0"\
"AUFNAHMEZEIT\0REPORT - ABSTAND\0RESULTATE\0SAMPLING - RATE\0ANDERUNGEN SICHERN\0SETUP SPEICHERN\0GESPEICH. EINSTELL\0"\
"SPEICHER WIRD AUF EREIGNISSE UNTERSUCHT\0S\0SEKUNDE\0SEKUNDEN\0SEIS TRIG\0SEIS. STANDORT\0SEISMISCH\0SEISMISCHER TRIGGER\0"\
"AUSWAHLEN\0SELF TRIGGER\0SENSOR A\0SENSOR B\0SENSOR CHECK\0SENSOR VERSTARKUNG\0SENSOR VERSTARKUNG / TYPE NICHT EINGESTELLT\0"\
"SENSOR TYPE\0SERIENNUMMER\0SERIENNUMMER NICHT EINGEGEBEN\0EINSTELLUNGEN WERDEN NICHT GELADEN\0SOFTWARE VERSION\0SCHALL\0"\
"SCHALL TRIG\0SCHALL TRIGGER\0SPANISCH\0STARTDATUM\0STARTZEIT\0STATUS\0ENDE DES MESSDATUMS\0DRUCK ABBRECHEN\0"\
"ENDE DER MESSZEIT\0ERFOLG\0UBERSICHT EREIGNISSE\0UBERSICHT INTERVALL\0UBERSICHT INTERVALL - RESULTATE\0TESTING\0"\
"DIESE FUNKTION IST MOMENTAN NICHT VERFUGBAR\0ZEIT\0TIMER - FREQUENZ\0TIMER - MODUS\0TIMER - MODUS DEAKTIVIERT\0"\
"TIMER - MODUS AKTIV\0TIMER - MODUS - SETUP NICHT ABGESCHLOSSEN\0TIMER - EINSTELLUNGEN UNBRAUCHBAR\0TRAN\0TRANSVERSAL\0"\
"GERAT LAUFT IM TIMER - MODUS\0MABEINHEITEN\0USBM/OSMRE REPORT\0VEKTORSUMME\0VERIFIZIEREN\0VERT\0VERTIKAL\0VOLT\0WARNUNG\0"\
"WAVEFORM - MODUS\0WOCHENTLICH\0GEWICHT PRO ZZSTUFE\0JA\0SIE BEFINDEN SICH JETZT IM HERSTELLER - SETUP\0EREIGNISNR. NULLEN\0"\
"NULLUNG DER SENSOREN\0OK\0ABBRECHEN\0MAX.\0ZEICHEN\0BEREICH\0ENGLISCH\0ITALIENISCH\0FRANZOSICH\0BRITISCH\0"\
"DIN 4150-3-TAB. 1\0US BOM\0X1 (508 MM/S)\0X2 (254 MM/S)\0X4 (127 MM/S)\0X8 (63 MM/S)\0BALKEN\0SELF TRG\0ENDE\0"\
"WIRKLICH ALLE EREIGNISSE LOSCHEN?\0MODEM SETUP\0MODEM INIT\0MODEM DIAL\0MODEM RESET\0SENSITIVITY\0HIGH\0UNLOCK CODE\0"\
"ACC (793L)\0DEUTSCH\0BAUD RATE\0POWER UNIT OFF EARLY?\0NAME ALREADY USED\0DELETE SAVED SETUP?\0"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\0BAR SCALE\0REPORT MILLIBARS\0LCD TIMEOUT\0IMPULSE RESULTS\0LCD IMPULSE TIME\0"\
"PRINT MONITOR LOG\0MONITOR LOG\0EVENTS RECORDED\0EVENT NUMBERS\0CANCEL ALL PRINT JOBS?\0CAL SUMMARY\0MODEM RETRY\0"\
"MODEM RETRY TIME\0FLASH WRAPPING\0FLASH STATS\0AUTO DIAL INFO\0PEAK DISPLACEMENT\0WAVEFORM AUTO CAL\0START OF WAVEFORM\0"\
"CALIBRATED ON\0VIEW MONITOR LOG\0PRINT IN MONITOR\0LOG RESULTS\0MONITOR LOG RESULTS\0PRINT MONITOR LOG ENTRY?\0"\
"PRINTER DISABLED\0PRINTING\0BARGRAPH REPORT\0NORMAL FORMAT\0SHORT FORMAT\0REPORT PEAK ACC\0PEAK ACCELERATION\0"\
"EVENT #s\0START OF LOG\0END OF LOG\0USED\0FREE\0EVENT DATA\0WRAPPED\0FLASH USAGE STATS\0SPACE REMAINING (CURR. SETTINGS)\0"\
"BEFORE OVERWRITE (CURR. SETTINGS)\0WAVEFORMS\0BAR HOURS\0LAST DL EVT\0LAST REC\0LAST CONNECT\0AUTO DIALOUT INFO\0"\
"UNITS OF AIR\0DECIBEL\0MILLIBAR\0115200 BAUD RATE\0BIT ACCURACY\0BIT\0HOURLY\0RECALIBRATE\0ON TEMP. CHANGE\0PRETRIGGER SIZE\0"\
"QUARTER SECOND\0HALF SECOND\0FULL SECOND\0\0"
};

char spanishLanguageTable[] = {
"A WEIGHTING\0A WEIGHTING OPTION\0A/D CALIBRATED OK\0ACTIVE DATE PERIOD\0ACTIVE TIME PERIOD\0AFTER EVERY 24 HR\0"\
"AFTER EVERY 48 HR\0AFTER EVERY 72 HR\0AIR\0AIR CHANNEL SCALE\0ALARM 1\0ALARM 1 AIR LEVEL\0ALARM 1 SEISMIC LVL\0"\
"ALARM 1 TIME\0ALARM 2\0ALARM 2 AIR LEVEL\0ALARM 2 SEISMIC LVL\0ALARM 2 TIME\0ALARM OUTPUT MODE\0ALL RIGHTS RESERVED\0"\
"ARE YOU DONE WITH CAL SETUP?\0ATTEMPTING TO HANDLE PARTIAL EVENT ERROR.\0AUTO CALIBRATION\0AUTO MONITOR\0"\
"AUTO MONITOR AFTER\0BAR GRAPH\0BAR INTERVAL\0BAR DISPLAY RESULT\0BARGRAPH ABORTED\0BARGRAPH MODE\0BARS\0BATT VOLTAGE\0"\
"BATTERY\0BATTERY VOLTAGE\0BOTH\0CAL DATE STORED.\0CAL SETUP\0CALIBRATING\0CALIBRATION\0CALIBRATION CHECK\0CALIBRATION DATE\0"\
"CALIBRATION DATE NOT SET.\0CALIBRATION GRAPH\0CALIBRATION PULSE\0CANCEL TIMER MODE?\0CLIENT\0COMBO\0COMBO MODE\0"\
"COMBO MODE NOT IMPLEMENTED.\0COMMENTS\0COMPANY\0CONFIG & OPTIONS\0CONFIG/OPTIONS MENU\0CONFIRM\0COPIES\0COPY\0"\
"CURRENTLY NOT IMPLEMENTED.\0DAILY (EVERY DAY)\0DAILY (WEEKDAYS)\0DARK\0DARKER\0DATE\0DATE/TIME\0DAY-MONTH-YEAR\0DEFAULT\0"\
"DEFAULT (BAR)\0DEFAULT (COMBO)\0DEFAULT (SELF TRG)\0DISABLED\0DISABLING TIMER MODE.\0DISCOVERED A BROKEN EVENT LINK.\0"\
"DISTANCE TO SOURCE\0DO NOT TURN THE UNIT OFF UNTIL THE OPERATION IS COMPLETE.\0DO YOU WANT TO ENTER MANUAL TRIGGER MODE?\0"\
"DO YOU WANT TO LEAVE CAL SETUP MODE?\0DO YOU WANT TO LEAVE MONITOR MODE?\0DO YOU WANT TO SAVE THE CAL DATE?\0EDIT\0EMPTY\0"\
"ENABLED\0ERASE COMPLETE.\0ERASE EVENTS\0ERASE MEMORY\0ERASE OPERATION IN PROGRESS.\0ERASE SETTINGS\0ERROR\0ESC HIT\0EVENT\0"\
"EVENT NUMBER ZEROING COMPLETE.\0EVENT SUMMARY\0FACTORY SETUP COMPLETE.\0FACTORY SETUP DATA COULD NOT BE FOUND.\0FAILED\0"\
"FEET\0FREQUENCY\0FREQUENCY PLOT\0FRQ\0FULL\0GRAPHICAL RECORD\0HELP INFO MENU\0HELP INFORMATION\0HELP MENU\0HOUR\0HOURS\0HR\0"\
"HZ\0IMPERIAL\0INCLUDED\0INSTRUMENT\0JOB NUMBER\0JOB PEAK RESULTS\0JOB VECTOR SUM RESULTS\0LANGUAGE\0LAST SETUP\0"\
"LCD CONTRAST\0LIGHT\0LIGHTER\0LINEAR\0LIST OF SUMMARIES\0LOCATION\0LOW\0METRIC\0MIC A\0MIC B\0MINUTE\0MINUTES\0MMPS/DIV\0"\
"MONITOR\0MONITOR BARGRAPH\0MONITORING\0MONTHLY\0NAME\0NEXT EVENT STORAGE LOCATION NOT EMPTY.\0NO\0NO AUTO CAL\0"\
"NO AUTO MONITOR\0NO STORED EVENTS FOUND.\0NOMIS 8100 MAIN\0NOT INCLUDED\0NOTES\0OFF\0ON\0ONE TIME\0OPERATOR\0"\
"OVERWRITE SETTINGS\0PARTIAL RESULTS\0PASSED\0PEAK\0PEAK AIR\0PLEASE BE PATIENT.\0"\
"PLEASE CONSIDER ERASING THE FLASH TO FIX PROBLEM.\0PLEASE POWER OFF UNIT.\0PLEASE PRESS ENTER.\0PLOT STANDARD\0"\
"POWERING UNIT OFF NOW.\0PRINT GRAPH\0PRINTER\0PRINTER ON\0PRINTING STOPPED\0PRINTOUT\0"\
"PROCEED WITHOUT SETTING DATE AND TIME?\0PROCESSING\0RAD\0RADIAL\0RAM SUMMARY TABLE IS UNINITIALIZED.\0"\
"RECORD TIME\0REPORT DISPLACEMENT\0RESULTS\0SAMPLE RATE\0SAVE CHANGES\0SAVE SETUP\0SAVED SETTINGS\0"\
"SCANNING STORAGE FOR VALID EVENTS.\0SEC\0SECOND\0SECONDS\0SEIS TRIG\0SEIS. LOCATION\0SEISMIC\0SEISMIC TRIGGER\0SELECT\0"\
"SELF TRIGGER\0SENSOR A\0SENSOR B\0SENSOR CHECK\0SENSOR GAIN/TYPE\0SENSOR GAIN/TYPE NOT SET.\0SENSOR TYPE\0SERIAL NUMBER\0"\
"SERIAL NUMBER NOT SET.\0SETTINGS WILL NOT BE LOADED.\0SOFTWARE VER\0SOUND\0AIR TRIG\0AIR TRIGGER\0SPANISH\0START DATE\0"\
"START TIME\0STATUS\0STOP DATE\0STOP PRINTING\0STOP TIME\0SUCCESS\0SUMMARIES EVENTS\0SUMMARY INTERVAL\0"\
"SUMMARY INTERVAL RESULTS\0TESTING\0THIS FEATURE IS NOT CURRENTLY AVAILABLE.\0TIME\0TIMER FREQUENCY\0TIMER MODE\0"\
"TIMER MODE DISABLED.\0TIMER MODE NOW ACTIVE.\0TIMER MODE SETUP NOT COMPLETED.\0TIMER SETTINGS INVALID.\0TRAN\0TRANSVERSE\0"\
"UNIT IS IN TIMER MODE.\0UNITS OF MEASURE\0USBM/OSMRE REPORT\0VECTOR SUM\0VERIFY\0VERT\0VERTICAL\0VOLTS\0WARNING\0"\
"WAVEFORM MODE\0WEEKLY\0WEIGHT PER DELAY\0YES\0YOU HAVE ENTERED THE FACTORY SETUP.\0ZERO EVENT NUMBER\0(ZEROING SENSORS)\0"\
"OK\0CANCEL\0MAX\0CHARS\0RANGE\0ENGLISH\0ITALIAN\0FRENCH\0BRITISH\0DIN 4150\0US BOM\0X1 (20 IPS)\0X2 (10 IPS)\0X4 (5 IPS)\0"\
"X8 (2.5 IPS)\0BAR\0SELF TRG\0END\0REALLY ERASE ALL EVENTS?\0MODEM SETUP\0MODEM INIT\0MODEM DIAL\0MODEM RESET\0SENSITIVITY\0"\
"HIGH\0UNLOCK CODE\0ACC (793L)\0GERMAN\0BAUD RATE\0POWER UNIT OFF EARLY?\0NAME ALREADY USED\0DELETE SAVED SETUP?\0"\
"NAME MUST HAVE AT LEAST ONE CHARACTER\0BAR SCALE\0REPORT MILLIBARS\0LCD TIMEOUT\0IMPULSE RESULTS\0LCD IMPULSE TIME\0"\
"PRINT MONITOR LOG\0MONITOR LOG\0EVENTS RECORDED\0EVENT NUMBERS\0CANCEL ALL PRINT JOBS?\0CAL SUMMARY\0MODEM RETRY\0"\
"MODEM RETRY TIME\0FLASH WRAPPING\0FLASH STATS\0AUTO DIAL INFO\0PEAK DISPLACEMENT\0WAVEFORM AUTO CAL\0START OF WAVEFORM\0"\
"CALIBRATED ON\0VIEW MONITOR LOG\0PRINT IN MONITOR\0LOG RESULTS\0MONITOR LOG RESULTS\0PRINT MONITOR LOG ENTRY?\0"\
"PRINTER DISABLED\0PRINTING\0BARGRAPH REPORT\0NORMAL FORMAT\0SHORT FORMAT\0REPORT PEAK ACC\0PEAK ACCELERATION\0"\
"EVENT #s\0START OF LOG\0END OF LOG\0USED\0FREE\0EVENT DATA\0WRAPPED\0FLASH USAGE STATS\0SPACE REMAINING (CURR. SETTINGS)\0"\
"BEFORE OVERWRITE (CURR. SETTINGS)\0WAVEFORMS\0BAR HOURS\0LAST DL EVT\0LAST REC\0LAST CONNECT\0AUTO DIALOUT INFO\0"\
"UNITS OF AIR\0DECIBEL\0MILLIBAR\0115200 BAUD RATE\0BIT ACCURACY\0BIT\0HOURLY\0RECALIBRATE\0ON TEMP. CHANGE\0PRETRIGGER SIZE\0"\
"QUARTER SECOND\0HALF SECOND\0FULL SECOND\0\0"
};
