#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <FS.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
// Force AsyncTCP to run on one core!
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define CONFIG_ASYNC_TCP_USE_WDT 1
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <sstream>
#include <vector>

#include <ArduinoJson.h>

#define DEBUG_BUILD 1

namespace WebsiteServer {
  
AsyncWebServer server(80);
AsyncWebSocket dataSocket("/dataSocket");
AsyncWebSocket statusSocket("/statusSocket");             // to send information about data transmission fails and successes


const char* CLIENT_MESSAGE_OK PROGMEM = "OK";
const char* CLIENT_MESSAGE_FAIL PROGMEM = "FAIL";


const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN PROGMEM = "Access-Control-Allow-Origin";
const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS PROGMEM = "Access-Control-Allow-Methods";
const char* CORS_ALLOWED_METHODS PROGMEM = "POST,GET,OPTIONS";

const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS PROGMEM = "Access-Control-Allow-Headers";
const char* CORS_ALLOWED_HEADERS PROGMEM = "Origin, X-Requested-With, Content-Type, Accept";


const uint16_t HTTP_STATUS_OK PROGMEM = 200;
const uint16_t HTTP_STATUS_OK_NO_CONTENT PROGMEM = 204;
const uint16_t HTTP_STATUS_BAD_REQUEST PROGMEM = 400;
const uint16_t HTTP_STATUS_INTERNAL_SERVER_ERROR PROGMEM = 500;



namespace DefaultValues {
  const char* Color PROGMEM = "#333333";
  const char* Color2 PROGMEM = "darkorange";
  const char* LedColor PROGMEM = "lime";
  const char* TextColor PROGMEM = "white";
  const char* FieldOutlineColor PROGMEM = "black";

  const uint8_t FontSize PROGMEM = 16;
  const uint16_t Width PROGMEM = 100;
  const uint16_t Height PROGMEM = 100;
  const uint16_t SliderHeight PROGMEM = 20;
  const uint16_t BarHeight PROGMEM = 20;
  const bool BooleanValue PROGMEM = false;
  const bool IsVertical PROGMEM = false;
}

namespace ComponentType {
  namespace Input {
    const char* Switch PROGMEM = "switch";
    const char* Slider PROGMEM = "slider";
    const char* NumberInput PROGMEM = "numberInput";
    const char* Button PROGMEM = "button";
  }
  namespace Output {
    const char* Label PROGMEM = "label";
    const char* Indicator PROGMEM = "indicator";
    const char* Chart PROGMEM = "chart";
    const char* Gauge PROGMEM = "gauge";
    const char* ProgressBar PROGMEM = "progressBar";
    const char* Field PROGMEM = "field";
  }
}

// DO NOT MODIFY!
namespace JsonKey {
  const char* Title PROGMEM = "title";
  const char* Body PROGMEM = "body";
  const char* Elements PROGMEM = "elements";

  const char* Name PROGMEM = "name";
  const char* Width PROGMEM = "width";
  const char* Height PROGMEM = "height";
  const char* PosX PROGMEM = "posX";
  const char* PosY PROGMEM = "posY";
  const char* ComponentType PROGMEM = "componentType";
  const char* Value PROGMEM = "value";
  const char* Color PROGMEM = "color";
  const char* FontSize PROGMEM = "fontSize";
  const char* Text PROGMEM = "text";
  const char* TextColor PROGMEM = "textColor";
  const char* IsVertical PROGMEM = "isVertical";
  const char* Size PROGMEM = "size";
  const char* FieldOutlineColor PROGMEM = "outlineColor";

  const char* MaxValue PROGMEM = "maxValue";
  const char* MinValue PROGMEM = "minValue";

}


namespace ErrorMessage {
  namespace Memory {
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
    const char* ComponentTypeNotFound = "Json Input - componentType not found";
    const char* OK PROGMEM = "Json Input - Ok";
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
#ifdef ESP8266
      stream.println(MemStats);

      stream.print(HeapFragmentationMsg);;
      stream.println(ESP.getHeapFragmentation());

      stream.print(FreeHeapMsg);
      stream.println(ESP.getFreeHeap());

      stream.print(MaxFreeHeapBlock);
      stream.println(ESP.getMaxFreeBlockSize());
#endif
#ifdef ESP32
      stream.println(MemStats);

      stream.print(FreeHeapMsg);
      stream.println(ESP.getFreeHeap());

