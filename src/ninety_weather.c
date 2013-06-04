#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "stdlib.h"
#include "string.h"
#include "config.h"
#include "my_math.h"
#include "suncalc.h"
#include "http.h"
#include "util.h"
#include "link_monitor.h"

// This is the Default APP_ID to work with old versions of httpebble
//#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x29, 0x08, 0xF1, 0x7C, 0x3F, 0xAC}

PBL_APP_INFO(MY_UUID,
	     "91 Weather", "rfrcarvalho",
	     1, 3, /* App major/minor version */
	     RESOURCE_ID_IMAGE_MENU_ICON,
	     APP_INFO_WATCH_FACE);

Window window;
TextLayer cwLayer; // The calendar week
TextLayer text_sunrise_layer;
TextLayer text_sunset_layer;
TextLayer text_temperature_layer;
TextLayer DayOfWeekLayer;
BmpContainer background_image;
BmpContainer time_format_image;

static int our_latitude, our_longitude, our_timezone = 99;
static bool located = false;
static bool calculated_sunset_sunrise = false;
static bool temperature_set = false;

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};


#define TOTAL_WEATHER_IMAGES 1
BmpContainer weather_images[TOTAL_WEATHER_IMAGES];

const int WEATHER_IMAGE_RESOURCE_IDS[] = {
    RESOURCE_ID_IMAGE_CLEAR_DAY,
	RESOURCE_ID_IMAGE_CLEAR_NIGHT,
	RESOURCE_ID_IMAGE_RAIN,
	RESOURCE_ID_IMAGE_SNOW,
	RESOURCE_ID_IMAGE_SLEET,
	RESOURCE_ID_IMAGE_WIND,
	RESOURCE_ID_IMAGE_FOG,
	RESOURCE_ID_IMAGE_CLOUDY,
	RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DAY,
	RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT,
	RESOURCE_ID_IMAGE_NO_WEATHER
};

#define TOTAL_DATE_DIGITS 8
BmpContainer date_digits_images[TOTAL_DATE_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_TIME_DIGITS 4
BmpContainer time_digits_images[TOTAL_TIME_DIGITS];

void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin) {
  layer_remove_from_parent(&bmp_container->layer.layer);
  bmp_deinit_container(bmp_container);

  bmp_init_container(resource_id, bmp_container);

  GRect frame = layer_get_frame(&bmp_container->layer.layer);
  frame.origin.x = origin.x;
  frame.origin.y = origin.y;
  layer_set_frame(&bmp_container->layer.layer, frame);

  layer_add_child(&window.layer, &bmp_container->layer.layer);
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}


int moon_phase(int y, int m, int d)
{
    /*
      calculates the moon phase (0-7), accurate to 1 segment.
      0 = > new moon.
      4 => full moon.
      */
    int c,e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25*y;
    e = 30.6*m;
    jd = c+e+d-694039.09;  	/* jd is total days elapsed */
    jd /= 29.53;        	/* divide by the moon cycle (29.53 days) */
    b = jd;		   			/* int(jd) -> b, take integer part of jd */
    jd -= b;		   		/* subtract integer part to leave fractional part of original jd */
    b = jd*8 + 0.5;	   		/* scale fraction from 0-8 and round by adding 0.5 */
    b = b & 7;		   		/* 0 and 8 are the same so turn 8 into 0 */
    return b;
}

void adjustTimezone(float* time) 
{
  *time += our_timezone;
  if (*time > 24) *time -= 24;
  if (*time < 0) *time += 24;
}

void updateSunsetSunrise()
{
	// Calculating Sunrise/sunset with courtesy of Michael Ehrmann
	// https://github.com/mehrmann/pebble-sunclock
	static char sunrise_text[] = "00:00";
	static char sunset_text[]  = "00:00";
	
	PblTm pblTime;
	get_time(&pblTime);

	char *time_format;

	if (clock_is_24h_style()) 
	{
	  time_format = "%R";
	} 
	else 
	{
	  time_format = "%I:%M";
	}

	float sunriseTime = calcSunRise(pblTime.tm_year, pblTime.tm_mon+1, pblTime.tm_mday, our_latitude / 10000, our_longitude / 10000, 91.0f);
	float sunsetTime = calcSunSet(pblTime.tm_year, pblTime.tm_mon+1, pblTime.tm_mday, our_latitude / 10000, our_longitude / 10000, 91.0f);
	adjustTimezone(&sunriseTime);
	adjustTimezone(&sunsetTime);
	
	if (!pblTime.tm_isdst) 
	{
	  sunriseTime+=1;
	  sunsetTime+=1;
	} 

	pblTime.tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
	pblTime.tm_hour = (int)sunriseTime;
	string_format_time(sunrise_text, sizeof(sunrise_text), time_format, &pblTime);
	text_layer_set_text(&text_sunrise_layer, sunrise_text);

	pblTime.tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
	pblTime.tm_hour = (int)sunsetTime;
	string_format_time(sunset_text, sizeof(sunset_text), time_format, &pblTime);
	text_layer_set_text(&text_sunset_layer, sunset_text);
}

