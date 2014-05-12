///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Record.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Record.h,v $
///	$Revision: 1.2 $
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
	REC_SUMMARY_DATA_TYPE,
	REC_DATE_TYPE,
	REC_DATE_TIME_TYPE,
	REC_DATE_TIME_AM_PM_TYPE,
	REC_DATE_TIME_MONITOR,
	REC_RESULTS_DATA_TYPE,
	REC_HELP_USER_MENU_TYPE,
	REC_FACTORY_SETUP_TYPE,
	REC_DATE_TIME_DISPLAY,
	REC_MODEM_SETUP_TYPE,
	REC_UNIQUE_EVENT_ID_TYPE,
	REC_UNIQUE_MONITOR_LOG_ID_TYPE
};

#define DEFAULT_RECORD 			0
#define MAX_NUM_OF_SAVED_SETUPS 14

#define VALID_RAM_SUMMARY_TABLE_KEY 0xA55AF00F
#define VALID_MONITOR_LOG_TABLE_KEY 0x0FF05A5A
#define VALID_AUTODIALOUT_TABLE_KEY 0x12ABCDEF

// Sensor information
#define NUMBER_OF_CHANNELS_DEFAULT 	4
#define SENSOR_ACCURACY_100X_SHIFT 	100
#define NUMBER_OF_SENSOR_TYPES 		8

#define ADC_RESOLUTION				0x8000	// +/- 0x800 (2048)

#define SENSOR_20_IN	2048	//4096	// 65536 // 4096
#define SENSOR_10_IN	1024	//2048	// 32768 // 2048
#define SENSOR_5_IN		512		//1024	// 16384 // 1024
#define SENSOR_2_5_IN	256		//512		// 8192 // 512
#define SENSOR_ACC		25600	//51200	// 819200 // 51200

#define INCH_PER_CM			 		2.54
#define LBS_PER_KG			 		2.2
#define FT_PER_METER				3.280833

#define ONE_GRAVITY_IN_INCHES		386.4
#define ONE_GRAVITY_IN_MM			9814.6

#define REC_MN_STRING_SIZE			20
#define TRIGGER_EVENT_STRING_SIZE	101
#define REC_MN_DATAVAL_SIZE			7

#define CALIBRATION_NUMBER_OF_SAMPLES 100

#define TOTAL_MONITOR_LOG_ENTRIES	((0x1000 - 8) / sizeof(MONITOR_LOG_ENTRY_STRUCT))

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

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
typedef struct {
	// Sensor type information 
	uint8	numOfChannels;			// The number of channels from a sensor.
	uint8   sensorAccuracy;			// = 100, sensor values are X 100 for numeric accuracy. 
	uint8	unitsFlag;	 			// 0 = SAE; 1 = Metric
	uint8	airUnitsFlag;			// 0 = Decibel; 1 = Millibar

	float	hexToLengthConversion;
	float	measurementRatio;	 	// 1 = SAE; 25.4 = Metric
	float	ameasurementRatio;		// ? = Decibel; 1 = Millibar
	float	sensorTypeNormalized;	

	uint16  shiftVal;
	uint16  ADCResolution;			// = 2048, Raw data Input Range, unless ADC is changed
	uint32  sensorValue;			// The value of the sensor, both metric and inches are X 100, 
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
	NUM_FIELD_STRUCT  numrec;
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
	// Warning, DO NOT change the order or types of the following
	// Even if defined as 'uint8 unused[7]' will cause a size change in the g_triggerRecord
	uint8 barChannel;
	uint8 barScale;
	int32 unused3;
	uint8 impulseMenuUpdateSecs;
} BAR_GRAPH_EXTRA_DATA_STRUCT;

typedef struct
{
	uint32 sensor_type;
	uint32 sensitivity;
} SENSOR_INFO_STRUCT;

typedef struct
{
	uint8 name[10];
	uint8 validRecord;
	uint8 op_mode;
	DATE_TIME_STRUCT time_stamp;
	TRIGGER_EVENT_DATA_STRUCT trec;
	BAR_GRAPH_EVENT_DATA_STRUCT bgrec;
	BAR_GRAPH_EXTRA_DATA_STRUCT berec; 
	SENSOR_INFO_STRUCT srec;
} REC_EVENT_MN_STRUCT;

