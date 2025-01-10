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

  String myToken = "SGeozmBcVvWF4K7SQXQ6oAWnymZo1NXBIPy3pgSX9G3c4o3ZedHdoL7l_XZh-z-96CEdm-O4L2M-zE5sdB4bhA==";
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
    sdebit =ReceivedData.substring(twopoint+1,mletter-1);
    debit = sdebit.toFloat(); // ne garde que 2 décimales
  }
  Serial.println(debit);
  if ((debit != savdebit)||(debit > 0)) // on envoie si le débit est différent du précédent envoyé/ ou > 0
  {
    savdebit = debit; 
    digitalWrite(RLED, HIGH);
      sendATCommand("AT",1000);
      sendATCommand("AT+HTTPINIT",1000);
    sendATCommand("AT+HTTPPARA=\"URL\",\"" + myHost + "/api/v2/write?org=" + myOrg + "&bucket=debitmetre&precision=s\"",1000);
      sendATCommand("AT+HTTPPARA=\"USERDATA\",\"Authorization:Token " + myToken + "\"",1000);
      sendATCommand("AT+HTTPPARA=\"CONTENT\",\"text/plain; charset=utf-8\"",1000);
      taille = sdebit.length() + 33;
      sendATCommand("AT+HTTPDATA=" + String(taille) + ",1000",1000); // là il faut la taille exacte
      sendATCommand("debit,client=KOB,unit=m3/h debit=" + sdebit,1000);
      digitalWrite(RLED, LOW);
      sendATCommand("AT+HTTPACTION=1",3000); 
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
