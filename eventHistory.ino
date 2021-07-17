// 
// eventHistory.ino - A ram based event log that saves log entries as text string with associated timestamp
//
// Records the date and time along with a text string for the last n events
// storage is a simple ram-based circular array.
//
// Dependent on two utility functions that use time.h with some additional processing (see dateTimeUtils.ino)
//
//  unsigned long int timeSinceEpoch()        - calls time.h time() function 
//             String epochTimeToDateString() - converts time since epoch to user friendly date string 
//
// API
//          void initializeEventHistoryStorage() 
//          void addEventHistoryEntry(String str)
//           int getNumberOfEventHistoryEntries() 
//        String getEventHistoryString(int howFarBackToGo) 
// unsigned long getEventHistoryTimestamp(int howFarBackToGo) 
// unsigned long getSecondsSinceLastEventHistoryEntry()
//        String getFullEventHistoryAsString(int maxEntries)
//
#define MAX_EVENT_HISTORY_ENTRIES 20

unsigned long EventHistoryTimestamps[MAX_EVENT_HISTORY_ENTRIES];
String EventHistoryStrings[MAX_EVENT_HISTORY_ENTRIES];
int currentHistoryIndex=0; // where next entry goes
int activeHistoryEntries=0;

void initializeEventHistoryStorage() {
  currentHistoryIndex=0;
  activeHistoryEntries=0;
}

void addEventHistoryEntry(String str) {
  
   unsigned long int timestamp = getCurrentEpochTime();
   EventHistoryTimestamps[currentHistoryIndex] = timestamp;
   EventHistoryStrings[currentHistoryIndex] = str;
   
   //Serial.printf("Adding History entry %d.  TimeStamp: %s String: %s\n", currentHistoryIndex, epochTimeToDateString(timestamp).c_str(), str.c_str());
  
   currentHistoryIndex++;
   if (currentHistoryIndex >= MAX_EVENT_HISTORY_ENTRIES)
    currentHistoryIndex = 0;
    
   if (activeHistoryEntries < MAX_EVENT_HISTORY_ENTRIES)
    activeHistoryEntries++;
}

unsigned long getSecondsSinceLastEventHistoryEntry() {
  
   unsigned long currentTime = getCurrentEpochTime();
   unsigned long lastTime = 0;
   
   if (activeHistoryEntries > 0)
    lastTime = getEventHistoryTimestamp(1);
    
   unsigned long secondsSinceLastEntry = difftime(currentTime, lastTime);
   //Serial.println("Seconds since last entry = " + String (secondsSinceLastEntry, DEC));
   return secondsSinceLastEntry;
}

int getNumberOfEventHistoryEntries() {
  return activeHistoryEntries;
}

unsigned long getEventHistoryTimestamp(int howFarBackToGo) {
  int idx=currentHistoryIndex;

  if (howFarBackToGo > activeHistoryEntries)
    return 0;
  for (int i=1; i<=howFarBackToGo;i++) 
    if (idx > 0)
       idx--;
    else
       idx = MAX_EVENT_HISTORY_ENTRIES-1;
  
  //Serial.println("Fetching History entry idx=" + String(idx));
  return EventHistoryTimestamps[idx];
}

String getEventHistoryString(int howFarBackToGo) {
  int idx=currentHistoryIndex;

  if (howFarBackToGo > activeHistoryEntries)
    return (String) "";
  for (int i=1; i<=howFarBackToGo;i++) 
    if (idx > 0)
       idx--;
    else
       idx = MAX_EVENT_HISTORY_ENTRIES-1;
  
  //Serial.println("Fetching History entry idx=" + String(idx));
  return EventHistoryStrings[idx];
}

String getFullEventHistoryAsString(int maxEntries) {
  String ptr = "";
  for (int i=1;((i<=getNumberOfEventHistoryEntries()) && (i<=maxEntries));i++) {
    char const *spacerStr = " ";
    if (i>9)
      spacerStr = "";
    ptr += "            Entry "; ptr += String(spacerStr); ptr += String(i,DEC); ptr += ":  ";
    ptr += epochTimeToDateString(getEventHistoryTimestamp(i)); 
    ptr += "  ";  ptr += getEventHistoryString(i); ptr += "\n";
  }
  return ptr;
}
