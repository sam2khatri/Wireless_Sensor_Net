/******************************************************************************************************
*                                                                                                     *
*                               Author:- Samaksh Khatri                                               *
*                                                                                                     *
******************************************************************************************************/ 


// Libraries for Firebase -----------------------------------------------------------------------------------------------------------------------------

#include <FirebaseESP8266.h>                  
#include <FirebaseESP8266HTTPClient.h>         
#include <FirebaseJson.h>

// Libraries to setup Access Point and Station Point ---------------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>                   
#include <ESP8266WebServer.h>

// Libraries for sending Email -------------------------------------------------------------------------------------------------------------------------

#include <WiFiServer.h>
#include <WiFiServerSecure.h>                   
                  
// Setting Global Variables ----------------------------------------------------------------------------------------------------------------------------

#define FIREBASE_HOST "yourLink"        //Link and Secret Key for Firebase Authentication
#define FIREBASE_AUTH "yourSecretKey"

const char* APssid = "ESP8266-Access-Point";       //Credentials for Access POint
const char* APpassword = "123456789";
const char* ssid = "";                      //Credentials for connecting to a WiFi
const char* password = "";

char smtp_server[] = "mail.smtp2go.com";          // The SMTP Server
String message="";                                //Global String variable to store data from sensor nodes

ESP8266WebServer server(80);                      //Server Object. Port number is 80 for HTTP

WiFiClient espClient;                             //Client Object to send Emails

FirebaseData firebaseObj;                                  //Firebase Object which will contain path and payload 
String firebasePath="";                                    //Your path in the Firebase
// Setting up WiFi, WebServer and Firebase -----------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(10);
  
 // Setting up WiFi -------------------------------
  Serial.println();
  Serial.print("Configuring access point...");
  
  WiFi.mode(WIFI_AP_STA);                               //WIFI_AP_STA sets up the WiFi in both Station Mode and Soft Access Point Mode
  WiFi.softAP(APssid, APpassword);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   }
  
  Serial.println("");
  Serial.println("WiFi connected");
   
 // Setting up the server ------------------------
  server.on("/",handleGenericArgs);           //handleGenericArgs is a handler function that will be called whenever '/' is requested by a client
                                              //You can omit this function and its handler
  server.on("/sensor",handleSpecificArg);      //Similarly handleSpecificArg is also a handler function
  server.begin();                             //Starting the server
  Serial.println("Server started");
   
 // Printing Local and Access Point IP addresses ----------------------------------
  Serial.println(WiFi.localIP());         
  Serial.println(WiFi.softAPIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);     //Initialising Firebase connection
}

void loop() {
server.handleClient();                              //Handles Client Requests and calls the appropriate functions
}

//Handler Functions ----------------------------------------------------------------------------------------------------------------------------------
void handleGenericArgs(){                           
  /*message = "Number of args received: ";
  message+=server.args();
  message+="\n";*/

  for (int i=0;i<server.args();i++)          
  {
    message += (String)i+"->";                      //Collecting Data from Client
    message += server.argName(i)+": ";
    message += server.arg(i)+"\n";
  }
  Serial.println(message);
  server.send(200,"text/plain","Received Data");
}

