/*
 * Globals.h
 *
 * Created: 8/18/2012 8:42:34 PM
 *  Author: Jeremy
 */ 

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Typedefs.h"
#include "Analog.h"
#include "Summary.h"
#include "Common.h"
#include "RealTimeClock.h"
#include "Record.h"
#include "FAT32_FileLib.h"
#include "RemoteCommon.h"
#include "Menu.h"
#include "InitDataBuffers.h"
#include "Display.h"
#include "EventProcessing.h"
#include "SysEvents.h"
#include "Keypad.h"
#include "ProcessBargraph.h"
#include "Uart.h"
#include "TextTypes.h"

// Global Externs ---------------------------------------------------------------------

extern ANALOG_CONTROL_STRUCT g_analogControl;
extern OFFSET_DATA_STRUCT g_channelOffset;
extern INPUT_MSG_STRUCT g_input_buffer[INPUT_BUFFER_SIZE];
extern char g_appVersion[15];
extern char g_appDate[15];
extern char g_appTime[15];
extern MONTH_TABLE_STRUCT g_monthTable[];
extern uint8 g_mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern uint8 g_contrast_value;
extern uint8 g_LcdPowerState;
extern uint16 g_nextEventNumberToUse;
extern uint32 __monitorLogTblKey;
extern uint16 __monitorLogTblIndex;
extern uint16 __monitorLogUniqueEntryId;
extern MONITOR_LOG_ENTRY_STRUCT __monitorLogTbl[TOTAL_MONITOR_LOG_ENTRIES];
extern uint32 __autoDialoutTblKey;
extern AUTODIALOUT_STRUCT __autoDialoutTbl;
extern uint32 __ramFlashSummaryTblKey;
extern SUMMARY_DATA __ramFlashSummaryTbl[TOTAL_RAM_SUMMARIES];
extern uint16 g_quarterSecBuff[PRE_TRIG_BUFF_SIZE_IN_WORDS];
extern uint16* g_startOfQuarterSecBuff;
extern uint16* g_tailOfQuarterSecBuff;
extern uint16* g_endOfQuarterSecBuff;
extern uint16 g_maxEventBuffers;
extern uint16 g_nextEventNumberToUse;
extern FL_FILE* g_currentEventFileHandle;
extern FL_FILE* g_comboDualCurrentEventFileHandle;
extern uint16 g_freeEventBuffers;
extern uint16 g_calTestExpected;
extern uint16 g_adChannelConfig;
extern uint32 g_samplesInBody;
extern uint32 g_samplesInPretrig;
extern uint32 g_samplesInCal;
extern uint32 g_samplesInEvent;
extern uint32 g_wordSizeInPretrig;
extern uint32 g_wordSizeInBody;
extern uint32 g_wordSizeInCal;
extern uint32 g_wordSizeInEvent;
extern uint16* g_startOfEventBufferPtr;
extern uint16* g_eventBufferPretrigPtr;
extern uint16* g_eventBufferBodyPtr;
extern uint16* g_eventBufferCalPtr;
extern uint16* g_delayedOneEventBufferCalPtr;
extern uint16* g_delayedTwoEventBufferCalPtr;
extern uint16* g_currentEventSamplePtr;
extern uint16* g_currentEventStartPtr;
extern uint16 g_eventBufferReadIndex;
extern uint16 g_airTriggerCount;
extern uint16 g_alarm1AirTriggerCount;
extern uint16 g_alarm2AirTriggerCount;
extern uint16 g_bitShiftForAccuracy;
extern SUMMARY_DATA g_summaryTable[MAX_RAM_SUMMARYS];
extern SUMMARY_DATA* g_lastCompletedRamSummaryIndex;
extern uint32 g_isTriggered;
extern uint32 g_processingCal;
extern uint16 g_eventsNotCompressed;
extern uint16* g_bargraphDataStartPtr;
extern uint16* g_bargraphDataWritePtr;
extern uint16* g_bargraphDataReadPtr;
extern uint16* g_bargraphDataEndPtr;
extern uint8 g_powerNoiseFlag;
extern uint8 g_doneTakingEvents;
extern uint8 g_busyProcessingEvent;
extern uint8 g_sampleProcessing;
extern uint8 g_modemConnected;
extern uint8 g_lcdBacklightFlag;
extern uint8 g_lcdPowerFlag;
extern uint8 g_kpadProcessingFlag;
extern uint8 g_kpadCheckForKeyFlag;
extern uint8 g_factorySetupSequence;
extern uint8 g_kpadLastKeyPressed;
extern uint32 g_keypadTimerTicks;
extern uint32 g_kpadKeyRepeatCount;
extern uint32 g_kpadLookForKeyTickCount;
extern uint32 g_keypadNumberSpeed;
extern uint8 g_keypadTable[8][8];
//extern char g_keypadTableText[8][11];
extern unsigned char g_smc_tab_cs_size[4];
// Sensor information and constants.
extern SENSOR_PARAMETERS_STRUCT g_SensorInfoStruct;
extern SENSOR_PARAMETERS_STRUCT* g_sensorInfoPtr;
// Contains the event record in ram.
extern EVT_RECORD g_pendingEventRecord;
extern EVT_RECORD g_pendingBargraphRecord;
// Factory Setup record.
extern FACTORY_SETUP_STRUCT g_factorySetupRecord;
// Structure to contain system paramters and system settings.
extern REC_EVENT_MN_STRUCT g_triggerRecord;				// Contains trigger specific information.
// Menu specific structures
extern uint16 g_activeMenu;							// For active menu number/enum.
extern MN_EVENT_STRUCT g_menuEventFlags;				// Menu event flags, for main loop processing.
extern MN_TIMER_STRUCT g_timerEventFlags;					// Menu timer strucutre.
// System Event Flags, for main loopo processing.
extern SYS_EVENT_STRUCT g_systemEventFlags;
extern MODEM_SETUP_STRUCT g_modemSetupRecord;			// Record for user input data.
extern MODEM_STATUS_STRUCT g_modemStatus;			// Record for modem data processing.
// Used as a circular buffer to continually caputer incomming data from the craft port.
extern CMD_BUFFER_STRUCT g_isrMessageBufferStruct;
extern CMD_BUFFER_STRUCT* g_isrMessageBufferPtr;
extern void (*menufunc_ptrs[TOTAL_NUMBER_OF_MENUS]) (INPUT_MSG_STRUCT);
extern MN_MEM_DATA_STRUCT g_menuPtr[DEFAULT_MN_SIZE];
extern USER_MENU_TAGS_STRUCT g_menuTags[TOTAL_TAGS];
extern uint8 g_monitorOperationMode;
extern uint8 g_waitForUser;
extern uint8 g_promtForLeavingMonitorMode;
extern uint8 g_promtForCancelingPrintJobs;
extern uint8 g_monitorModeActiveChoice;
extern uint8 g_monitorEscapeCheck;
extern uint8 g_displayBargraphResultsMode;
extern uint8 g_displayAlternateResultState;
extern SUMMARY_DATA* g_bargraphSummaryPtr;

