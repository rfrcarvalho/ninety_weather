//Monitors the connection to the phone
//Can alert the user if it's broken
#ifndef LINKMON_H
#define LINKMON_H_H

#include "sync.h"
	
void link_monitor_ping();
void link_monitor_handle_failure(int error, struct Data* data);
void link_monitor_handle_success(struct Data* data);

#endif