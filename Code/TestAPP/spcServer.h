/*
 * SmartPID Controller server for remote management
 *
 *  Copyright (C) 2016 Arzaman
 */

#ifndef SPCSERVER_H_
#define SPCSERVER_H_

/* Process the server task
 * This function is called repeatedly (with no specified periodicity) whenever
 * the server is operating; its task is to receive and serve requests from any
 * connected clients.
 * Parameters: none
 * Returns: none
 */
void spcServerProcess(void);

/* Execute time-critical operations for the server task
 * This function is called at each tick of the system timer.
 * Parameters: none
 * Returns: none
 */
void spcServerTick(void);

#endif
