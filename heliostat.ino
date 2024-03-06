#include <Arduino.h>
#include "A4988.h"
#include "U8glib.h"
#include "Wire.h"
#include "Time.h"
#include "DS1307RTC.h"
#include <math.h>
#include <AccelStepper.h>


char timebuf[10];
char datebuf[10];

int Step = 3; 
int Dire  = 2; 

int MS1 = 7; 
int MS2 = 6; 
int MS3 = 5; 
int Dir2 = 8;
int Step2 = 9;
int MS3_2 = 10;
int MS2_2 = 11;
int MS1_2 = 12;
const int spr = 200; //Steps per revolution
int RPM = 100; //Motor Speed in revolutions per minute
int Microsteps = 4; //Stepsize

A4988 stepper1(spr, Dire, Step, MS1, MS2, MS3);
A4988 stepper2(spr, Dir2, Step2, MS1_2, MS2_2, MS3_2);

double prev_alpha = 0.0;
double prev_azimuth = 0.0;

double alpha_difference_sum = 0.0;
double azimuth_difference_sum = 0.0;

struct SolarData {
  double alpha;
  double azimuth;
};

void setup() {
  Serial.begin(9600);
  pinMode(Step, OUTPUT); 
  pinMode(Dire,  OUTPUT); 
  digitalWrite(Step, LOW); 
  digitalWrite(Dire, LOW);
  pinMode(Step2, OUTPUT); 
  pinMode(Dir2,  OUTPUT); 
  digitalWrite(Step2, LOW);
  digitalWrite(Dir2, LOW);
  
  stepper1.begin(RPM, Microsteps);
  stepper2.begin(RPM, Microsteps);

  SolarData initial_solar_data = position();
  prev_alpha = initial_solar_data.alpha;
  prev_azimuth = initial_solar_data.azimuth;

}

