/*
 * File:   health_check.h
 * Author: Kavin
 *
 * Created on May 21, 2022, 11:42 AM
 */

#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include <stdbool.h>

#define BATT_CURR_SCALAR 8 // Will give current in mA
#define BATTERY_CURRENT_THRESHOLD 300

bool check_bus_current_error(void);

#endif /* HEALTH_CHECK_H */
