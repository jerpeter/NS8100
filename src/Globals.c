///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

ANALOG_CONTROL_STRUCT g_analogControl;
OFFSET_DATA_STRUCT g_channelOffset;
INPUT_MSG_STRUCT g_input_buffer[INPUT_BUFFER_SIZE];
//char g_appVersion[16];
//char g_appDate[16];
//char g_appTime[16];
MONTH_TABLE_STRUCT g_monthTable[] = { {0, "\0\0\0\0", 0}, {JAN, "JAN\0", 31}, {FEB, "FEB\0", 28}, {MAR, "MAR\0", 31}, {APR, "APR\0", 30}, {MAY, "MAY\0", 31},
	{JUN, "JUN\0", 30}, {JUL, "JUL\0", 31}, {AUG, "AUG\0", 31}, {SEP, "SEP\0", 30}, {OCT, "OCT\0", 31}, {NOV, "NOV\0", 30}, {DEC, "DEC\0", 31} };
uint8 g_mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
uint8 g_contrast_value;
uint8 g_LcdPowerState = ENABLED;
uint8 g_fileProcessActiveUsbLockout = OFF;
uint8 g_powerSavingsForSleepEnabled = NO;
uint16 g_nextEventNumberToUse = 1;
uint32 __monitorLogTblKey;
uint16 __monitorLogTblIndex;
uint16 __monitorLogUniqueEntryId;
MONITOR_LOG_ENTRY_STRUCT __monitorLogTbl[TOTAL_MONITOR_LOG_ENTRIES];
uint32 __autoDialoutTblKey;
AUTODIALOUT_STRUCT __autoDialoutTbl;
uint32 __ramFlashSummaryTblKey;
SUMMARY_DATA __ramFlashSummaryTbl[TOTAL_RAM_SUMMARIES];
//uint16 g_pretriggerBuff[PRE_TRIG_BUFF_SIZE_IN_WORDS];
uint16 g_pretriggerBuff[(SAMPLE_RATE_16K * 4)];
uint16* g_startOfPretriggerBuff;
uint16* g_tailOfPretriggerBuff;
uint16* g_endOfPretriggerBuff;
uint16 g_maxEventBuffers;
uint16 g_nextEventNumberToUse;
FL_FILE* g_currentEventFileHandle;
FL_FILE* g_comboDualCurrentEventFileHandle;
uint16 g_freeEventBuffers;
uint16 g_calTestExpected;
uint16 g_adChannelConfig;
uint32 g_samplesInBody;
uint32 g_samplesInPretrig;
uint32 g_samplesInCal;
uint32 g_samplesInEvent;
uint32 g_wordSizeInPretrig;
uint32 g_wordSizeInBody;
uint32 g_wordSizeInCal;
uint32 g_wordSizeInEvent;
uint16* g_startOfEventBufferPtr;
uint16* g_eventBufferPretrigPtr;
uint16* g_eventBufferBodyPtr;
uint16* g_eventBufferCalPtr;
uint16* g_delayedOneEventBufferCalPtr;
uint16* g_delayedTwoEventBufferCalPtr;
uint16* g_currentEventSamplePtr;
uint16* g_currentEventStartPtr;
uint16 g_eventBufferReadIndex;
uint16 g_bitShiftForAccuracy;
SUMMARY_DATA g_summaryTable[MAX_RAM_SUMMARYS];
SUMMARY_DATA* g_lastCompletedRamSummaryIndex;
uint32 g_isTriggered = 0;
uint32 g_processingCal = 0;
uint16 g_eventsNotCompressed = 0; 
uint16* g_bargraphDataStartPtr;
uint16* g_bargraphDataWritePtr;
uint16* g_bargraphDataReadPtr;
uint16* g_bargraphDataEndPtr;
uint8 g_powerNoiseFlag = PRINTER_OFF;
uint8 g_doneTakingEvents = NO;
uint8 g_busyProcessingEvent = NO;
uint8 g_sampleProcessing = IDLE_STATE;
uint8 g_modemConnected = NO;
uint8 g_lcdBacklightFlag  = ENABLED;
uint8 g_lcdPowerFlag = ENABLED;
uint8 g_kpadProcessingFlag = DEACTIVATED;
uint8 g_kpadCheckForKeyFlag = DEACTIVATED;
uint8 g_factorySetupSequence = SEQ_NOT_STARTED;
uint8 g_kpadLastKeyPressed = KEY_NONE;
uint8 g_kpadInterruptWhileProcessing = 0;
volatile uint32 g_keypadTimerTicks = 0;
uint32 g_kpadKeyRepeatCount = 0;
uint32 g_kpadLookForKeyTickCount = 0;
uint32 g_keypadNumberSpeed = 1;
uint8 g_keypadTable[8][8] = { {KEY_BACKLIGHT, KEY_HELP, KEY_ESCAPE, KEY_UPARROW, KEY_DOWNARROW, KEY_MINUS, KEY_PLUS, KEY_ENTER} };
uint8 g_smc_tab_cs_size[4];
SENSOR_PARAMETERS_STRUCT g_SensorInfoStruct;
SENSOR_PARAMETERS_STRUCT* g_sensorInfoPtr = &g_SensorInfoStruct;
EVT_RECORD g_pendingEventRecord;
EVT_RECORD g_pendingBargraphRecord;
EVT_RECORD g_resultsEventCache[50];
uint16 g_resultsCacheIndex = 0;
FACTORY_SETUP_STRUCT g_factorySetupRecord;
REC_EVENT_MN_STRUCT g_triggerRecord;
uint16 g_activeMenu;
MN_EVENT_STRUCT g_menuEventFlags;
MN_TIMER_STRUCT g_timerEventFlags;
SYS_EVENT_STRUCT g_systemEventFlags;
MODEM_SETUP_STRUCT g_modemSetupRecord;
MODEM_STATUS_STRUCT g_modemStatus;
CMD_BUFFER_STRUCT g_isrMessageBufferStruct;
CMD_BUFFER_STRUCT* g_isrMessageBufferPtr = &g_isrMessageBufferStruct;
void (*menufunc_ptrs[TOTAL_NUMBER_OF_MENUS]) (INPUT_MSG_STRUCT) = { MainMenu, LoadRecordMenu, SummaryMenu, MonitorMenu, ResultsMenu, OverwriteMenu,
	BatteryMn, DateTimeMn, LcdContrastMn, TimerModeTimeMenu, TimerModeDateMenu, CalSetupMn, UserMenu, MonitorLogMn };
