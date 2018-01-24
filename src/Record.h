///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _REC_H_
#define _REC_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Common.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
enum {
	REC_TRIGGER_USER_MENU_TYPE = 1,
	REC_PRINTER_USER_MENU_TYPE,
	REC_ALARM_USER_MENU_TYPE,
	REC_UNUSED_TYPE,
	REC_DATE_TYPE,
	REC_DATE_TIME_TYPE,
	REC_DATE_TIME_AM_PM_TYPE,
	REC_DATE_TIME_MONITOR,
	REC_RESULTS_DATA_TYPE,
	REC_UNIT_CONFIG_TYPE,
	REC_FACTORY_SETUP_TYPE,
	REC_FACTORY_SETUP_CLEAR_TYPE,
	REC_DATE_TIME_DISPLAY,
	REC_MODEM_SETUP_TYPE,
	REC_UNIQUE_EVENT_ID_TYPE,
	REC_UNIQUE_MONITOR_LOG_ID_TYPE,
	REC_UNIQUE_MONITOR_LOG_ID_CLEAR_TYPE
};

#define DEFAULT_RECORD 			0
#define MAX_NUM_OF_SAVED_SETUPS 14

#define VALID_EVENT_NUMBER_CACHE_KEY	0xA55AF00F
#define VALID_MONITOR_LOG_TABLE_KEY		0x0FF05A5A
#define VALID_AUTODIALOUT_TABLE_KEY		0x12ABCDEF

#define EEPROM_SPI_NPCS		0

#define EEPROM_WRITE_ENABLE		0x06
#define EEPROM_WRITE_DISABLE	0x04
#define EEPROM_READ_DATA		0x03
#define EEPROM_WRITE_DATA		0x02
#define EEPROM_READ_STATUS		0x05
#define EEPROM_WRITE_STATUS		0x01

#define EEPROM_AT25640_TOTAL_BYTES	8192

// Sensor information
#define NUMBER_OF_CHANNELS_DEFAULT 	4
#define BYTES_PER_CHANNEL			2
#define SENSOR_ACCURACY_100X_SHIFT 	100

#define ADC_RESOLUTION				0x8000	// +/- 0x800 (2048)

#define SENSOR_20_IN			4096
#define SENSOR_10_IN			2048
#define SENSOR_5_IN				1024
#define SENSOR_2_5_IN			512
#define SENSOR_ACCELEROMETER	51200

#define SENSOR_MIC_148			0xA1
#define SENSOR_MIC_160			0xA4 // This mic is 4 times the scale of the 148

#define INCH_PER_CM			 		2.54
#define LBS_PER_KG			 		2.2
#define FT_PER_METER				3.280833

#define ONE_GRAVITY_IN_INCHES		386.4
#define ONE_GRAVITY_IN_MM			9814.6

#define DEFAULT_SEISMIC_TRIGGER_LEVEL_IN_INCHES_WITH_ADJUSTMENT	10 // 0.05 * 200 = 10, 0.05 inches at low sensitivity

#define REC_MN_STRING_SIZE			20
#define TRIGGER_EVENT_STRING_SIZE	101
#define REC_MN_DATAVAL_SIZE			7

#define CALIBRATION_NUMBER_OF_SAMPLES 100

#define TOTAL_MONITOR_LOG_ENTRIES	((0x1000 - 8) / sizeof(MONITOR_LOG_ENTRY_STRUCT))

enum {
	EVENT_REFERENCE_VALID = 1,
	EVENT_FILE_FOUND,
	INVALID_EVENT_FILE_FOUND
};

enum TRIGGER_DEFAULT_RECORD_INDEX
{
	// Options MUST stay in this order
	NOT_USED_INDEX = 0,
	CLIENT_INDEX,
	SEISMIC_LOCATION_INDEX,
	COMMENTS_INDEX,
	DISTANCE_TO_SOURCE_INDEX,
	WEIGHT_PER_DELAY_INDEX,
	OPERATOR_INDEX,
	SEISMIC_TRIGGER_INDEX,
	AIR_TRIGGER_INDEX,
	RECORD_TRIGGER_INDEX,
	SAMPLE_RATE_INDEX,
	LAST_INDEX
};

enum {
	EMPTY_LOG_ENTRY = 0,
	COMPLETED_LOG_ENTRY,
	PARTIAL_LOG_ENTRY,
	INCOMPLETE_LOG_ENTRY
};

enum {
	AIR_SCALE_LINEAR = 0,
	AIR_SCALE_A_WEIGHTING = 1
};

