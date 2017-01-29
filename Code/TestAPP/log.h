/*
 * Log management
 *
 *  Copyright (C) 2016 Arzaman
 */

#ifndef LOG_H_
#define LOG_H_

#include <AsyncClient.h>

#define LOG_WIFI	(1 << 0)
#define LOG_EEPROM	(1 << 1)

extern enum logServerStatus {
	LOG_SERVER_STATUS_OK,
	LOG_SERVER_STATUS_ERROR,
} logServerStatus;

extern enum logChanStatus {
	LOG_CHAN_STATUS_OK,
	LOG_CHAN_STATUS_ERROR,
} logChanStatus;

/* Initialize the logging logic
 * Parameters: none
 * Returns: none
 */
void logInit(void);

/* Start logging for a new PID process
 * Parameters: none
 * Returns: none
 */
void logStart(void);

/* Process the logging task during an ongoing PID process
 * This function is called repeatedly (with no specified periodicity) during an
 * ongoing process; its task is to sample process parameters and send these
 * samples to the active logging interface(s).
 * Parameters: none
 * Returns: none
 */
void logProcess(void);

/* Retrieve the EEPROM memory occupancy level
 * Parameters: none
 * Returns: number ranging from 0 (memory empty) to 100 (memory full)
 */
int logEepromMemStatus(void);

/* Function to be called periodically, that manages tasks related to the logging
 * functionality.
 * This function must be called regularly, at the system tick, regardless of
 * whether a PID process is ongoing
 * Parameters: none
 * Returns: none
 */
void logTick(void);

#endif
