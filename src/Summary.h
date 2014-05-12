///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Summary.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:00 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Summary.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _SUMMARY_CMMN_H_
#define _SUMMARY_CMMN_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Common.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define NEW_TABLE_ENTRY			0
#define HEAD_TABLE_ENTRY 		1
#define CURR_TABLE_ENTRY 		2

// Defined in rec. h
//#define TRIGGER_EVENT_STRING_SIZE		101
#define VERSION_STRING_SIZE				8
#define MODEL_STRING_SIZE				20
#define SERIAL_NUMBER_STRING_SIZE		20

#define COMPANY_NAME_STRING_SIZE		32
#define SEISMIC_OPERATOR_STRING_SIZE	32
#define SESSION_LOCATION_STRING_SIZE	32
#define SESSION_COMMENTS_STRING_SIZE	102


// slowest sample rate * samllest time
#define MAX_RAM_SUMMARYS 				120 

#define BARGRAPH_PEAK_BUFFER_SIZE 		30
#define BARGRAPH_INTERVAL_BUFFER_SIZE	4

//#define PRE_TRIGGER_SIZE 				0.25
#define PRE_TRIGGER_TIME_MSEC			250
#define PRE_TRIGGER_TIME_SEC			0.25

// ===============================
// New Event Record Sub-Structures
// ===============================

// Version Information, version, model, and serial number.
typedef struct
{
	// System unit information.
	uint8	modelNumber[MODEL_STRING_SIZE];
	uint8	serialNumber[SERIAL_NUMBER_STRING_SIZE];
	uint8	softwareVersion[VERSION_STRING_SIZE];
	uint8	unused[20];
} VERSION_INFO_STRUCT;

// Seismic Channel Information - describes the initial channel data.
typedef struct
{
	uint8 type;
	uint8 input;
	uint8 group;
	uint8 options;
} SEISMIC_CHANNEL_INFO_STRUCT;

// Parameter Information, Initial condition information and system level settings. 
#pragma pack(1)
typedef struct
{
	uint32	distToSource;
	uint32	weightPerDelay;
	uint16	sampleRate;
	uint16	seismicSensorType;
	uint16	airSensorType;
	uint8	bitAccuracy;
	uint8  	aWeighting;
	uint8	numOfChannels;
	uint8	activeChannels;
	uint8	appMajorVersion;	// Used for modem config
	uint8	appMinorVersion;	// Used for modem config
	SEISMIC_CHANNEL_INFO_STRUCT	channel[8];

	// Waveform specific - Intitial conditions.
	uint32	seismicTriggerLevel;
	uint32	airTriggerLevel;
	uint32  recordTime;
	uint16	numOfSamples;
	uint16	preBuffNumOfSamples;
	uint16	calDataNumOfSamples;

	// Bargraph specific - Intitial conditions.
	uint16	barInterval;
	uint16	summaryInterval;

	uint8	companyName[COMPANY_NAME_STRING_SIZE];
	uint8	seismicOperator[SEISMIC_OPERATOR_STRING_SIZE];
	uint8	sessionLocation[SESSION_LOCATION_STRING_SIZE];
	uint8	sessionComments[SESSION_COMMENTS_STRING_SIZE];
	
	uint8	unused[40];						// Space for expansion
} PARAMETERS_STRUCT;
#pragma pack()

// Capture Information - After an event, System data at the end of an event. 
#pragma pack(1)
typedef struct
{
	DATE_TIME_STRUCT	calDate;			// Calibration date
	uint32				batteryLevel;		// Battery Level
	uint8				printerStatus;		// Printer status information.
	uint8				unused01;			// Unused
	uint8				unused02;			// Unused
	uint8				unused03;			// Unused
	DATE_TIME_STRUCT	eventTime;			// Waveform and bargraph start information. 
	DATE_TIME_STRUCT	endTime;			// Bargraph specific

	uint8				unused[18];			// Space for expansion
} CAPTURE_INFO_STRUCT;
#pragma pack()

// Channel Calculated Data - Data captured or calculated during
//  an event. Can be for waveform or bargraph events.
typedef struct
{
	uint16 peak;							// Max peak, kept in raw values, not in units of measurements.
	uint16 frequency;						// The Count of a period. The frequency is calculated from this. 
	uint32 displacement;					// Peak Displacement = Peak / (2 * PI * Freq)
	uint16 acceleration;					// TBD
	uint16 unused1;							// Spare for expansion.
} CHANNEL_CALCULATED_DATA_STRUCT;

