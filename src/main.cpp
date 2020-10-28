#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <vector>
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


String booleanServerOutput;
String numberServerOutput;


class WebsiteComponent {
public:
  static int actualID;
  WebsiteComponent(const JsonObject& inputObject){
    this->id = actualID;
    actualID++;
    initializedOK = true;
    if(inputObject.containsKey("posX")) {
      this->posX = inputObject["posX"];
    } else initializedOK = false;
    if(inputObject.containsKey("posY")){
      this->posY = inputObject["posY"];
    } else initializedOK = false;
    if(inputObject.containsKey("readonly")){
      this->readOnly = inputObject["readonly"];
    } else initializedOK = false;
    if(inputObject.containsKey("dataType")){
      this->dataType = inputObject["dataType"].as<String>();
    } else initializedOK = false;
  }


  virtual JsonObject toJsonOutput() = 0;

protected:
  bool initializedOK;
  uint16_t id;
  uint16_t posX;
  uint16_t posY;
  bool readOnly;
  String dataType;
};

int WebsiteComponent::actualID = 0;



class Button : private WebsiteComponent {
public:
  Button(const JsonObject &inputObject) : WebsiteComponent(inputObject) {
    if(inputObject.containsKey("value")){
      this->value = inputObject["value"];
    } else initializedOK = false;
    if(inputObject.containsKey("color")){
      this->color = inputObject["color"].as<String>();
    } else initializedOK = false;
  }


  JsonObject toJsonOutput() override {

  }

  void setValue(bool newValue) {this->value = newValue;}
  bool getValue() const {return this->value;}
private:
  bool value;
  String color;
};


std::vector<WebsiteComponent*> components;


const String testWebsiteConfigStr = {R"(
{
  "title" : "Nice website",
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







enum class InputJsonStatus : uint8_t {OK, TITLE_NOT_FOUND, ELEMENTS_NOT_FOUND};
InputJsonStatus readWebsiteComponentsFromJson(const String& json){
  DynamicJsonDocument jsonDocument( JSON_OBJECT_SIZE(json.length() ) );
  deserializeJson(jsonDocument, json);
  JsonObject inputObject = jsonDocument.as<JsonObject>();
  if(!inputObject.containsKey("title")) return InputJsonStatus::TITLE_NOT_FOUND;
  if(!inputObject.containsKey("elements")) return InputJsonStatus::ELEMENTS_NOT_FOUND;

  JsonArray elements = inputObject["elements"].as<JsonArray>();

  components.reserve(elements.size());

  for(const JsonObject element : elements){
    serializeJsonPretty(element,Serial);
    components.push_back(new InputComponent(element));
  }

  return InputJsonStatus::OK;
}


void fullCorsAllow(AsyncWebServerResponse* response){
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS, CORS_ALLOWED_METHODS);
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, CORS_ALLOWED_HEADERS);
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

  });

  webServer.on("/status", HTTP_POST, [] (AsyncWebServerRequest* request){}, nullptr,
          [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {


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
  if(!SPIFFS.exists("/index.html")) {
    Serial.println("index.html not found");
  }
  WiFiInit();


  InputJsonStatus status = readWebsiteComponentsFromJson(testWebsiteConfigStr);
  if(status != InputJsonStatus::OK) Serial.println("Some error xd");

}



void loop(){

}

