#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <vector>

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

namespace DefaultValues {
  const char* Color PROGMEM = "#333333";
  const bool BooleanValue PROGMEM = false;
}

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
    const char* Indicator PROGMEM = "indicator";
    const char* Chart PROGMEM = "chart";
    const char* Gauge PROGMEM = "gauge";
  }
}

namespace DataType{
  const char* Boolean PROGMEM = "boolean";
  const char* Number PROGMEM = "number";
}


namespace JsonKey {
  const char* Title PROGMEM = "title";
  const char* Body PROGMEM = "body";
  const char* Elements PROGMEM = "elements";

  const char* Name PROGMEM = "name";
  const char* Width PROGMEM = "width";
  const char* Height PROGMEM = "height";
  const char* PosX PROGMEM = "posX";
  const char* PosY PROGMEM = "posY";
  const char* DataType PROGMEM = "dataType";
  const char* ComponentType PROGMEM = "componentType";
  const char* Value PROGMEM = "value";
  const char* Color PROGMEM = "color";
}


namespace Website {

  class WebsiteComponent {
  public:
    explicit WebsiteComponent(const JsonObject& inputObject);
    virtual ~WebsiteComponent() = default;

      // ** THIS METHOD USES COMMON MEMORY(DOCUMENT) FOR STORING JSON TO AVOID MULTIPLE HEAP ALLOCATIONS **
      // ** WHEN YOU GET OBJECT, THE PREVIOUS ONE IS DELETED AUTOMATICALLY **
    virtual JsonObject toWebsiteJson() = 0;
    bool isInitializedOK() const {return initializedOK;}
    const String& getName() const  {return name;}

  protected:
    static DynamicJsonDocument jsonMemory;
    bool initializedOK;
    uint16_t width;
    uint16_t height;
    uint16_t posX;
    uint16_t posY;
    uint8_t desktopScale{};
    String name;
    String dataType;
  };

  DynamicJsonDocument WebsiteComponent::jsonMemory(OUTPUT_JSON_INITIAL_SIZE);

