#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
  
#define DHTPIN 34
#define DHTTYPE DHT11

const char* ssid = "ESP8266-Access-Point";
const char* password ="123456789";

int ldr=3;
int soilPin=22;

String ldr_status;
String temp;
String hum;
String soilMoisture;
String payLoad;

DHT dht(DHTPIN,DHTTYPE);

void setup() {
  
  pinMode(ldr,INPUT);
  pinMode(soilPin,INPUT);
  pinMode(DHTPIN,INPUT);
  pinMode(soilPin,INPUT);
  
  Serial.begin(115200);
  delay(2000);   //Delay needed before calling the WiFi.begin
  
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to the WiFi network : "+(String)ssid);
} 
void readLDRStatus(){
  if (digitalRead(ldr)==HIGH)
    {
     ldr_status="ldrstatus=No Light";
    }   
  else
    { 
    ldr_status="ldrstatus=Light";
    }
}
void readDHTStatus()
{
 float t = dht.readTemperature();
 float h = dht.readHumidity();

 temp="temp="+(String)t;
 hum="hum="+(String)h;
}
void readSoilMoisture()
{
  int value= analogRead(soilPin);
  value = map(value,0,4055,100,0);
  soilMoisture="soilMoisture="+(String)value;
}
void generatePayload()
{
  readLDRStatus();
  readDHTStatus();
  readSoilMoisture();
  payLoad=ldr_status+"&"+temp+"&"+hum+"&"+soilMoisture;  
  Serial.println(payLoad);
}

void loop() {
  generatePayload();
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
   HTTPClient http;   

   http.begin("http://192.168.4.1/light");  //Specify destination for HTTP request
   
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");             //Specify content-type header
   int httpResponseCode = http.POST(payLoad);                             //Send the actual POST request
  
   if(httpResponseCode==200){
  
    //String response = http.getString();                       //Get the response to the request
  
    Serial.println(httpResponseCode);   //Print return code
    //Serial.println(response);           //Print request answer
  
   }
   else
   {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);  
   }
  
   http.end();  //Free resources
  
 }else{
  
    Serial.println("Error in WiFi connection");   
  
 }
  
  delay(10000);  //Send a request every 10 seconds
  
}