// For bargraph processing.
extern uint8 g_oneMinuteCount;
extern uint32 g_oneSecondCnt;
extern uint32 g_barIntervalCnt;
extern uint32 g_totalBarIntervalCnt;
extern uint32 g_summaryIntervalCnt;
extern uint16 g_summaryCount;

extern BARGRAPH_FREQ_CALC_BUFFER g_bargraphFreqCalcBuffer;

// A queue of buffers containing summary Interval data, so the
// summary interval data can be printed outside of the ISR context.
extern CALCULATED_DATA_STRUCT g_bargraphSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
extern CALCULATED_DATA_STRUCT* g_bargraphSumIntervalWritePtr;
extern CALCULATED_DATA_STRUCT* g_bargraphSumIntervalReadPtr;
extern CALCULATED_DATA_STRUCT* g_bargraphSumIntervalEndPtr;

// A queue of buffers containing bar Interval data, so the
// bar interval data can be printed outside of the ISR context.
extern BARGRAPH_BAR_INTERVAL_DATA g_bargraphBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
extern BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalWritePtr;
extern BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalReadPtr;
extern BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalEndPtr;
extern uint16 g_bitAccuracyMidpoint;
extern uint16 g_aImpulsePeak;
extern uint16 g_rImpulsePeak;
extern uint16 g_vImpulsePeak;
extern uint16 g_tImpulsePeak;
extern uint32 g_vsImpulsePeak;
extern uint16 g_impulseMenuCount;
extern uint16 g_aJobPeak;
extern uint16 g_aJobFreq;
extern uint16 g_rJobPeak;
extern uint16 g_rJobFreq;
extern uint16 g_vJobPeak;
extern uint16 g_vJobFreq;
extern uint16 g_tJobPeak;
extern uint16 g_tJobFreq;
extern uint32 g_vsJobPeak;
// This ptr points to the current/in use (ram) summary table entry. It is
// used to access the linkPtr which is really the flash event record Ptr.
// And the record ptr is the location in ram. i.e.
extern SUMMARY_DATA* g_comboSummaryPtr;
extern BARGRAPH_FREQ_CALC_BUFFER g_comboFreqCalcBuffer;