int timeFromStartOfYear() {
  tmElements_t tm;
  RTC.read(tm);

  int day = tm.Day;
  int month = tm.Month;
  
  static int daysInMonthNormalYear[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  static int daysInMonthLeapYear[] = {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
    
  const int* daysInMonth = ((tm.Year % 4 == 0 && tm.Year % 100 != 0) || (tm.Year % 400 == 0)) ? daysInMonthLeapYear : daysInMonthNormalYear;   // check for leap year

  int d = daysInMonth[month] + day;
  return d;
}

SolarData position() {
  tmElements_t tm;
  RTC.read(tm);

  //Local time
  int LT = tm.Hour * 60 + tm.Minute;
  int timezone = 1.0;
  double longitude = 16.89;
  double latitude = 52.4; 
  //Local standard time meridian
  double LSTM = 15.0 * timezone; // 1h

  double d = timeFromStartOfYear(); // days since the start of the year
  double B = (360.0/365.0) * (d - 81) * PI / 180.0;

  // Equation of Time
  double EoT = 9.87 * sin(2 * B) - 7.53 * cos(B) - 1.5 * sin(B); //minutes 

  //Time Correction Factor
  double TC = EoT + 4 * (longitude - LSTM);

  //Local Solar Time
  double LST = (LT / 60.0) + (TC / 60.0); //local time and time correction in hours

  //Hour Angle
  double HRA_degrees = (15 * (LST - 12));  //in degrees
  double HRA_rads = (15 * (LST - 12)) * PI / 180.0;  //in rads

  //Declination
  double delta_degrees = 23.45 * sin((360.0/365.0 * (d - 81.0) * PI / 180.0)); //in degrees
  double delta_rads = delta_degrees * (PI / 180.0); //in rads

  //Elevation / Altitude
  double latitude_rads = latitude * (PI / 180.0);

  double alpha = asin((sin(delta_rads) * sin(latitude_rads)) + (cos(delta_rads) * cos(latitude_rads) * cos(HRA_rads))); //in rads
  double alpha_degrees = alpha * (180.0 / PI);

  double zenith = (PI / 2 - alpha) * 180.0 / PI; //in degrees

  //Azimuth
  double azimuth = acos(((sin(delta_rads) * cos(latitude_rads)) - (cos(delta_rads) * sin(latitude_rads) * cos(HRA_rads))) / cos(alpha)); //in rads

  //double azimuth_degrees = azimuth * (180.0 / PI);

  double azimuth_correction = (2 * PI - azimuth) * (180.0 / PI);  

  Serial.print("Hour: ");
  Serial.println(tm.Hour);
  Serial.print("Minute: ");
  Serial.println(tm.Minute);

  Serial.print("Local time: ");
  Serial.println(LT);
  Serial.print("d: ");
  Serial.println(d);
  Serial.print("B: ");
  Serial.println(B, 6);
  Serial.print("EoT: ");
  Serial.println(EoT, 6);
  Serial.print("TC: ");
  Serial.println(TC, 6);
  Serial.print("LST: ");
  Serial.println(LST, 6);
  Serial.print("HRA: ");
  Serial.println(HRA_degrees, 6);
  Serial.print("HRA to rads: ");
  Serial.println(HRA_rads, 6);

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);
  Serial.print("Latitude to rads: ");
  Serial.println(latitude_rads, 6);


  Serial.print("Declination: ");
  Serial.println(delta_degrees, 6);
  Serial.print("Declination in rads: ");
  Serial.println(delta_rads, 6);

  Serial.print("Zenith: ");
  Serial.println(zenith, 6);
  
  Serial.print("Alpha: ");
  Serial.println(alpha, 6);
  Serial.print("Alpha in degrees: ");
  Serial.println(alpha_degrees, 6);
  Serial.print("Azimuth: ");
  Serial.println(azimuth, 6);

  Serial.print("Azimuth test: ");
  Serial.println(azimuth_correction, 6);
  Serial.print("---------\n");

  SolarData solar_data;
  solar_data.alpha = alpha;
  solar_data.azimuth = azimuth;
  return solar_data;
}

void checkAlphaAndAzimuth() {

  SolarData solar_data = position();
  double current_alpha = solar_data.alpha;
  double current_azimuth = solar_data.azimuth;

  double alpha_difference = current_alpha - prev_alpha;
  double azimuth_difference = current_azimuth - prev_azimuth;

  // Add the absolute differences to the accumulated difference
  alpha_difference_sum += abs(alpha_difference);
  azimuth_difference_sum += abs(azimuth_difference);

  Serial.print("Previous Alpha: ");
  Serial.println(prev_alpha, 6);
  Serial.print("Previous Azimuth: ");
  Serial.println(prev_azimuth, 6);

  int signed_degree = (prev_alpha < current_alpha) ? 1 : -1; //check if direction needs to be changed for alpha

  // Update previous alpha and azimuth values
  prev_alpha = current_alpha;
  prev_azimuth = current_azimuth;

  Serial.print("Alpha Difference Sum before: ");
  Serial.println(alpha_difference_sum, 6);
  Serial.print("Azimuth Difference Sum before: ");
  Serial.println(azimuth_difference_sum, 6);

  // Check if accumulated difference exceeds one degree
  if (alpha_difference_sum >= 0.0174532925) {  // 1 degree = 0,0174532925 rads
    stepper1.rotate(signed_degree);
    Serial.print("Rotate stepper 1\n");

    alpha_difference_sum -= 0.0174532925;
  }

  if (azimuth_difference_sum >= 0.0174532925) {
    stepper2.rotate(1);
    Serial.print("Rotate stepper 2\n");


    azimuth_difference_sum -= 0.0174532925;
}


  Serial.print("Current Alpha: ");
  Serial.println(current_alpha, 6);
  Serial.print("Current Azimuth: ");
  Serial.println(current_azimuth, 6);
  Serial.print("Alpha Difference Sum: ");
  Serial.println(alpha_difference_sum, 6);
  Serial.print("Azimuth Difference Sum: ");
  Serial.println(azimuth_difference_sum, 6);
  Serial.print("=================\n");
}


void loop() {
  
  static unsigned long last_check_time = 0;
  unsigned long current_time = millis();
  if (current_time - last_check_time >= 60000) { //60s +000
    checkAlphaAndAzimuth();
    last_check_time = current_time;
  }
}