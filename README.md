# Esp8266Starter
A  relatively self-contained starter system for use in Esp8266 based projects.  

The idea is to use this project as a base when you start your project.  This base system offers a web control panel, settings edit/save, and event logging.  With these basic features already built/working, you can focus on coding the specifics of your project.

This system includes the following base capabilities
 - WiFi support (through Esp8266WiFi, including using WiFiManager to save credentials
 - A simple LittleFS based preferences system with web edit/save of configuration file (for non-wifi settings)
 - A fully functioning system control website with 
    1) a system status view
    2) a view/edit settings capability
    3) an event history (logging) capability
    4) some basic system controls (reset, clear WiFi settings, send email msg, etc)
    
 - Code to send email using smt2go.com smtp services as well as integration of email settings into preferences system
 - A simple event history system that records event messages strings along with a timestamp of when event happened
 - Misc date and time utilities to help with displaying current date and time and elapsedmillis timestamps
 
 Dependencies - ESP8266WiFi, DNSServer, ESP8266WebServer, WiFiManager, 
                NTPClient, WiFiClient, LittleFS, time.h, TZ.h (from Esp8266 core)
                ElapsedMillis

Web Control Panel Overview

Button         | Function
------------   | -------------
Home           | Displays basic operational status
History        | Links to a page with the Event History (see eventHistory.ino)
Settings       | Links to a page to edit/save the settings file managed by prefsMfr.ino  (uses html <textarea>)
Restart        | Calls doSoftReset() function in Esp8266Starter.ino to inits counters, timers, etc
Send Mail 1    | Results in a call to sendmail(1) through a helper function in Esp8266Starter.ino
Send Mail 2    | Results in a call to sendmail(2) through a helper function in Esp8266Starter.ino
Erase WiFi     | Calls WiFiManager.resetSettings() then ESP.reset() (after confirm popup)
Hard Reset     | Calls ESP.reset() (after confirm popup) 

Check out the wiki for screenshots: 
 https://github.com/magellannh/Esp8266Starter/wiki/Esp8266-Web-Control-Panel
