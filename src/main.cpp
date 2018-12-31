// Coded by: Jhunu

// Includes
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <ArduinoOTA.h>

// Web definitions
const char* ssid = "aranha";
const char* password = "allankardec";
WiFiServer server(80);

// Pins definitions
const int lamp1 = 16; //D0
const int lamp2 = 14; //D5
const int reset = 12; //D6

// lamp states
int lamp1_state_value = 0;
int lamp2_state_value = 0;

void setup() {
  pinMode(lamp1,OUTPUT);
  pinMode(lamp2,OUTPUT);
  pinMode(reset,OUTPUT);
  digitalWrite(reset,LOW);

  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failamp! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // OTA sequence
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("stahliroom");

  // No authentication by default
  ArduinoOTA.setPassword("8913020");

  // Password can be set with it's md5 value1 as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  // Boot sequence
  Serial.println("Stahli Project");
  Serial.println("Version 1.0.4");
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start web server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  ArduinoOTA.handle();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  if (request.indexOf("/lamp1=ON") != -1)  {
    lamp1_state_value = HIGH;
  }
  if (request.indexOf("/lamp1=OFF") != -1)  {
    lamp1_state_value = LOW;
  }

  if (request.indexOf("/lamp2=ON") != -1)  {
    lamp2_state_value = HIGH;
  }
  if (request.indexOf("/lamp2=OFF") != -1)  {
    lamp2_state_value = LOW;
  }

  if (request.indexOf("/lamp12=ON") != -1)  {
    lamp1_state_value = HIGH;
    lamp2_state_value = HIGH;
  }
  if (request.indexOf("/lamp12=OFF") != -1)  {
    lamp1_state_value = LOW;
    lamp2_state_value = LOW;
  }
  if (request.indexOf("/reset=TRUE") != -1)  {
    digitalWrite(lamp1, LOW);
    digitalWrite(lamp2, LOW);
    ESP.reset();
  }

  // Set lampPin according to the request
  digitalWrite(lamp1, lamp1_state_value);
  digitalWrite(lamp2, lamp2_state_value);

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("<p> Lamp 1 pin is now: ");
  if(lamp1_state_value == HIGH) {
    client.print("On </p>");
  } else {
    client.print("Off </p>");
  }

  client.print("<p> Lamp 2 pin is now: ");
  if(lamp2_state_value == HIGH) {
    client.print("On </p>");
  } else {
    client.print("Off </p>");
  }

  client.println("<br><br>");
  client.println("<a href=\"/lamp1=ON\"\"><button> Turn On Lamp 1</button></a>");
  client.println("<a href=\"/lamp1=OFF\"\"><button> Turn Off Lamp 1</button></a><br />");
  client.println("<a href=\"/lamp2=ON\"\"><button> Turn On Lamp 2</button></a>");
  client.println("<a href=\"/lamp2=OFF\"\"><button> Turn Off Lamp 2</button></a><br />");
  client.println("<a href=\"/lamp12=ON\"\"><button> Turn On Lamp 1 and 2</button></a>");
  client.println("<a href=\"/lamp12=OFF\"\"><button> Turn Off Lamp 1 and 2</button></a><br />");
  client.println("<a href=\"/reset=TRUE\"\"><button> Reset</button></a><br />");
  client.println("</html>");

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}
