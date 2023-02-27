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
  0x30, 0x43, 0xA7, 0xC8, 0x78, 0x2E
};
IPAddress ip(172, 60, 48, 61);  // ip benötigt falls kein dhcp verwendet
EthernetServer server(80);
bool dhcp = true;               // true if dhcp is to be used


// Sensor config
#define DHTPIN 2           //Input pin
#define DHTTYPE DHT22      //Sensor type
DHT dht(DHTPIN, DHTTYPE);  //creates DHT object


//value storage
float hum;
int iDHCP_configured = 0;
bool error = false;
std::vector<float> werte;
std::vector<float>::iterator i;


//timer
unsigned long time;


#ifdef DEBUG
bool client_was_here = false;
#endif  // DEBUG


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
  Serial.begin(9600);  //time for serial monitor to kick in
  delay(5000);
  Serial.println(client_was_here);
#endif

  Ethernet.init(17);        //Use pin 17 for CS
  if(dhcp == true){
    while(iDHCP_configured != 1){
      iDHCP_configured = Ethernet.begin(mac);    //Startz an ehternet object in dhcp mode
      //Ethernet.begin(mac);
        #ifdef DEBUG
        Serial.println(iDHCP_configured);
        Serial.println("i tried to dhcp");
        #endif
    }
  }else{
    Ethernet.begin(mac, ip);  //Start an ethernet object without dhcp
  }
  delay(500);

#ifdef DEBUG
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {  //Check for networkhardware, if nothing, wait 1 second
    Serial.println("No Networkhardware found");
    while (true) {
      delay(2000);
      Serial.println("I´m still standing...");
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
  while (Ethernet.linkStatus() != LinkON) {
    delay(2000);
  }
#endif

  server.begin();  //start server
  dht.begin();     //start dht instance
  pinMode(LED_BUILTIN, OUTPUT);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  delay(500);
  if (werte.size() >= 10) {
    werte.erase(werte.begin(), werte.begin() + 2);  // Trimm length of vector to 10 data points
  }

  hum = dht.readHumidity();  //Read data and store it to variable hum
  if (hum != hum) {
    error = true;
  } else {
    error = false;  //reset error flag
    werte.push_back(hum);
    i = werte.end();
    i--;
    for (int n = 0; n < 5; n++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(150);
      digitalWrite(LED_BUILTIN, LOW);
      delay(150);
    }
  }

#ifdef DEBUG
  if (error) {
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
  time = millis();
#endif

  EthernetClient client = server.available();
  while (!client) {
#ifdef DEBUG
    Serial.println("Ich war hier");
#endif
    delay(250);
    EthernetClient client = server.available();
    if (client) {
      break;
    }
    //client==true ? break : continue;
  };  //listen for incoming clients


#ifdef DEBUG
  Serial.println("Time passed since started to listen for clients");
  Serial.println(millis() - time);
#endif


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
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 2");                        // refresh the page automatically every x seconds
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          error == false ? client.print(*i) : client.print("NAN");  //writing the value
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {  // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;  // you've gotten a character on the current line
        }
      }
    }
    delay(50);  // give the web browser time to receive the data
  }

#ifdef DEBUG
  else {
    client_was_here = false;
  }
#endif

  client.stop();
  Ethernet.maintain();      //function to renew dhcp lease. In this case probably not needed
#ifdef DEBUG
  Serial.println(client_was_here);
#endif

  //delay(5000);
}
