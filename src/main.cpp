#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <StreamString.h>
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
  const char* SwitchSize PROGMEM = "default";
  const uint8_t FontSize PROGMEM = 16;
  const bool BooleanValue PROGMEM = false;
  const uint16_t Width PROGMEM = 100;
  const uint16_t Height PROGMEM = 100;
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
  const char* Str PROGMEM = "string";
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
  const char* FontSize PROGMEM = "fontSize";

  const char* SwitchSize PROGMEM = "size";

  const char* MaxValue PROGMEM = "maxValue";
  const char* MinValue PROGMEM = "minValue";

}


namespace ErrorMessage{
  namespace Memory{
    const char* HeapFragmentationTooHigh PROGMEM = "Memory - Heap Fragmentation too high, stability problems may occur";
    const char* LowHeapSpace PROGMEM = "Memory - Low Heap Space, stability problems may occur";
  }

  namespace JsonInput {
    const char* InputErrorHeader PROGMEM = "JSON Parse Error: ";

    const char* TitleNotFound PROGMEM = "Title not found";
    const char* JsonOverflow PROGMEM = "Json Input - overflow";
    const char* AllocError PROGMEM = "Json Input - Allocation Error";
    const char* ElementsNotFound PROGMEM = "Json Input - elements not found";
    const char* ElementsArrayEmpty PROGMEM = "Json Input - array of elements is empty";
    const char* ObjectNotValid PROGMEM = "Json Input - Object is not valid";
    const char* NameNotFound PROGMEM = "Json Input - Object name is not found";
    const char* UnexpectedChangeOfInputSize PROGMEM = "Json Input - Unexpected change of input size";
    const char* InvalidInput PROGMEM = "Json Input - Invalid input";
    const char* OK PROGMEM = "Json Input - Ok";
  }
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


    static void setJsonMemory ( DynamicJsonDocument *mem );
    static bool isMemoryInitialized() {return memoryInitialized;}

