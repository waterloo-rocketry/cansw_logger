/*
 * File:   health_check.h
 * Author: Kavin
 *
 * Created on May 21, 2022, 11:42 AM
 */

#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include <stdbool.h>

bool check_bus_current_error(void);
bool check_bus_voltage_error(void);

#endif /* HEALTH_CHECK_H */
