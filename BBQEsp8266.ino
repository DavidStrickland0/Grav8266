/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "max6675.h"

// Web Server
#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

// Max6675 Variables
int thermoDO = 12;
int thermoCS = 15;
int thermoCLK = 14;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Relay Variables
const int relay = 2;

// Serving Hello world
void getHelloWord() {
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

// Serving Hello world
void getSettings() {
    String response = "{";
 
    response+= "\"ip\": \""+WiFi.localIP().toString()+"\"";
    response+= ",\"gw\": \""+WiFi.gatewayIP().toString()+"\"";
    response+= ",\"nm\": \""+WiFi.subnetMask().toString()+"\"";
 
    if (server.arg("signalStrength")== "true"){
        response+= ",\"signalStrengh\": \""+String(WiFi.RSSI())+"\"";
    }
 
    if (server.arg("chipInfo")== "true"){
        response+= ",\"chipId\": \""+String(ESP.getChipId())+"\"";
        response+= ",\"flashChipId\": \""+String(ESP.getFlashChipId())+"\"";
        response+= ",\"flashChipSize\": \""+String(ESP.getFlashChipSize())+"\"";
        response+= ",\"flashChipRealSize\": \""+String(ESP.getFlashChipRealSize())+"\"";
    }
    if (server.arg("freeHeap")== "true"){
        response+= ",\"freeHeap\": \""+String(ESP.getFreeHeap())+"\"";
    }
    response+="}";
 
    server.send(200, "text/json", response);
}
 
// Define routing
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server"));
    });
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on(F("/settings"), HTTP_GET, getSettings);
    server.on(F("/temperature"), HTTP_GET, getTemperature);
}

// Variables to be exposed to the API
int currentTemperature;
int targetTemperature;
 void getTemperature() {
  if(server.arg("temp")!="")
  {
    targetTemperature = server.arg("temp").toInt();
  }
  server.send(200, "text/json", "{ currenttemperature:"+String(currentTemperature)+", targettemperature:"+String(targetTemperature)+"}");
 }
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// WiFi parameters
const char* ssid = "Strickland";
const char* password = "PacoPaco11";


void setup(void)
{
  //Setup Relay
  pinMode(relay, OUTPUT);

  //Add Routing
  restServerRouting();
  server.begin();  
  
  // Start Serial
  Serial.begin(9600);

  // Init variables and expose them to REST API
  currentTemperature = 0;
  targetTemperature = 100;

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Set server routing
  restServerRouting();
  
  // Set not found response
  server.onNotFound(handleNotFound);
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}
int loopCount = 0;
void loop() {
  loopCount++;
  if(loopCount%1000000 == 0)
  {
    currentTemperature = thermocouple.readFahrenheit();
    if(currentTemperature > targetTemperature)
    {
      Serial.println(":Current Temp "+String(currentTemperature)+" is High Turning Off Fan");
      digitalWrite(relay,HIGH);
    }
    else
    {
      Serial.println(":Current Temp "+String(currentTemperature)+" is Low Turning on Fan");
      digitalWrite(relay,LOW);
    }
    loopCount = 0;
  }
  server.handleClient();
}

//// Custom function accessible by the API
//int ledControl(String command) {

//  // Get state from command
//  int state = command.toInt();

//  digitalWrite(6,state);
//  return 1;
//}
