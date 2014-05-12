///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Msgs430.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:54 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Msgs430.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _MSGS430_H_
#define _MSGS430_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define GAIN_SELECT_x2	0x01
#define GAIN_SELECT_x4	0x00

#pragma pack(1)

typedef struct
{
   uint8 channel_num;
   uint8 channel_type;
   uint8 group_num;
   uint8 options;
   uint16 trig_lvl_1;
   uint16 trig_lvl_2;
   uint16 trig_lvl_3;
} SENSOR_CHANNEL_STRUCT;

typedef struct
{
   uint8 cmd_id;
   uint8 capture_mode;
   SENSOR_CHANNEL_STRUCT channel[8];
   uint8 bg_sample_interval;
   uint8 total_record_time;
   uint16 sample_per_second;
   uint16 end_mark; 
} START_MSG_DATA_STRUCT;


typedef struct
{
  uint16 end_mark;
} STOP_MSG_DATA_STRUCT;


typedef union
{
  START_MSG_DATA_STRUCT startMsg430;
  STOP_MSG_DATA_STRUCT  stopMsg430;
} MSGS430_UNION;

#pragma pack()

#endif // _MSGS430_H_