  protected:
    static DynamicJsonDocument* jsonMemory;
    static bool memoryInitialized;
    bool initializedOK;
    uint16_t posX;
    uint16_t posY;
    uint8_t desktopScale;
    String name;
    String dataType;
  };
  DynamicJsonDocument* WebsiteComponent::jsonMemory = nullptr;
  bool WebsiteComponent::memoryInitialized = false;

  WebsiteComponent::WebsiteComponent(const JsonObject& inputObject){
    initializedOK = true;
    if(inputObject.containsKey(JsonKey::Name)){
      this->name = inputObject[JsonKey::Name].as<String>();
    } else {
      Serial.println("Name not found");
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

  }


  void WebsiteComponent::setJsonMemory ( DynamicJsonDocument *mem ) {
    if ( mem != nullptr ) {
      jsonMemory = mem;
      memoryInitialized = true;
    }
  }

  // ----------------------------------------------------------------------------
  //                         COMPONENTS CLASSES
  // ----------------------------------------------------------------------------


  class InputComponent : public WebsiteComponent {
  public:
    explicit InputComponent(const JsonObject& inputObject)
      : WebsiteComponent(inputObject) {
    }
    virtual JsonObject toVisuinoJson() = 0; //using common memory
  };

  class OutputComponent : public WebsiteComponent {
  public:
    explicit OutputComponent(const JsonObject& inputObject)
      : WebsiteComponent(inputObject){
    }

    virtual void setState(JsonObject& object) = 0;

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
      if(inputObject.containsKey(JsonKey::SwitchSize)){
        this->size = inputObject[JsonKey::SwitchSize].as<String>();
      } else this->size = DefaultValues::SwitchSize;

      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject outputObj = jsonMemory->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::DataType] = this->dataType;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject websiteObj = jsonMemory->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::SwitchSize] = this->size;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::Switch;
      return websiteObj;
    }
  private:
    bool value;
    String color;
    String size;
  };


  class Label : public OutputComponent {
  public:
    explicit Label(const JsonObject& inputObject)
    : OutputComponent(inputObject) {
      if(inputObject.containsKey(JsonKey::FontSize)){
        this->fontSize = inputObject[JsonKey::FontSize];
      } else fontSize = DefaultValues::FontSize;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value].as<String>();
      } else this->value = "";
    }

    JsonObject toWebsiteJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject websiteObj = jsonMemory->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::FontSize] = this->fontSize;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Label;
      return websiteObj;
    }

    void setState(JsonObject& object) override {
      if(object.containsKey(JsonKey::FontSize)){
        this->fontSize = object[JsonKey::FontSize];
      }
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<String>();
      }
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value].as<String>();
      }
    }

    const String& getValue() const {return value;}
    void setValue(const String& nvalue) {this->value = nvalue;}


  private:
    uint8_t fontSize;
    String color;
    String value;
  };




  class Gauge : public OutputComponent{
  public:
    explicit Gauge(const JsonObject& inputObject)
      : OutputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::MaxValue)){
        this->valueMax = inputObject[JsonKey::MaxValue];
      } else {
        this->valueMax = 0;
        initializedOK = false;
      }
      if(inputObject.containsKey(JsonKey::MinValue)){
        this->valueMin = inputObject[JsonKey::MinValue];
      } else {
        this->valueMax = 0;
        initializedOK = false;
      }
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::Height;
    }

    JsonObject toWebsiteJson() override{
      if(!isMemoryInitialized()) return {};
      JsonObject websiteObj = jsonMemory->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::MaxValue] = this->valueMax;
      websiteObj[JsonKey::MinValue] = this->valueMin;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Gauge;
      return websiteObj;
    }

    void setState(JsonObject &object) override{
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      } else this->value = 0;
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
    }

  private:
    uint32_t value;
    uint32_t valueMin;
    uint32_t valueMax;
    uint16_t width;
    uint16_t height;
    String color;
  };








  class Card {
  public:
    Card() = default;
    ~Card() {this->garbageCollect();}
    enum class ComponentStatus : uint8_t{
      OK,
      OBJECT_NOT_VALID,
      ALREADY_EXISTS
    };

    static void setJsonMemory(DynamicJsonDocument* mem);
    ComponentStatus add(JsonObject object);
    JsonObject onHTTPRequest();
    void reserve(size_t size);
    void garbageCollect();
    const std::vector<WebsiteComponent*>& getComponents() const {return components;}
  private:
    template <typename componentType> bool parseInputComponent(const char* componentName, JsonObject& object);
    template <typename componentType> bool parseOutputComponent(const char* componentName, JsonObject& object);
    static DynamicJsonDocument* jsonMemory;
    static bool isMemoryInitialized;
    WebsiteComponent* getComponentByName(const char* name);
    bool componentAlreadyExists(const char* componentName);
    std::vector<WebsiteComponent*> components;
  };

  DynamicJsonDocument* Card::jsonMemory = nullptr;
  bool Card::isMemoryInitialized = false;



  // TODO JsonObject as const reference
  Card::ComponentStatus Card::add(JsonObject object) {
    if(!object.containsKey(JsonKey::Name) || !object.containsKey(JsonKey::ComponentType)) return ComponentStatus::OBJECT_NOT_VALID;
    const char* componentName = object[JsonKey::Name];
    const char* componentType = object[JsonKey::ComponentType];
    using namespace ComponentType;
    // TODO abstract strncmp()
    if(!strncmp(componentType, Input::Switch, strlen(componentType))) {
      parseInputComponent<Switch>(componentName, object);
    } else if(!strncmp(componentType, Output::Label, strlen(componentType))){
      parseOutputComponent<Label>(componentName, object);
    } else if(!strncmp(componentType, Output::Gauge, strlen(componentType))){
      parseOutputComponent<Gauge>(componentName, object);
    }
    return ComponentStatus::OK;
  }

  JsonObject Card::onHTTPRequest() {
    // TODO refactor it
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

  template<typename componentType>
  bool Card::parseOutputComponent(const char* componentName, JsonObject& object) {
    if(!componentAlreadyExists(componentName)){
      auto component = new componentType(object);
      if(component->isInitializedOK()) components.push_back(component);
      else {
        delete component;
        return false;
      }
    } else {
      auto component = reinterpret_cast<componentType*>(getComponentByName(componentName));
      component->setState(object);
    }
    return true;
  }

  template<typename componentType>
  bool Card::parseInputComponent(const char *componentName, JsonObject &object) {
    if(!componentAlreadyExists(componentName)){
      auto component = new componentType(object);
      if(component->isInitializedOK()) components.push_back(component);
      else {
        delete component;
        return false;
      }
    } else return false;

    return true;
  }

}




