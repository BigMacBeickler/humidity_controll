//replace standart arduino ethernet library with
//this one https://github.com/WIZnet-ArduinoEthernet/Ethernet
//for this to work
//set values for IP address and MAC address

#include <DHT.h>
#include <vector>
#include <SPI.h>
#include <Ethernet.h>


//#define DEBUG


// Network config//
byte mac[] = {
  0x30, 0x43, 0xA7, 0xC8, 0x78, 0x1E
};
IPAddress ip(172, 16, 60, 101); //trockenschrank 1
EthernetServer server(80);


// Sensor config
#define DHTPIN 2           //Input pin
#define DHTTYPE DHT22      //Sensor type
DHT dht(DHTPIN, DHTTYPE);  //creates DHT object


//value storage
float hum;
bool error = false;
std::vector<float> werte;
std::vector<float>::iterator i;


#ifdef DEBUG
bool client_was_here = false;
#endif // DEBUG


void hum_average() {
  float avr = 0;
  for (int n = 0; n < werte.size(); n++) {
    avr += werte[n];
  }
  avr = avr / werte.size();
  Serial.print("Average Humidity: ");
  Serial.println(avr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {



  #ifdef DEBUG  
    Serial.begin(9600);                                      //time for serial monitor to kick in  
    delay(5000);
    Serial.println(client_was_here);
  #endif
  
  Ethernet.init(17);                                           //Use pin 17 for CS
  Ethernet.begin(mac, ip);                                     //Start an Ethernet object
  delay(500);

  #ifdef DEBUG
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {     //Check for networkhardware, if nothing, wait 1 second
      Serial.println("No Networkhardware found");
      while (true) {
        delay(2000); 
        Serial.println ("I´m still standing...");
        if (Ethernet.linkStatus() != LinkON) {
          Serial.println("Most likely no networkcable");
        } else {
          Serial.println("Most likely networkcabel");
        }
        break;
      }
    } else {
      delay(5000);
      Serial.println("I´m going in");
      delay(500);
    }
  #endif

  #ifndef DEBUG
    while(Ethernet.linkStatus() != LinkON){
      delay(2000);
    }
  #endif
  
  server.begin();       //start server
  dht.begin();          //start dht instance
  pinMode(LED_BUILTIN, OUTPUT);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
delay(5000);
  if (werte.size() >= 10) {
    werte.erase(werte.begin(), werte.begin() + 2);             // Trimm length of vector to 10 data points
  }
   
  hum = dht.readHumidity();                                    //Read data and store it to variable hum
  if(hum != hum) {
    error = true;
  } else {
    error = false;                                             //reset error flag
    werte.push_back(hum);
    i = werte.end();
    i--;
    for(int n = 0; n < 5; n++){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
    }
  }

  #ifdef DEBUG
    if(error) {
      Serial.println("could not read data from sensor");
    } else {
      Serial.print("Humidity: ");
      Serial.println(*i);
      hum_average();
      Serial.println("here starts the server thingy");
    }
    if (Ethernet.linkStatus() != LinkON) {
    Serial.println("Most likely no networkcable");
  } else {
    Serial.println("Most likely networkcabel");
  }
  #endif

  EthernetClient client = server.available();                  //listen for incoming clients
  if (client) {
    
    #ifdef DEBUG
    client_was_here = true;
    #endif

    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");                 // the connection will be closed after completion of the response
          client.println("Refresh: 5");                        // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          error == false ? client.print(*i) :  client.print ("NAN");                                    //writing the value
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {                                       // you're starting a new line
            currentLineIsBlank = true;
        } else if (c != '\r') {
            currentLineIsBlank = false;                        // you've gotten a character on the current line
        }
      }
    }
    delay(50);                                                 // give the web browser time to receive the data
  } 
    
    #ifdef DEBUG
      else {
      client_was_here = false;
      }
    #endif

  client.stop();
   
  #ifdef DEBUG
    Serial.println(client_was_here);
  #endif

  //delay(5000);
}
