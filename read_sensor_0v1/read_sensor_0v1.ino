//replace standart arduino ethernet library with
//this one https://github.com/WIZnet-ArduinoEthernet/Ethernet
//for this to work
//set values for IP address and MAC address

#include <DHT.h>
#include <vector>
#include <SPI.h>
#include <Ethernet.h>


#define DEBUG

// Network config//
byte mac[] = {
  0x30, 0x43, 0xA7, 0xC8, 0x78, 0x1E
};
IPAddress ip(192, 168, 1, 2);
EthernetServer server(80);


// Sensor config
#define DHTPIN 2           //Input pin
#define DHTTYPE DHT22      //Sensor type
DHT dht(DHTPIN, DHTTYPE);  //creates DHT object


//value storage
float hum;  //Stores humidity value
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


void setup() {

  #ifdef DEBUG
    Serial.begin(9600);     //time for serial monitor to kick in  
    delay(5000);
    Serial.println(client_was_here);
  #endif
  
  Ethernet.init(17);        //Use pin 17 for CS
  Ethernet.begin(mac, ip);  //Start an Ethernet object
  delay(500);



  //debug
  //Check for networkhardware, if nothing, wait 1 second
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No Networkhardware found");
    while (true) {
      delay(2000);  //delays in ms

      //debug
      Serial.println("I´m still standing...");
      if (Ethernet.linkStatus() != LinkON) {
        Serial.println("Most likely no networkcable");
      } else {
        Serial.println("Most likely networkcabel");
      }
    }
  } else {
    delay(5000);
    Serial.println("I´m going in");

    delay(500);
  }

  if (Ethernet.linkStatus() != LinkON) {
    Serial.println("Most likely no networkcable");
  } else {
    Serial.println("Most likely networkcabel");
  }
  //End debug

  server.begin();       //start server
  dht.begin();          //start dht instance
  werte.clear();        //clear vector, unnecessery?
}


void loop() {

  // Trimm length of vector
  if (werte.size() >= 10) {
    werte.erase(werte.begin(), werte.begin() + 2);
  }
   
  hum = dht.readHumidity();     //Read data and store it to variable hum
  if(hum != hum){
    error = true;
  }else{
    error = false;
    werte.push_back(hum);
    i = werte.end();
    i--;
  }

#ifdef DEBUG
  if(error){
    Serial.println("could not read data from sensor");
  }else{
    Serial.print("Humidity: ");
    Serial.println(*i);
    hum_average();
    Serial.println("jetzt kommt der server shit");
  }
#endif // DEBUG

  // //listen for incoming clients
  // EthernetClient client = server.available();
  // if (client) {
  //   client_was_here = true;
  //   bool currentLineIsBlank = true;  //necessary?
  //   while (client.connected()) {
  //     if (client.available()) {
  //       char c = client.read();
  //       if (c == '\n' && currentLineIsBlank) {
  //         // send a standard HTTP response header
  //         client.println("HTTP/1.1 200 OK");
  //         client.println("Content-Type: text/html");
  //         client.println("Connection: close");  // the connection will be closed after completion of the response
  //         client.println("Refresh: 5");         // refresh the page automatically every 5 sec
  //         client.println();
  //         client.println("<!DOCTYPE HTML>");
  //         client.println("<html>");
  //         //writing the value
  //         client.print(*i);
  //         client.println("<br />");
  //         client.println("</html>");
  //         break;
  //       }

  //       //important?
  //       if (c == '\n') {
  //         // you're starting a new line
  //         currentLineIsBlank = true;
  //       } else if (c != '\r') {
  //         // you've gotten a character on the current line
  //         currentLineIsBlank = false;
  //       }
  //     }
  //   }
  //   // give the web browser time to receive the data
  //   delay(50);
  //   // close the connection:
  // } else {
  //   Serial.println("NOOOOOOOOOOOOOOOOOOOOOO");
  // }
  // client.stop();
  // Serial.println(client_was_here);
  delay(5000);
}
