///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: TextTypes.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:01 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/TextTypes.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _TEXT_TYPES_H_
#define _TEXT_TYPES_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Macros
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern char englishLanguageTable[];
extern char frenchLanguageTable[];
extern char italianLanguageTable[];
extern char germanLanguageTable[];

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define getLangText(x)	g_languageLinkTable[x]

///----------------------------------------------------------------------------
///	Text Types - Language Table/Translation/Strings
///----------------------------------------------------------------------------
enum {
	A_WEIGHTING_TEXT = 0,
	A_WEIGHTING_OPTION_TEXT,
	A_D_CALIBRATED_OK_TEXT,
	ACTIVE_DATE_PERIOD_TEXT,
	ACTIVE_TIME_PERIOD_TEXT,
	AFTER_EVERY_24_HRS_TEXT,
	AFTER_EVERY_48_HRS_TEXT,
	AFTER_EVERY_72_HRS_TEXT,
	AIR_TEXT,
	AIR_CHANNEL_SCALE_TEXT,
	ALARM_1_TEXT,
	ALARM_1_AIR_LEVEL_TEXT,
	ALARM_1_SEISMIC_LVL_TEXT,
	ALARM_1_TIME_TEXT,
	ALARM_2_TEXT,
	ALARM_2_AIR_LEVEL_TEXT,
	ALARM_2_SEISMIC_LVL_TEXT,
	ALARM_2_TIME_TEXT,
	ALARM_OUTPUT_MODE_TEXT,
	ALL_RIGHTS_RESERVED_TEXT,
	ARE_YOU_DONE_WITH_CAL_SETUP_Q_TEXT,
	ATTEMPTING_TO_HANDLE_PARTIAL_EVENT_ERROR_TEXT,
	AUTO_CALIBRATION_TEXT,
	AUTO_MONITOR_TEXT,
	AUTO_MONITOR_AFTER_TEXT,
	BAR_GRAPH_TEXT,
	BAR_INTERVAL_TEXT,
	BAR_PRINTOUT_RESULTS_TEXT,
	BARGRAPH_ABORTED_TEXT,
	BARGRAPH_MODE_TEXT,
	BARS_TEXT,
	BATT_VOLTAGE_TEXT,
	BATTERY_TEXT,
	BATTERY_VOLTAGE_TEXT,
	BOTH_TEXT,
	CAL_DATE_STORED_TEXT,
	CAL_SETUP_TEXT,
	CALIBRATING_TEXT,
	CALIBRATION_TEXT,
	CALIBRATION_CHECK_TEXT,
	CALIBRATION_DATE_TEXT,
	CALIBRATION_DATE_NOT_SET_TEXT,
	CALIBRATION_GRAPH_TEXT,
	CALIBRATION_PULSE_TEXT,
	CANCEL_TIMER_MODE_Q_TEXT,
	CLIENT_TEXT,
	COMBO_TEXT,
	COMBO_MODE_TEXT,
	COMBO_MODE_NOT_IMPLEMENTED_TEXT,
	COMMENTS_TEXT,
	COMPANY_TEXT,
	CONFIG_AND_OPTIONS_TEXT,
	CONFIG_OPTIONS_MENU_TEXT,
	CONFIRM_TEXT,
	COPIES_TEXT,
	COPY_TEXT,
	CURRENTLY_NOT_IMPLEMENTED_TEXT,
	DAILY_EVERY_DAY_TEXT,
	DAILY_WEEKDAYS_TEXT,
	DARK_TEXT,
	DARKER_TEXT,
	DATE_TEXT,
	DATE_TIME_TEXT,
	DAY_MONTH_YEAR_TEXT,
	DEFAULT_TEXT,
	DEFAULT_BAR_TEXT,
	DEFAULT_COMBO_TEXT,
	DEFAULT_SELF_TRG_TEXT,
	DISABLED_TEXT,
	DISABLING_TIMER_MODE_TEXT,
	DISCOVERED_A_BROKEN_EVENT_LINK_TEXT,
	DISTANCE_TO_SOURCE_TEXT,
	DO_NOT_TURN_THE_UNIT_OFF_UNTIL_THE_OPERATION_IS_COMPLETE_TEXT,
	DO_YOU_WANT_TO_ENTER_MANUAL_TRIGGER_MODE_Q_TEXT,
	DO_YOU_WANT_TO_LEAVE_CAL_SETUP_MODE_Q_TEXT,
	DO_YOU_WANT_TO_LEAVE_MONITOR_MODE_Q_TEXT,
	DO_YOU_WANT_TO_SAVE_THE_CAL_DATE_Q_TEXT,
	EDIT_TEXT,
	EMPTY_TEXT,
	ENABLED_TEXT,
	ERASE_COMPLETE_TEXT,
	ERASE_EVENTS_TEXT,
	ERASE_MEMORY_TEXT,
	ERASE_OPERATION_IN_PROGRESS_TEXT,
	ERASE_SETTINGS_TEXT,
	ERROR_TEXT,
	ESC_HIT_TEXT,
	EVENT_TEXT,
	EVENT_NUMBER_ZEROING_COMPLETE_TEXT,
	EVENT_SUMMARY_TEXT,
	FACTORY_SETUP_COMPLETE_TEXT,
	FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT,
	FAILED_TEXT,
	FEET_TEXT,
	FREQUENCY_TEXT,
	FREQUENCY_PLOT_TEXT,
	FRQ_TEXT,
	FULL_TEXT,
	GRAPHICAL_RECORD_TEXT,
	HELP_INFO_MENU_TEXT,
	HELP_INFORMATION_TEXT,
	HELP_MENU_TEXT,
	HOUR_TEXT,
	HOURS_TEXT,
	HR_TEXT,
	HZ_TEXT,
	IMPERIAL_TEXT,
	INCLUDED_TEXT,
	INSTRUMENT_TEXT,
	JOB_NUMBER_TEXT,
	JOB_PEAK_RESULTS_TEXT,
	JOB_VECTOR_SUM_RESULTS_TEXT,
	LANGUAGE_TEXT,
	LAST_SETUP_TEXT,
	LCD_CONTRAST_TEXT,
	LIGHT_TEXT,
	LIGHTER_TEXT,
	LINEAR_TEXT,
	LIST_OF_SUMMARIES_TEXT,
	LOCATION_TEXT,
	LOW_TEXT,
	METRIC_TEXT,
	MIC_A_TEXT,
	MIC_B_TEXT,
	MINUTE_TEXT,
	MINUTES_TEXT,
	MMPS_DIV_TEXT,
	MONITOR_TEXT,
	MONITOR_BARGRAPH_TEXT,
	MONITORING_TEXT,
	MONTHLY_TEXT,
	NAME_TEXT,
	NEXT_EVENT_STORAGE_LOCATION_NOT_EMPTY_TEXT,
	NO_TEXT,
	NO_AUTO_CAL_TEXT,
	NO_AUTO_MONITOR_TEXT,
	NO_STORED_EVENTS_FOUND_TEXT,
	NOMIS_MAIN_MENU_TEXT,
	NOT_INCLUDED_TEXT,
	NOTES_TEXT,
	OFF_TEXT,
	ON_TEXT,
	ONE_TIME_TEXT,
	OPERATOR_TEXT,
	OVERWRITE_SETTINGS_TEXT,
	PARTIAL_RESULTS_TEXT,
	PASSED_TEXT,
	PEAK_TEXT,
	PEAK_AIR_TEXT,
	PLEASE_BE_PATIENT_TEXT,
	PLEASE_CONSIDER_ERASING_THE_FLASH_TO_FIX_PROBLEM_TEXT,
	PLEASE_POWER_OFF_UNIT_TEXT,
	PLEASE_PRESS_ENTER_TEXT,
	PLOT_STANDARD_TEXT,
	POWERING_UNIT_OFF_NOW_TEXT,
	PRINT_GRAPH_TEXT,
	PRINTER_TEXT,
	PRINTER_ON_TEXT,
	PRINTING_STOPPED_TEXT,
	PRINTOUT_TEXT,
	PROCEED_WITHOUT_SETTING_DATE_AND_TIME_Q_TEXT,
	PROCESSING_TEXT,
	RAD_TEXT,
	RADIAL_TEXT,
	RAM_SUMMARY_TABLE_IS_UNINITIALIZED_TEXT,
	RECORD_TIME_TEXT,
	REPORT_DISPLACEMENT_TEXT,
	RESULTS_TEXT,
	SAMPLE_RATE_TEXT,
	SAVE_CHANGES_TEXT,
	SAVE_SETUP_TEXT,
	SAVED_SETTINGS_TEXT,
	SCANNING_STORAGE_FOR_VALID_EVENTS_TEXT,
	SEC_TEXT,
	SECOND_TEXT,
	SECONDS_TEXT,
	SEIS_TRIG_TEXT,
	SEIS_LOCATION_TEXT,
	SEISMIC_TEXT,
	SEISMIC_TRIGGER_TEXT,
	SELECT_TEXT,
	SELF_TRIGGER_TEXT,
	SENSOR_A_TEXT,
	SENSOR_B_TEXT,
	SENSOR_CHECK_TEXT,
	SENSOR_GAIN_TYPE_TEXT,
	SENSOR_GAIN_TYPE_NOT_SET_TEXT,
	SENSOR_TYPE_TEXT,
	SERIAL_NUMBER_TEXT,
	SERIAL_NUMBER_NOT_SET_TEXT,
	SETTINGS_WILL_NOT_BE_LOADED_TEXT,
	SOFTWARE_VER_TEXT,
	SOUND_TEXT,
	AIR_TRIG_TEXT,
	AIR_TRIGGER_TEXT,
	SPANISH_TEXT,
	START_DATE_TEXT,
	START_TIME_TEXT,
	STATUS_TEXT,
	STOP_DATE_TEXT,
	STOP_PRINTING_TEXT,
	STOP_TIME_TEXT,
	SUCCESS_TEXT,
	SUMMARIES_EVENTS_TEXT,
	SUMMARY_INTERVAL_TEXT,
	SUMMARY_INTERVAL_RESULTS_TEXT,
	TESTING_TEXT,
	THIS_FEATURE_IS_NOT_CURRENTLY_AVAILABLE_TEXT,
	TIME_TEXT,
	TIMER_FREQUENCY_TEXT,
	TIMER_MODE_TEXT,
	TIMER_MODE_DISABLED_TEXT,
	TIMER_MODE_NOW_ACTIVE_TEXT,
	TIMER_MODE_SETUP_NOT_COMPLETED_TEXT,
	TIMER_SETTINGS_INVALID_TEXT,
	TRAN_TEXT,
	TRANSVERSE_TEXT,
	UNIT_IS_IN_TIMER_MODE_TEXT,
	UNITS_OF_MEASURE_TEXT,
	USBM_OSMRE_REPORT_TEXT,
	VECTOR_SUM_TEXT,
	VERIFY_TEXT,
	VERT_TEXT,
	VERTICAL_TEXT,
	VOLTS_TEXT,
	WARNING_TEXT,
	WAVEFORM_MODE_TEXT,
	WEEKLY_TEXT,
	WEIGHT_PER_DELAY_TEXT,
	YES_TEXT,
	YOU_HAVE_ENTERED_THE_FACTORY_SETUP_TEXT,
	ZERO_EVENT_NUMBER_TEXT,
	ZEROING_SENSORS_TEXT,
	OK_TEXT,
	CANCEL_TEXT,
	MAX_TEXT,
	CHARS_TEXT,
	RANGE_TEXT,
	ENGLISH_TEXT,
	ITALIAN_TEXT,
	FRENCH_TEXT,
	BRITISH_TEXT,
	DIN_4150_TEXT,
	US_BOM_TEXT,
	X1_20_IPS_TEXT,
	X2_10_IPS_TEXT,
	X4_5_IPS_TEXT,
	X8_2_5_IPS_TEXT,
	BAR_TEXT,
	SELF_TRG_TEXT,
	END_TEXT,
	REALLY_ERASE_ALL_EVENTS_Q_TEXT,
	MODEM_SETUP_TEXT,
	MODEM_INIT_TEXT,
	MODEM_DIAL_TEXT,
	MODEM_RESET_TEXT,
	SENSITIVITY_TEXT,
	HIGH_TEXT,
	UNLOCK_CODE_TEXT,
	ACC_793L_TEXT,
	GERMAN_TEXT,
	BAUD_RATE_TEXT,
	POWER_UNIT_OFF_EARLY_Q_TEXT,
	NAME_ALREADY_USED_TEXT,
	DELETE_SAVED_SETUP_Q_TEXT,
	NAME_MUST_HAVE_AT_LEAST_ONE_CHARACTER_TEXT,
	BAR_SCALE_TEXT,
	REPORT_MILLIBARS_TEXT,
	LCD_TIMEOUT_TEXT,
	IMPULSE_RESULTS_TEXT,
	LCD_IMPULSE_TIME_TEXT,
	PRINT_MONITOR_LOG_TEXT,
	MONITOR_LOG_TEXT,
	EVENTS_RECORDED_TEXT,
	EVENT_NUMBERS_TEXT,
	CANCEL_ALL_PRINT_JOBS_Q_TEXT,
	CAL_SUMMARY_TEXT,
	MODEM_RETRY_TEXT,
	MODEM_RETRY_TIME_TEXT,
	FLASH_WRAPPING_TEXT,
	FLASH_STATS_TEXT,
	AUTO_DIAL_INFO_TEXT,
	PEAK_DISPLACEMENT_TEXT,
	WAVEFORM_AUTO_CAL_TEXT,
	START_OF_WAVEFORM_TEXT,
	CALIBRATED_ON_TEXT,
	VIEW_MONITOR_LOG_TEXT,
	PRINT_IN_MONITOR_TEXT,
	LOG_RESULTS_TEXT,
	MONITOR_LOG_RESULTS_TEXT,
	PRINT_MONITOR_LOG_ENTRY_Q_TEXT,
	PRINTER_DISABLED_TEXT,
	PRINTING_TEXT,
	BARGRAPH_REPORT_TEXT,
	NORMAL_FORMAT_TEXT,
	SHORT_FORMAT_TEXT,
	REPORT_PEAK_ACC_TEXT,
	PEAK_ACCELERATION_TEXT,
	EVENT_NUMBER_TEXT,
	START_OF_LOG_TEXT,
	END_OF_LOG_TEXT,
	USED_TEXT,
	FREE_TEXT,
	EVENT_DATA_TEXT,
	WRAPPED_TEXT,
	FLASH_USAGE_STATS_TEXT,
	SPACE_REMAINING_TEXT,
	BEFORE_OVERWRITE_TEXT,
	WAVEFORMS_TEXT,
	BAR_HOURS_TEXT,
	LAST_DIAL_EVENT_TEXT,
	LAST_RECEIVED_TEXT,
	LAST_CONNECTED_TEXT,
	AUTO_DIALOUT_INFO_TEXT,
	UNITS_OF_AIR_TEXT,
	DECIBEL_TEXT,
	MILLIBAR_TEXT,
	BAUD_RATE_115200_TEXT,
	BIT_ACCURACY_TEXT,
	BIT_TEXT,
	HOURLY_TEXT,
	RECALIBRATE_TEXT,
	ON_TEMP_CHANGE_TEXT,
	NULL_TEXT,
	TOTAL_TEXT_STRINGS
};

#endif // _TEXT_TYPES_H_
