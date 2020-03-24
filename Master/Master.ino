#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


#define STASSID "SERVER"                           //AP ssid and password
#define STAPSK  "12345678"

int val = 1023;
const char* ssid = STASSID;
const char* password = STAPSK;
const int led = 13;

ESP8266WebServer server(80);

const int rmf = 9;                        //Right motor forward
const int rmb = 10;                        //Right motor backward
const int lmf = 5;                       //Left motor forward
const int lmb = 3;                        //Left motor backward

const int trigPin_l = 11;                 //Left sensor trigger pin
const int echoPin_l = 12;                //Left sensor echo pin
const int trigPin_r = 7;                  //Right sensor trigger pin
const int echoPin_r = 6;                 //Right sensor echo pin
const int trigPin_s = 4;                  //Straight sensor trigger pin
const int echoPin_s = 8;                 //Straight sensor echo pin
long duration_r, cm_r, duration_l, cm_l, duration_s, cm_s;  //establish variables for duration of the ping, and the distance result in cm
int lm_status, rm_status;         //left motor status, right motor status, 0 = none , 1 = forward, -1 = backward 


int i, arr[50],d[]={0},j=0,a=0; // i for sensor, arr[] for its value, j for junction,d[] for diection
int solve=0;
String command;



void setup() {
  Serial.begin(115200);
  
  pinMode(lmf, OUTPUT);
  pinMode(lmb, OUTPUT);
  pinMode(rmf, OUTPUT);
  pinMode(rmb, OUTPUT);
  pinMode(trigPin_l, OUTPUT);
  pinMode(trigPin_s, OUTPUT);
  pinMode(trigPin_r, OUTPUT);
  pinMode(echoPin_l, INPUT);
  pinMode(echoPin_s, INPUT);
  pinMode(echoPin_r, INPUT);

  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("");
  IPAddress myIP = WiFi.softAPIP() ; 
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
 
}

void loop() {
  
  digitalWrite(trigPin_l, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_l, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_l, LOW);
  duration_l = pulseIn(echoPin_l, HIGH);
  cm_l = microsecondsToCentimeters(duration_l);
  if(cm_l > 1000){
    cm_l = 0;
  }

  digitalWrite(trigPin_r, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_r, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_r, LOW);
  duration_r = pulseIn(echoPin_r, HIGH);
  cm_r = microsecondsToCentimeters(duration_r);
  if(cm_r > 1000){
    cm_r = 0;
  }

  digitalWrite(trigPin_s, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_s, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_s, LOW);
  duration_s = pulseIn(echoPin_s, HIGH);
  cm_s = microsecondsToCentimeters(duration_s);
  if(cm_s > 1000){
    cm_s = 0;
  }

  /*Serial.print(cm_l);
  Serial.print("cm , ");
  Serial.print(cm_s);
  Serial.print("cm , ");
  Serial.print(cm_r);
  Serial.println("cm");*/
 if(Serial.available() > 0)
 command = Serial.read();
  
 if(command=="Stop"){
  stopBot();
  solve=1 ;
  send_to_server();
 }
    
 else{   
   
  
  if(cm_s < 25 || cm_l > 50 || cm_r > 50){
    j++ ;                                                          //increments value of j at a junction
    Serial.print((String)"\n Junction number"+ j); 
    stopBot();
    
  }  
  
   if(cm_s < 25 && cm_l < 25 && cm_r < 25){                  //blocked, U-turn 
    rightTurn(140); 
    forward_s(80);
    stopBot();
    j=j-2;  
    }                                           


   else if(cm_r > 50){                                  //first priority for right turn, d[j]=1
    
    rightTurn(70);
    forward_s(80);
    stopBot();
    a= d[j];
    d[j] = 1;
    d[j]=d[j]+a;
    if(d[j]==4){
      d[j]=0;
      j=j-2;
      Serial.print((String)"\n junction number: " + j);
      
      
    }
    
   }
   else if(cm_s > 50 && cm_l > 50) {                                    //second priotity for straight, d[j]=2
    
    forward();  
    a= d[j];
    d[j] = 2;
    d[j]=d[j]+a;
    if(d[j]==4){
      d[j]=0;                                                          
      j=j-2;
      Serial.print((String)"\n junction number: " + j);
      
    }
    
    
     
   }
   else if(cm_l > 50){                                    //thirdly left turn, d[j]=3
    
    leftTurn(70);
    forward_s(80); 
    stopBot();
    a= d[j];
    d[j] = 3;
    d[j]=d[j]+a;
    if(d[j]==4){
      d[j]=0;
      j=j-2;
      Serial.print((String)"\n junction number: " + j);
      
    }
    
    
   }
   else if(cm_s > 50){
    forward();
   }
  
   for(i=1;i<=j;i++){
    Serial.print((String)"d["+i+"]="+d[i]);
   }
   path();
  
  
 }
}

void leftTurn(int time) {
  analogWrite(lmf, 0);
  analogWrite(rmb, 0);
  analogWrite(lmb, 220);
  analogWrite(rmf, 220);
  delay(time);
}

void rightTurn(int time) {
  analogWrite(lmb, 0);
  analogWrite(rmf, 0);
  analogWrite(lmf, 230);
  analogWrite(rmb, 230);
  delay(time);
}

void forward_s(int time){
  analogWrite(lmb, 0);
  analogWrite(rmb, 0);
  analogWrite(lmf, 150);
  analogWrite(rmf, 150);
  delay(time);
}

void forward()  {
  digitalWrite(trigPin_s, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_s, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_s, LOW);
  duration_s = pulseIn(echoPin_s, HIGH);
  cm_s = microsecondsToCentimeters(duration_s);

  digitalWrite(trigPin_l, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_l, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_l, LOW);
  duration_l = pulseIn(echoPin_l, HIGH);
  cm_l = microsecondsToCentimeters(duration_l);
  //Serial.print(cm_l);
  //Serial.print("cm , ");

  digitalWrite(trigPin_r, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_r, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_r, LOW);
  duration_r = pulseIn(echoPin_r, HIGH);
  cm_r = microsecondsToCentimeters(duration_r);
  //Serial.print(cm_r);
  //Serial.println("cm");

  if(cm_l > 500){cm_l = 0;}
  if(cm_l < 25){
    rightTurn(20);
  }
  else if(cm_r < 25){
    leftTurn(20); 
  }
  
  analogWrite(lmb, 0);
  analogWrite(rmb, 0);
  analogWrite(lmf, 150);
  analogWrite(rmf, 150);
  delay(50);
}

void stopBot(){
  analogWrite(lmb, 0);
  analogWrite(rmb, 0);
  analogWrite(lmf, 0);
  analogWrite(rmf, 0);
}

long microsecondsToCentimeters(long microseconds){
    return (microseconds / 29) / 2;
  }


  // Web Server functions

  void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "Swarm Robotics");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

void send_to_server() {
 
  server.on("/path", path);
  server.on("/junction",junction);
  server.onNotFound(handleNotFound);
  
 }

String junction() {
  String  intersection= String(j);
  server.send(200, "text/plain", intersection);
}

String path() {
 
   String way = convertToString(d,j);
    Serial.println(way);
    server.send(200, "text/plain", way);
}

String convertToString(int* a, int size) 
{ 
    int i,temp; 
    String s = ""; 
    for (i = 1; i <= size; i++) { 
      temp= a[i];
      s=s+String(temp);
    } 
    return s; 
} 
