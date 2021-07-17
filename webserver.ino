//
// webserver.ino -  Implement a web control panel for the Esp9266 Starter system.
//
// Pages of the control website include a left-side button panel with buttons that
// either link to other pages of the system or invoke operations as follows:
//
//    Button            Function
//  -----------      -----------------
//     Home          Displays basic operational status
//   History         Links to a page with the Event History (see eventHistory.ino)
//   Settings        Links to a page to edit/save the settings file managed by prefsMfr.ino  (uses html <textarea>)
//   Restart         Calls doSoftReset() function in Esp8266Starter.ino to inits counters, timers, etc
//  Send Mail 1      Results in a call to sendmail(1) through a helper function in Esp8266Starter.ino
//  Send Mail 2      Results in a call to sendmail(2) through a helper function in Esp8266Starter.ino
//  Erase WiFi       calls WiFiManager.resetSettings() then ESP.reset() (after confirm popup)
//  Hard Reset       calls ESP.reset() (after confirm popup) 
//
// To run webserver,  call setupWebserver() in setup() and  handleWebseverProcessing() in loop()
//
// Dependencies
// - ESP8266WebServer, WiFiManager
// -      From prefsMgr.ino - loadPreferencesFileToString(), saveUpdatedPreferencesFile()
// -  From eventHistory.ino - addEventHistoryEntry(), getEventHistoryTimestamp(),
//                            getEventHistoryString(), getFullEventHistoryAsString (from eventHistory.ino)
// - From dateTimeUtils.ino - elapsedMillisToVerboseString(), elapsedMillisToDateString(), 
//                            elapsedMillisToString(), epochTimeToDateString()
// - Various global vars and functions from Esp8266Starter.ino
//
// The webserver implementation is fully functional but is intended as a starter system that
// will be extensively customized based on its intended application
//
ESP8266WebServer server(80);

#define SHOW_STATUS_IN_CONTENT_PANE   1
#define SHOW_HISTORY_IN_CONTENT_PANE  2
#define SHOW_SETTINGS_IN_CONTENT_PANE 3

void setupWebserver()
{
  server.on("/", handle_OnConnect);
  server.on("/show_event_history", handle_history);
  server.on("/configure_settings", handle_settings);
  server.on("/settings_save", handle_settings_save);
  server.on("/send_mail1", handle_sendMail1);
  server.on("/send_mail2", handle_sendMail2);
  server.on("/do_soft_reset", handle_soft_reset);
  server.on("/do_erase_wifi", handle_erase_wifi);
  server.on("/do_hard_reset", handle_hard_reset);
  
  server.onNotFound(handle_NotFound);  
  server.begin();
}

void handleWebseverProcessing() {
  server.handleClient();   
}

void handle_soft_reset() {
  timeSinceLastWebActivity=0;
  Serial.println("Performing soft reset");
  doSoftReset();
  server.sendHeader("Location","/");       
  server.send(303); 
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE));
}

void handle_hard_reset() {
  Serial.println("Performing hard reset");
  server.sendHeader("Location","/");       
  server.send(303);   
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE)); 
  delay(500);
  ESP.reset();
}

void handle_erase_wifi() {
  Serial.println("Erasing wifi settings");
  delay(500);
  WiFiManager wifiManager;  
  wifiManager.resetSettings();  
  server.sendHeader("Location","/");       
  server.send(303); 
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE));
  delay(700);
  ESP.reset();
}

void handle_sendMail1() {
  timeSinceLastWebActivity=0;
  Serial.println("Calling sendMailMessageType1().");
  sendMailMessageType1();
  server.sendHeader("Location","/");       
  server.send(303);
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE)); 
}

void handle_sendMail2() {
  timeSinceLastWebActivity=0;
  Serial.println("Calling sendMailMessageType2().");
  sendMailMessageType2();
  server.sendHeader("Location","/");       
  server.send(303);
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE)); 
}
void handle_history() {
  server.send(200, "text/html", SendHTML(SHOW_HISTORY_IN_CONTENT_PANE)); 
}

void handle_settings() {
  timeSinceLastWebActivity=0;
  server.send(200, "text/html", SendHTML(SHOW_SETTINGS_IN_CONTENT_PANE)); 
}

