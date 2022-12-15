

#include <DHT.h>
#include <vector>
#include <SPI.h>
#include <Ethernet.h>

//replace ethernet library with
//this one https://github.com/WIZnet-ArduinoEthernet/Ethernet
//for this to work


// Network config//
byte mac[] = {
  0x30, 0x43, 0xA7, 0xC8, 0x78, 0x1E
};
IPAddress ip(192, 168, 1, 2);
EthernetServer server(80);


// Sensor config//
#define DHTPIN 2           //Input pin
#define DHTTYPE DHT22      //Sensor type
DHT dht(DHTPIN, DHTTYPE);  //creates DHT object


//value storage
float hum;  //Stores humidity value
std::vector<float> werte;

//debug
bool client_was_here = false;

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

  Ethernet.init(17);
  //debug, remove
  Serial.begin(9600);

  //time for serial monitor to kick in
  delay(5000);
  Serial.println(client_was_here);
  //Start an Ethernet object

  delay(500);
  Ethernet.begin(mac, ip);
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

  //start server
  server.begin();

  dht.begin();    //start dht instance
  werte.clear();  //clear vector, unnecessery?
}


void loop() {

  if (werte.size() >= 10) {
    werte.erase(werte.begin(), werte.begin() + 2);
  }
  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();

  hum != hum ? Serial.println("Oh no") : Serial.println("Go on");

  werte.push_back(hum);
  std::vector<float>::iterator i = werte.end();
  i--;

  // delay(500);  //Delay 2 sec.

  Serial.print("Humidity: ");
  Serial.println(*i);
  hum_average();

  delay(500);
  Serial.println("jetzt kommt der server shit");

  //listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    client_was_here = true;
    bool currentLineIsBlank = true;  //necessary?
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");         // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          //writing the value
          client.print(*i);
          client.println("<br />");
          client.println("</html>");
          break;
        }

        //important?
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(50);
    // close the connection:
  } else {
    Serial.println("NOOOOOOOOOOOOOOOOOOOOOO");
  }
  client.stop();
  Serial.println(client_was_here);
  delay(5000);
}