namespace Log {

  const char* InfoHeader PROGMEM = "Server Info: ";
  const char* ErrorHeader PROGMEM = "Server Error: ";

  const char* MemStats PROGMEM = "Memory stats: ";
  const char* HeapFragmentationMsg PROGMEM = "- Heap fragmentation: ";
  const char* FreeHeapMsg PROGMEM = "- Free heap: ";
  const char* MaxFreeHeapBlock PROGMEM = "- Largest free memory block: ";

  void memoryInfo(Stream& stream) {
    stream.println(MemStats);
    stream.print(HeapFragmentationMsg);
    stream.println(ESP.getHeapFragmentation());

    stream.print(FreeHeapMsg);
    stream.println(ESP.getFreeHeap());

    stream.print(MaxFreeHeapBlock);
    stream.println(ESP.getMaxFreeBlockSize());
  }

  void info (const char* msg, Stream& stream) {
    stream.print(InfoHeader);
    stream.print(msg);
  }

  void error (const char* msg, Stream& stream) {
    stream.print(ErrorHeader);
    stream.println(msg);
    memoryInfo(stream);
  }
}

const String wrongWebsiteStr = {R"(gugugwno)"};

const String testWebsiteConfigStr = {R"(
{
  "elements" : [
      {
        "name" : "Lamp2",
        "size" : "default",
        "componentType" : "switch",
        "posX" : 100,
        "posY" : 100,
        "color" : "blue",
        "value": false
      },
      {
        "name" : "Motor",
        "size" : "large",
        "componentType" : "switch",
        "posX" : 400,
        "posY" : 400,
        "color" : "#abcdef",
        "value": false
      },
      {
        "name" : "Nothing",
        "size" : "large",
        "componentType" : "switch",
        "posX" : 170,
        "posY" : 100,
        "color" : "lightgreen",
        "value": false
      },
      {
        "name" : "Something",
        "size" : "small",
        "componentType" : "switch",
        "posX" : 50,
        "posY" : 150,
        "color" : "#bbaaaa",
        "value": true
      },
      {
        "name" : "Servo",
        "size" : "large",
        "componentType" : "switch",
        "dataType" : "boolean",
        "posX" : 170,
        "posY" : 150,
        "color" : "#abcada",
        "value": true
      },
      {
        "name" : "LightBulb",
        "size" : "default",
        "componentType" : "switch",
        "posX" : 100,
        "posY" : 150,
        "color" : "#00ffe5",
        "value": true
      },
      {
        "name" : "bulb",
        "size" : "small",
        "componentType" : "switch",
        "posX" : 50,
        "posY" : 100,
        "color" : "orange",
        "value": true
      },
      {
        "name" : "Info",
        "componentType" : "label",
        "posX" : 200,
        "posY" : 350,
        "color" : "#ff2233",
        "value": "Siema wam wszystkim"
      },
      {
        "name" : "Switch Info",
        "componentType" : "label",
        "posX" : 120,
        "posY" : 200,
        "color" : "#444",
        "value": "Switches"
      },
      {
        "name" : "Speed",
        "componentType" : "gauge",
        "posX" : 200,
        "posY" : 300,
        "color" : "red",
        "value": 740,
        "maxValue": 800,
        "minValue": 0,
        "width" : 200,
        "height": 100
      }
  ]

})"};



Website::Card card;

namespace JsonReader {
  size_t memorySize = 0;
  DynamicJsonDocument* inputJsonMemory = nullptr;
  DynamicJsonDocument* componentJsonMemory = nullptr;
  enum class InputJsonStatus : uint8_t {
    OK,
    JSON_OVERFLOW,
    INVALID_INPUT,
    ALLOC_ERROR,
    TITLE_NOT_FOUND,
    ELEMENTS_NOT_FOUND,
    ELEMENTS_ARRAY_EMPTY,
    OBJECT_NOT_VALID,
    NAME_NOT_FOUND,
  };