///----------------------------------------------------------------------------
///	Structures
///----------------------------------------------------------------------------
typedef struct {
	// Sensor type information 
	uint8 numOfChannels;			// The number of channels from a sensor
	uint8 sensorAccuracy;			// = 100, sensor values are X 100 for numeric accuracy
	uint8 unitsFlag;	 			// 0 = SAE, 1 = Metric
	uint8 airUnitsFlag;				// 0 = Decibel, 1 = Millibar

	float	hexToLengthConversion;
	float	measurementRatio;	 	// 1 = SAE, 25.4 = Metric
	float	ameasurementRatio;		// ? = Decibel, 1 = Millibar
	float	sensorTypeNormalized;	

	uint16 shiftVal;
	uint16 ADCResolution;			// = 2048, Raw data Input Range, unless ADC is changed
	uint32 sensorValue;				// The value of the sensor, both metric and inches are X 100
} SENSOR_PARAMETERS_STRUCT;

typedef struct
{
	uint32 cindex;
	uint32 cmax;
	uint32 cmin; 
} CHAR_FIELD_STRUCT;

typedef struct
{
	uint8 num_type;
	uint32 nindex;
	float nmax;
	float nmin;
	float incr_value; 
	float tindex;
} NUM_FIELD_STRUCT;

typedef struct
{
	uint16 lindex;
	uint16 lmax;
	uint16 lmin;
} LIST_FIELD_STRUCT;

typedef struct
{
	uint8 data_val[REC_MN_DATAVAL_SIZE][REC_MN_STRING_SIZE];
	uint8 range[REC_MN_STRING_SIZE];
	uint16 type;
	uint16 enterflag;
	uint16 max_rlines;
	uint16 rlines;
	uint16 wrapflag;
	CHAR_FIELD_STRUCT charrec;
	NUM_FIELD_STRUCT numrec;
	LIST_FIELD_STRUCT listrec;
} REC_MN_STRUCT;

typedef struct
{
	uint8 client[TRIGGER_EVENT_STRING_SIZE];
	uint16 unused1;
	uint8 loc[TRIGGER_EVENT_STRING_SIZE];
	uint16 unused2;
	uint8 comments[TRIGGER_EVENT_STRING_SIZE];
	uint16 unused3;
	float dist_to_source;
	float weight_per_delay;
	uint8 oper[TRIGGER_EVENT_STRING_SIZE];
	uint16 unused4;
	uint32 seismicTriggerLevel;
	uint32 airTriggerLevel;
	uint32 record_time;
	uint32 record_time_max;
	uint32 sample_rate;
	uint8 bitAccuracy;
	uint8 adjustForTempDrift;
} TRIGGER_EVENT_DATA_STRUCT;

typedef struct
{
	uint32 barInterval;
	uint32 summaryInterval;
} BAR_GRAPH_EVENT_DATA_STRUCT;

typedef struct
{
	// Warning, following structure is aligned on a boundary
	uint8 barChannel;
	uint8 barScale;
	uint8 barIntervalDataType;
	uint8 unused1;
	uint8 unused2;
	uint8 unused3;
	uint8 unused4;
	uint8 unused5;
	uint8 impulseMenuUpdateSecs;
} BAR_GRAPH_EXTRA_DATA_STRUCT;

typedef struct
{
	uint32 unused;
	uint32 sensitivity;
} SENSOR_INFO_STRUCT;

typedef struct
{
	uint8 name[10];
	uint8 validRecord;
	uint8 opMode;
	DATE_TIME_STRUCT timeStamp;
	TRIGGER_EVENT_DATA_STRUCT trec;
	BAR_GRAPH_EVENT_DATA_STRUCT bgrec;
	BAR_GRAPH_EXTRA_DATA_STRUCT berec; 
	SENSOR_INFO_STRUCT srec;
} REC_EVENT_MN_STRUCT;

typedef struct
{
	uint16 validationKey;
	uint8 baudRate;
	uint8 rs232PowerSavings;
	uint8 adChannelVerification;
	uint8 externalTrigger;
	uint8 saveCompressedData;
	uint8 airScale;
	uint8 vectorSum;
	uint8 autoCalForWaveform;
	uint8 cycleEndTimeHour;
	uint8 unused1;
	uint8 flashWrapping;
	uint8 autoMonitorMode;
	uint8 autoCalMode;
	uint8 copies;
	uint8 freqPlotMode;
	uint8 freqPlotType;
	uint8 languageMode;
	uint8 lcdContrast;
	uint8 lcdTimeout;
	uint8 autoPrint;
	uint8 unitsOfMeasure;
	uint8 unitsOfAir;
	uint8 alarmOneMode;
	uint8 alarmTwoMode;
	uint8 usbSyncMode;
	uint8 pretrigBufferDivider;
	uint32 alarmOneSeismicLevel;
	uint32 alarmOneSeismicMinLevel;
	uint32 alarmOneAirLevel;
	uint32 alarmOneAirMinLevel;
	uint32 alarmTwoSeismicLevel;
	uint32 alarmTwoSeismicMinLevel;
	uint32 alarmTwoAirLevel;
	uint32 alarmTwoAirMinLevel;
	float alarmOneTime;
	float alarmTwoTime;
	uint32 TimerModeActiveMinutes;
	uint8 timerMode;
	uint8 timerModeFrequency;
	TM_TIME_STRUCT timerStartTime;
	TM_TIME_STRUCT timerStopTime;
	TM_DATE_STRUCT timerStartDate;
	TM_DATE_STRUCT timerStopDate;
} UNIT_CONFIG_STRUCT;

