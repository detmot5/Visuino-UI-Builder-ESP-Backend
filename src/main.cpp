#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <vector>
#include <string>
#define OUTPUT_JSON_INITIAL_SIZE 512

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


String booleanServerOutput;
String numberServerOutput;


namespace ErrorMessages{
  namespace Memory{

  }
  namespace JsonInput {

  }

}


namespace ComponentType {
  namespace Input {
    const char* Switch PROGMEM = "switch";
    const char* Slider PROGMEM = "slider";
    const char* CheckBox PROGMEM = "checkbox";
    const char* NumberInput PROGMEM = "numberinput";
  }
  namespace Output {
    const char* Label PROGMEM = "label";
    const char* Chart PROGMEM = "chart";
    const char* Indicator PROGMEM = "indicator";
    const char* Gauge PROGMEM = "gauge";
  }
}

namespace DataType{
  const char* Boolean PROGMEM = "boolean";
  const char* Number PROGMEM = "number";
}

namespace JsonKey {
  const char* Title PROGMEM = "title";
  const char* Elements PROGMEM = "elements";

    // output properties
  const char* Name PROGMEM = "name";
  const char* PosX PROGMEM = "posX";
  const char* PosY PROGMEM = "posY";
  const char* ReadOnly PROGMEM = "readonly";
  const char* DataType PROGMEM = "datatype";
  const char* Value PROGMEM = "value";
  const char* Color PROGMEM = "color";
}



class WebsiteComponent {
public:
  explicit WebsiteComponent(const JsonObject& inputObject);
  virtual ~WebsiteComponent() = default;
  virtual JsonObject toOutputJson() = 0;
  bool isInitializedOK() const {return initializedOK;}
protected:
  static DynamicJsonDocument jsonMemory;
  bool initializedOK;
  uint16_t posX;
  uint16_t posY;
  String name;
  String dataType;
};

DynamicJsonDocument WebsiteComponent::jsonMemory(OUTPUT_JSON_INITIAL_SIZE);

WebsiteComponent::WebsiteComponent(const JsonObject& inputObject){
  initializedOK = true;
  if(inputObject.containsKey(JsonKey::Name)){
    this->name = inputObject[JsonKey::Name].as<String>();
  } else initializedOK = false;
  if(inputObject.containsKey(JsonKey::PosX)) {
    this->posX = inputObject[JsonKey::PosX];
  } else initializedOK = false;
  if(inputObject.containsKey(JsonKey::PosY)){
    this->posY = inputObject[JsonKey::PosY];
  } else initializedOK = false;
  if(inputObject.containsKey(JsonKey::DataType)){
    this->dataType = inputObject[JsonKey::DataType].as<String>();
  } else initializedOK = false;
}




class Switch : public WebsiteComponent {
public:
  explicit Switch(const JsonObject& inputObject)
          : WebsiteComponent(inputObject){

    if(inputObject.containsKey(JsonKey::Value)){
      this->value = inputObject[JsonKey::Value];
    } else initializedOK = false;
    if(inputObject.containsKey(JsonKey::Color)){
      this->color = inputObject[JsonKey::Color].as<String>();
    } else initializedOK = false;
  }

  JsonObject toOutputJson() override {
    jsonMemory.clear();
    JsonObject outputObj = jsonMemory.as<JsonObject>();
    outputObj[JsonKey::Name] = this->name;
    outputObj[JsonKey::DataType] = this->dataType;
    outputObj[JsonKey::Value] = this->value;
    return outputObj;
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
      "name" : "Lamp2",
      "dataType" : "boolean",
      "componentType" : "switch";
      "posX" : 20,
      "posY" : 40,
      "color" : "#ffeefe",
      "value": false
    },
  ]
})"};


inline void componentsGarbageCollect(){
  for (auto component : components) delete component;
}

namespace JsonReader {
  enum class InputJsonStatus : uint8_t {OK, TITLE_NOT_FOUND, ELEMENTS_NOT_FOUND, OBJECT_NOT_VALID, NAME_NOT_FOUND};

  InputJsonStatus createComponent(JsonObject object, std::vector<WebsiteComponent*>& elements) {
    if( !object.containsKey(JsonKey::DataType) ) return InputJsonStatus::NAME_NOT_FOUND;
  }


  InputJsonStatus readWebsiteComponentsFromJson(const String& json){
    static DynamicJsonDocument jsonDocument( JSON_OBJECT_SIZE(json.length() ) ); // static for allocate memory once
    static bool isVectorReserved = false;
    deserializeJson(jsonDocument, json);
    JsonObject inputObject = jsonDocument.as<JsonObject>();
    if(!inputObject.containsKey(JsonKey::Title)) return InputJsonStatus::TITLE_NOT_FOUND;
    if(!inputObject.containsKey(JsonKey::Elements)) return InputJsonStatus::ELEMENTS_NOT_FOUND;

    JsonArray elements = inputObject[JsonKey::Elements].as<JsonArray>();

    if( !isVectorReserved ) {
      components.reserve(elements.size());
      isVectorReserved = true;
    }

    WebsiteComponent* componentPtr;
    for(const JsonObject element : elements){
      serializeJsonPretty(element,Serial);
      componentPtr = new Switch(element);
      if( !componentPtr->isInitializedOK() ) {
        delete componentPtr;
        return InputJsonStatus::OBJECT_NOT_VALID;
      }
      components.push_back(componentPtr);
    }

    return InputJsonStatus::OK;
  }
  void inputJsonParse_ErrorHandler(InputJsonStatus status){
    switch (status) {
      case InputJsonStatus::TITLE_NOT_FOUND:
        break;
      case InputJsonStatus::ELEMENTS_NOT_FOUND:
        break;
      case InputJsonStatus::OBJECT_NOT_VALID:
        break;
      case InputJsonStatus::NAME_NOT_FOUND:
        break;
      case InputJsonStatus::OK:
        break;
    }
  }
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


  JsonReader::InputJsonStatus status = JsonReader::readWebsiteComponentsFromJson(testWebsiteConfigStr);
  if(status != JsonReader::InputJsonStatus::OK) Serial.println("Some error xd");

}



void loop(){

}

