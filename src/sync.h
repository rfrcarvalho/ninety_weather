
#ifndef SYNC_H
#define SYNC_H
	
#include "pebble_app.h"
#include "pebble_os.h"

	
enum {
	TUPLE_STATE = 0x100, // TUPLE_STRING		
	TUPLE_MISSED_CALLS = 0x101, // TUPLE_INT
	TUPLE_UNREAD_SMS = 0x102, // TUPLE_INT
	// TUPLE_BATTERY_LEVEL = 0x103, // TUPLE_INT
	// TUPLE_NETWORK = 0x3, // TUPLE_INT
};

enum { REQUEST_DATA_KEY = 0x10, };
enum { REQUEST_MISSED = 0x101, REQUEST_UNREAD = 0x102, };// TUPLE_INTEGER
enum LinkStatus
{
	LinkStatusUnknown = 0,
	LinkStatusFailed = 1,
	LinkStatusOK = 2,
};

static struct Data {
	const char* info;
	uint8_t missed;
  	uint8_t unread;
	enum LinkStatus link_status;
} data = {.info = "XXX calls, XXX sms. #00", .missed = 11, .unread=11};


static bool callbacks_registered;
static AppMessageCallbacksNode app_callbacks;

void request_phone_state();

#endif 