void handleSpecificArg()
{
  if (!server.hasArg("ldrstatus") && server.arg("ldrstatus")==NULL && !server.hasArg("temp") && server.arg("temp")== NULL 
        && !server.hasArg("hum") && server.arg("hum")== NULL
        && !server.hasArg("soilMoisture") && server.arg("soilMoisture")== NULL)                                //Sending response to client that data is invalid
   { 
     message="Data Invalid";
     server.send(400,"text/plain",message);
   }
  else
    {
      String ldr_status = (String)server.arg("ldrstatus");
      String temp = (String)server.arg("temp");
      String hum = (String)server.arg("hum");
      String soilMoisture = (String)server.arg("soilMoisture");
      
      message="LDR Status = "+ldr_status;                            
      message+="\nTemperature = "+temp+"C";
      message+="\nHumidity = "+hum+"%";
      message+="\nSoil Moisture = "+soilMoisture+"%"; 
      Serial.println(message);
      byte ret = sendEmail(message);                                                    //Sending email whenever handler function is called and data is valid
      if (ret)
        Serial.println("Email Sent Successully!");
      else
        Serial.println("Error in sending email!");
      Firebase.setString(firebaseObj,firebasePath+"/ldr",ldr_status);    //Pushing data to Firebase
      Firebase.setInt(firebaseObj,firebasePath+"/temp",temp.toInt());
      Firebase.setInt(firebaseObj,firebasePath+"/hum",hum.toInt());
      Firebase.setInt(firebaseObj,firebasePath+"/soil",soilMoisture.toInt());
      server.send(200,"text/plain","Received Data");                                                //Sending repsonse to client that valid data is received
    }   
}

// Function Declaration to Send Email --------------------------------------------------------------------------------------------------------------------

byte sendEmail(String sensor_data)
{
  
  if (espClient.connect(smtp_server, 2525) == 1)                                  //Establishing connection to mail server
  {
    Serial.println(F("connected"));
  }
  else
  {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!emailResp())
    return 0;

  Serial.println(F("Sending EHLO"));                      //Sending EHLO to server. It is equivalent to greeting the server. Initially it was HELO but was changed later
  espClient.println("EHLO www.example.com");              //Sending www.example.com as a domain. It is a reserved domain that can be used without any permission
  if (!emailResp())
    return 0;

  Serial.println(F("Sending auth login"));                //Sending authentication
  espClient.println("AUTH LOGIN");
  if (!emailResp())
    return 0;

  Serial.println(F("Sending User"));                      //Sending Username of SMTP2GO account
  espClient.println("-----------------------");           // Encoded Username in Base64 format. You can calculate it from base64encoded.org  
  if (!emailResp())
    return 0;

  Serial.println(F("Sending Password"));
  espClient.println("-----------------------");                  //Encoded Password in Base64 Format
  if (!emailResp())
    return 0;

  Serial.println(F("Sending From"));
  espClient.println(F("MAIL From: senderemailid@example.com")); // Enter Sender Mail Id. DON'T FORGET TO CHANGE THE EMAIL ADDRESS
  if (!emailResp())
    return 0;

  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: receiveremailid@example.com")); // Enter Receiver Mail Id. DON'T FORGET TO CHANGE THE EMAIL ADDRESS
  if (!emailResp())
    return 0;

  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp())
    return 0;

  Serial.println(F("Sending email"));
  espClient.println(F("To:  receiveremailid@example.com"));    // Enter Receiver Mail Id. DON'T FORGET TO CHANGE THE EMAIL ADDRESS
  espClient.println(F("From: senderemailid@example.com"));    // Enter Sender Mail Id. DON'T FORGET TO CHANGE THE EMAIL ADDRESS
  espClient.println(F("Subject: Sensor Status\r\n"));
  espClient.println(sensor_data);
  espClient.println(F("."));

  if (!emailResp())
    return 0;

  Serial.println(F("Sending QUIT"));                    //Sending QUIT Indicating Email is over
  espClient.println(F("QUIT"));
  if (!emailResp())
    return 0;

  espClient.stop();
  Serial.println(F("disconnected"));                    //Disconnecting from Mail Server
  return 1;
}

 

byte emailResp()                                        //Function to handle response by Mail Server
{
  byte responseCode;
  byte readByte;
  int loopCount = 0;
 
  while (!espClient.available())
  {
    delay(1);
    loopCount++;

    if (loopCount > 20000)
    {
      espClient.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  responseCode = espClient.peek();
  while (espClient.available())
  {
    readByte = espClient.read();
    Serial.write(readByte);
  }

  if (responseCode >= '4')
  {
    return 0;
  }
  return 1;
}