typedef struct
{
	char hour;
	char min;
	char sec;
} TM_TIME_STRUCT;

typedef struct
{
	char day;
	char month;
	char year;
} TM_DATE_STRUCT;

typedef struct
{
	uint16 encode_ln;
	uint8 unused[6];
	uint8 unused1;
	uint8 auto_cal_in_waveform;
	uint8 report_peak_acceleration;
	uint8 report_displacement;
	uint8 flash_wrapping;
	uint8 auto_monitor_mode;
	uint8 auto_cal_mode;
	uint8 copies;
	uint8 freq_plot_mode;
	uint8 freq_plot_type;
	uint8 lang_mode;
	uint8 lcd_contrast;
	uint8 lcd_timeout;
	uint8 auto_print;
	uint8 units_of_measure;
	uint8 units_of_air;
	uint8 alarm_one_mode;
	uint8 alarm_two_mode;
	uint8 print_monitor_log;
	uint32 alarm_one_seismic_lvl;
	uint32 alarm_one_seismic_min_lvl;
	uint32 alarm_one_air_lvl;
	uint32 alarm_one_air_min_lvl;
	uint32 alarm_two_seismic_lvl;
	uint32 alarm_two_seismic_min_lvl;
	uint32 alarm_two_air_lvl;
	uint32 alarm_two_air_min_lvl;
	float alarm_one_time;
	float alarm_two_time;
	uint8 baud_rate;
	uint8 vector_sum;
	uint8 timer_mode;
	uint8 timer_mode_freq;
	uint16 timer_mode_active_minutes;
	TM_TIME_STRUCT  tm_start_time;
	TM_TIME_STRUCT  tm_stop_time;
	TM_DATE_STRUCT  tm_start_date;
	TM_DATE_STRUCT  tm_stop_date;
} REC_HELP_MN_STRUCT;

typedef struct
{
	uint16 invalid;
	DATE_TIME_STRUCT cal_date;
	uint16 sensor_type;
	char serial_num[16];
	uint8 aweight_option;
	uint8 spare;
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
void saveRecData(void*, uint32, uint8);
void getRecData(void*, uint32, uint8);
void convertTimeStampToString(char*, void*, uint8);
void copyFlashBlock(uint16* src, uint16* dst, uint32 len);
void copyRecIntoFlashBk(uint16*, uint16*, uint32, uint32);
uint8 checkForAvailableTriggerRecordEntry(char* name, uint8* match);
void loadTrigRecordDefaults(REC_EVENT_MN_STRUCT *rec_ptr, uint8 op_mode);
void loadHelpRecordDefaults(REC_HELP_MN_STRUCT *rec_ptr);
void activateHelpRecordOptions(void);
void loadModemSetupRecordDefaults(void);
void validateModemSetupParameters(void);

// Monitor Log prototypes
void initMonitorLog(void);
void advanceMonitorLogIndex(void);
uint16 getStartingMonitorLogTableIndex(void);
uint16 getStartingEventNumberForCurrentMonitorLog(void);
void clearMonitorLogEntry(void);
void newMonitorLogEntry(uint8 mode);
void updateMonitorLogEntry();
void closeMonitorLogEntry();
void printMonitorLogEntry(uint8 mode, MONITOR_LOG_ENTRY_STRUCT* logEntry);
void initMonitorLogUniqueEntryId(void);
void storeMonitorLogUniqueEntryId(void);
uint8 getNextMonitorLogEntry(uint16 uid, uint16 startIndex, uint16* tempIndex, MONITOR_LOG_ENTRY_STRUCT* logEntry);
uint16 numOfNewMonitorLogEntries(uint16 uid);
void appendMonitorLogEntryFile(void);
void initMonitorLogTableFromLogFile(void);

// Parameter Memory
void GetParameterMemory(uint8* dest, uint16 address, uint16 size);
void SaveParameterMemory(uint8* src, uint16 address, uint16 size);
void EraseParameterMemory(uint16 address, uint16 size);
uint8 ReadParameterMemory(uint16 address);
void WriteParameterMemory(uint16 address, uint8 data);
void ResetWriteEnableLatchParameterMemory(void);
void SetWriteEnableLatchParameterMemory(void);
uint8 ReadStatusParameterMemory(void);
void WriteStatusParameterMemory(uint8 data);

#endif // _REC_H_
