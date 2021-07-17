// 
// Esp8266 Starter System - A Self-contained starter system for development of ESP8266 based projects
//
// This system includes the following base capabilities
// - WiFi support, including using WiFiManager to save credentials
// - A simple LittleFS based preferences system with web edit/save of configuration file (for non-wifi settings)
// - A fully functioning system control website with 
//    1) a system status view
//    2) a view/edit settings capability
//    3) an event history (logging) capability
//    4) some basic system controls (reset, clear WiFi settings, send email msg, etc)
// - Code to send email using smt2go.com smtp services as well as integration of email settings into prferences system
// - A simple event history system that records event messages strings along with a timestamp of when event happened
// - Misc date and time utilities to help with displaying current date and time and elapsedmillis timestamps
// 
// Dependencies - ESP8266WiFi, DNSServer, ESP8266WebServer, WiFiManager, 
//                NTPClient, WiFiClient, LittleFS, time.h, TZ.h
//                ElapsedMillis
// All of these are probably available through the Arduino IDE Library Manager, but some may need to be installed by
// .zip files downloaded from github. I seem to have downloaded .zips for wifimanager, ntpclient, and elapsedmillis
// but I think they're all available through library manager now.
//
// This project was developed on a NodeMCU 1.0 ESP-12E module using the Arduino IDE 1.8.15
// with this additional board manager: http://arduino.esp8266.com/stable/package_esp8266com_index.json
//

#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <time.h>
#include <sys/time.h>
#include <TZ.h>

#define MYTZ TZ_America_New_York

#define BUILTIN_LED_ON      LOW // Note: Write 0 to turn ON built-in LED on NodeMCU
#define BUILTIN_LED_OFF    HIGH

// Define some timing values (all in milliseconds)
#define WEB_SESSION_TIMEOUT    (unsigned long) 300*1000 // New session if > 5 mins since activity (also blink if active)
#define EMAIL_ERROR_BLINK_TIME (unsigned long)  60*1000 // blink for 1 min after email error
 
#define ACTIVITY_LED_ON_TIME   50   //  ms
#define ACTIVITY_LED_OFF_TIME 100   //  ms

#define SLOW_BLINK_ON_TIME 1000 // times in ms
#define SLOW_BLINK_RATE    2000 //
#define FAST_BLINK_ON_TIME  150 //
#define FAST_BLINK_RATE     300 // 

#define MILLIS_IN_A_DAY    86400000
#define MILLIS_IN_A_WEEK   604800000

elapsedMillis timeSinceSystemStarted = 0;
elapsedMillis timeSinceSoftReset = 0;
elapsedMillis timeSinceWebSessionStarted  = MILLIS_IN_A_DAY;
elapsedMillis timeSinceLastWebActivity    = MILLIS_IN_A_DAY;
elapsedMillis timeSinceLastEmailAttempt   = MILLIS_IN_A_DAY;
elapsedMillis timeSinceLastEmailSendError = MILLIS_IN_A_DAY;
elapsedMillis timeSinceLastEmailSentOk    = MILLIS_IN_A_DAY;
elapsedMillis timeSinceStatusBlinkStarted = MILLIS_IN_A_DAY;

// Additions here my need to be re initialized in clearStats()
unsigned long int softResetCount=0;
unsigned long int email1SentCount=0;
unsigned long int email2SentCount=0;
unsigned long int emailSupressedCount=0;
unsigned long int emailErrorCount=0;

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(300);

  digitalWrite(LED_BUILTIN, BUILTIN_LED_OFF); 

  initializeEventHistoryStorage();
  
  initializePreferencesManager();
  
  // Use WiFiManager to handle persistence of ssid and password.
  // If ssid and password is saved, wifi manager will autoconnecct
  // On first time or if password changed, wifi mgr  starts in AP mode
  // which allows connection from any device to configure ssid and password.
  // Access point server IP defaults to 192.168.4.1
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect(); // auto generated name ESP + ChipID
  
  // if you get here you have connected to the WiFi
  String ipString = WiFi.localIP().toString();
  Serial.println("WiFi Connected, IP Address is " + ipString);

  setupWebserver();
  Serial.println("HTTP server started");
  
  // Set time zone for localtime() calls
  configTime(MYTZ, "pool.ntp.org");
  
  Serial.println("Initialization Complete");
}