      stream.print(MaxFreeHeapBlock);
      stream.print(ESP.getMaxAllocHeap());
#endif
      stream.println();
      isDataReady = true;
    }
    void info (const char* msg, Stream& stream = errorStream) {
      stream.print(InfoHeader);
      stream.print(" ");
      stream.println(msg);
      isDataReady = true;
    }
    void error (const char* msg, Stream& stream = errorStream) {
      stream.print(ErrorHeader);
      stream.print(" ");
      stream.println(msg);
      memoryInfo(stream);
      isDataReady = true;
    }
  }



// Abstraction on Json Document which is common for few parts of app to make it thread safe
class CommonJsonMemory {
public:
  ~CommonJsonMemory() {this->garbageCollect();}
  bool allocate(size_t size) {
    mem = new DynamicJsonDocument(size);
    if (mem->capacity() != 0) {
      m_isInitialized = true;
      return true;
    }
    else return false;
  }
  void garbageCollect() {delete mem;}
  void lock() {this->m_isLocked = true;}
  void unlock() {this->m_isLocked = false;}
  bool isReadyToUse() {return (!m_isLocked) && m_isInitialized;}
  DynamicJsonDocument* get() {return this->mem;}
private:
  DynamicJsonDocument* mem;
  bool m_isLocked;
  bool m_isInitialized;
};

namespace Website {

  class WebsiteComponent {
  public:
    explicit WebsiteComponent(const JsonObjectConst& inputObject);
    virtual ~WebsiteComponent() = default;

      // ** THIS METHOD USES COMMON MEMORY(DOCUMENT) FOR STORING JSON TO AVOID MULTIPLE HEAP ALLOCATIONS **
      // ** WHEN YOU GET OBJECT, THE PREVIOUS ONE IS DELETED AUTOMATICALLY **
      // ** REMEMBER TO CHECK THAT MEMORY IS NOT LOCKED ADN WRAP THIS METHOD IN lock() and release() functions to make it thread safe (ESP32)
    virtual JsonObject toWebsiteJson() = 0;
    virtual void setState(const JsonObjectConst& object) = 0;
    bool isInitializedOK() const {return initializedOK;}
    const String& getName() const  {return name;}

    static void setJsonMemory (CommonJsonMemory* mem);
    static CommonJsonMemory* getComponentJsonMemory() { return jsonMemory; }
  protected:
    static CommonJsonMemory* jsonMemory;
    bool initializedOK;
    uint16_t posX;
    uint16_t posY;
    String name;
  };
  CommonJsonMemory* WebsiteComponent::jsonMemory = nullptr;