MN_MEM_DATA_STRUCT g_menuPtr[DEFAULT_MN_SIZE];
USER_MENU_TAGS_STRUCT g_menuTags[TOTAL_TAGS] = {
	{"",	NO_TAG},
	{"1. ",	ITEM_1},
	{"2. ",	ITEM_2},
	{"3. ",	ITEM_3},
	{"4. ",	ITEM_4},
	{"5. ",	ITEM_5},
	{"6. ",	ITEM_6},
	{"7. ",	ITEM_7},
	{"8. ",	ITEM_8},
	{"9. ",	ITEM_9},
	{"-=",	MAIN_PRE_TAG},
	{"=-",	MAIN_POST_TAG},
	{"-",	TITLE_PRE_TAG},
	{"-",	TITLE_POST_TAG},
	{"",	LOW_SENSITIVITY_MAX_TAG},
	{"",	HIGH_SENSITIVITY_MAX_TAG},
	{"",	BAR_SCALE_FULL_TAG},
	{"",	BAR_SCALE_HALF_TAG},
	{"",	BAR_SCALE_QUARTER_TAG},
	{"",	BAR_SCALE_EIGHTH_TAG}
};
uint8 g_monitorOperationMode;
uint8 g_waitForUser = FALSE;
uint8 g_promtForLeavingMonitorMode = FALSE;
uint8 g_promtForCancelingPrintJobs = FALSE;
uint8 g_monitorModeActiveChoice = MB_FIRST_CHOICE;
uint8 g_monitorEscapeCheck = YES;
uint8 g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
uint8 g_displayAlternateResultState = DEFAULT_RESULTS;
SUMMARY_DATA* g_bargraphSummaryPtr = 0;
uint16 g_oneMinuteCount = 0;
uint32 g_oneSecondCnt = 0;
uint32 g_barIntervalCnt = 0;
uint32 g_totalBarIntervalCnt = 0;
uint32 g_summaryIntervalCnt = 0;
uint16 g_summaryCount = 1;
BARGRAPH_FREQ_CALC_BUFFER g_bargraphFreqCalcBuffer;
CALCULATED_DATA_STRUCT g_bargraphSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalWritePtr = &(g_bargraphSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalReadPtr = &(g_bargraphSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalEndPtr = &(g_bargraphSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS-1]);
BARGRAPH_BAR_INTERVAL_DATA g_bargraphBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalEndPtr = &(g_bargraphBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS - 1]);
uint16 g_bitAccuracyMidpoint = 0x8000;
uint16 g_aImpulsePeak;
uint16 g_rImpulsePeak;
uint16 g_vImpulsePeak;
uint16 g_tImpulsePeak;
uint32 g_vsImpulsePeak;
uint16 g_impulseMenuCount;
uint16 g_aJobPeak;
uint16 g_aJobFreq;
uint16 g_rJobPeak;
uint16 g_rJobFreq;
uint16 g_vJobPeak;
uint16 g_vJobFreq;
uint16 g_tJobPeak;
uint16 g_tJobFreq;
uint32 g_vsJobPeak;
SUMMARY_DATA* g_comboSummaryPtr = 0;
BARGRAPH_FREQ_CALC_BUFFER g_comboFreqCalcBuffer;
CALCULATED_DATA_STRUCT g_comboSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
CALCULATED_DATA_STRUCT* g_comboSumIntervalWritePtr = &(g_comboSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_comboSumIntervalReadPtr = &(g_comboSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_comboSumIntervalEndPtr = &(g_comboSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS-1]);
BARGRAPH_BAR_INTERVAL_DATA g_comboBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalWritePtr = &(g_comboBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalReadPtr = &(g_comboBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalEndPtr = &(g_comboBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS - 1]);
uint16 g_manualCalSampleCount = 0;
uint8 g_manualCalFlag = FALSE;
uint8 g_forcedCalibration = NO;
uint8 g_skipAutoCalInWaveformAfterMidnightCal = NO;
uint8 g_autoRetries;
DATE_TIME_STRUCT  g_currentTime;
SOFT_TIMER_STRUCT g_rtcTimerBank[NUM_OF_SOFT_TIMERS];
uint32 g_rtcSoftTimerTickCount = 0;
volatile uint32 g_rtcCurrentTickCount = 0;
uint32 g_updateCounter = 0;
UNIT_CONFIG_STRUCT g_unitConfig;
uint8 g_disableDebugPrinting;
uint8 g_autoCalDaysToWait = 0;
uint8 g_autoDialoutState = AUTO_DIAL_IDLE;
uint8 g_modemDataTransfered = NO;
uint16 g_CRLF = 0x0D0A;
CMD_BUFFER_STRUCT g_msgPool[CMD_MSG_POOL_SIZE];
DEMx_XFER_STRUCT g_demXferStruct;
DSMx_XFER_STRUCT g_dsmXferStruct;
DQMx_XFER_STRUCT g_dqmXferStruct;
COMMAND_MESSAGE_HEADER g_modemInputHeaderStruct;
COMMAND_MESSAGE_HEADER g_modemOutputHeaderStruct;
DEMx_XFER_STRUCT* g_demXferStructPtr = &g_demXferStruct;
DSMx_XFER_STRUCT* g_dsmXferStructPtr = &g_dsmXferStruct;
DQMx_XFER_STRUCT* g_dqmXferStructPtr = &g_dqmXferStruct;
COMMAND_MESSAGE_HEADER* g_inCmdHeaderPtr = &g_modemInputHeaderStruct;
COMMAND_MESSAGE_HEADER* g_outCmdHeaderPtr = &g_modemOutputHeaderStruct;
uint32 g_transferCount;
uint32 g_transmitCRC;
uint8 g_binaryXferFlag = CONVERT_DATA_TO_ASCII;
uint8 g_modemResetStage = 0;
uint8 g_updateResultsEventRecord = NO;
uint8 g_enterMonitorModeAfterMidnightCal = NO;
SUMMARY_DATA *g_resultsRamSummaryPtr;
uint16 g_resultsRamSummaryIndex;
uint32 g_summaryEventNumber;
uint8 g_summaryListMenuActive = NO;
uint8 g_summaryListArrowChar = BOTH_ARROWS_CHAR;
volatile uint8 g_spi1AccessLock = NO;
volatile uint8 g_externalTrigger = NO;
uint32 g_cyclicEventDelay = 0;
uint32 g_updateOffsetCount = 0;
USER_MENU_CACHE_STRUCT g_userMenuCache[MAX_MENU_ENTRIES];
USER_MENU_CACHE_STRUCT* g_userMenuCachePtr = &g_userMenuCache[0];
USER_MENU_CACHE_DATA g_userMenuCacheData;
void (*g_userMenuHandler)(uint8, void*);
uint16 g_eventBufferWriteIndex;
uint8 g_timerModeLastRun = NO;
uint8 g_tcSampleTimerActive = NO;
uint8 g_tcTypematicTimerActive = NO;
uint8 g_lowBatteryState = NO;
uint32 g_sleepModeState = AVR32_PM_SMODE_STOP;
char* g_languageLinkTable[TOTAL_TEXT_STRINGS];
uint32 g_tempTriggerLevelForMenuAdjsutment;
volatile uint16 g_storedTempReading;
volatile uint16 g_currentTempReading;
volatile uint16 g_previousTempReading;
uint16 g_debugBufferCount = 0;
char g_languageTable[LANGUAGE_TABLE_MAX_SIZE];
uint8 g_spareBuffer[SPARE_BUFFER_SIZE];
uint8 g_debugBuffer[GLOBAL_DEBUG_BUFFER_SIZE];
uint16 g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS];
uint32 g_execCycles = 0;
FLASH_USAGE_STRUCT g_flashUsageStats;
volatile uint32 g_sampleCount = 0;
uint32 g_sampleCountHold = 0;
uint8 g_channelSyncError = NO;
uint8 g_powerOffActivated = NO;

// End of the list
