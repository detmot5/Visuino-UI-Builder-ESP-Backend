#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>


#define JSON_FROM_WEBSITE_SIZE 256

#define CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN  "Access-Control-Allow-Origin"
#define CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS "Access-Control-Allow-Methods"
#define CORS_ALLOWED_METHODS "POST,GET,OPTIONS"

#define CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS "Access-Control-Allow-Headers"
#define CORS_ALLOWED_HEADERS "Origin, X-Requested-With, Content-Type, Accept"



#define HTTP_STATUS_OK 200
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_INTERNAL_SERVER_ERROR 500

AsyncWebServer server(80);

const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

const char* DATATYPE_BOOLEAN = "boolean";
const char* DATATYPE_NUMBER = "number";






String serverInputFromVisuino;

String serverInputFromWebsite;

String booleanServerOutput;
String numberServerOutput;

StaticJsonDocument<JSON_FROM_WEBSITE_SIZE> jsonFromWebsite;



const char* testWebsiteConfigStr = {R"(
{
  "title" : "Cokolwiek",
  "elements" : [
    {
      "name" : "Led1",
      "type" : "boolean",
      "value": "false"
    },

    {
      "name" : "Led2",
      "type" : "boolean",
      "value": "true"
    },

    {
      "name" : "Led3",
      "type" : "boolean",
      "value": "false"
    },

    {
      "name" : "Led4",
      "type" : "boolean",
      "value": "true"
    },

    {
      "name" : "Led6",
      "type" : "boolean",
      "value": "true"
    },

    {
      "name" : "temperature1",
      "type" : "number",
      "value" : "62.5"
    }
  ]
})"};


StaticJsonDocument<1024> testWebsiteConfig;




void fullCorsAllow(AsyncWebServerResponse* response){
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS, CORS_ALLOWED_METHODS);
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, CORS_ALLOWED_HEADERS);
}


void sendDataToServerOutput(String& serverOutput){
  StaticJsonDocument<JSON_FROM_WEBSITE_SIZE> tmpJson;
  JsonObject obj = jsonFromWebsite.as<JsonObject>();
  tmpJson.clear();
  serverOutput.clear();

  tmpJson["name"] = obj["name"];
  tmpJson["value"] = obj["value"];



  String name = obj["name"];
  String elName;
  JsonArray arr = testWebsiteConfig["elements"].as<JsonArray>();
  for(auto el : arr){
    elName = el["name"].as<String>();
    if(elName.equals(name)){
      el["value"] = obj["value"];
    }
  }



  serializeJson(tmpJson, serverOutput);
}


void HTTPSendWebsiteFiles(AsyncWebServer& webServer){

  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.html");
  });

  webServer.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css","text/css");
  });

  webServer.on("/ValueDisplay.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ValueDisplay.css","text/css");
  });

  webServer.on("/numberInput.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/numberInput.css","text/css");
  });

  webServer.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.js","application/javascript");
  });

  webServer.on("/ValueDisplay.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ValueDisplay.js","application/javascript");
  });

  webServer.on("/numberInput.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/numberInput.js","application/javascript");
  });

  webServer.on("/renderer.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/renderer.js","application/javascript");
  });

  webServer.on("/Assets/bulbOn.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Assets/bulbOn.svg","image/svg+xml");
  });

  webServer.on("/Assets/bulbOff.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Assets/bulbOff.svg","image/svg+xml");
  });

  webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/favicon.ico","image/ico");
  });
}

void HTTPSetMappings(AsyncWebServer& webServer){

  webServer.on("/init", HTTP_GET, [] (AsyncWebServerRequest* request){
    AsyncWebServerResponse* response = request->beginResponse(HTTP_STATUS_OK);
  });

  webServer.on("/input", HTTP_GET, [] (AsyncWebServerRequest* request){
    AsyncWebServerResponse* response = request->beginResponse(HTTP_STATUS_OK, "application/json", testWebsiteConfig.as<String>());
    fullCorsAllow(response);
    request->send(response);
  });

  webServer.on("/status", HTTP_POST, [] (AsyncWebServerRequest* request){}, nullptr,
          [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){

    jsonFromWebsite.clear();
    DeserializationError status;
    status = deserializeJson(jsonFromWebsite, reinterpret_cast<const char*>(data), len);
    const char* dataType = jsonFromWebsite["type"];

    if(status == DeserializationError::Ok){
      if( !strncmp(dataType, DATATYPE_BOOLEAN, strlen(DATATYPE_BOOLEAN)) ) {
        sendDataToServerOutput(booleanServerOutput);
        //Serial.println(booleanServerOutput);
        request->send(HTTP_STATUS_OK);
      } else if( !strncmp(dataType, DATATYPE_NUMBER, strlen(DATATYPE_NUMBER)) ) {
        sendDataToServerOutput(numberServerOutput);
        //Serial.println(numberServerOutput);
        request->send(HTTP_STATUS_OK);
      } else {
        request->send(HTTP_STATUS_BAD_REQUEST);
      }
    } else {
      // some error handling
      request->send(HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }
  });
}



void WiFiInit(){
  Serial.print("Setting AP (Access Point)…");
  if(WiFi.softAP(ssid, password)) Serial.println("connected");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  HTTPSendWebsiteFiles(server);
  HTTPSetMappings(server);
  server.begin();
}

void setup(){
  Serial.begin(9600);
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  if(!SPIFFS.exists("/index.html")){
    Serial.println("index.html not found");
  }
    // reserve memory in string to prevent many reallocations
  booleanServerOutput.reserve(256);
  numberServerOutput.reserve(256);
  serverInputFromWebsite.reserve(1024);

  deserializeJson(testWebsiteConfig, testWebsiteConfigStr);
  WiFiInit();



}



void loop(){

}