  WebsiteComponent::WebsiteComponent(const JsonObjectConst& inputObject){
    initializedOK = true;
    if(inputObject.containsKey(JsonKey::Name)) {
      this->name = inputObject[JsonKey::Name].as<const char*>();
    } else {
      Log::error("Name not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::PosX)) {
      this->posX = inputObject[JsonKey::PosX];
    } else {
      Log::error("PosX not found");
      initializedOK = false;
    }
    if(inputObject.containsKey(JsonKey::PosY)) {
      this->posY = inputObject[JsonKey::PosY];
    } else {
      Log::error("PosY not found");
      initializedOK = false;
    }

  }


  void WebsiteComponent::setJsonMemory (CommonJsonMemory* mem) {
    if ( mem != nullptr ) {
      jsonMemory = mem;
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
      : WebsiteComponent(inputObject){}
  };



  class Switch : public InputComponent {
  public:
    explicit Switch(const JsonObjectConst& inputObject)
            : InputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else {
        this->value = DefaultValues::BooleanValue;
      }
      if(inputObject.containsKey(JsonKey::Size)){
        this->size = inputObject[JsonKey::Size];
      } else this->size = 10;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = jsonMemory->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Size] = this->size;
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
    uint16_t size;
  };

  String Switch::str;
  bool Switch::isDataReady = false;

  class Slider : public InputComponent {
  public:
    explicit Slider(const JsonObjectConst& inputObject)
      : InputComponent(inputObject) {
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::SliderHeight;
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
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = jsonMemory->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
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
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = jsonMemory->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }


    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
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
        this->text = inputObject[JsonKey::Text].as<const char*>();
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::TextColor)){
        this->textColor = inputObject[JsonKey::TextColor].as<const char*>();
      } else this->textColor = DefaultValues::TextColor;
      if(inputObject.containsKey(JsonKey::IsVertical)){
        this->isVertical = inputObject[JsonKey::IsVertical];
      } else this->isVertical = DefaultValues::IsVertical;
      this->value = false;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = jsonMemory->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::TextColor] = this->textColor;
      websiteObj[JsonKey::Text] = this->text;
      websiteObj[JsonKey::FontSize] = this->fontSize;
      websiteObj[JsonKey::IsVertical] = this->isVertical;
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
    bool isVertical;
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
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value].as<const char*>();
      } else this->value = "";
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
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
        this->color = object[JsonKey::Color].as<const char*>();
      }
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value].as<const char*>();
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
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::Height;
    }

    JsonObject toWebsiteJson() override{
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst& object) override{
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      } else this->value = 0;
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<const char*>();
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

  class LedIndicator : public OutputComponent {
  public:
    explicit LedIndicator(const JsonObjectConst& inputObject)
      : OutputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Value)){
        this->value = inputObject[JsonKey::Value];
      } else this->value = DefaultValues::BooleanValue;
      if(inputObject.containsKey(JsonKey::Size)){
        this->size = inputObject[JsonKey::Size];
      } else this->size = 10;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::LedColor;
    }

    JsonObject toWebsiteJson() override{
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::Size] = this->size;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Indicator;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override{
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      }
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<const char*>();
      }
    }

  private:
    bool value;
    uint16_t size;
    String color;
  };


  class ProgressBar : public OutputComponent{
  public:
    explicit ProgressBar(const JsonObjectConst& inputObject)
      : OutputComponent(inputObject) {
      if(inputObject.containsKey(JsonKey::MinValue)) {
        this->minValue = inputObject[JsonKey::MinValue];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::MaxValue)) {
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else initializedOK = false;
      if(inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if(inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color2;
      if(inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::BarHeight;
      if(inputObject.containsKey(JsonKey::IsVertical)) {
        this->isVertical = inputObject[JsonKey::IsVertical];
      } else this->isVertical = DefaultValues::IsVertical;
    }


    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::MaxValue] = this->maxValue;
      websiteObj[JsonKey::MinValue] = this->minValue;
      websiteObj[JsonKey::IsVertical] = this->isVertical;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::ProgressBar;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override{
      if(object.containsKey(JsonKey::Value)){
        this->value = object[JsonKey::Value];
      }
      if(object.containsKey(JsonKey::Color)){
        this->color.clear();
        this->color = object[JsonKey::Color].as<const char*>();
      }
    }
  private:
    String color;
    uint16_t maxValue;
    uint16_t minValue;
    float value;
    uint16_t width;
    uint16_t height;
    bool isVertical;
  };

  class ColorField : public OutputComponent{
  public:
    explicit ColorField(const JsonObjectConst& inputObject)
      : OutputComponent(inputObject){
      if(inputObject.containsKey(JsonKey::Width)){
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if(inputObject.containsKey(JsonKey::Height)){
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::Height;
      if(inputObject.containsKey(JsonKey::Color)){
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
      if(inputObject.containsKey(JsonKey::FieldOutlineColor)){
        this->outlineColor = inputObject[JsonKey::FieldOutlineColor].as<const char*>();
      } else this->outlineColor = DefaultValues::FieldOutlineColor;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = jsonMemory->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::FieldOutlineColor] = this->outlineColor;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Field;
      return websiteObj;
    }
    void setState(const JsonObjectConst& object) override {
      if(object.containsKey(JsonKey::Color)){
        this->color = object[JsonKey::Color].as<String>();
      }
    }
  private:
    uint16_t width;
    uint16_t height;
    String color;
    String outlineColor;
  };



  class Card {
  public:
    Card() = default;
    ~Card() {this->garbageCollect();}
    enum class ComponentStatus : uint8_t{
      OK,
      OBJECT_NOT_VALID,
      COMPONENT_TYPE_NOT_FOUND,
    };

    ComponentStatus add(const JsonObjectConst& object);
    JsonObject onHTTPRequest();
    bool onComponentStatusHTTPRequest(const uint8_t *data, size_t len);
    void reserve(size_t size);
    void garbageCollect();
    const String& getTitle() const {return this->title;}
    void setTitle(const String& nTitle) {this->title = nTitle;}

    static void setJsonMemory(CommonJsonMemory* mem);
    static void setJsonMemoryForVisuino(CommonJsonMemory* mem);
    static void lockJsonMemory();
    static void releaseJsonMemory();
    static bool isMemoryReadyToUse();

    static void lockOutputJsonMemory();
    static void releaseOutputJsonMemory();
    static bool isOutputMemoryReadyToUse();

    static CommonJsonMemory* getJsonMemory() { return jsonMemory; }
    static CommonJsonMemory* getOutputJsonMemory() { return outputJsonMemory; }

  private:
    template <typename componentType> bool parseInputComponentToWebsite(const JsonObjectConst& object);
    template <typename componentType> bool parseOutputComponentToWebsite(const JsonObjectConst& object);
    template <typename componentType> bool parseInputComponentToVisuino(const JsonObjectConst& object);
    WebsiteComponent* getComponentByName(const char* name);
    bool componentAlreadyExists(const char* componentName);
    std::vector<WebsiteComponent*> components;
    String title;
    static CommonJsonMemory* jsonMemory;                        // main JSON memory, used for preparing data to /input HTTP request, size is all components size * 2 (common with parsing json)
    static CommonJsonMemory* outputJsonMemory;                  // json document for visuino output - required for multicore ESP32 - on 8266 points on the same as "jsonMemory"
};

  CommonJsonMemory* Card::jsonMemory;
  CommonJsonMemory* Card::outputJsonMemory;

  Card::ComponentStatus Card::add(const JsonObjectConst& object) {
    if(!object.containsKey(JsonKey::Name) || !object.containsKey(JsonKey::ComponentType)) return ComponentStatus::OBJECT_NOT_VALID;
    const char* componentType = object[JsonKey::ComponentType];
    if(strlen(componentType) < 1) return ComponentStatus::COMPONENT_TYPE_NOT_FOUND;
    using namespace ComponentType;

    if(!strncmp(componentType, Input::Switch, strlen(componentType))) {
      parseInputComponentToWebsite<Switch>(object);
    } else if(!strncmp(componentType, Input::NumberInput, strlen(componentType))) {
      parseInputComponentToWebsite<NumberInput>(object);
    } else if(!strncmp(componentType, Input::Slider, strlen(componentType))){
      parseInputComponentToWebsite<Slider>(object);
    } else if(!strncmp(componentType, Input::Button, strlen(componentType))){
      parseInputComponentToWebsite<Button>(object);
    }
    else if(!strncmp(componentType, Output::Label, strlen(componentType))){
      parseOutputComponentToWebsite<Label>(object);
    } else if(!strncmp(componentType, Output::Gauge, strlen(componentType))){
      parseOutputComponentToWebsite<Gauge>(object);
    } else if(!strncmp(componentType, Output::Indicator, strlen(componentType))){
      parseOutputComponentToWebsite<LedIndicator>(object);
    } else if(!strncmp(componentType, Output::ProgressBar, strlen(componentType))){
      parseOutputComponentToWebsite<ProgressBar>(object);
    } else if(!strncmp(componentType, Output::Field, strlen(componentType))){
      parseOutputComponentToWebsite<ColorField>(object);
    }

    else {
      return ComponentStatus::OBJECT_NOT_VALID;
    }
    return ComponentStatus::OK;
  }

  JsonObject Card::onHTTPRequest() {
    //jsonMemory should be already locked before calling this func!!
    JsonObject object = jsonMemory->get()->to<JsonObject>();
    JsonArray elements = object[JsonKey::Body].createNestedArray(JsonKey::Elements);
    auto componentJsonMemory = WebsiteComponent::getComponentJsonMemory();
    if(componentJsonMemory->isReadyToUse()){
      componentJsonMemory->lock();     // lock component memory
      for(auto component : this->components) {
        elements.add(component->toWebsiteJson());
      }
      componentJsonMemory->unlock(); // components are copied to main memory, so we can release it.
    }
    return object;
  }

  bool Card::onComponentStatusHTTPRequest(const uint8_t* data, size_t len){
    deserializeJson(*outputJsonMemory->get(), reinterpret_cast<const char*>(data), len);
    auto receivedJson = outputJsonMemory->get()->as<JsonObject>();
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


  WebsiteComponent* Card::getComponentByName(const char* name) {
    for(auto component : this->components){
      if(component->getName().equals(name)) return component;
    }
    return nullptr;
  }

  void Card::setJsonMemory(CommonJsonMemory* mem) {
    jsonMemory = mem;
  }

  void Card::setJsonMemoryForVisuino(CommonJsonMemory* mem) {
    outputJsonMemory = mem;
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
      if(component != nullptr) component->setState(object);
      else return false;
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
    }
    return true;
  }

  template<typename componentType>
  bool Card::parseInputComponentToVisuino(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
    auto component = reinterpret_cast<InputComponent*>(getComponentByName(componentName));
    if(component == nullptr) return false;
    component->setState(object);
    auto componentJsonMemory = WebsiteComponent::getComponentJsonMemory();
    if(componentJsonMemory->isReadyToUse()){
      componentJsonMemory->lock();
      componentType::setVisuinoOutput(component->toVisuinoJson());
      componentJsonMemory->unlock();
    } else return false;
    return true;
  }

}


String testWebsiteConfigStr = {R"(
{
  "elements" : [
      {
        "name" : "Lamp2",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 100,
        "posY" : 100,
        "color" : "blue",
        "value": false
      },
      {
        "name" : "Motor",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 400,
        "posY" : 400,
        "color" : "#abcdef",
        "value": false
      },
      {
        "name" : "Nothing",
        "size" : 25,
        "componentType" : "switch",
        "posX" : 170,
        "posY" : 100,
        "color" : "lightgreen",
        "value": false
      },
      {
        "name" : "Something",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 50,
        "posY" : 150,
        "color" : "#bbaaaa",
        "value": true
      },
      {
        "name" : "Servo",
        "size" : 20,
        "componentType" : "switch",
        "dataType" : "boolean",
        "posX" : 170,
        "posY" : 150,
        "color" : "#abcada",
        "value": true
      },
      {
        "name" : "LightBulb",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 100,
        "posY" : 150,
        "color" : "#00ffe5",
        "value": true
      },
      {
        "name" : "bulb",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 50,
        "posY" : 100,
        "color" : "orange",
        "value": true
      },
      {
        "name" : "bulb2",
        "size" : 20,
        "componentType" : "switch",
        "posX" : 10,
        "posY" : 100,
        "color" : "red",
        "value": false
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
        "name" : "Info2",
        "componentType" : "label",
        "posX" : 200,
        "posY" : 4000,
        "color" : "#ff2233",
        "value": "Jak tam"
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
      "name" : "progress-bar",
      "componentType" : "progressBar",
      "posX" : 700,
      "posY" : 350,
      "color" : "orangered",
      "value": 700,
      "maxValue": 800,
      "minValue": 0,
      "width" : 250,
      "height": 20,
      "isVertical": true
    },
    {
      "name" : "progress-bar2",
      "componentType" : "progressBar",
      "posX" : 750,
      "posY" : 600,
      "color" : "lime",
      "value": 700,
      "maxValue": 800,
      "minValue": 0,
      "width" : 250,
      "height": 20,
      "isVertical": false
    },
    {
      "name": "btn2",
      "textColor": "white",
      "fontSize": 16,
      "text": "Horizontal",
      "width": 40,
      "height": 200,
      "color": "darkorange",
      "posX": 1400,
      "posY": 500,
      "isVertical": true,
      "componentType": "button"
    },
    {
      "name": "btn",
      "textColor": "white",
      "fontSize": 16,
      "text": "Vertical",
      "width": 200,
      "height": 40,
      "color": "blue",
      "posX": 1000,
      "posY": 700,
      "componentType": "button"
    },
    {
      "name": "controls",
      "color": "#bbb",
      "width": 800,
      "height" : 400,
      "componentType": "field",
      "posY": 0,
      "posX": 0
    },
    {
      "name": "controls2",
      "color": "#bbb",
      "width": 800,
      "height" : 400,
      "componentType": "field",
      "posY": 0,
      "posX": 800
    },
    {
      "name": "controls3",
      "color": "#ddd",
      "width": 800,
      "height" : 400,
      "componentType": "field",
      "posY": 400,
      "posX": 0
    },
    {
      "name": "controls4",
      "color": "#ddd",
      "width": 800,
      "height" : 400,
      "componentType": "field",
      "posY": 400,
      "posX": 800
    },
    {
      "name": "btna",
      "textColor": "white",
      "fontSize": 16,
      "text": "Vertical",
      "width": 200,
      "height": 40,
      "color": "blue",
      "posX": 50,
      "posY": 700,
      "componentType": "button"
    },
    {
      "name": "btnb",
      "textColor": "white",
      "fontSize": 16,
      "text": "Vertical",
      "width": 200,
      "height": 40,
      "color": "blue",
      "posX": 300,
      "posY": 700,
      "componentType": "button"
    },
      {
        "name" : "buttonInfo",
        "componentType" : "label",
        "posX" : 50,
        "posY" : 670,
        "color" : "blue",
        "value": "Button",
        "fontSize" : "25"
      },
      {
        "name" : "buttonInfo2",
        "componentType" : "label",
        "posX" : 300,
        "posY" : 670,
        "color" : "blue",
        "value": "Button2",
        "fontSize" : "25"
      }
  ]
})"};



Website::Card card;

namespace JsonReader {
  size_t memorySize = 0;
  CommonJsonMemory inputJsonMemory;
  CommonJsonMemory componentJsonMemory;
#ifdef ESP32
  CommonJsonMemory visuinoOutputJsonMemory;
#endif

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
    COMPONENT_TYPE_NOT_FOUND,
  };

  bool validateJson(const String& json){
    return !(json.isEmpty() || json.indexOf(JsonKey::Name) < 0);
  }

  size_t getBiggestObjectSize(const JsonArrayConst& arr){
    size_t biggest = 0;
    for (const auto& item : arr) {
      if(item.memoryUsage() > biggest) biggest = item.memoryUsage();
    }
    return biggest;
  }
  
  size_t getBufferSize(size_t memoryUsage){
    size_t newSize = memoryUsage * 2;
    return newSize;
  }

  // ESP32 needs second JSON document because of multicore architecture and it could not be common with input memory
  bool setVisuinoOutputMemory(size_t size){
#ifdef ESP32
    if(visuinoOutputJsonMemory.allocate(size)){
      Website::Card::setJsonMemoryForVisuino(&visuinoOutputJsonMemory);
      return true;
    } else{
      visuinoOutputJsonMemory.garbageCollect();
      return false;
    }
#endif
#ifdef ESP8266
    Website::Card::setJsonMemoryForVisuino(&inputJsonMemory);
    return true;
#endif
  }

  InputJsonStatus readWebsiteComponentsFromJson(const String& json) {
    using namespace Website;
    static bool isValid = false;
    static bool isInitialized = false;
    static bool isElementsInitialized = false;
    isValid = validateJson(json);
    if (isValid) {
      if (!isInitialized) {
        memorySize = json.length();
        Serial.println((int)memorySize);
        if (inputJsonMemory.allocate(getBufferSize(memorySize))) {
          Website::Card::setJsonMemory(&inputJsonMemory);
          isInitialized = true;
        } else {
          inputJsonMemory.garbageCollect();
          return InputJsonStatus::ALLOC_ERROR;
        }
      }

      if(inputJsonMemory.isReadyToUse()){

        // unlock json memory before returning
        auto releaseAndReturn = [&] (InputJsonStatus status) {
          inputJsonMemory.unlock();
          return status;
        };

        inputJsonMemory.lock();
        deserializeJson(*inputJsonMemory.get(), json);
        if (inputJsonMemory.get()->overflowed()) return releaseAndReturn(InputJsonStatus::JSON_OVERFLOW);

        JsonObject inputObject = inputJsonMemory.get()->as<JsonObject>();
        if (!inputObject.containsKey(JsonKey::Elements)) return releaseAndReturn(InputJsonStatus::ELEMENTS_NOT_FOUND);
        JsonArray elements = inputObject[JsonKey::Elements].as<JsonArray>();
        if (elements.size() == 0) return releaseAndReturn(InputJsonStatus::ELEMENTS_ARRAY_EMPTY);
        if(!isElementsInitialized) {
          size_t biggestObjectSize = getBiggestObjectSize(elements);

          if(!setVisuinoOutputMemory(getBufferSize(biggestObjectSize))) return releaseAndReturn(InputJsonStatus::ALLOC_ERROR);
          if(componentJsonMemory.allocate(getBufferSize(biggestObjectSize))){
            WebsiteComponent::setJsonMemory(&componentJsonMemory);
            isElementsInitialized = true;
          } else {
            componentJsonMemory.garbageCollect();
            return releaseAndReturn(InputJsonStatus::ALLOC_ERROR);
          }
        }

        card.reserve(elements.size());
        for (JsonObject element : elements) {
          auto res = card.add(element);
          if(res == Card::ComponentStatus::COMPONENT_TYPE_NOT_FOUND){
            return releaseAndReturn(InputJsonStatus::COMPONENT_TYPE_NOT_FOUND);
          }
          else if (res != Card::ComponentStatus::OK) {
            return releaseAndReturn(InputJsonStatus::OBJECT_NOT_VALID);
          }
        }
        inputJsonMemory.unlock();
      }


    }
    return InputJsonStatus::OK;
  }


  const char* hanndleJsonReadErrors(InputJsonStatus status){
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
      case InputJsonStatus::COMPONENT_TYPE_NOT_FOUND:
        statusStr = ComponentTypeNotFound;
        break;
    }
    return statusStr;
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

namespace WebSocketIO {
  void onReceive(AsyncWebSocket* server, AsyncWebSocketClient* client,
                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    Serial.println("on receive");
    if (type == WS_EVT_DATA) {
      auto json = Website::Card::getOutputJsonMemory();
      if (json->isReadyToUse()) {
        Serial.println("Received");
        json->lock();
        bool parsingResult = WebsiteServer::card.onComponentStatusHTTPRequest(data, len);
        statusSocket.text(client->id(), parsingResult ? CLIENT_MESSAGE_OK : CLIENT_MESSAGE_FAIL);
        json->unlock();
      } else {
        Serial.println("Json not ready");
      }
    }
  }
  void sendWebsiteData() {
    static String websiteData;
    auto json = Website::Card::getJsonMemory();
    if(json->isReadyToUse()) {
      json->lock();
      serializeJson(WebsiteServer::card.onHTTPRequest()[JsonKey::Body], websiteData);
      if(dataSocket.availableForWriteAll()) {
        Serial.println("Writeall");
        dataSocket.textAll(websiteData);
        Serial.println("Writed");
      }
      websiteData.clear();
      json->unlock();
    }
  }


  // TODO: params only in debug builds to run it using RTOS task - should be deleted in visuino
  void onVisuinoInput(void* params) {
    while (true) {
      JsonReader::hanndleJsonReadErrors(JsonReader::readWebsiteComponentsFromJson(testWebsiteConfigStr));
      sendWebsiteData();
      vTaskDelay(500 / portTICK_RATE_MS);
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
#ifdef DEBUG_MODE
    Log::info("Component", Serial);
    Log::memoryInfo(Serial);
#endif
  });

  webServer.on("/webSocketIO.js", HTTP_GET, [] (AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/webSocketIO.js", "application/javascript");
  });

  webServer.on("/Libs/pureknobMin.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Libs/pureknobMin.js","application/javascript");
#ifdef DEBUG_MODE
    Log::info("Knob", Serial);
    Log::memoryInfo(Serial);
#endif
  });

  webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/favicon.ico","image/ico");
  });
}