unsigned short the_last_hour = 25;

void request_weather();

void failed(int32_t cookie, int http_status, void* context) {
	
	if((cookie == 0 || cookie == WEATHER_HTTP_COOKIE) && !temperature_set) {
		set_container_image(&weather_images[0], WEATHER_IMAGE_RESOURCE_IDS[10], GPoint(12, 5));
		text_layer_set_text(&text_temperature_layer, "---°");
	}
	
	//link_monitor_handle_failure(http_status);
	
	//Re-request the location and subsequently weather on next minute tick
	located = false;
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
	
	if(cookie != WEATHER_HTTP_COOKIE) return;
	
	Tuple* icon_tuple = dict_find(received, WEATHER_KEY_ICON);
	if(icon_tuple) {
		int icon = icon_tuple->value->int8;
		if(icon >= 0 && icon < 10) {
			set_container_image(&weather_images[0], WEATHER_IMAGE_RESOURCE_IDS[icon], GPoint(12, 5));  // ---------- Weather Image
		} else {
			set_container_image(&weather_images[0], WEATHER_IMAGE_RESOURCE_IDS[10], GPoint(12, 5));
		}
	}
	
	Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE);
	if(temperature_tuple) {
		
		static char temp_text[5];
		memcpy(temp_text, itoa(temperature_tuple->value->int16), 4);
		int degree_pos = strlen(temp_text);
		memcpy(&temp_text[degree_pos], "°", 3);
		text_layer_set_text(&text_temperature_layer, temp_text);
		temperature_set = true;
	}
	
	//link_monitor_handle_success()
}

void location(float latitude, float longitude, float altitude, float accuracy, void* context) {
	// Fix the floats
	our_latitude = latitude * 10000;
	our_longitude = longitude * 10000;
	located = true;
	request_weather();
}

void reconnect(void* context) {
	located = false;
	request_weather();
}

void receivedtime(int32_t utc_offset_seconds, bool is_dst, uint32_t unixtime, const char* tz_name, void* context)
{
	our_timezone = (utc_offset_seconds / 3600) - 1;
	
	if (located && our_timezone != 99 && !calculated_sunset_sunrise)
    {
        updateSunsetSunrise();
	    calculated_sunset_sunrise = true;
    }
}

