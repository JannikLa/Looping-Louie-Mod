/*******************************************************
 * Datei Name:  20190111_LoopingLouiVar2_2
 * Autor:       Jannik Lassen
 * Datum:       11.01.2019
 * _____________________________________________________
 * Zweck:       Ansteuerung des DC Motors über einen ESP8266
 *              mithilfe eines Webservers.
 *              Der benutzer hat die Möglichkeit über den
 *              Server den Modus des Spiels zu wählen.
 *              Der Normal Mode erlaubt den Spieler wie 
 *              gewohnt mit konstanter Geschwindikeit das 
 *              Spiel zu spielen. Der Random Mode hingegen
 *              macht das Spiel ein wenig spannender, indem
 *              die Geschwindigkeit und Richtung mithilfe von
 *              Zufallszahlen geändert wird.
 *              
 * Eingabe:     Normal Mode/Random Mode/Boost/Stop 
 *              über den Web Server
 * Ausgabe:     - Je nach Modus verscheidene PWM Signale an
 *                den Motortreiber
 ********************************************************/


#include <ESP8266WiFi.h>

const char* ssid = "Klick for some Fun";
const char* password = "12345678";


// Server Port
WiFiServer server(80);

// Output Pins definieren
const int IO0 = 0;
const int IO2 = 2;

// Counter
long int counter = 0;

void setup() 
{

  // Output Pins als Outputs initialsieren
  pinMode(IO0, OUTPUT);
  pinMode(IO2, OUTPUT);
  // Output Pins auf LOW setzen
  digitalWrite(IO0, LOW);
  digitalWrite(IO2, LOW);


  // PWM Frequenz für den Motor
  analogWriteFreq(100);


  
  Serial.begin(115200);
  delay(1);
  
  // WiFi AP Mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
}

void loop() 
{ 
  // Zufallszahlen generieren

  randomSeed(counter);
  if(counter > 2147483647)
  {
    counter = 0;
  }
  counter++;
  
  // Überprüfen, ob Client verbunden
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }

  // WiFi Verbindung Eltablieren und Kommunikation mit dem Server
  
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Erste Zeile auslesen
  String sRequest = client.readStringUntil('\r');
  client.flush();
  
  // Loop beenden falls leere Anforderung
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }


  // get path; ende von path ist entweder leerzeichen oder "?"
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);

    // gibt es Parameter?
    if(iEndSpace>0)
    {
      if(iEndQuest>0) // Es gibt Parameter
      {
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else    // Keine Parameter
      {
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }


  // Output Parameter auf Serial stellen
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  // HTML Rückgabewerte formatieren
  String sResponse,sHeader;
  
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  // Webseite Formatieren
  else
  {
    sResponse  = "<html><head><title>Looping Loui Server</title></head><body>";
    sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>Looping Loui Server</h1>";
    sResponse += "Funktion 1 schaltet das Spiel auf eine konstante Geschwindigkeit.<BR>";
    sResponse += "Funktion 2 schaltet den Random Mode ein.<BR>";
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<p>Normal Mode <a href=\"?pin=FUNCTION1ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>ausschalten</button></a></p>";
    sResponse += "<p>BOOST  FW   <a href=\"?pin=FUNCTION2ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>ausschalten</button></a></p>";
    sResponse += "<p>STOP        <a href=\"?pin=FUNCTION3ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION3OFF\"><button>ausschalten</button></a></p>";
    sResponse += "<p>Random Mode <a href=\"?pin=FUNCTION4ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION4OFF\"><button>ausschalten</button></a></p>";

    // Reaktion auf Rückgabewerte der Website
    
    if (sCmd.length()>0)
    {
      // übergabe des Kommandos and die html seite
      sResponse += "Kommando:" + sCmd + "<BR>";
      
      // switch GPIO
      if(sCmd.indexOf("FUNCTION1ON")>=0)        // Normal Mode
      {
        analogWrite(IO0, 600);    // PWM Für die Output Pins
        analogWrite(IO2, 0);
      }
      else if(sCmd.indexOf("FUNCTION1OFF")>=0)
      {
        analogWrite(IO0, 0);
        analogWrite(IO2, 0);
      }
      else if(sCmd.indexOf("FUNCTION2ON")>=0)   // BOOST
      {
        digitalWrite(IO0, HIGH);
        digitalWrite(IO2, LOW);
      }                                         // STOP
      else if(sCmd.indexOf("FUNCTION3ON")>=0||sCmd.indexOf("FUNCTION2OFF")>=0||sCmd.indexOf("FUNCTION3OFF")>=0)
      {
        analogWrite(IO0, 0);
        analogWrite(IO2, 0);
      }
      else if(sCmd.indexOf("FUNCTION4ON")>=0)   // Random Mode
      {
        while(true)
        {
          counter++;
          int phase = rand()%10;

          // Zufällige Motoransteuerung durch Switch Case
          
          switch(phase)
          {

                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                  {
                    analogWrite(IO0, 500);
                    analogWrite(IO2, 0);
                    delay(10000);
                  }
                  break;

                case 5:
                  {
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 0);
                    delay(2000);
                  }
                  break;

                case 6:
                  {
                    analogWrite(IO0, 1023);
                    analogWrite(IO2, 0);
                    delay(4000);
                  }
                  break;

                case 7:
                  {
                    analogWrite(IO0, 300);
                    analogWrite(IO2, 0);
                    delay(2000);
                  }
                  break;

                case 8:
                  {
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 0);
                    delay(50);
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 500);
                    delay(400);
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 0);
                    delay(50);
                  }
                  break;

                case 9:
                  {
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 0);
                    delay(50);
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 1000);
                    delay(3000);
                    analogWrite(IO0, 0);
                    analogWrite(IO2, 0);
                    delay(50);
                  }
                  break;

                default:
                  break;
                  
              }// End switch-case
        } // End While
      }
      else if(sCmd.indexOf("FUNCTION4OFF")>=0)
      {
        analogWrite(IO0, 0);
        analogWrite(IO2, 0);
      }

    }
    
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<BR>";
    sResponse += "Jannik Lassen<BR>";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }



  
  
  // Ende der Loop
  
  client.print(sHeader);
  client.print(sResponse);
  

  client.stop();
  Serial.println("Client disonnected");
}
