#ifndef SUNCALC_H
#define SUNCALC_H

#define ZENITH_OFFICIAL 90.83
#define ZENITH_CIVIL    96.0
#define ZENITH_NAUTICAL 102.0
#define ZENITH_ASTRONOMICAL 108.0

float calcSun(int year, int month, int day, float latitude, float longitude, int sunset, float zenith);
float calcSunRise(int year, int month, int day, float latitude, float longitude, float zenith);
float calcSunSet(int year, int month, int day, float latitude, float longitude, float zenith);
int moon_phase(int y, int m, int d);
void adjustTimezone(float* time);

#endif