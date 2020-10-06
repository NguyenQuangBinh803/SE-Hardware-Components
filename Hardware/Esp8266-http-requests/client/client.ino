// Setup the client to send sensor data to the server
#include <ESP8266WiFi.h>

// Initialize sensor parameters
float volts = 0.0, temperatureC = 0.0;

// Initialize network parameters
const char* ssid = "DAT";
const char* password = "000000000";
const char* host = "192.168.11.4"; // as specified in server.ino

// Set up the client objet
WiFiClient client;

// Configure deep sleep in between measurements
const int sleepTimeSeconds = 2;

void setup() {
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connect to server with IP Address (AP): "); Serial.println(WiFi.localIP());
}

void loop() {
//  Serial.print("Temperature C: "); Serial.println(temperatureC);
  String inData;
  // Connect to the server and send the data as a URL parameter
  if(client.connect(host,80)) {
    if (Serial.available()){
      inData = Serial.readStringUntil('\n');
      if (inData == "ON"){
        Serial.print("===============================\n");
        Serial.println(inData+"\n");
        String url = "/update?value=";
        url += String(inData);
        client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +  "\r\n" + 
                     "Connection: keep-alive\r\n\r\n"); // minimum set of required URL headers
        Serial.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +  "\r\n" + 
                     "Connection: keep-alive\r\n\r\n");
//        Serial.print("===============================================\n");
//        Serial.print(String(client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +  "\r\n" + 
//                     "Connection: keep-alive\r\n\r\n")));
        delay(10);
        // Read all the lines of the response and print them to Serial
//        Serial.println("Response: ");
//        while(client.available()){
//          String line = client.readStringUntil('\r');
//          Serial.print(line+"\n");
//          }
        }

      if (inData == "OFF"){
        Serial.print("===============================\n");
        Serial.println(inData+"\n");
        String url = "/update?value=";
        url += String(inData);
        client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +  "\r\n" + 
                     "Connection: keep-alive\r\n\r\n"); // minimum set of required URL headers
        delay(10);
        // Read all the lines of the response and print them to Serial
        Serial.println("Response: ");
        while(client.available()){
          String line = client.readStringUntil('\r');
          Serial.print(line+"\n");
          }
        }
    }
  }
}
