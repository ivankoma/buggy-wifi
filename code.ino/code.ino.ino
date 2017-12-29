#include <ESP8266WiFi.h>

//////////////////////
// Config:          //
bool nodemcuAsAP = true;
const char WIFI_AP_NAME[] = "buggy";
const char WIFI_AP_PASS[] = "password";

//to connect to already existing wifi
const char WIFI_NAME[] = "name";
const char WIFI_PASS[] = "password";

const bool DEBUG = false;
//////////////////////

struct SENSOR{
  String forward;
  String left;
  String right;
  unsigned long timestamp_forward;
  unsigned long timestamp_left;
  unsigned long timestamp_right;
};

SENSOR sensor = {"-1", "-1", "-1"};

WiFiServer server(80);
WiFiClient client;

void setup() 
{
  Serial.begin(9600);
    
  if(nodemcuAsAP){
    setupWiFiAP();
  }else{
    setupWiFi();
  }

  server.begin();
}

void loop() 
{ 
  String received = "";
  char character;
  
  while(Serial.available() > 0){
      //character = Serial.read();  //this two don't because server.available() is moved below
      //received.concat(character); //Returns the first byte of incoming serial data available (or -1 if no data is available) - int 
      received = Serial.readString();  //this is really slow
  }  
  if (received != "") {
    if(DEBUG) Serial.println("received=" + received);

    if(received.indexOf("f=") == 0){
      if(DEBUG) Serial.println("front");
      sensor.forward=received;
      sensor.timestamp_forward = millis();    
    }else{
      //sensor.forward="-1";
    }

    if(received.indexOf("l=") == 0){
      if(DEBUG) Serial.println("left");
      sensor.left=received;
      sensor.timestamp_left = millis();
    }else{
      //sensor.left="-1";
    }
    
    if(received.indexOf("r=") == 0){
      if(DEBUG) Serial.println("right");
      sensor.right=received;
      sensor.timestamp_right = millis();
    }else{
      //sensor.right="-1";
    }
  }
  
  // Check if a client has connected (if somebody is sending http request to load a page)
  client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  //Serial.println(req);
  client.flush();

  if (req.indexOf("/get") != -1){
    respondSensorValues();
  }
  else if (req.indexOf("/go/l") != -1){
    Serial.print("l|");
    respond("ok");
  }
  else if (req.indexOf("/go/r") != -1){
    Serial.print("r|");
    respond("ok");
  }
  else if (req.indexOf("/go/f") != -1){
    Serial.print("f|");
    respond("ok");
  }
  else if (req.indexOf("/go/s") != -1){
    Serial.print("s|");
    respond("ok");
  }
  //read frontal sensor
  else if (req.indexOf("/read/f") != -1){
    Serial.print("rf|");
    respond("ok");
  }
  //read sensor on the left
  else if (req.indexOf("/read/l") != -1){
    Serial.print("rl|");
    respond("ok");
  }
  //read sensor on the right
  else if (req.indexOf("/read/r") != -1){
    Serial.print("rl|");
    respond("ok");
  }else{
    String s = "Invalid request. Try:<br>/get<br>/go/f /go/l /go/r /go/s<br>/read/f /read/l /read/f";
    respond(s);
  }
 
  //if there are no clients connected to buggy's AP stop buggy (when buggy goes out of wifi range)
  //this only works when nodemcu is AP
  
  if(nodemcuAsAP && WiFi.softAPgetStationNum()==0){ 
    Serial.print("s|");
    delay(500); 
  }

  client.flush();
  if(DEBUG) Serial.println("Client disonnected");
}

//connect to already existing IP address
void setupWiFi(){
    /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(DEBUG) Serial.print(".");
  }

  if (DEBUG){
    Serial.println("Connected to WiFi");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setupWiFiAP()
{
  WiFi.mode(WIFI_AP);
  String AP_NameString = WIFI_AP_NAME;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WIFI_AP_PASS);
}

void respond(String html){
    String s = "";
    s += "HTTP/1.1 200 OK\r\n";;
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n"; 
    s += html;
    s += "</html>\n";
    client.print(s);
    delay(1);
}

void respondSensorValues(){
  String sensorsHTML = "";
  sensorsHTML += sensor.forward + " (" + String((millis() - sensor.timestamp_forward) / 1000) + "s)";
  sensorsHTML += "<br>" + sensor.left + " (" + String((millis() - sensor.timestamp_left) / 1000) + "s)";
  sensorsHTML += "<br>" + sensor.right + " (" + String((millis() - sensor.timestamp_right) / 1000) + "s)"; 

  respond(sensorsHTML);
}