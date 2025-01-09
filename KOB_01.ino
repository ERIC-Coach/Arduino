#include <SoftwareSerial.h>
/*
Lecture des débits sur port série Rs485
Envoi des débits positifs sur le cloud 
*/

// Définition des pins

#define TX_PIN 4      // Pin TX pour la communication 485 DI
#define DE_PIN 5       // Data Enable                     DE
#define RE_PIN 6       // Receive Enable                  RE
#define RX_PIN 7      // Pin RX pour la communication 485 RO

#define RG_PIN 8      // Pin Rx pour la communication 4G
#define TG_PIN 9      // Pin TX pour la communication 4G
#define RLED  12      // Témoin de transfert...

float debit = 1.0;    // pour poster un 0 au bébut.
float savdebit = 0.0;

String Server = "https://api.thingspeak.com/update?api_key=";
String Apikey = "U6P6V3B14YDZEPOS";

// Création de l'objet pour l'interface 4G
SoftwareSerial c4GSerial(RG_PIN, TG_PIN);

// Création de l'objet pour l'interface rs485
SoftwareSerial rs485Serial(RX_PIN, TX_PIN);

void setup()
{
  // start du port série 4G
  c4GSerial.begin(115200);
   
  // start du port série 485
  rs485Serial.begin(9600);
    
  // Port série hardware pour debug (optionnel)
  Serial.begin(115200);
  
  // Configuration des pins RS485
  pinMode(RE_PIN, OUTPUT);
  pinMode(DE_PIN, OUTPUT);

  digitalWrite(RE_PIN, LOW);
  digitalWrite(DE_PIN, LOW);

  pinMode(RLED, OUTPUT);
  digitalWrite(RLED, LOW);
}


void loop()
{
  if (rs485Serial. available()) {
  String ReceivedData = rs485Serial.readStringUntil('\n');
  Serial.println(ReceivedData);  // Attendu FLOW: 3.4567 m3/h
  int twopoint = ReceivedData.indexOf(':');
  int mletter = ReceivedData.indexOf('m'); // remplacer par l si on passe le débitmètre en l/s
  String sdebit =ReceivedData.substring(twopoint+1,mletter-1);
  debit = sdebit.toFloat(); // ne garde que 2 décimales
  Serial.println(debit);
}
if (debit != savdebit) // on envoie si le débit est différent du précédent envoyé/ 
{
  savdebit = debit; 
  digitalWrite(RLED, HIGH);
  String http_str = "AT+HTTPPARA=\"URL\",\"" + Server + Apikey + "&field1=" + String(debit) + "\"";
    sendATCommand("AT",1000);
    sendATCommand("AT+HTTPINIT",1000);
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",1000);  
    sendATCommand(http_str,3000); //merci d'optimiser les timeouts.
  digitalWrite(RLED, LOW);
    sendATCommand("AT+HTTPACTION=0",2000);
    sendATCommand("AT+HTTPTERM",1000);
}
 delay(500); 
}


String sendATCommand(String cmd, const int timeout) {
  String response = "";
  c4GSerial.println(cmd);
  long int time = millis();
  while((time+timeout) > millis()) {
    while(c4GSerial.available()) {
      char c = c4GSerial.read();
      response += c;
    }
  }
  Serial.println(response);
  return response;
}
