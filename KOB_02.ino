  #include <SoftwareSerial.h>
  /*
  Lecture des débits sur port série Rs485
  Envoi des débits positifs sur le cloud influxDB
  */

  // Définition des pins

  #define TX_PIN 4      // Pin TX pour la communication 485 DI
  #define DE_PIN 5       // Data Enable                     DE
  #define RE_PIN 6       // Receive Enable                  RE
  #define RX_PIN 7      // Pin RX pour la communication 485 RO

  #define RG_PIN 8      // Pin Rx pour la communication 4G
  #define TG_PIN 9      // Pin TX pour la communication 4G
  #define RLED  12      // Témoin de transfert...

  String myToken = "kjhkjh";
  String myHost = "https://eu-central-1-1.aws.cloud2.influxdata.com";
  String myOrg = "b806d7e07da0b5e0";

  float debit = 0.0;    
  String sdebit;
  float savdebit = 0.0;
  int taille = 0;

  // Création de l'objet pour l'interface 4G
  SoftwareSerial c4GSerial(RG_PIN, TG_PIN);

  // Création de l'objet pour l'interface rs485
  SoftwareSerial rs485Serial(RX_PIN, TX_PIN);

  void setup()
  {
  // Port série hardware pour debug (optionnel)
    Serial.begin(115200);
    delay(1000);

    // start du port série débitmètre
    rs485Serial.begin(9600);
    delay(1000);
    
    // start du port série 4G
    c4GSerial.begin(115200);
    delay(1000);
    
    // Configuration des pins RS485
    pinMode(RE_PIN, OUTPUT);
    pinMode(DE_PIN, OUTPUT);

    digitalWrite(RE_PIN, LOW);
    digitalWrite(DE_PIN, LOW);

    pinMode(RLED, OUTPUT);
    digitalWrite(RLED, LOW);

    delay(1000);      
  }


  void loop()
  {
    if (! rs485Serial.isListening()){
      rs485Serial.listen();
      Serial.println("Ecoute rs485Serial");
    }
    if (rs485Serial.available() > 0) 
    {
        String ReceivedData = rs485Serial.readStringUntil('\n');
        Serial.println(ReceivedData);  // Attendu FLOW: 3.4567 m3/h
        int twopoint = ReceivedData.indexOf(':');
        int mletter = ReceivedData.indexOf('m'); // remplacer par l si on passe le débitmètre en l/s
        sdebit = ReceivedData.substring(twopoint+2,mletter-1);
        debit = sdebit.toFloat(); // ne garde que 2 décimales
        Serial.println("sdebit=" + sdebit);
        //
        if ((debit != savdebit) || (debit > 0.0)) // on envoie si le débit est différent du précédent envoyé/ ou > 0
        {
          if (savdebit == 0.0)
          {
            sendInfluxDataPoint("0"); // on poste un 0 juste avant les débits positifs pour permettre un calcul précis des intégrales
          }
          sendInfluxDataPoint(sdebit); // on envoie le point de débit dans Influx
          savdebit = debit;
        }
    } else {
      Serial.println("Rien de nouveau à lire sur rs485Serial");
    }
      delay(1000); 
  }
  

void sendInfluxDataPoint(String sdebit){
  taille = sdebit.length() + 33;
  c4GSerial.listen();
  digitalWrite(RLED, HIGH);
  sendATCommand("AT",1000);
  sendATCommand("AT+HTTPINIT",1000);
  sendATCommand("AT+HTTPPARA=\"URL\",\"" + myHost + "/api/v2/write?org=" + myOrg + "&bucket=debitmetre&precision=s\"",1000);
  sendATCommand("AT+HTTPPARA=\"USERDATA\",\"Authorization:Token " + myToken + "\"",1000);
  sendATCommand("AT+HTTPPARA=\"CONTENT\",\"text/plain; charset=utf-8\"",1000);
  sendATCommand("AT+HTTPDATA=" + String(taille) + ",1000",1000); // là il faut indiquer la taille exacte du body text
  sendATCommand("debit,client=KOB,unit=m3/h debit=" + sdebit,1000);
  sendATCommand("AT+HTTPACTION=1",8000); 
  sendATCommand("AT+HTTPTERM",1000);
  digitalWrite(RLED, LOW);

}

String sendATCommand(String cmd, const int timeout) {
  Serial.println("sendATcommand :" + cmd);
  String response = "";
  c4GSerial.println(cmd);
  long int time = millis();
  while((time+timeout) > millis()) {
    while(c4GSerial.available() > 0) {
      char c = c4GSerial.read();
      response += c;
    }
  }
  Serial.println(response);
  return response;
}
