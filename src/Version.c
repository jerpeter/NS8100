///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#if 0
$RCSfile: Version.c,v $
$Author: jgetz $
$Date: 2012/04/26 01:10:04 $
$Source: /Nomis_NS8100/ns7100_Port/src/Version.c,v $
$Revision: 1.2 $
#endif

// =====================================================
// These global strings are automatically updated by CVS
// ** DO NOT MODIFY **
//const char applicationVersion[] = "$Revision: 0.9 $";
//const char applicationDate[] = "$Date: 2012/08/08 01:10:04 $";
const char g_buildVersion[] = "2.14.C";
const char g_buildDate[] = __DATE__ " " __TIME__;
// =====================================================

/*=================================================================================================
=  Notes // ns8100
===================================================================================================
Version 2.0.9:	Fix a cyclic buffer logic bug for Bargraph and Combo mode that 
				caused a failure to allow monitoring to finish. 

===================================================================================================
=  End of Notes // ns8100
=================================================================================================*/

/*======================================================
=  Notes // ns7100
========================================================
Version 1.2:  Initial cross over version from the old style.
Version 1.3:  Includes a fix for the off key processing while monitoring with the
				mini unit. Fixed the display of the date on startup to now be
				"day month year". The forced off condition in case of a lockup
				now uses the ESC key for both unit types. Also moved power off
				control to outside of monitor processing. Added a MessageBox when
				the user selects "YES" in the Erase Events menu to verify the 
				operation.
Version 1.4:  Fix a bug where comparing the decrementing value of an integer or a 
				float would result in a possible logic flaw with unsigned 
				elements past the zero point.
Version 1.5:  Fix a bug with the Summary menu and an empty list that would
				sometimes cause the app to crash. Added in logic to disable
				monitor mode if the voltage drops below 5.
Version 1.6:  Needless bump to make sure even numbered versions are releases and 
				odd numbered versions are tests.
Version 1.7:  Test build.
Version 1.8:  Release which includes modem communication, French language support
				for the LCD, bug fixes, and new bargraph data storage mechanism.
Version 1.9:  Test build.
Version 1.10: Release includes modem fixes and updates, language translations on 
				printouts, and some updated printout formatting.
Version 1.11: Test build.
Version 1.12: Release includes a ton of updates/features. The German language was
				introduced. Numerous Bargraph updates were added. Baud rate 
				selection was added. Alarm level detection was added for all 
				Waveform event data and now Bargraph. Startup logic was completely
				revamped. Now zeroing the sensors before starting any mode. 
				Updated Timer mode logic and handling and now displaying the 
				current settings. Modem configuration logic was introduced.
Version 1.13: Test build.
Version 1.14: Release includes filtering to remove the noise spike generated by
				turning the printer on, a final solution for the MCore A/D battery
				problem, the ability to do remote configuration and the validation
				of correct settings, and optimized bargraph printing.
Version 1.15: Test build.
Version 1.16: Release includes minor changes, mostly surrounding printing and 
				scaling for bargraph and the ability to select scale.
Version 1.17: Test build.
Version 1.18: Release includes bug fixes, calibration pulse requirements changes
				and fixes, printing updates, add labels for different standards,
				and remote configuration updates.
Version 1.19: Test build.
Version 1.20: Release includes binary download and other craft interface fixes,
				ability to select millibar reporting, several bug fixes and 
				requirements changes.
Version 1.21: Test build.
Version 1.22: Release includes printer changes to fix the calibration graph.
Version 1.23: Test build.
Version 1.24: Release includes modem disconnect functionality after a completed
				remote session and at midnight.
Version 1.25: Test build.
Version 1.26: Release includes a critical bug fix for delayed cal pulses when 
				events wrap in the event buffer, plus the bargraph LCD updates
				including the new Impulse menu and updated bargraph processing.
Version 1.27: Test build.
Version 1.28: Release includes a fix for the satellite modems to delay 15 seconds
				after issuing a clear DTR to allow a proper disconnect. 
Version 1.29: Test build.
Version 1.30: Release includes loads of changes; Modem reset processing in 
				multiple stages, Number of Copies now functional, Monitor Log 
				processing and printing, VML command for remote Monitor Log
				download, New Print queuing logic, Updated Waveform and 
				Bargraph print code broken into stages, Modem processing and the 
				MiniLZO compression algorithm, Updates to communicate with the 
				Nomis DLL, New print escape logic, and various bug fixes.
Version 1.31: Test build.
Version 1.32: Release includes updated Manual Calibration functionality that will
				allow calibration per user request while in Monitor mode and 
				force a calibration at the start of Bargraph mode. Also various
				bug fixes plus the resolution of the frozen time problem.
Version 1.33: Test build.
Version 1.34: Release includes a bug fix for summary.parameters.numOfSamples 
				problem where the value for large record times can be larger than 
				the uint16 will hold.
Version 1.35: Test build.
Version 1.36: Release includes updates to the Ram Summary table handling where the
				highest numbered event will be used to determine the flash data
				pointer to be used. Also includes a bug fix in the Ram Summary
				table recreation that would not include a wrapped event because it
				failed the safeguards. Also reduced the number or Ram Summary 
				entries to 800 to prevent the bootloader from sometimes over witting
				entries 837 and above.
Version 1.37: Test build.
Version 1.38: Release includes a fix to force the sample rate for Manual Calibration
				size and calculations at 1024 regardless of the system sample rate
				setting. This prevents problems with bad manual cal results and
				issues with incorrect data sizes.				 
Version 1.39: Test build.
Version 1.40: Release includes the ability to enable or disable the flash wrapping
				feature. Added in new remote commands DMM, UMM, GFS and updated the
				UCM to validating the incoming CRC. Also several bug fixes.
========================================================
=  End of Notes // ns7100
======================================================*/