  WebsiteComponent::WebsiteComponent(const JsonObject& inputObject){
    initializedOK = true;
    if(inputObject.containsKey(JsonKey::Name)){
      this->name = inputObject[JsonKey::Name].as<String>();
    } else {
      Serial.println("Name not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::Width)){
      this->width = inputObject[JsonKey::Width];
    } else {
      Serial.println("Width not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::Height)){
      this->height = inputObject[JsonKey::Height];
    } else {
      Serial.println("Height not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::PosX)) {
      this->posX = inputObject[JsonKey::PosX];
    } else {
      Serial.println("PosX not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::PosY)){
      this->posY = inputObject[JsonKey::PosY];
    } else {
      Serial.println("PosY not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::DataType)){
      this->dataType = inputObject[JsonKey::DataType].as<String>();
    } else {
      Serial.println("DataType not found");
      initializedOK = false;
    }
    

  }


  // ----------------------------------------------------------------------------
  //                         COMPONENTS CLASSES
  // ----------------------------------------------------------------------------


  class InputComponent : public WebsiteComponent {
  public:
    explicit InputComponent(const JsonObject& inputObject)
      : WebsiteComponent(inputObject){
    }
    virtual JsonObject toVisuinoJson() = 0;
  private:

  };




  class Switch : public InputComponent {
  public:
    explicit Switch(const JsonObject& inputObject)
            : InputComponent(inputObject){

      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else {
        Serial.println("Value not found");
        this->value = DefaultValues::BooleanValue;
      }
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else {
        Serial.println("Color not found");
        this->color = DefaultValues::Color;
      }
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = jsonMemory.to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::DataType] = this->dataType;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory.to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::DataType] = this->dataType;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::Switch;
      return websiteObj;
    }

    void setValue(bool newValue) {this->value = newValue;}
    bool getValue() const {return this->value;}
  private:
    bool value;
    String color;
  };

  class Card {
  public:
    Card() = default;
    ~Card() {this->garbageCollect();}
    enum class ComponentStatus : uint8_t{
      OK,
      OBJECT_NOT_VALID,
    };

    static void setJsonMemory(DynamicJsonDocument* mem);
    ComponentStatus add(JsonObject object);
    JsonObject onHTTPRequest();
    void reserve(size_t size);
    void garbageCollect();
    const std::vector<WebsiteComponent*>& getComponents() const {return components;}
  private:
    static DynamicJsonDocument* jsonMemory;
    static bool isMemoryInitialized;
    WebsiteComponent* getComponentByName(const char* name);
    bool componentAlreadyExists(const char* componentName);
    std::vector<WebsiteComponent*> components;
  };

  DynamicJsonDocument* Card::jsonMemory = nullptr;
  bool Card::isMemoryInitialized = false;

  Card::ComponentStatus Card::add(JsonObject object) {
    if(!object.containsKey(JsonKey::Name) || !object.containsKey(JsonKey::ComponentType)) return ComponentStatus::OBJECT_NOT_VALID;
    String componentType = object[JsonKey::ComponentType];
    using namespace ComponentType;
      // TODO: abstract it
    if(componentType.equals(Input::Switch)) {
      WebsiteComponent* component = new Switch(object);
      Serial.println(componentType);
      Serial.println(component->getName());
      if(component->isInitializedOK()) components.push_back(component);
      else {
        delete component;
        return ComponentStatus::OBJECT_NOT_VALID;
      }
    }
    //else if(componentType.equals(Input::Slider)) components.push_back(new Slider(object));
    return ComponentStatus::OK;
  }

  JsonObject Card::onHTTPRequest() {
    if(isMemoryInitialized){
      jsonMemory->clear();
      JsonObject object = jsonMemory->to<JsonObject>();
      JsonArray elements = object[JsonKey::Body].createNestedArray(JsonKey::Elements);
      for(auto component : this->components) {
        elements.add(component->toWebsiteJson());
      }
      return object;
    } else{
      Serial.println("not initialized");
      return {};
    }
  }

  bool Card::componentAlreadyExists(const char* componentName) {
    bool res = false;
    if(getComponentByName(componentName) != nullptr) res = true;
    return res;
  }

  void Card::reserve(size_t size){
    components.reserve(size);
  }


  void Card::garbageCollect() {
    for(auto component : this->components) delete component;
    components.clear();
  }


  WebsiteComponent* Card::getComponentByName(const char *name) {
    for(auto component : this->components){
      if(component->getName().equals(name)) return component;
    }
    return nullptr;
  }

  void Card::setJsonMemory(DynamicJsonDocument *mem) {
    jsonMemory = mem;
    isMemoryInitialized = true;
  }

}




namespace Log{

  const char* infoHeader = "Server Info: ";
  const char* errorHeader = "Server Error: ";

  void info (const char* msg, Stream& stream) {
    stream.print(infoHeader);

    stream.print(msg);
  }

  void error (const char* msg, Stream& stream) {

  }
}


const String testWebsiteConfigStr = {R"(
{
  "title" : "Nice website",
  "elements" : [
    {
      "name" : "Lamp2",
      "dataType" : "boolean",
      "width" : 20,
      "height" : 10,
      "componentType" : "switch",
      "posX" : 20,
      "posY" : 40,
      "color" : "#ffeefe",
      "value": false
    },
    {
      "name" : "Motor",
      "dataType" : "boolean",
      "width" : 21,
      "height" : 15,
      "componentType" : "switch",
      "posX" : 250,
      "posY" : 300,
      "color" : "#abcdef",
      "value": false
    },
    {
      "name" : "Nothing",
      "dataType" : "boolean",
      "width" : 22,
      "height" : 11,
      "componentType" : "switch",
      "posX" : 200,
      "posY" : 150,
      "color" : "#aaaaaa",
      "value": false
    },
    {
      "name" : "Something",
      "dataType" : "boolean",
      "width" : 27,
      "height" : 14,
      "componentType" : "switch",
      "posX" : 200,
      "posY" : 350,
      "color" : "#bbaaaa",
      "value": true
    },
    {
      "name" : "Servo",
      "dataType" : "boolean",
      "width" : 18,
      "height" : 11,
      "componentType" : "switch",
      "posX" : 200,
      "posY" : 300,
      "color" : "#abcada",
      "value": true
    },
    {
      "name" : "LightBulb",
      "dataType" : "boolean",
      "width" : 1,
      "height" : 29,
      "componentType" : "switch",
      "posX" : 100,
      "posY" : 350,
      "color" : "#abcgdb",
      "value": true
    }
  ]
})"};



Website::Card card;

namespace JsonReader {
  size_t memorySize = 0;
  DynamicJsonDocument* jsonMemory;
  enum class InputJsonStatus : uint8_t {
    OK,
    JSON_OVERFLOW,
    ALLOC_ERROR,
    TITLE_NOT_FOUND,
    ELEMENTS_NOT_FOUND,
    ELEMENTS_ARRAY_EMPTY,
    OBJECT_NOT_VALID,
    NAME_NOT_FOUND,
    UNEXPECTED_CHANGE_OF_INPUT_SIZE,
  };


  bool validateJson(const String& json){
    if(json.isEmpty() && json.indexOf(JsonKey::Name) < 0) return false;
    return true;
  }




  InputJsonStatus readWebsiteComponentsFromJson(const String& json) {
    static bool isValid = false;
    static bool isInitialized = false;
    isValid = validateJson(json);

    if (isValid) {
      if (!isInitialized) {
        memorySize = json.length();
        jsonMemory = new DynamicJsonDocument(memorySize);
        if (jsonMemory->capacity() != 0) {
          Website::Card::setJsonMemory(jsonMemory);
          isInitialized = true;
        } else {
          delete jsonMemory;
          return InputJsonStatus::ALLOC_ERROR;
        }
      } else if (memorySize != json.length()) {
        return InputJsonStatus::UNEXPECTED_CHANGE_OF_INPUT_SIZE;
      }
      deserializeJson(*jsonMemory, json);
      if (jsonMemory->overflowed()) return InputJsonStatus::JSON_OVERFLOW;

      JsonObject inputObject = jsonMemory->as<JsonObject>();
      if (!inputObject.containsKey(JsonKey::Elements)) return InputJsonStatus::ELEMENTS_NOT_FOUND;
      JsonArray elements = inputObject[JsonKey::Elements].as<JsonArray>();
      if (elements.size() == 0) return InputJsonStatus::ELEMENTS_ARRAY_EMPTY;
      card.reserve(elements.size());

      using Website::Card;
      for (JsonObject element : elements) {
        if (card.add(element) != Card::ComponentStatus::OK) {
          return InputJsonStatus::ALLOC_ERROR;
        }
      }
      return InputJsonStatus::OK;
    }
  }

  void inputJsonParse_ErrorHandler(InputJsonStatus status){
    switch (status) {
      case InputJsonStatus::TITLE_NOT_FOUND:
        Serial.println("title not found");
        break;
      case InputJsonStatus::ELEMENTS_NOT_FOUND:
        Serial.println("elements not found");
        break;
      case InputJsonStatus::OBJECT_NOT_VALID:
        Serial.println("object not valid");
        break;
      case InputJsonStatus::NAME_NOT_FOUND:
        Serial.println("name not found");
        break;
      case InputJsonStatus::ALLOC_ERROR:
        Serial.println("alloc error");
        break;
      case InputJsonStatus::JSON_OVERFLOW:
        Serial.println("json overflow");
        break;
      case InputJsonStatus::ELEMENTS_ARRAY_EMPTY:
        Serial.println("Elements array is empty");
        break;
      case InputJsonStatus::UNEXPECTED_CHANGE_OF_INPUT_SIZE:
        Serial.println("Unexpected change of json input size");
        break;
      case InputJsonStatus::OK:
        Serial.println("ok");
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

  webServer.on("/component.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.css","text/css");
  });

  webServer.on("/numberInput.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/numberInput.css","text/css");
  });

  webServer.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.js","application/javascript");
  });

  webServer.on("/component.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.js","application/javascript");
  });

  webServer.on("/numberInput.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/numberInput.js","application/javascript");
  });

  webServer.on("/renderer.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/renderer.js","application/javascript");
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
    AsyncWebServerResponse* response = request->beginResponse(HTTP_STATUS_OK, "application/json", card.onHTTPRequest()[JsonKey::Body]);
    fullCorsAllow(response);
    request->send(response);
  });

  webServer.on("/status", HTTP_POST, [] (AsyncWebServerRequest* request){}, nullptr,
          [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

    for(size_t i = 0; i < len; i++){
      Serial.print(static_cast<char>(data[i]));
    }
    Serial.println();

    request->send(HTTP_STATUS_OK);
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
  if(status == JsonReader::InputJsonStatus::ALLOC_ERROR) Serial.println("Zjebales xd");

}



void loop(){

}