// A queue of buffers containing summary Interval data, so the
// summary interval data can be printed outside of the ISR context.
extern CALCULATED_DATA_STRUCT g_comboSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
extern CALCULATED_DATA_STRUCT* g_comboSumIntervalWritePtr;
extern CALCULATED_DATA_STRUCT* g_comboSumIntervalReadPtr;
extern CALCULATED_DATA_STRUCT* g_comboSumIntervalEndPtr;

// A queue of buffers containing bar Interval data, so the
// bar interval data can be printed outside of the ISR context.
extern BARGRAPH_BAR_INTERVAL_DATA g_comboBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
extern BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalWritePtr;
extern BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalReadPtr;
extern BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalEndPtr;
extern uint16 g_manualCalFlag;
extern uint16 g_manualCalSampleCount;
extern uint8 g_bargraphForcedCal;
extern uint8 g_skipAutoCalInWaveformAfterMidnightCal;
extern DATE_TIME_STRUCT g_currentTime;
extern SOFT_TIMER_STRUCT g_rtcTimerBank[NUM_OF_SOFT_TIMERS];
extern uint32 g_rtcSoftTimerTickCount;
extern uint32 g_rtcCurrentTickCount;
extern uint32 g_UpdateCounter;
extern uint8 g_autoCalDaysToWait;
extern REC_HELP_MN_STRUCT g_helpRecord;
extern uint8 g_printMillibars;
extern uint8 g_autoDialoutState;
extern uint8 g_modemDataTransfered;
extern uint16 g_CRLF;
// Holds a pool of buffers for processing input from the craft port
extern CMD_BUFFER_STRUCT g_msgPool[CMD_MSG_POOL_SIZE];
extern DEMx_XFER_STRUCT* g_demXferStructPtr;
extern DSMx_XFER_STRUCT* g_dsmXferStructPtr;
extern DQMx_XFER_STRUCT* g_dqmXferStructPtr;
extern COMMAND_MESSAGE_HEADER* g_inCmdHeaderPtr;
extern COMMAND_MESSAGE_HEADER* g_outCmdHeaderPtr;
extern uint32 	g_transferCount;
extern uint32 	g_transmitCRC;
extern uint8	g_binaryXferFlag;
extern uint8 g_modemResetStage;
extern uint8 g_updateResultsEventRecord;
extern SUMMARY_DATA *g_resultsRamSummaryPtr;
extern uint16 g_resultsRamSummaryIndex;
extern uint32 g_summaryEventNumber;
extern uint8 g_summaryListMenuActive;
extern uint8 g_summaryListArrowChar;
extern uint8 g_disableDebugPrinting;
extern USER_MENU_CACHE_STRUCT g_userMenuCache[36];
extern USER_MENU_CACHE_STRUCT* g_userMenuCachePtr;
extern USER_MENU_CACHE_DATA g_userMenuCacheData;
extern uint8 g_enterMonitorModeAfterMidnightCal;
extern void (*g_userMenuHandler)(uint8, void*);
extern uint16 g_eventBufferWriteIndex;
extern uint8 g_spareBuffer[];
extern uint8 g_tcSampleTimerActive;
extern uint8 g_tcTypematicTimerActive;
extern char* g_languageLinkTable[TOTAL_TEXT_STRINGS];
extern char g_languageTable[8192];
extern uint16 g_testTrigger;
extern uint8 g_fileProcessActiveUsbLockout;
extern uint8 g_spi1AccessLock;
extern EVT_RECORD g_resultsEventCache[50];
extern uint16 g_resultsCacheIndex;
extern uint32 g_cyclicEventDelay;
extern uint16 g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS];

// Test
extern uint32 g_execCycles;
extern uint8 g_channelSyncError;
extern uint32 g_sampleCount;
extern uint32 g_sampleCountHold;

#endif /* GLOBALS_H_ */