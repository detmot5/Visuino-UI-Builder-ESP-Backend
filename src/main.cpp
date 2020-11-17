#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <vector>
#include <sstream>

#include <ArduinoJson.h>
#include <StreamString.h>




namespace WebsiteServer{
  
AsyncWebServer server(80);



const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN PROGMEM = "Access-Control-Allow-Origin";
const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS PROGMEM = "Access-Control-Allow-Methods";
const char* CORS_ALLOWED_METHODS PROGMEM = "POST,GET,OPTIONS";

const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS PROGMEM = "Access-Control-Allow-Headers";
const char* CORS_ALLOWED_HEADERS PROGMEM = "Origin, X-Requested-With, Content-Type, Accept";


const uint16_t HTTP_STATUS_OK PROGMEM = 200;
const uint16_t HTTP_STATUS_BAD_REQUEST PROGMEM = 400;
const uint16_t HTTP_STATUS_INTERNAL_SERVER_ERROR PROGMEM = 500;




namespace DefaultValues {
  const char* Color PROGMEM = "#333333";
  const char* TextColor PROGMEM = "white";
  const char* SwitchSize PROGMEM = "default";
  const uint8_t FontSize PROGMEM = 16;
  const uint16_t Width PROGMEM = 100;
  const uint16_t Height PROGMEM = 100;
  const bool BooleanValue PROGMEM = false;
}

namespace ComponentType {
  namespace Input {
    const char* Switch PROGMEM = "switch";
    const char* Slider PROGMEM = "slider";
    const char* CheckBox PROGMEM = "checkbox";
    const char* NumberInput PROGMEM = "numberInput";
    const char* Button PROGMEM = "button";
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
  const char* Text PROGMEM = "text";
  const char* TextColor PROGMEM = "textColor";

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
    const char* InvalidInput PROGMEM = "Json Input - Invalid input";
    const char* OK PROGMEM = "Json Input - Ok";
  }
}


namespace Website {

  class WebsiteComponent {
  public:
    explicit WebsiteComponent(const JsonObjectConst& inputObject);
    virtual ~WebsiteComponent() = default;


      // ** THIS METHOD USES COMMON MEMORY(DOCUMENT) FOR STORING JSON TO AVOID MULTIPLE HEAP ALLOCATIONS **
      // ** WHEN YOU GET OBJECT, THE PREVIOUS ONE IS DELETED AUTOMATICALLY **
    virtual JsonObject toWebsiteJson() = 0;
    virtual void setState(const JsonObjectConst& object) = 0;
    bool isInitializedOK() const {return initializedOK;}
    const String& getName() const  {return name;}


    static void setJsonMemory (DynamicJsonDocument* mem);
    static bool isMemoryInitialized() {return memoryInitialized;}
  protected:
    static DynamicJsonDocument* jsonMemory;
    static bool memoryInitialized;
    bool initializedOK;
    uint16_t posX;
    uint16_t posY;
    // TODO: implement it
    uint8_t desktopScale;
    String name;
    String dataType;
  };
  DynamicJsonDocument* WebsiteComponent::jsonMemory = nullptr;
  bool WebsiteComponent::memoryInitialized = false;

  WebsiteComponent::WebsiteComponent(const JsonObjectConst& inputObject){
    initializedOK = true;
    if(inputObject.containsKey(JsonKey::Name)) {
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
    if(inputObject.containsKey(JsonKey::PosY)) {
      this->posY = inputObject[JsonKey::PosY];
    } else {
      Serial.println("PosY not found");
      initializedOK = false;
    }

  }


  void WebsiteComponent::setJsonMemory (DynamicJsonDocument* mem) {
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
    explicit InputComponent(const JsonObjectConst& inputObject)
      : WebsiteComponent(inputObject) {
    }
    virtual JsonObject toVisuinoJson() = 0; //using common memory
  private:
  };

  class OutputComponent : public WebsiteComponent {
  public:
    explicit OutputComponent(const JsonObjectConst& inputObject)
      : WebsiteComponent(inputObject){
    }
  };



  class Switch : public InputComponent {
  public:
    explicit Switch(const JsonObjectConst& inputObject)
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

    void setState(const JsonObjectConst& object) override {
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      }
    }

    static void setVisuinoOutput(const JsonObjectConst& obj) {
      str.clear();
      serializeJson(obj, str);
      isDataReady = true;
    }
    static const String& getVisuinoOutput() {return str;}
    static bool isDataReady;
  private:
    static String str;
    bool value;
    String color;
    String size;
  };