void update_display(PblTm *current_time) {
  
  unsigned short display_hour = get_display_hour(current_time->tm_hour);
  
  //Hour
  set_container_image(&time_digits_images[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(4, 94));
  set_container_image(&time_digits_images[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(37, 94));
  
  //Minute
  set_container_image(&time_digits_images[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(80, 94));
  set_container_image(&time_digits_images[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(111, 94));
   
  if (the_last_hour != display_hour){
	  
	  // Day of week
	  text_layer_set_text(&DayOfWeekLayer, DAY_NAME_LANGUAGE[current_time->tm_wday]); 
	  
	  // Day
	  set_container_image(&date_digits_images[0], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday/10], GPoint(day_month_x[0], 71));
	  set_container_image(&date_digits_images[1], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday%10], GPoint(day_month_x[0] + 13, 71));
	  
	  // Month
	  set_container_image(&date_digits_images[2], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)/10], GPoint(day_month_x[1], 71));
	  set_container_image(&date_digits_images[3], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)%10], GPoint(day_month_x[1] + 13, 71));
	  
	  // Year
	  set_container_image(&date_digits_images[4], DATENUM_IMAGE_RESOURCE_IDS[((1900+current_time->tm_year)%1000)/10], GPoint(day_month_x[2], 71));
	  set_container_image(&date_digits_images[5], DATENUM_IMAGE_RESOURCE_IDS[((1900+current_time->tm_year)%1000)%10], GPoint(day_month_x[2] + 13, 71));
		
	  if (!clock_is_24h_style()) {
		if (current_time->tm_hour >= 12) {
		  set_container_image(&time_format_image, RESOURCE_ID_IMAGE_PM_MODE, GPoint(118, 140));
		} else {
		  layer_remove_from_parent(&time_format_image.layer.layer);
		  bmp_deinit_container(&time_format_image);
		}

		if (display_hour/10 == 0) {
		  layer_remove_from_parent(&time_digits_images[0].layer.layer);
		  bmp_deinit_container(&time_digits_images[0]);
		}
	  }
	  
	  // -------------------- Calendar week  
	  static char cw_text[] = "XX00";
	  string_format_time(cw_text, sizeof(cw_text), TRANSLATION_CW , current_time);
	  text_layer_set_text(&cwLayer, cw_text); 
	  // ------------------- Calendar week  
	  
	  the_last_hour = display_hour;
  }
	
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;	

    update_display(t->tick_time);
	
	if(!located || !(t->tick_time->tm_min % 15))
	{
		// Every 15 minutes, request updated weather
		http_location_request();
		
		// Every 15 minutes, request updated time
		http_time_request();
	}
	else
	{
		// Every minute, ping the phone
		link_monitor_ping();
	}
	
	if(!calculated_sunset_sunrise)
    {
	    // Start with some default values
	    text_layer_set_text(&text_sunrise_layer, "Wait!");
	    text_layer_set_text(&text_sunset_layer, "Wait!");
    }
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "91 Weather");
  window_stack_push(&window, true /* Animated */);
  
  window_set_background_color(&window, GColorBlack);
  
  resource_init_current_app(&APP_RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image);
  layer_add_child(&window.layer, &background_image.layer.layer);
	
  if (clock_is_24h_style()) {
    bmp_init_container(RESOURCE_ID_IMAGE_24_HOUR_MODE, &time_format_image);

    time_format_image.layer.layer.frame.origin.x = 118;
    time_format_image.layer.layer.frame.origin.y = 140;

    layer_add_child(&window.layer, &time_format_image.layer.layer);
  }

  // Calendar Week Text
  text_layer_init(&cwLayer, GRect(108, 50, 80 /* width */, 30 /* height */));
  layer_add_child(&background_image.layer.layer, &cwLayer.layer);
  text_layer_set_text_color(&cwLayer, GColorWhite);
  text_layer_set_background_color(&cwLayer, GColorClear);
  text_layer_set_font(&cwLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  // Sunrise Text
  text_layer_init(&text_sunrise_layer, window.layer.frame);
  text_layer_set_text_color(&text_sunrise_layer, GColorWhite);
  text_layer_set_background_color(&text_sunrise_layer, GColorClear);
  layer_set_frame(&text_sunrise_layer.layer, GRect(7, 152, 100, 30));
  text_layer_set_font(&text_sunrise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(&window.layer, &text_sunrise_layer.layer);

  // Sunset Text
  text_layer_init(&text_sunset_layer, window.layer.frame);
  text_layer_set_text_color(&text_sunset_layer, GColorWhite);
  text_layer_set_background_color(&text_sunset_layer, GColorClear);
  layer_set_frame(&text_sunset_layer.layer, GRect(110, 152, 100, 30));
  text_layer_set_font(&text_sunset_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(&window.layer, &text_sunset_layer.layer); 
  
  // Text for Temperature
  text_layer_init(&text_temperature_layer, window.layer.frame);
  text_layer_set_text_color(&text_temperature_layer, GColorWhite);
  text_layer_set_background_color(&text_temperature_layer, GColorClear);
  layer_set_frame(&text_temperature_layer.layer, GRect(68, 3, 64, 68));
  text_layer_set_font(&text_temperature_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_40)));
  layer_add_child(&window.layer, &text_temperature_layer.layer);  
  
  // Day of week text
  text_layer_init(&DayOfWeekLayer, GRect(5, 62, 130 /* width */, 30 /* height */));
  layer_add_child(&background_image.layer.layer, &DayOfWeekLayer.layer);
  text_layer_set_text_color(&DayOfWeekLayer, GColorWhite);
  text_layer_set_background_color(&DayOfWeekLayer, GColorClear);
  text_layer_set_font(&DayOfWeekLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  http_register_callbacks((HTTPCallbacks){.failure=failed,.success=success,.reconnect=reconnect,.location=location,.time=receivedtime}, (void*)ctx);
   
  // Avoids a blank screen on watch start.
  PblTm tick_time;

  get_time(&tick_time);
  update_display(&tick_time);

}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&background_image);
  bmp_deinit_container(&time_format_image);

  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    bmp_deinit_container(&date_digits_images[i]);
  }

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    bmp_deinit_container(&time_digits_images[i]);
  }

}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    },
	.messaging_info = {
		.buffer_sizes = {
			.inbound = 124,
			.outbound = 124,
		}
	}
  };
  app_event_loop(params, &handlers);
}

void request_weather() {
	
	if(!located) {
		http_location_request();
		return;
	}
	
	// Build the HTTP request
	DictionaryIterator *body;
	HTTPResult result = http_out_get("http://www.zone-mr.net/api/weather.php", WEATHER_HTTP_COOKIE, &body);
	if(result != HTTP_OK) {
		return;
	}
	
	dict_write_int32(body, WEATHER_KEY_LATITUDE, our_latitude);
	dict_write_int32(body, WEATHER_KEY_LONGITUDE, our_longitude);
	dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM);
	
	// Send it.
	if(http_out_send() != HTTP_OK) {
		return;
	}
}
