//
// dateTimeUtils.ino - A collection of utility functions for displaying date and time.
//
// API
//  unsigned long getCurrentEpochTime() 
//         String elapsedMillisToString(unsigned long val)              -> "5d 13h 42m 25s"
//         String elapsedMillisToVerboseString(unsigned long val)       -> "5 days, 13 hrs, 42 mins, 25 seconds"
//         String getLocalTimeString()                                  -> "Sat Jul 17  10:25:26"
//         String elapsedMillisToDateString(unsigned long milliseconds) -> "Sat Jul 17  10:25:26"
//         String epochTimeToDateString(unsigned long epochTime)        -> "Sat Jul 17  10:25:26"
//

// macros from DateTime.h 
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

String elapsedMillisToString(unsigned long val) {
  char str[500];
  String retStr;

 val /= 1000;
 int days = elapsedDays(val);
 int hours = numberOfHours(val);
 int minutes = numberOfMinutes(val);
 int seconds = numberOfSeconds(val);
 
 //hours += (24*days);
 if (days > 0)
  sprintf(str,"%dd %dh %dm %ds", days, hours, minutes, seconds);
 else if (hours > 0)
  sprintf(str,"%dh %dm %ds", hours, minutes, seconds);
 else if (minutes > 0)
  sprintf(str,"%dm %ds", minutes, seconds);
 else
  sprintf(str,"%ds", seconds);
 retStr = (String) str;
 return retStr;
}

String elapsedMillisToVerboseString(unsigned long val) {
 static char str[500];
 static String retStr;
 val /= 1000; 
 int days = elapsedDays(val);
 int hours = numberOfHours(val);
 int minutes = numberOfMinutes(val);
 int seconds = numberOfSeconds(val);
 
 //hours += (24*days);
 if (days > 0)
  sprintf(str,"%d days, %d hrs, %d mins, %d seconds", days, hours, minutes, seconds);
 else if (hours > 0)
  sprintf(str,"%d hrs, %d mins, %d seconds", hours, minutes, seconds);
 else if (minutes > 0)
  sprintf(str,"%d minutes, %d seconds", minutes, seconds);
 else
  sprintf(str,"%2d seconds", seconds);
 retStr = (String) str;
 return retStr;
}

// Function that gets current epoch time
unsigned long getCurrentEpochTime() {
  time_t now;
  struct tm timeinfo;
  //if (!localtime(&timeinfo)) {
  //  //Serial.println("Failed to obtain time");
  //  return(0);
  //}
  time(&now);
  return now;
}

String getLocalTimeString() {
  static char str[80];
  static String retStr;
  
  time_t timeSinceEpoch = (time_t) getCurrentEpochTime();  
  strftime(str, 80, "%H:%M:%S", localtime(&timeSinceEpoch));
  retStr = (String) str;
  return retStr;
}

String elapsedMillisToDateString(unsigned long milliseconds) {
  static char str[80];
  static String retStr;
  
  time_t timeSinceEpoch = (time_t) (getCurrentEpochTime() - (milliseconds/1000));  
  strftime(str, 80, "%a %b %e  %H:%M:%S", localtime(&timeSinceEpoch));
  retStr = (String) str;
  return retStr;
}

String epochTimeToDateString(unsigned long epochTime) {
  static char str[80];
  static String retStr;
  
  time_t timeSinceEpoch = (time_t) epochTime;  
  strftime(str, 80, "%a %b %e  %H:%M:%S", localtime(&timeSinceEpoch));

  retStr = (String) str;
  return retStr;
}