void handle_settings_save() {
 String configBuffer = server.arg("configBuffer"); 
 timeSinceLastWebActivity=0;
 
 saveUpdatedPreferencesFile(configBuffer);
 addEventHistoryEntry("Configuration settings were saved");
 
 server.sendHeader("Location","/configure_settings");       
 server.send(303);
 server.send(200, "text/html", SendHTML(SHOW_SETTINGS_IN_CONTENT_PANE)); 
}

void handle_OnConnect() {
  timeSinceLastWebActivity=0;
  server.send(200, "text/html", SendHTML(SHOW_STATUS_IN_CONTENT_PANE)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


String ptr;
String SendHTML(int pageToShowInContentPane){

  ptr.reserve(500); // Needs to be big.  Set initial size and allocation size to avoid heap frag
  
  ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"initial-scale=1.0\">\n";
  ptr += "<title>ESP8266 Starter System</title>\n";
  ptr += "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\n";
  ptr += "<style>html,body {height: 100%;display: flex;flex-direction: column;font-family: Helvetica;margin: 0px;}\n";
  ptr += "#header {padding: 10px;background-color: blue;text-align:center;color: white;border-bottom: 1px solid black;}\n";
  ptr += "#headerRightText {float: right;  line-height: 2.8; }\n";
  ptr += "#container {padding: 0;margin: 0;display: flex;flex-direction: row;flex-grow: 1;}\n";
  ptr += "#sidebar {padding: 12px;background-color: #e7e7e7;}\n";
  ptr += "#content {flex-grow: 1;padding: 10px;}\n";
  ptr += "h1 {display: inline; margin: 0;text-align: center;}\n";
  ptr += "h3 {text-align: center; color: #444444;}\n";
  ptr += "p {font-size: 16px; color: #888;}\n";
  ptr += ".button {display:block;color:white;padding: 7px;text-align:center;text-decoration: none;font-size: 16px; margin: 0px auto 15px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-green {background-color: #1abc9c;}\n";
  ptr += ".button-red {background-color: #B5153F;}\n";
  ptr += ".button-yellow {background-color: #E2d606;}\n";
  ptr += "</style></head>\n";
  ptr += "<body><div id=\"header\"><h1>ESP8266 System Monitor</h1></div>\n";
  ptr += "<div id=\"container\"><div id=\"sidebar\">\n"; //<h2>Controls</h2>\n";
  ptr += "<a class=\"button button-green\" href=\"/\">Home</a>\n";
  ptr += "<a class=\"button button-green\" href=\"/show_event_history\">History</a>\n";
  ptr += "<a class=\"button button-green\" href=\"/configure_settings\">Settings</a>\n";
  ptr += "<a class=\"button button-yellow\" href=\"/do_soft_reset\"\">Restart</a>\n";
  ptr += "<a class=\"button button-yellow\" href=\"/send_mail1\">Send Mail 1</a>\n";
  ptr += "<a class=\"button button-yellow\" href=\"/send_mail2\">Send Mail 2</a>\n";
  ptr += "<a class=\"button button-red\" href=\"/do_erase_wifi\" onclick=\"return confirm('Erase all WiFi settings and reboot as 192.168.4.1. Are you sure?')\">Erase Wifi</a>\n";
  ptr += "<a class=\"button button-red\" href=\"/do_hard_reset\"onclick=\"return confirm('Perform hardware reset. Are you sure?')\">Hard Reset</a>\n";

  if (pageToShowInContentPane == SHOW_STATUS_IN_CONTENT_PANE)
    addCurrentStatusWebPageContents(ptr);
  else if (pageToShowInContentPane == SHOW_HISTORY_IN_CONTENT_PANE)
    addHistoryWebPageContents(ptr);
  else if (pageToShowInContentPane == SHOW_SETTINGS_IN_CONTENT_PANE)
    addSettingsWebPageContents(ptr);    
  ptr += "</pre></div></div></body></html>\n";
  return ptr;
}

void addCurrentStatusWebPageContents(String& ptr) {

  ptr += "</div><div id=\"content\"><h3>Current Status</h3><pre>\n";
  
  // For better heap management, use  "ptr+=" when concating strings (mutate object instead of creating new object) 
  // this is more efficient than ptr += "a" + "b" + "c"; which creates many objects and heap fragmentation
  
  ptr += "    System Uptime: "; ptr += elapsedMillisToVerboseString(timeSinceSystemStarted); ptr += "\n";
  if (timeSinceWebSessionStarted < MILLIS_IN_A_DAY)
    {ptr += "     Session Time: "; ptr += elapsedMillisToVerboseString(timeSinceWebSessionStarted); ptr += "\n";}
  ptr += "      Free Memory: "; ptr += String(freeMemory(), DEC); ptr += " bytes\n\n";
  if (email1SentCount > 0)
    {ptr += " Email 1 Messages: "; ptr += String(email1SentCount, DEC); ptr += "\n";}
  if (email2SentCount > 0)
    {ptr += " Email 2 Messages: "; ptr += String(email2SentCount, DEC); ptr += "\n";}
  if (emailSupressedCount > 0)
    {ptr += " Emails Supressed: "; ptr += String(emailSupressedCount, DEC); ptr += "\n";}
  if (emailErrorCount > 0)
    {ptr += "Email Send Errors: "; ptr += String(emailErrorCount, DEC); ptr += "\n";}
  if (getNumberOfEventHistoryEntries() > 0)
    {ptr += "  History Entries: "; ptr += String(getNumberOfEventHistoryEntries(), DEC); ptr += "\n";}

  ptr +="\n        Current Time: "; ptr += elapsedMillisToDateString(0); ptr += "\n\n";

  if ((email1SentCount > 0) || (email2SentCount > 0)) {
     ptr +="     Last Email Sent: "; ptr += elapsedMillisToDateString(timeSinceLastEmailSentOk); 
     ptr += " ("; ptr += elapsedMillisToString(timeSinceLastEmailSentOk); ptr += " ago)\n"; }
  if ((email1SentCount > 0) || (email2SentCount > 0) || (emailSupressedCount > 0) || (emailErrorCount > 0))  {
     ptr +="  Last Email Attempt: "; ptr += elapsedMillisToDateString(timeSinceLastEmailAttempt); 
     ptr += " ("; ptr += elapsedMillisToString(timeSinceLastEmailAttempt); ptr += " ago)\n"; }
  if (emailErrorCount > 0) {
     ptr +="    Last Email Error: "; ptr += elapsedMillisToDateString(timeSinceLastEmailSendError); 
     ptr += " ("; ptr += elapsedMillisToString(timeSinceLastEmailSendError); ptr += " ago)\n"; }  
  if (softResetCount > 0) {
     ptr +="     Last Soft Reset: "; ptr += elapsedMillisToDateString(timeSinceSoftReset);
     ptr += " ("; ptr += elapsedMillisToString(timeSinceSoftReset); ptr += " ago)\n"; }
  if (getNumberOfEventHistoryEntries() > 0) {
     ptr +="\n          Last Event: "; ptr += epochTimeToDateString(getEventHistoryTimestamp(1));
     ptr +=" ";  ptr += getEventHistoryString(1); ptr += "\n"; }
}

void addHistoryWebPageContents(String& ptr) {
  
  ptr += "</div><div id=\"content\"><h3>Event History</h3><pre>\n";
  
  ptr +="        Current Time:  "; ptr += elapsedMillisToDateString(0); ptr += "\n\n";
  ptr += getFullEventHistoryAsString(100);
}

void addSettingsWebPageContents(String& ptr) {
  ptr += "</div><div id=\"content\"><h3>Current Configuration Settings</h3>\n";
  ptr += "<FORM METHOD=\"POST\" action=\"/settings_save\">";
  ptr += "<textarea id=\"configBuffer\" name=\"configBuffer\" style=\"display:block\" rows=\"20\" cols=\"90\" spellcheck=\"false\">";
  ptr += loadPreferencesFileToString();
  ptr += "</textarea><input type=\"submit\" value=\"Save Changes\"></form>"; 
  ptr += "<pre>";
}
