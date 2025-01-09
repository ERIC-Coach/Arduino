#include <SoftwareSerial.h>

int i;
SoftwareSerial mySerial(8,9);
float debit = 2.6;


String Apikey = "U6P6V3B14YDZEPOS";

void setup()
{
  mySerial.begin(115200);   // Setting the baud rate of 4G LTE Module  
  Serial.begin(115200);    // Setting the baud rate of Serial Monitor (Arduino)
  delay(1000);
  String http_str = "AT+HTTPPARA=\"URL\",\"https://api.thingspeak.com/update?api_key=" + Apikey + "&field1=" + String(debit) + "\"";
    sendATCommand("AT",1000);
    sendATCommand("AT+HTTPINIT",1000);
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",1000);  
    sendATCommand(http_str,3000); //merci d'optimiser les timeouts.
    sendATCommand("AT+HTTPACTION=0",2000);
    sendATCommand("AT+HTTPTERM",1000);


}


void loop()
{
   if(mySerial.available())  // check if the ESP is sending a message
  {
    while(mySerial.available())
    {
      char c = mySerial.read();  // read the next character.

      Serial.write(c);  // writes data to the serial monitor
    }
  }

  if(Serial.available())
  {
    delay(10);  // wait to let all the input command in the serial buffer
    // read the input command in a string
    String cmd = "";
    while(Serial.available())
    {
      cmd += (char)Serial.read();
    }
    // send
    mySerial.println(cmd); 
  }
}

String sendATCommand(String cmd, const int timeout) {
  String response = "";
  mySerial.println(cmd);
  long int time = millis();
  while((time+timeout) > millis()) {
    while(mySerial.available()) {
      char c = mySerial.read();
      response += c;
    }
  }
  Serial.println(response);
  return response;
}