void loop() {
  
  updateStatusLED();
    
  handleWebseverProcessing();

  if ((timeSinceLastWebActivity < WEB_SESSION_TIMEOUT) && (timeSinceWebSessionStarted > MILLIS_IN_A_DAY))
    timeSinceWebSessionStarted = 0;
  else if (timeSinceLastWebActivity > WEB_SESSION_TIMEOUT)
    timeSinceWebSessionStarted = MILLIS_IN_A_DAY;
 
  // Prevent timers from wrapping
  if (timeSinceWebSessionStarted > MILLIS_IN_A_WEEK)
    timeSinceWebSessionStarted = MILLIS_IN_A_DAY;
  if (timeSinceLastWebActivity > MILLIS_IN_A_WEEK)
    timeSinceLastWebActivity = MILLIS_IN_A_DAY;
  if (timeSinceLastEmailAttempt > MILLIS_IN_A_WEEK)
    timeSinceLastEmailAttempt = MILLIS_IN_A_DAY;
  if (timeSinceLastEmailSendError > MILLIS_IN_A_WEEK)
    timeSinceLastEmailSendError = MILLIS_IN_A_DAY;
  if (timeSinceLastEmailSentOk > MILLIS_IN_A_WEEK)
    timeSinceLastEmailSentOk = MILLIS_IN_A_DAY;  
  if (timeSinceStatusBlinkStarted > MILLIS_IN_A_WEEK)
    timeSinceStatusBlinkStarted = MILLIS_IN_A_DAY;
}


void updateStatusLED() {
  uint8_t LED_State = BUILTIN_LED_ON;
  
  if (timeSinceLastEmailSendError < EMAIL_ERROR_BLINK_TIME) {
    // Blink LED fast for a while after  email send error
    if (timeSinceStatusBlinkStarted > FAST_BLINK_ON_TIME) {
        LED_State = BUILTIN_LED_OFF;
        if (timeSinceStatusBlinkStarted > FAST_BLINK_RATE)
          timeSinceStatusBlinkStarted = 0;
    }
  }  else if (timeSinceLastWebActivity < WEB_SESSION_TIMEOUT) {
    // Blink LED slowly if a web session is active
    if (timeSinceStatusBlinkStarted > SLOW_BLINK_ON_TIME) {
        LED_State = BUILTIN_LED_OFF;
        if (timeSinceStatusBlinkStarted > SLOW_BLINK_RATE)
          timeSinceStatusBlinkStarted = 0;
    }
  } else {
     // No recent emails sent keep LED off
     LED_State = BUILTIN_LED_OFF;
  } 
  
  digitalWrite(LED_BUILTIN, LED_State);
}

void sendMailMessageType1() {
  timeSinceLastEmailAttempt = 0;
  if (getPreferencesBoolean("sendMailEnabled") == false) {
    emailSupressedCount++;
    addEventHistoryEntry("Email not sent. SendMail is disabled in settings");
    return;
  }
  
  byte ok = sendEmail(1);
  
  if (!ok) {
     emailErrorCount++;
     timeSinceLastEmailSendError=0;
  } else {
     flashBuiltinLED(ACTIVITY_LED_ON_TIME, ACTIVITY_LED_OFF_TIME);
     email1SentCount++;
     timeSinceLastEmailSentOk=0;
     addEventHistoryEntry("Email 1 message was sent successfully");
  }
}

void sendMailMessageType2() {
  timeSinceLastEmailAttempt = 0;
  if (getPreferencesBoolean("sendMailEnabled") == false) {
    emailSupressedCount++;
    addEventHistoryEntry("Email not sent. SendMail is disabled in settings");
    return;
  }
  byte ok = sendEmail(2);
  if (!ok) {
     emailErrorCount++;
     timeSinceLastEmailSendError=0;
  } else {
     flashBuiltinLED(ACTIVITY_LED_ON_TIME, ACTIVITY_LED_OFF_TIME);
     email2SentCount++;
     timeSinceLastEmailSentOk=0;
     addEventHistoryEntry("Email 2 message was sent successfully");
  }
}

void doSoftReset() {

  timeSinceWebSessionStarted  = MILLIS_IN_A_DAY;
  timeSinceLastWebActivity    = MILLIS_IN_A_DAY;
  timeSinceLastEmailAttempt   = MILLIS_IN_A_DAY;
  timeSinceLastEmailSendError = MILLIS_IN_A_DAY;
  timeSinceLastEmailSentOk    = MILLIS_IN_A_DAY;

  email1SentCount=0;
  email2SentCount=0;
  emailSupressedCount=0;
  emailErrorCount=0;

  initializeEventHistoryStorage();
  softResetCount++;
  timeSinceSoftReset = 0;
  addEventHistoryEntry("Soft restart completed");
}

void flashBuiltinLED(int onTimeDelay, int offTimeDelay) {
      digitalWrite(LED_BUILTIN, BUILTIN_LED_ON);
      delay(onTimeDelay);
      digitalWrite(LED_BUILTIN, BUILTIN_LED_OFF);
      delay(offTimeDelay);
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

unsigned long freeMemory() {
#if defined(ARDUINO_ARCH_AVR)
  char top;
  return &top - (__brkval ? __brkval : __malloc_heap_start);
#elif defined(ARDUINO_ARCH_SAMD)
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(TEENSYDUINO)
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(ESP8266)
  return system_get_free_heap_size();
#elif defined(ESP32)
  return ESP.getFreeHeap();
#elif defined(EPOXY_DUINO)
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
#else
  #error Unsupported platform
#endif
}
