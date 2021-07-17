//
// prefsMgr.ino - Implements simple text based preferences/configuratio settings system.
//
// Preferences are stored in a text file using LittleFS and can be retrieved by key.
// - Could easily be modified to use SPIFFS
// - Combines with ESPWebServer for web based load/edit/save of preferences file (with custom html <textarea>)
//
// Preference file format is multiple lines of "text-string-key text-string-value" statements
// - Key is single string with no whitespace in the key itself (key can't be multiple words).  
// - Whitespace before and after key is ignored.
// - Whitespace within text-string-value is preserved
// - Option to automatically convert any "/n" sequences in text-string-values into newline characters
//
// Preference string-values can be accessed as type String, Int, Boolean, or Float (assuming string data converts ok)
// API Calls
// -   void initializePreferencesManager()
// -   void createDefaultPreferencesFile()  (Note: If used, this function must be edited for each new key added/changed)
// - String getPreferencesString(const char *key) 
// -    int getPreferencesBoolean(const char *key) (returns 0/1 - only looks at first char of string - 0, 1, t, f, y, n)
// -    int getPreferencesInteger(const char *key) 
// -  Float getPreferencesFloat(const char *key) 
// - String loadPreferencesFileToString()
// -   void saveUpdatedPreferencesFile(String prefsFileAsString)
//
// Preference Keys are not hashed so accessing keys is not optimized for frequent/real-time use
// - This can be overcome by creating a ram based struct to store some settings for quick access.
// - (would need to refresh any ram settings when settings save is done through web interface
//
// This preferences system is inherently insecure and shouldn't be used for sensitive data.
// - Can be used in combination with WiFiManager so wireless login credentials are managed seperately
// - Allows changing non-wifi settings without having to enter WiFiManager's 192.168.4.1 AP mode
//

#include <LittleFS.h>

#define PREFERENCES_FILE_PATH "/prefsMgr.txt"
#define AUTO_CONVERT_NEWLINE_ESCAPE_SEQUENCES // uncomment to activate auto-conversion of /n sequences in text strings
#define AUTO_CREATE_PREFS_FILE_IF_NOT_FOUND  // uncomment to auto-create default prefs file if not found (must be customized)
void initializePreferencesManager() {

  if(!LittleFS.begin()) {
    Serial.println("PrefsMgr: ERROR starting LittleFS. Preferences system is not operational.");
    return;
  }
  
  //LittleFS.remove(PREFERENCES_FILE_PATH); // Uncomment this line to test auto-creation of default file
  
  if (LittleFS.exists(PREFERENCES_FILE_PATH) == 0) {
#ifdef AUTO_CREATE_PREFS_FILE_IF_NOT_FOUND
    createDefaultPreferencesFile();
#else
    Serial.println("PrefsMgr: ERROR starting LittleFS. Preferences file not found.");
    return;
#endif
  }
  Serial.printf("Preferences system initialized using file %s\n",PREFERENCES_FILE_PATH);
}

// Search the preferences file for 'key' and return the rest of the line that starts with that key
String getPreferencesString(const char *key) {
  File file = LittleFS.open(PREFERENCES_FILE_PATH, "r");
  if(!file) {
    Serial.printf("PrefsMgr: Couldn't open %s for reading.\n", PREFERENCES_FILE_PATH);
    return (String) "";
  }

  String retString = "";
  int found = false;
  while ((file.available()) && (found == false)) {
   String line = file.readStringUntil('\n');
   int idx = line.indexOf(key);
   if (idx >= 0) {
    found = true;
    retString = line.substring(idx+strlen(key)+1);
    retString.trim();
#ifdef AUTO_CONVERT_NEWLINE_ESCAPE_SEQUENCES
    retString.replace("\\n", "\n");
#endif
   }
  }
  file.close();
  if (found != true)
     Serial.printf("PrefsMgr: Couldn't find key %s in preferences file.\n", key);
  else {
     //Serial.printf("PrefsMgr: Key %s matched to string: %s\n",key, retString.c_str());
  }
  return retString;
}

int getPreferencesInteger(const char *key) {
  String prefString = getPreferencesString(key);
  return prefString.toInt();
}

int getPreferencesBoolean(const char *key) {

  String prefString = getPreferencesString(key);
  prefString.replace(" ", "");
  char ch = prefString.charAt(0);
  int retVal=0;  
  switch (ch)
      {
       case '1':
       case 't':
       case 'y':
            retVal = 1;
            break;
       case '0': 
       case 'f':
       case 'n':
       default:
            retVal = 0;
            break;
      }
  return(retVal); 
}

