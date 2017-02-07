/*
 * SmartPID Controller hardware diagnostic tool
 *
 *  Copyright (C) 2016 Arzaman
 */

#ifndef HWTEST_H_
#define HWTEST_H_

/* Initialize the diagnostic task
 * This function is called at the start of firmware execution.
 * Parameters: none
 * Returns: none
 */
void hwTestSetup();

/* Process the diagnostic task
 * This function is called repeatedly (with no specified periodicity) during
 * firmware execution; its task is display a graphical user interface and accept
 * user input to perform diagnostic tests on hardware components.
 * Parameters: none
 * Returns: none
 */
void hwTestLoop(void);

#endif