typedef struct
{
	uint16 invalid;						// 0x00
	CALIBRATION_DATE_STRUCT calDate;	// 0x02
	uint8 calibrationDateSource;		// 0x06
	uint8 acousticSensorType;			// 0x07
	uint8 unused[4];					// 0x08
	uint8 hardwareID;					// 0x0C
	uint8 buildID;						// 0x0D
	uint16 seismicSensorType;			// 0x0E
	char unitSerialNumber[16];			// 0x10
	uint8 aWeightOption;				// 0x20
	uint8 analogChannelConfig;			// 0x21
} FACTORY_SETUP_STRUCT;

typedef struct
{
	uint16 invalid;
	uint16 currentEventNumber;
} CURRENT_EVENT_NUMBER_STRUCT;

typedef struct
{
	uint16 invalid;
	uint16 currentMonitorLogID;
} MONITOR_LOG_ID_STRUCT;

typedef struct
{
	uint16 invalid;
	uint16 modemStatus;
	uint16 unlockCode;
	char init[64];
	char dial[32];
	char reset[16];
	uint8 retries;
	uint8 retryTime;
} MODEM_SETUP_STRUCT;

typedef struct
{
	uint16				uniqueEntryId;
	uint8				status;
	uint8				mode;
	DATE_TIME_STRUCT	startTime;
	DATE_TIME_STRUCT	stopTime;
	uint16				eventsRecorded;
	uint16				startEventNumber;
	uint32				seismicTriggerLevel;
	uint32				airTriggerLevel;
	uint8				bitAccuracy;
	uint8				adjustForTempDrift;
	uint16				seismicSensorType;
	uint32				sensitivity;
} MONITOR_LOG_ENTRY_STRUCT;

typedef struct
{
	uint16				lastStoredEvent;
	uint16				lastDownloadedEvent;
	DATE_TIME_STRUCT	lastConnectTime;
	uint32				unused;
} AUTODIALOUT_STRUCT;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void SaveRecordData(void*, uint32, uint8);
void GetRecordData(void*, uint32, uint8);
void ConvertTimeStampToString(char*, DATE_TIME_STRUCT*, uint8);
void CopyFlashBlock(uint16* dst, uint16* src, uint32 len);
void CopyRecordIntoFlashBk(uint16*, uint16*, uint32, uint32);
uint8 CheckForAvailableTriggerRecordEntry(char* name, uint8* match);
void LoadTrigRecordDefaults(REC_EVENT_MN_STRUCT *rec_ptr, uint8 opMode);
void LoadUnitConfigDefaults(UNIT_CONFIG_STRUCT *rec_ptr);
void ActivateUnitConfigOptions(void);
void LoadModemSetupRecordDefaults(void);
void ValidateModemSetupParameters(void);

// Monitor Log prototypes
void InitMonitorLog(void);
void AdvanceMonitorLogIndex(void);
uint16 GetStartingMonitorLogTableIndex(void);
uint16 GetStartingEventNumberForCurrentMonitorLog(void);
void ClearMonitorLogEntry(void);
void NewMonitorLogEntry(uint8 mode);
void UpdateMonitorLogEntry();
void CloseMonitorLogEntry();
void printMonitorLogEntry(uint8 mode, MONITOR_LOG_ENTRY_STRUCT* logEntry);
void InitMonitorLogUniqueEntryId(void);
void StoreMonitorLogUniqueEntryId(void);
uint8 GetNextMonitorLogEntry(uint16 uid, uint16 startIndex, uint16* tempIndex, MONITOR_LOG_ENTRY_STRUCT* logEntry);
uint16 NumOfNewMonitorLogEntries(uint16 uid);
void AppendMonitorLogEntryFile(void);
void InitMonitorLogTableFromLogFile(void);
void AddOnOffLogTimestamp(uint8 onOffState);
void FillInAdditionalExceptionReportInfo(int exceptionReportFile);
void WriteDebugBufferToFile(void);
void SwitchDebugLogFile(void);

// Parameter Memory
void GetParameterMemory(uint8* dest, uint16 address, uint16 size);
void SaveParameterMemory(uint8* src, uint16 address, uint16 size);
void EraseParameterMemory(uint16 address, uint16 size);
void GetFlashUserPageFactorySetup(FACTORY_SETUP_STRUCT* factorySetup);
void SaveFlashUserPageFactorySetup(FACTORY_SETUP_STRUCT* factorySetup);
void EraseFlashUserPageFactorySetup(void);

#endif // _REC_H_
