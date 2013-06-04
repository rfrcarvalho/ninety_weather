#ifndef CONFIG_H
#define CONFIG_H

//NOTE: longitude is positive for East and negative for West
#define LATITUDE    				51.0
#define LONGITUDE 					8.0
#define TIMEZONE 					0
#define DAY_NAME_LANGUAGE 			DAY_NAME_ENGLISH 		// Valid values: DAY_NAME_ENGLISH, DAY_NAME_GERMAN, DAY_NAME_FRENCH
#define MOONPHASE_NAME_LANGUAGE 	MOONPHASE_TEXT_ENGLISH 	// Valid values: MOONPHASE_TEXT_ENGLISH, MOONPHASE_TEXT_GERMAN, MOONPHASE_TEXT_FRENCH
#define day_month_x 				day_month_month_first 	// Valid values: day_month_month_first, day_month_day_first
#define TRANSLATION_CW 				"CW%V" 					// Translation for the calendar week (e.g. "CW%V")

// Any of "us", "ca", "uk" (for idiosyncratic US, Candian and British measurements),
// "si" (for pure metric) or "auto" (determined by the above latitude/longitude)
#define UNIT_SYSTEM "auto"

// POST variables
#define WEATHER_KEY_LATITUDE 1
#define WEATHER_KEY_LONGITUDE 2
#define WEATHER_KEY_UNIT_SYSTEM 3
	
// Received variables
#define WEATHER_KEY_ICON 1
#define WEATHER_KEY_TEMPERATURE 2

#define WEATHER_HTTP_COOKIE 1949327671
#define TIME_HTTP_COOKIE 1131038282

// ---- Constants for all available languages ----------------------------------------

const int day_month_day_first[] = {
	55,
	87,
	115
};

const int day_month_month_first[] = {
	87,
	55,
	115
};

const char *DAY_NAME_ENGLISH[] = {
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT"
};

const char *MOONPHASE_TEXT_ENGLISH[] = {
	"NM",
	"WXC",
	"FQ",
	"WXG",
	"FM",
	"WNG",
	"TQ",
	"WNC"
};


#endif