  String Switch::str;
  bool Switch::isDataReady = false;

  class Slider : public InputComponent {
  public:
    explicit Slider(const JsonObjectConst& inputObject)
      : InputComponent(inputObject) {
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::MinValue)){
        this->minValue = inputObject[JsonKey::MinValue];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::MaxValue)){
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject outputObj = jsonMemory->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
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
      websiteObj[JsonKey::MaxValue] = this->maxValue;
      websiteObj[JsonKey::MinValue] = this->minValue;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::Slider;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override {
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      }
    }

    static void setVisuinoOutput(const JsonObjectConst& obj) {
      str.clear();
      serializeJson(obj, str);
      isDataReady = true;
    }
    static const String& getVisuinoOutput() {return str;}
    static bool isDataReady;
  private:
    static String str;
    String color;
    uint16_t width;
    uint16_t height;
    uint32_t value;
    uint32_t minValue;
    uint32_t maxValue;
  };
  String Slider::str;
  bool Slider::isDataReady;


  class NumberInput : public InputComponent {
  public:
    explicit NumberInput(const JsonObjectConst& inputObject)
      : InputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0.0f;
      if(inputObject.containsKey(JsonKey::FontSize)){
        this->fontSize = inputObject[JsonKey::FontSize];
      } else this->fontSize = DefaultValues::FontSize;
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else this->width = 100;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      }
    }

    JsonObject toVisuinoJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject outputObj = jsonMemory->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
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
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::NumberInput;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override {
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      }
    }

    static void setVisuinoOutput(const JsonObjectConst& obj) {
      str.clear();
      serializeJson(obj, str);
      isDataReady = true;
    }
    static const String& getVisuinoOutput() {return str;}
    static bool isDataReady;

  private:
    static String str;
    float value;
    uint16_t width;
    uint16_t fontSize;
    String color;
  };

  String NumberInput::str;
  bool NumberInput::isDataReady;


  class Button : public InputComponent {
  public:
    explicit Button(const JsonObjectConst& inputObject)
      : InputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::FontSize)){
        this->fontSize = inputObject[JsonKey::FontSize];
      } else this->fontSize = DefaultValues::FontSize;
      if(inputObject.containsKey(JsonKey::Text)){
        this->text = inputObject[JsonKey::Text].as<String>();
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::TextColor)){
        this->textColor = inputObject[JsonKey::TextColor].as<String>();
      } else this->textColor = DefaultValues::TextColor;
      this->value = false;
    }

    JsonObject toVisuinoJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject outputObj = jsonMemory->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      if(!isMemoryInitialized()) return {};
      JsonObject websiteObj = jsonMemory->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::TextColor] = this->textColor;
      websiteObj[JsonKey::Text] = this->text;
      websiteObj[JsonKey::FontSize] = this->fontSize;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::Button;
      return websiteObj;
    }
    void setState(const JsonObjectConst& object) override {
      if(object.containsKey(JsonKey::Value))
        this->value = object[JsonKey::Value];
    }

    static void setVisuinoOutput(const JsonObjectConst& obj) {
      str.clear();
      serializeJson(obj, str);
      isDataReady = true;
    }

    static const String& getVisuinoOutput() {return str;}
    static bool isDataReady;


  private:
    static String str;
    bool value;
    uint16_t width;
    uint16_t height;
    uint16_t fontSize;
    String text;
    String color;
    String textColor;
  };
  String Button::str;
  bool Button::isDataReady;


  class Label : public OutputComponent {
  public:
    explicit Label(const JsonObjectConst& inputObject)
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

    void setState(const JsonObjectConst& object) override {
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

  private:
    uint16_t fontSize;
    String color;
    String value;
  };



  class Gauge : public OutputComponent{
  public:
    explicit Gauge(const JsonObjectConst& inputObject)
      : OutputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::MaxValue)){
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else {
        this->maxValue = 0;
        initializedOK = false;
      }
      if(inputObject.containsKey(JsonKey::MinValue)){
        this->minValue = inputObject[JsonKey::MinValue];
      } else {
        this->maxValue = 0;
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
      websiteObj[JsonKey::MaxValue] = this->maxValue;
      websiteObj[JsonKey::MinValue] = this->minValue;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Gauge;
      return websiteObj;
    }

    void setState(const JsonObjectConst &object) override{
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      } else this->value = 0;
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<String>();
      } else this->color = DefaultValues::Color;
    }

  private:
    uint32_t value;
    uint32_t minValue;
    uint32_t maxValue;
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
      UNKNOWN_NAME,
    };

    static void setJsonMemory(DynamicJsonDocument* mem);
    ComponentStatus add(const JsonObjectConst& object);
    JsonObject onHTTPRequest();
    bool onComponentStatusHTTPRequest(const uint8_t *data, size_t len);
    void reserve(size_t size);
    void garbageCollect();
    const std::vector<WebsiteComponent*>& getComponents() const {return components;}
  private:
    template <typename componentType> bool parseInputComponentToWebsite(const JsonObjectConst& object);
    template <typename componentType> bool parseOutputComponentToWebsite(const JsonObjectConst& object);
    template<typename componentType> bool parseInputComponentToVisuino(const JsonObjectConst& object);
    static DynamicJsonDocument* jsonMemory;
    static bool isMemoryInitialized;
    WebsiteComponent* getComponentByName(const char* name);
    bool componentAlreadyExists(const char* componentName);
    std::vector<WebsiteComponent*> components;

  };

  DynamicJsonDocument* Card::jsonMemory = nullptr;
  bool Card::isMemoryInitialized = false;



  Card::ComponentStatus Card::add(const JsonObjectConst& object) {
    if(!object.containsKey(JsonKey::Name) || !object.containsKey(JsonKey::ComponentType)) return ComponentStatus::OBJECT_NOT_VALID;
    const char* componentType = object[JsonKey::ComponentType];
    using namespace ComponentType;

    if(!strncmp(componentType, Input::Switch, strlen(componentType))) {
      parseInputComponentToWebsite<Switch>(object);
    } else if(!strncmp(componentType, Input::NumberInput, strlen(componentType))) {
      parseOutputComponentToWebsite<NumberInput>(object);
    } else if(!strncmp(componentType, Input::Slider, strlen(componentType))){
      parseOutputComponentToWebsite<Slider>(object);
    } else if(!strncmp(componentType, Input::Button, strlen(componentType))){
      parseOutputComponentToWebsite<Button>(object);
    }
    else if(!strncmp(componentType, Output::Label, strlen(componentType))){
      parseOutputComponentToWebsite<Label>(object);
    } else if(!strncmp(componentType, Output::Gauge, strlen(componentType))){
      parseOutputComponentToWebsite<Gauge>(object);
    } else {
      return ComponentStatus::OBJECT_NOT_VALID;
    }
    return ComponentStatus::OK;
  }

  JsonObject Card::onHTTPRequest() {
    if(isMemoryInitialized){
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

  bool Card::onComponentStatusHTTPRequest(const uint8_t* data, size_t len){
    if(!isMemoryInitialized) return false;
    deserializeJson(*jsonMemory, reinterpret_cast<const char*>(data), len);
    auto receivedJson = jsonMemory->as<JsonObject>();
    const char* componentType = receivedJson[JsonKey::ComponentType];
    using namespace ComponentType;
    if(!strncmp(componentType, Input::Switch, strlen(componentType))) {
      return parseInputComponentToVisuino<Switch>(receivedJson);
    }else if(!strncmp(componentType, Input::Slider, strlen(componentType))){
      return parseInputComponentToVisuino<Slider>(receivedJson);
    } else if(!strncmp(componentType, Input::NumberInput, strlen(componentType))){
      return parseInputComponentToVisuino<NumberInput>(receivedJson);
    } else if(!strncmp(componentType, Input::Button, strlen(componentType))){
      return parseInputComponentToVisuino<Button>(receivedJson);
    }
    return false;
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
  bool Card::parseOutputComponentToWebsite(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
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
  bool Card::parseInputComponentToWebsite(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
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

  template<typename componentType>
  bool Card::parseInputComponentToVisuino(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
    auto component = reinterpret_cast<InputComponent*>(getComponentByName(componentName));
    if(component == nullptr) return false;
    component->setState(object);
    componentType::setVisuinoOutput(component->toVisuinoJson());
    return true;
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
      },
      {
        "name" : "Slidee",
        "componentType" : "slider",
        "posX" : 300,
        "posY" : 150,
        "color" : "blue",
        "value": 740,
        "maxValue": 800,
        "minValue": 0,
        "width" : 500,
        "height": 20
      },
      {
        "name" : "Sliderrrre",
        "componentType" : "slider",
        "posX" : 300,
        "posY" : 250,
        "color" : "green",
        "value": 320,
        "maxValue": 800,
        "minValue": 0,
        "width" : 250,
        "height": 20
      },
      {
        "name": "other temperature",
        "color": "blue",
        "fontSize": 16,
        "width": 150,
        "value": 44.2,
        "desktopScale": 2,
        "componentType": "numberInput",
        "posY": 200,
        "posX": 400
    },
    {
        "name": "btn",
        "componentType": "button",
        "textColor": "white",
        "fontSize": 16,
        "text": "Click me",
        "width": 200,
        "height": 40,
        "color": "#444",
        "posX": 500,
        "posY": 80,
        "desktopScale": 2
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

  size_t getBiggestObjectSize(const JsonArrayConst& arr){
    size_t biggest = 0;
    for (const auto &item : arr) {
      if(item.memoryUsage() > biggest) biggest = item.memoryUsage();
    }
    return biggest;
  }

  InputJsonStatus readWebsiteComponentsFromJson(const String& json) {
    static bool isValid = false;
    static bool isInitialized = false;
    static bool isElementsInitialized = false;
    isValid = validateJson(json);
    if (isValid) {
      if (!isInitialized) {
        memorySize = json.length();
        inputJsonMemory =  new DynamicJsonDocument(memorySize * 2);
        if (inputJsonMemory->capacity() != 0) {
          Website::Card::setJsonMemory(inputJsonMemory);
          isInitialized = true;
        } else {
          delete inputJsonMemory;
          return InputJsonStatus::ALLOC_ERROR;
        }
      }
      deserializeJson(*inputJsonMemory, json);
      if (inputJsonMemory->overflowed()) return InputJsonStatus::JSON_OVERFLOW;

      JsonObject inputObject = inputJsonMemory->as<JsonObject>();
      if (!inputObject.containsKey(JsonKey::Elements)) return InputJsonStatus::ELEMENTS_NOT_FOUND;
      JsonArray elements = inputObject[JsonKey::Elements].as<JsonArray>();
      if (elements.size() == 0) return InputJsonStatus::ELEMENTS_ARRAY_EMPTY;
      if(!isElementsInitialized) {
        size_t biggestObjectSize = getBiggestObjectSize(elements);
        componentJsonMemory = new DynamicJsonDocument(biggestObjectSize * 2);
        if(componentJsonMemory->capacity() != 0){
          Website::WebsiteComponent::setJsonMemory(componentJsonMemory);
          isElementsInitialized = true;
        } else {
          delete componentJsonMemory;
          return InputJsonStatus::ALLOC_ERROR;
        }
      }

      card.reserve(elements.size());
      for (JsonObject element : elements) {
        if (card.add(element) != Website::Card::ComponentStatus::OK) {
          return InputJsonStatus::OBJECT_NOT_VALID;
        }
      }

    } else return InputJsonStatus::INVALID_INPUT;
    return InputJsonStatus::OK;
  }


  const char* errorHandler(InputJsonStatus status){
    using namespace ErrorMessage::JsonInput;
    const char* statusStr = nullptr;
    switch (status) {
      case InputJsonStatus::TITLE_NOT_FOUND:
        statusStr = TitleNotFound;
        break;
      case InputJsonStatus::ELEMENTS_NOT_FOUND:
        statusStr = ElementsNotFound;
        break;
      case InputJsonStatus::OBJECT_NOT_VALID:
        statusStr = ObjectNotValid;
        break;
      case InputJsonStatus::NAME_NOT_FOUND:
        statusStr = NameNotFound;
        break;
      case InputJsonStatus::ALLOC_ERROR:
        statusStr = AllocError;
        break;
      case InputJsonStatus::JSON_OVERFLOW:
        statusStr = JsonOverflow;
        break;
      case InputJsonStatus::ELEMENTS_ARRAY_EMPTY:
        statusStr = ElementsArrayEmpty;
        break;
      case InputJsonStatus::INVALID_INPUT:
        statusStr = InvalidInput;
        break;
      case InputJsonStatus::OK:
        statusStr = ErrorMessage::JsonInput::OK;
        break;
    }
    return statusStr;
  }
}
  namespace Log {
    StreamString errorStream;
    bool isDataReady = false;
    const char* InfoHeader PROGMEM = "Server Info: ";
    const char* ErrorHeader PROGMEM = "Server Error: ";

    const char* MemStats PROGMEM = "Memory stats: ";
    const char* HeapFragmentationMsg PROGMEM = "- Heap fragmentation: ";
    const char* FreeHeapMsg PROGMEM = "- Free heap: ";
    const char* MaxFreeHeapBlock PROGMEM = "- Largest free memory block: ";

    void memoryInfo(Stream& stream = errorStream) {
      stream.println(MemStats);
      stream.print(HeapFragmentationMsg);
      stream.println(ESP.getHeapFragmentation());

      stream.print(FreeHeapMsg);
      stream.println(ESP.getFreeHeap());

      stream.print(MaxFreeHeapBlock);
      stream.println(ESP.getMaxFreeBlockSize());
      isDataReady = true;
    }

    void info (const char* msg, Stream& stream = errorStream) {
      stream.print(InfoHeader);
      stream.print(msg);
      isDataReady = true;
    }

    void error (const char* msg, Stream& stream = errorStream) {
      stream.print(ErrorHeader);
      stream.println(msg);
      memoryInfo(stream);
      isDataReady = true;
    }
  }


namespace JsonWriter{
  void write() {
    using namespace Website;
    if(Log::isDataReady){
      //LogOutput.Send(Log::errorStream.c_str());
      Serial.println(Log::errorStream.c_str());
      Log::errorStream.clear();
      Log::isDataReady = false;
    }
    if(Switch::isDataReady){
      Serial.println(Switch::getVisuinoOutput());
      //ServerSwitchOutput.Send(Switch::toString());
      Switch::isDataReady = false;
    }
    if(Slider::isDataReady){
      Serial.println(Slider::getVisuinoOutput());
      Slider::isDataReady = false;
    }
    if(NumberInput::isDataReady) {
      Serial.println(NumberInput::getVisuinoOutput());
      NumberInput::isDataReady = false;
    }
    if(Button::isDataReady){
      Serial.println(Button::getVisuinoOutput());
      Button::isDataReady = false;
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
    request->send(response);
  });


  webServer.on("/input", HTTP_GET, [] (AsyncWebServerRequest* request){
    AsyncWebServerResponse* response = request->beginResponse(HTTP_STATUS_OK, "application/json", card.onHTTPRequest()[JsonKey::Body]);
    fullCorsAllow(response);
    request->send(response);
  });

  webServer.on("/status", HTTP_POST, [] (AsyncWebServerRequest* request){}, nullptr,
          [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

    card.onComponentStatusHTTPRequest(data, len);
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
  Log::errorStream.reserve(100);
  server.begin();
}
}


void setup(){

  Serial.begin(9600);
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  if(!SPIFFS.exists("/index.html")) {
    Serial.println("index.html not found");
  }
  WebsiteServer::WiFiInit();

  uint32_t before = millis();
  using namespace WebsiteServer;
  using namespace WebsiteServer::JsonReader;
  InputJsonStatus status = WebsiteServer::JsonReader::readWebsiteComponentsFromJson(WebsiteServer::testWebsiteConfigStr);
  const char* msg = errorHandler(status);
  if(status != InputJsonStatus::OK) Log::error(msg);
  uint32 after = millis();
  Serial.print("Execution time: ");
  Serial.println(after - before);
  WebsiteServer::Log::error("error test", Serial);
}



void loop(){
  WebsiteServer::JsonWriter::write();
}

