#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;

const char* ssid = "SERVER";
const char* password = "12345678";
const int led = 13;

//Your IP address or domain name with URL path
const char* serverNamePath = "http://192.168.4.1/path";
const char* serverNameJunction = "http://192.168.4.1/junction";

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



String path;
String junction;
int i=0,j, d[]={0};
int k=0;                         //k stands for the currrent junction number of the slave


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  while((WiFiMulti.run() == WL_CONNECTED)) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
 
}

void loop() {
 
  // put your main code here, to run repeatedly:

                 //Recieving the values of j and d[j]
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    junction= httpGETRequest(serverNameJunction);  //gets the string of final junction no., that is j
    j= junction.toInt();
  
    
    path  = httpGETRequest(serverNamePath);  //gets the string of path
    Serial.println("Path " + path  );
    for(i=0;i<j;i++) {
       d[i+1]= (path.charAt(i) - '0') ;
    }
    
                      //Sensing
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

                       // Motion Control

    if(cm_s < 25 || cm_l > 50 || cm_r > 50){
        
      k++ ;                                                          //increments value of k at a junction
      Serial.print((String)"\n Junction number"+ k); 
      stopBot();
  
      if(d[k]==1){
        rightTurn(70);
        forward_s(80);
        forward();      
      }
      else if(d[k]==2){
        forward();
      }
      else if(d[k]==3){
        leftTurn(70);
        forward_s(80);
        forward();  
      }
      else{
        stopBot();
      }
      
      
        
    }
    else {
      forward();
    }
   
                          
  
       
       
  }
  else {
     Serial.println("Wifi Disconnected");
  }



}

                // Client GETRequest

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
   http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

                            //Motor
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

                            //Sensor
long microsecondsToCentimeters(long microseconds){
    return (microseconds / 29) / 2;
  }
                            
