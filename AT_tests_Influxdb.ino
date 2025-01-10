#include <SoftwareSerial.h>
String myToken = "SGeozmBcVvWF4K7SQXQ6oAWnymZo1NXBIPy3pgSX9G3c4o3ZedHdoL7l_XZh-z-96CEdm-O4L2M-zE5sdB4bhA==";
String myHost = "https://eu-central-1-1.aws.cloud2.influxdata.com";
String myOrg = "b806d7e07da0b5e0";

String myDebit = "4.3521";
int taille = 0;

SoftwareSerial mySerial(8,9);

void setup()
{
  mySerial.begin(115200);   // Setting the baud rate of 4G LTE Module  
  Serial.begin(115200);    // Setting the baud rate of Serial Monitor (Arduino)
  delay(1000);
  //String http_str = "AT+HTTPPARA=\"URL\",\"https://api.thingspeak.com/update?api_key=" + Apikey + "&field1=" + String(debit) + "\"";
    sendATCommand("AT",1000);
    sendATCommand("AT+HTTPINIT",1000);
    sendATCommand("AT+HTTPPARA=\"URL\",\"" + myHost + "/api/v2/write?org=" + myOrg + "&bucket=debitmetre&precision=s\"",1000);
    sendATCommand("AT+HTTPPARA=\"USERDATA\",\"Authorization:Token " + myToken + "\"",1000);
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"text/plain; charset=utf-8\"",1000);
    taille = myDebit.length() + 33;
    sendATCommand("AT+HTTPDATA=" + String(taille) + ",1000",1000); // là il faut la taille exacte
    sendATCommand("debit,client=KOB,unit=m3/h debit=" + myDebit,1000);
    sendATCommand("AT+HTTPACTION=1",3000); 
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