void HTTPSetMappings(AsyncWebServer& webServer){

  webServer.on("/init", HTTP_GET, [] (AsyncWebServerRequest* request){
#ifdef DEBUG_BUILD
    request->send(HTTP_STATUS_OK, "text/plain", "Debug Build");
#else
    const String& title = card.getTitle();
    if (!title.isEmpty() || title.equals("")){
      request->send(HTTP_STATUS_OK, "text/plain", title);
    } else request->send(HTTP_STATUS_OK_NO_CONTENT);
#endif
  });
}

#if DEBUG_BUILD
void registerWiFIDebugEvents(void){
  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("AP connected!");
  },SYSTEM_EVENT_AP_STACONNECTED);

  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("AP_Disconnected!");
  },SYSTEM_EVENT_AP_STADISCONNECTED);

  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("STA_Connected");
  }, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("STA_Disconnected");
  }, SYSTEM_EVENT_STA_DISCONNECTED);
}
#endif


void ServerInit(){
#if DEBUG_BUILD
  registerWiFIDebugEvents();
#endif
  HTTPServeWebsite(server);
  HTTPSetMappings(server);
  server.addHandler(&dataSocket);
  server.addHandler(&statusSocket);
  dataSocket.onEvent(WebSocketIO::onReceive);
  server.begin();
  Log::errorStream.reserve(100);
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

  WiFi.softAP("esp_ap", "123456789");
  WebsiteServer::ServerInit();
  TaskHandle_t handle = nullptr;


  xTaskCreate(WebsiteServer::WebSocketIO::onVisuinoInput,
                          "Proc",
                          2048,
                          nullptr,
                          tskIDLE_PRIORITY,
                          &handle);


}



void loop(){
  WebsiteServer::JsonWriter::write();

}