  bool validateJson(const String& json){
    if(json.isEmpty() || json.indexOf(JsonKey::Name) < 0) return false;
    return true;
  }


  InputJsonStatus readWebsiteComponentsFromJson(const String& json) {
    static bool isValid = false;
    static bool isInitialized = false;
    isValid = validateJson(json);
    if (isValid) {
      if (!isInitialized) {
        memorySize = json.length() * 2;
        inputJsonMemory =  new DynamicJsonDocument(memorySize);
        componentJsonMemory = new DynamicJsonDocument(memorySize);
        if (inputJsonMemory->capacity() != 0 && componentJsonMemory->capacity() != 0) {
          Website::Card::setJsonMemory(inputJsonMemory);
          Website::WebsiteComponent::setJsonMemory(componentJsonMemory);
          isInitialized = true;
        } else {
          delete inputJsonMemory;
          delete componentJsonMemory;
          return InputJsonStatus::ALLOC_ERROR;
        }
      }
      deserializeJson(*inputJsonMemory, json);
      if (inputJsonMemory->overflowed()) return InputJsonStatus::JSON_OVERFLOW;

      JsonObject inputObject = inputJsonMemory->as<JsonObject>();
      if (!inputObject.containsKey(JsonKey::Elements)) return InputJsonStatus::ELEMENTS_NOT_FOUND;
      JsonArray elements = inputObject[JsonKey::Elements].as<JsonArray>();
      if (elements.size() == 0) return InputJsonStatus::ELEMENTS_ARRAY_EMPTY;
      card.reserve(elements.size());

      for (JsonObject element : elements) {
        if (card.add(element) != Website::Card::ComponentStatus::OK) {
          return InputJsonStatus::ALLOC_ERROR;
        }
      }
    } else return InputJsonStatus::INVALID_INPUT;
    return InputJsonStatus::OK;
  }

  void errorHandler(InputJsonStatus status, Stream& stream){
    using namespace ErrorMessage::JsonInput;
    if(status != InputJsonStatus::OK) stream.print(InputErrorHeader);
    switch (status) {
      case InputJsonStatus::TITLE_NOT_FOUND:
        stream.println(TitleNotFound);
        break;
      case InputJsonStatus::ELEMENTS_NOT_FOUND:
        stream.println(ElementsNotFound);
        break;
      case InputJsonStatus::OBJECT_NOT_VALID:
        stream.println(ObjectNotValid);
        break;
      case InputJsonStatus::NAME_NOT_FOUND:
        stream.println(NameNotFound);
        break;
      case InputJsonStatus::ALLOC_ERROR:
        stream.println(AllocError);
        break;
      case InputJsonStatus::JSON_OVERFLOW:
        stream.println(JsonOverflow);
        break;
      case InputJsonStatus::ELEMENTS_ARRAY_EMPTY:
        stream.println(ElementsArrayEmpty);
        break;
      case InputJsonStatus::INVALID_INPUT:
        stream.println(InvalidInput);
        break;
      case InputJsonStatus::OK:
        stream.println(ErrorMessage::JsonInput::OK);
        break;
    }
  }
}



void fullCorsAllow(AsyncWebServerResponse* response){
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS, CORS_ALLOWED_METHODS);
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, CORS_ALLOWED_HEADERS);
}

void HTTPServeWebsite(AsyncWebServer& webServer){

  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.html");
  });

  webServer.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css","text/css");
  });

  webServer.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.js","application/javascript");
  });

  webServer.on("/component.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.css","text/css");
  });

  webServer.on("/component.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.js","application/javascript");
  });

  webServer.on("/Libs/switch.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Libs/switch.js","application/javascript");
  });

  webServer.on("/Libs/switch.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Libs/switch.css","text/css");
  });

  webServer.on("/Libs/pureknobMin.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Libs/pureknobMin.js","application/javascript");
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
  HTTPServeWebsite(server);
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

  uint32_t before = millis();
  JsonReader::InputJsonStatus status = JsonReader::readWebsiteComponentsFromJson(testWebsiteConfigStr);
  JsonReader::errorHandler(status, Serial);
  uint32 after = millis();
  Serial.print("Execution time: ");
  Serial.println(after - before);
  Log::error("error test", Serial);
}



void loop(){

}