// Calculated Data Struct - The following has defined to keep the data format 
// consistent with the previous release. Yes if we move data structure into 
// channel data structure, the data layout would be neater, but users of 
// the data has already defined their variable to use the old formats.
#pragma pack(1)
typedef struct
{
	// Used for both Waveform and Bargraph
	CHANNEL_CALCULATED_DATA_STRUCT	a;
	CHANNEL_CALCULATED_DATA_STRUCT	r;
	CHANNEL_CALCULATED_DATA_STRUCT	v;
	CHANNEL_CALCULATED_DATA_STRUCT	t;
	CHANNEL_CALCULATED_DATA_STRUCT	unused1;
	uint32				vectorSumPeak;		// Exception. Need a uint32 to hold the value.

	// Bargraph specific variables.
	DATE_TIME_STRUCT	a_Time;			// Added = 12bytes
	DATE_TIME_STRUCT	r_Time;			// Added = 12bytes
	DATE_TIME_STRUCT	v_Time;			// Added = 12bytes
	DATE_TIME_STRUCT	t_Time;			// Added = 12bytes
	DATE_TIME_STRUCT	vs_Time;		// Added = 12bytes
	DATE_TIME_STRUCT	intervalEnd_Time;	

	uint32				batteryLevel;
	uint32				barIntervalsCaptured;	// Number of bar interval samples save in flash.
	uint16				summariesCaptured;		// Number bar summaries captured and saved in flsah.

	uint8				unused[50]; // was 52
	uint32				calcStructEndFlag;
} CALCULATED_DATA_STRUCT;
#pragma pack()


// ======================================
// End of New Event Record Sub-Structures
// ======================================

// Waveform Event struct - This is the grouping of the event data 
// for a waveform event. Each channel consists of a 16 bit vaule.
typedef struct
{
	uint16 a;
	uint16 r;
	uint16 v;
	uint16 t;
} SAMPLE_DATA_STRUCT;

typedef struct
{
	int16 a;
	int16 r;
	int16 v;
	int16 t;
} OFFSET_DATA_STRUCT;

// Bargraph Event struct - This is the grouping of the event data 
// for a bargraph event. We store the vectorsum data (squared), and
// then later perform a square root operation to get the correct result.
// The squaring takes too much time in the ISR context.
#pragma pack(1)
typedef struct
{
	uint16 air;					// air is the max peak for an interval
	uint16 rvt;					// rvt is the max peak for all channels for an interval.
	uint32 vs;					// We are storing the vectorsum data (squared),
} BAR_INTERVAL_DATA_STRUCT;
#pragma pack()

/////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint16 peak;
	uint16 freq;
	uint16* peakPtr;
} CHAN_PEAK_AND_FREQ;

typedef struct
{
	union
	{
		CHAN_PEAK_AND_FREQ chan[4];
		struct {
			CHAN_PEAK_AND_FREQ a;
			CHAN_PEAK_AND_FREQ r;
			CHAN_PEAK_AND_FREQ v;
			CHAN_PEAK_AND_FREQ t;
		};
	};
	uint32 vs;
} SUMMARY_WAVESHAPE;

typedef struct
{ 
	uint16* linkPtr;
	uint8  mode;
	SUMMARY_WAVESHAPE waveShapeData;
} SUMMARY_DATA;


////////////////////////////////////////////////////////////
// structs for flash summarys


typedef struct
{
	uint16 sign;
	uint16 freq_count;
	uint8 updateFlag;
	uint8 matchFlag;
} BARGRAPH_FREQ_CHANNEL_BUFFER;

typedef struct
{
	BARGRAPH_FREQ_CHANNEL_BUFFER a;
	BARGRAPH_FREQ_CHANNEL_BUFFER r;
	BARGRAPH_FREQ_CHANNEL_BUFFER v;
	BARGRAPH_FREQ_CHANNEL_BUFFER t;
} BARGRAPH_FREQ_CALC_BUFFER;

typedef struct 
{
	uint16 aMax;
	uint16 rvtMax;
	uint32 vsMax;		// Changed to 32 to hold the square of the sums, 
} BARGRAPH_BAR_INTERVAL_DATA;

typedef struct
{
	uint16 startFlag;
	uint16 recordVersion;
	uint16 headerLength;
	uint16 summaryLength;
	uint32 dataLength;
	uint16 dataCompression;
	uint16 summaryChecksum;
	uint16 dataChecksum;
	uint16 unused1;
	uint16 unused2;
	uint16 unused3;
} EVENT_HEADER_STRUCT;

typedef struct
{
	VERSION_INFO_STRUCT		version;
	PARAMETERS_STRUCT		parameters;
	CAPTURE_INFO_STRUCT		captured;
	CALCULATED_DATA_STRUCT	calculated;
	uint16 					eventNumber;
	uint8					mode;
	uint8					unused;
} EVENT_SUMMARY_STRUCT;

typedef struct
{
	EVENT_HEADER_STRUCT 		header;
	EVENT_SUMMARY_STRUCT		summary;
} EVT_RECORD;




#endif //_SUMMARY_CMMN_H_

