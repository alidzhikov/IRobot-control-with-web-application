#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ESP8266mDNS.h> 
#include <FS.h>
#include <ArduinoJson.h>

const char* ssid = "Red1234";
const char* password = "012345678";
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
//SoftwareSerial sw(13, 15); 5 and 4
const int trigPin = 5;
const int echoPin = 4;
long duration;
int distance;
String page = "";
String getContentType(String filename); // convert the file extension to the MIME type
bool handleServerFileRequest(String fileName); // send the right file to the client (if it exists)
//spirane problem skorost problem song problem kvadratno 2 pluzgacha i start i stop zaglavnata i deklaraciqta dvustranno 30reda
StaticJsonBuffer<200> jsonBuffer;

void setup(void){   
  delay(2000); // NEEDED!!!! To let the robot initialize
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(57600);
  while (!Serial) { }
  //sw.begin(57600);
  SPIFFS.begin();  
  page = handleFileRead("/control-iRobot-create.html");
  //configAddress();
  WiFi.begin(ssid, password); //begin WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS responder started");
  }else{
    Serial.println("Error setting up MDNS responder!");
  }
   
  server.on("/", [](){ server.send(200, "text/html", page); });
  server.on("/irobotOn", irobotOn);
  server.on("/operatingMode", changeOperatingMode);
  server.on("/startDemo", startDemo);
  server.on("/irobotLeds", irobotLeds);
  server.on("/driveWithJoystick", driveWithJoystickHandle);
  server.on("/driveInASquare", driveInASquareHandle);
  server.on("/driveDiff", driveDiffHandle); 
  server.on("/driveStop", driveStopHandle); 
  server.on("/playSong", playSongHandle);
  server.on("/writeSong", writeSongHandle);
  server.on("/files", handleFileRequest);
  
  server.begin(); 
  Serial.println("Web server started!");
}
 
void loop(void){
  server.handleClient();
  measureFrontDistance();
}

void irobotOn(){
  Serial.println("irobotOn was called");
  Serial.println(128);
  Serial.write(128);
  server.send(200, "application/json","{iRobotState: on}");
  delay(1000); 
}
void changeOperatingMode(){
  String postData = server.arg("plain");
  JsonObject& root = jsonBuffer.parseObject(postData);
  int newMode = root["mode"];   
  Serial.println("changeOperatingMode was called");
  Serial.println(newMode);
  Serial.write(newMode);
  jsonBuffer.clear();
  server.send(200, "application/json", "{ newMode: newMode}");
  delay(1000); 
}
void startDemo(){
  String postData = server.arg("plain");
  JsonObject& root = jsonBuffer.parseObject(postData);
  int whichDemo = root["demo"];
  String byteSeq = "";
  byteSeq += (String)136 + " " +  (String)whichDemo;
  Serial.println("startDemo was called");
  Serial.println(byteSeq);
  Serial.write(136);
  Serial.write((byte)whichDemo);
  jsonBuffer.clear();
  server.send(200, "application/json", "{startDemo: byteSeq}");
  delay(1000); 
}
void irobotLeds(){
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);
    int leds = root["leds"][0];
    int color = root["leds"][1];
    int intensity = root["leds"][2];
    String byteSeq = "";
    byteSeq += (String)139 + " " +  (String)leds + " " + (String)color + " " + (String)intensity;
    Serial.println("irobotLeds was called");
    Serial.println(byteSeq);
    Serial.write(139);
    Serial.write((byte)leds);
    Serial.write((byte)color);
    Serial.write((byte)intensity);
    jsonBuffer.clear();
    server.send(200, "application/json", "{irobotLeds: byteSeq}");
    delay(1000); 
}
void driveWithJoystickHandle() {
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);
    int velocity0 = root["velocity"][0];
    int velocity1 = root["velocity"][1];
    int radius0 = root["radius"][0];
    int radius1 = root["radius"][1]; 
    int dataBytes[] = {137, velocity0, velocity1, radius0, radius1};
    //Serial.println("driveWithJoystick was called");
    String byteSeq = "";
    byteSeq += (String)137 + " " +  (String)velocity0 + " " + (String)velocity1 + " " + (String)radius0 + " " + (String)radius1;
   Serial.println(byteSeq);
    Serial.write(137);
    Serial.write((byte)velocity0);
    Serial.write((byte)velocity1);
    Serial.write((byte)radius0);
    Serial.write((byte)radius1);                       
    jsonBuffer.clear();
    server.send(200, "application/json", "{dataBytes: byteSeq}");
    delay(1000); 
}
void driveInASquareHandle() {
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);
    Serial.write(152);
    Serial.write((byte)17);
    Serial.write((byte)137);
    Serial.write((byte)1);
    Serial.write((byte)44);
    Serial.write((byte)128);
    Serial.write((byte)0);
    Serial.write((byte)156);
    Serial.write((byte)1);
    Serial.write((byte)144);
    Serial.write((byte)137);
    Serial.write((byte)1);
    Serial.write((byte)44);
    Serial.write((byte)0);
    Serial.write((byte)1);
    Serial.write((byte)157);
    Serial.write((byte)0);
    Serial.write((byte)90);
    Serial.write((byte)153);
    Serial.read();
    Serial.write(153);
    
    //152 17 137 1 44 128 0 156 1 144 137 1 44 0 1 157 0 90 153
    jsonBuffer.clear();
    server.send(200, "application/json", "{dataBytes: 23}");
    delay(1000); 
}
void driveStopHandle(){
  String postData = server.arg("plain");
  JsonObject& root = jsonBuffer.parseObject(postData);
  driveStopCommand();
  jsonBuffer.clear();
  server.send(200, "application/json", "{irobotStopped: true}");
  delay(1000); 
}
void driveStopCommand(){
  Serial.write(137);
  Serial.write((byte)0);
  Serial.write((byte)0);
  Serial.write((byte)0);
  Serial.write((byte)0);
}

