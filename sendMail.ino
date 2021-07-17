// 
// sendMail.ino - Implements a sendMail() function using WiFiClient libraryd
//
//This was adapted from various examples found on the Internet like this one:
//    https://www.smtp2go.com/setupguide/arduino/
//
// For smtp2go, SMTP username/password must be encoded to Base64 format 
// Online Base64 encoder: https://www.base64encode.org/   
//
// Dependencies
//   WiFiClient (should be part of base Arduino IDE/ESP8266 system)
//   String getPreferencesString(const char *key)      - from prefsMgr.ino
//     void addEventHistoryEntry(String eventMsg)      - from eventHistory.ino
//   String getFullEventHistoryAsString(maxNumEntries) - from eventHistory.ino
//

WiFiClient espClient;

#define MAIL_MESSAGE_TYPE1 1
#define MAIL_MESSAGE_TYPE2 2
  
byte sendEmail(int messageType) {
    
  if (espClient.connect(getPreferencesString("mailServerName"), getPreferencesInteger("mailServerPort")) == 1)  {
//       Serial.println(F("connected"));
  } else {
       Serial.println(F("connection failed"));
       addEventHistoryEntry("sendMail() connection failed.  Message not sent");
       return 0;
  }
  if (!emailResp()) {
    addEventHistoryEntry("sendMail() no connection response. Message not sent");
    return 0;
  }
//  Serial.println(F("Sending EHLO"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) {
       addEventHistoryEntry("sendMail() EHLO failed.  Message not sent");
       return 0;
  }
 
//  Serial.println(F("Sending auth login"));
  espClient.println("AUTH LOGIN");
  if (!emailResp()) {
       addEventHistoryEntry("sendmail() AUTH LOGIN failed. Message not sent");
       return 0;
  }
//  Serial.println(F("Sending User"));
 
 
  espClient.println(getPreferencesString("mailServerUsername")); // Paste your encoded Username here
  if (!emailResp()) {
       addEventHistoryEntry("sendMail() received error after sending username");
       return 0;
  }
 
//  Serial.println(F("Sending Password"));
 
  espClient.println(getPreferencesString("mailServerPassword")); // Paste your encoded Password here

  if (!emailResp()) {
       addEventHistoryEntry("sendMail() received error after sending password");
       return 0;
  }
 
//  Serial.println(F("Sending From"));
 
  espClient.println("MAIL From: " + getPreferencesString("mailFromAddress")); // Replace with sender email address
  if (!emailResp()) {
       addEventHistoryEntry("sendMail() received error after sending \"MAIL From\"");
       return 0;
  }
//  Serial.println(F("Sending To"));
  espClient.println("RCPT To: " + getPreferencesString("mailToAddress")); // Replace with receiver email address
  if (!emailResp()) {
       return 0;
       addEventHistoryEntry("sendMail() received error after sending \"RCPT To\"");
  }
 
//  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) {
       addEventHistoryEntry("sendMail() received error after sending \"DATA\"");
       return 0;
  }
//  Serial.println(F("Sending email"));
  
  espClient.println("To:  " + getPreferencesString("mailToAddress"));
  espClient.println("From: " + getPreferencesString("mailFromAddress"));
  
  if (messageType == MAIL_MESSAGE_TYPE1) {
    espClient.println("Subject: " + getPreferencesString("mailSubject1"));
    espClient.println(getPreferencesString("mailBody1"));
    if (getPreferencesBoolean("mail1EventHistory")) {      
           espClient.println("Most Recent Events");
           espClient.println(getFullEventHistoryAsString(10));
    }
  } else {
    espClient.println("Subject: " + getPreferencesString("mailSubject2"));
    espClient.println(getPreferencesString("mailBody2"));
    if (getPreferencesBoolean("mail2EventHistory")) {
           espClient.println("Most Recent Events");
           espClient.println(getFullEventHistoryAsString(10));
    }
  }
  espClient.println(getPreferencesString("mailBodyCommonFooter"));

  espClient.println(F("."));
  if (!emailResp()) {
       addEventHistoryEntry("sendMail() received error after sending \".\"");
       return 0;
  }
  
  //Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) {
      addEventHistoryEntry("sendMail() received error after sending \"QUIT\"");
      return 0;
  }
  espClient.stop();
  
  //Serial.println(F("disconnected"));
  return 1;
}
 
byte emailResp() {
  byte responseCode;
  byte readByte;
  int loopCount = 0;
 
  while (!espClient.available()) {
       delay(1);
       loopCount++;
 
       if (loopCount > 20000)
       {
       espClient.stop();
       Serial.println(F("\r\nESPClient Timeout waiting for email response"));
       return 0;
       }
  }
 
  responseCode = espClient.peek();
  while (espClient.available()) {
       readByte = espClient.read();
//       Serial.write(readByte);
  }
 
  if (responseCode >= '4') {
       return 0;
  }
  return 1;
}