float getPreferencesFloat(const char *key)
{
  String prefString = getPreferencesString(key);
  return prefString.toFloat();   
}

String loadPreferencesFileToString() {
    
  File file = LittleFS.open(PREFERENCES_FILE_PATH, "r");
  if(!file) {
    Serial.printf("PrefsMgr Couldn't open %s for reading.  Attempting to create it.\n", PREFERENCES_FILE_PATH);
    createDefaultPreferencesFile();
    file = LittleFS.open(PREFERENCES_FILE_PATH, "r");
    if (!file) {
       Serial.printf("PrefsMgr: Failed to open preferences file to load as string. Aborting.\n");
       return String("");
    }
  }
  String str = file.readString();
  file.close();
  return str;
}

void saveUpdatedPreferencesFile(String prefsFileAsString) {
  const char *prefsFileBfr = prefsFileAsString.c_str();
  int fileLength = strlen(prefsFileBfr);
  
  File file = LittleFS.open(PREFERENCES_FILE_PATH, "w");
    if(!file){
    Serial.printf("PrefsMgr: Failed to open %s for writing\n", PREFERENCES_FILE_PATH);
    return;
  }
  int idx = 0;
  int numBytesWritten=1;
  while ((numBytesWritten == 1) && (prefsFileBfr[idx] != 0) && (idx < fileLength)) {
    numBytesWritten = file.write(prefsFileBfr[idx++]);
    yield();
  }
  if (numBytesWritten != 1)
    Serial.printf("PrefsMgr: Error saving preferences text buffer to file.  Aborting.\n");
  file.close();
}

#ifdef AUTO_CREATE_PREFS_FILE_IF_NOT_FOUND
// This function must be customized for each application based on project needs
void createDefaultPreferencesFile() {
  Serial.printf("PrefsMgr: Creating default preferences file: %s\n", PREFERENCES_FILE_PATH);
  File f = LittleFS.open(PREFERENCES_FILE_PATH, "w");
  if (!f) {
    Serial.printf("PrefsMgr: Failed to open preferences file %s for writing\n", PREFERENCES_FILE_PATH);
    return;
  }

  int bytesWritten = 0;
  int e=0; // error flag
  bytesWritten += writeFileLn(f, &e, "; Basic Configuration file for the ESP8622 Starter System");
  bytesWritten += writeFileLn(f, &e, "; These settings must be edited for sending email to work"); 
  bytesWritten += writeFileLn(f, &e, "sendMailEnabled       no\n");
  bytesWritten += writeFileLn(f, &e, "mailServerPort        2525");
  bytesWritten += writeFileLn(f, &e, "mailServerName        mail.smtp2go.com");
  bytesWritten += writeFileLn(f, &e, "mailServerUsername    Base64EncodedUserName");
  bytesWritten += writeFileLn(f, &e, "mailServerPassword    Base64EncodedPassword");
  bytesWritten += writeFileLn(f, &e, "mailFromAddress       me@gmail.com");
  bytesWritten += writeFileLn(f, &e, "mailToAddress         someone@gmail.com\n\n");
  bytesWritten += writeFileLn(f, &e, "mailSubject1          ESP8266 Monitor Alert: System event 1 has occurred");
  bytesWritten += writeFileLn(f, &e, "mailBody1             This likely means that something has happened.\\n");
  bytesWritten += writeFileLn(f, &e, "mail1EventHistory     yes");
  bytesWritten += writeFileLn(f, &e, "mailSubject2          ESP8266 Monitor Alert: System event 2 has occurred");
  bytesWritten += writeFileLn(f, &e, "mail2EventHistory     no");
  bytesWritten += writeFileLn(f, &e, "mailBody2             Perhaps the batteries on the remote sensor are running low?\\n");
  bytesWritten += writeFileLn(f, &e, "mailBodyCommonFooter  This e-mail was sent by the ESP8266 System Monitor Program.");
  f.close();
  if (e == 0)
    Serial.printf("PrefsMgr: Default preferences file created (size is %d bytes)\n", bytesWritten);
  else
    Serial.printf("PrefsMgr: Error encountered writing new preferences file (%d bytes written successfully)\n", bytesWritten);    
}
int writeFileLn(File file, int *errorp, const char *str) {
  int bytes;
  if (*errorp !=0) 
    return 0;
  else  if ((bytes = file.println(str)) <= 0)
    *errorp = 1;
  return bytes;
}
#endif