void driveDiffHandle(){
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);
    int motorA0 = root["motorA"][0];
    int motorA1 = root["motorA"][1];
    int motorB0 = root["motorB"][0];
    int motorB1 = root["motorB"][1]; 
    int dataBytes[] = {137, motorA0, motorA1, motorB0, motorB1};
    Serial.write(137);
    Serial.write((byte)motorA0);
    Serial.write((byte)motorA1);
    Serial.write((byte)motorB0);
    Serial.write((byte)motorB1);                       
    jsonBuffer.clear();
    String byteSeq = "{dataBytes: 137 ";
    byteSeq += (String)motorA0 + " " + (String)motorA1 + " " + (String)motorB0 + " " + (String)motorB1;
    byteSeq += "}";
    server.send(200, "application/json", byteSeq);
    delay(1000); 
}

void handleFileRequest(){
    if (!handleServerFileRequest(server.arg("file"))){
        server.send(404, "text/plain", "404: Not Found");
    }
}
String handleFileRead(String path) { 
  if (SPIFFS.exists(path)) {                            // If the file exists
    Serial.println("file exists");
    File file = SPIFFS.open(path, "r");                 // Open it
    String pageRead = "";
    while(file.available()){
      pageRead = pageRead + (char)file.read();
    }
    file.close();                                       // Then close the file again
    return pageRead;
  }
  Serial.println("\tFile Not Found");
  return "";                                         // If the file doesn't exist, return false
}
String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}
bool handleServerFileRequest(String fileName) { // send the right file to the client (if it exists)
  String filePath = "/" + fileName;
  Serial.println("handleServerFileRequest: " + filePath);
  String contentType = getContentType(filePath);            // Get the MIME type
  if (SPIFFS.exists(filePath)) {                            // If the file exists
    File file = SPIFFS.open(filePath, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
void configAddress(){
  IPAddress ip(192, 168, 0, 101); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 0, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your

  WiFi.config(ip, gateway, subnet);
}
void measureFrontDistance(){
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance= duration*0.034/2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);
    if(distance<25){
      driveStopCommand();
      playSong(0);
    }
    delay(500);
}
void playSongHandle(){
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);
    int songId = root["songId"];
    playSong(songId);
    jsonBuffer.clear();
    server.send(200, "application/json", "{songId: songId}");
    delay(1000); 
}

void writeSongHandle(){
    String postData = server.arg("plain");
    JsonObject& root = jsonBuffer.parseObject(postData);

    Serial.write(140);
    Serial.write(0);
    Serial.write(9);
    Serial.write(60);
    Serial.write(64);
    Serial.write(65);
    Serial.write(40);

    Serial.write(140);
    Serial.write(1);
    Serial.write(9);
    Serial.write(66);
    Serial.write(67);
    Serial.write(68);
    Serial.write(40);

    jsonBuffer.clear();
    server.send(200, "application/json", "{songId: songId}");
    delay(1000); 
}
void playSong(int songId){
   Serial.write(140);
    Serial.write((byte)0);
    Serial.write((byte)9);
    Serial.write((byte)60);
    Serial.write((byte)12);
    Serial.write((byte)65);
    Serial.write((byte)12);
    Serial.write((byte)69);
    Serial.write((byte)12);
    
    Serial.read();
   Serial.write(141);
   Serial.write((byte)0);
}
