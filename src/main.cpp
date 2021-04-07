#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <FS.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <SPIFFS.h>
// Force AsyncTCP to run on one core!
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define CONFIG_ASYNC_TCP_USE_WDT 1
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <vector>
#include <ArduinoJson.h>

#define DEBUG_BUILD 1
static String testWebsiteConfigStr = {R"(
{
  "main": [
        {
          "name" : "Lamp2",
          "size" : 20,
          "componentType" : "switch",
          "posX" : 0,
          "posY" : 0,
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
          "size" : 30,
          "componentType" : "switch",
          "posX" : 170,
          "posY" : 100,
          "color" : "lightgreen",
          "value": false
        },
        {
          "name" : "Something",
          "size" : 30,
          "componentType" : "switch",
          "posX" : 50,
          "posY" : 150,
          "color" : "#bbaaaa",
          "value": true
        },
        {
          "name" : "Servo",
          "size" : 40,
          "componentType" : "switch",
          "dataType" : "boolean",
          "posX" : 170,
          "posY" : 150,
          "color" : "#abcada",
          "value": true
        },
        {
          "name" : "LightBulb",
          "size" : 25,
          "componentType" : "switch",
          "posX" : 100,
          "posY" : 150,
          "color" : "#00ffe5",
          "value": true
        },
        {
          "name" : "bulb",
          "size" : 25,
          "componentType" : "switch",
          "posX" : 50,
          "posY" : 100,
          "color" : "orange",
          "value": true
        },
        {
          "name" : "Info",
          "componentType" : "label",
          "posX" : 100,
          "posY" : 300,
          "fontSize": 24,
          "color" : "red",
          "isVertical": true,
          "value": "Pionowy napis"
        },
        {
          "name" : "Info2",
          "componentType" : "label",
          "posX" : 100,
          "posY" : 700,
          "fontSize": 30,
          "color" : "lime",
          "isVertical": false,
          "value": "Poziomy napis"
        },
        {
          "name" : "Speed",
          "componentType" : "gauge",
          "posX" : 200,
          "posY" : 300,
          "color" : "red",
          "value": 480,
          "maxValue": 800,
          "minValue": 0,
          "width" : 500,
          "height": 400
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
        "color": "#333",
        "fontSize": 48,
        "width": 150,
        "value": 44.2,
        "componentType": "numberInput",
        "posY": 200,
        "posX": 400
      },
      {
        "name": "btn1",
        "textColor": "white",
        "fontSize": 16,
        "text": "Vertical",
        "width": 40,
        "height": 200,
        "color": "#555",
        "posX": 1400,
        "posY": 400,
        "isVertical": true,
        "componentType": "button"
      },
      {
        "name": "btn2",
        "textColor": "white",
        "fontSize": 16,
        "text": "Horizontal",
        "width": 200,
        "height": 40,
        "color": "#333",
        "posX": 1400,
        "posY": 700,
        "isVertical": false,
        "componentType": "button"
      },
      {
        "name": "led",
        "componentType": "indicator",
        "value": true,
        "posY": 100,
        "posX": 900,
        "color": "red",
        "size": 50
      },
      {
        "name": "led2",
        "componentType": "indicator",
        "value": true,
        "posY": 100,
        "posX": 950,
        "color": "yellow",
        "size": 50
      },
      {
        "name": "led3",
        "componentType": "indicator",
        "value": true,
        "posY": 100,
        "posX": 1000,
        "color": "lime",
        "size": 50
      },
      {
        "name": "led4",
        "componentType": "indicator",
        "posY": 100,
        "posX": 1050,
        "value": true,
        "size": 50,
        "color": "yellow"
      },
      {
        "name": "controls3",
        "color": "orangered",
        "width": 800,
        "height" : 400,
        "componentType": "field",
        "posY": 400,
        "posX": 800,
        "outlineColor": "black"
      },
      {
        "name": "controls4",
        "color": "#333",
        "width": 800,
        "height" : 400,
        "componentType": "field",
        "posY": 400,
        "posX": 0,
        "outlineColor": "black"
      },
      {
        "name": "image1",
        "width": 0,
        "height" : 0,
        "componentType": "image",
        "posY": 0,
        "posX": 0,
        "fileName": "test.jpg"
      }
    ],
    "settings": [
        {
          "name" : "progress-bar",
          "componentType" : "progressBar",
          "posX" : 700,
          "posY" : 350,
          "color" : "#cc0000",
          "value":  600,
          "maxValue": 800,
          "minValue": 0,
          "width" : 400,
          "height": 20,
          "isVertical": true
        },
        {
          "name": "settings-controls",
          "color": "#999",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 0,
          "posX": 0,
          "outlineColor": "black"
        },
        {
          "name": "settings-controls2",
          "color": "darkorange",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 0,
          "posX": 800,
          "outlineColor": "black"
        },
        {
          "name": "settings-controls3",
          "color": "orangered",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 400,
          "posX": 800,
          "outlineColor": "black"
        },
        {
          "name": "settings-image1",
          "width": 0,
          "height" : 0,
          "componentType": "image",
          "posY": 100,
          "posX": 300,
          "fileName": "test.jpg"
        },

      {
        "name": "settings-btn1",
        "textColor": "white",
        "fontSize": 16,
        "text": "Vertical",
        "width": 40,
        "height": 200,
        "color": "#85a",
        "posX": 1400,
        "posY": 400,
        "isVertical": true,
        "componentType": "button"
      }],
      "settings2": [
        {
          "name" : "progress2-bar",
          "componentType" : "progressBar",
          "posX" : 700,
          "posY" : 350,
          "color" : "#cc0000",
          "value":  600,
          "maxValue": 800,
          "minValue": 0,
          "width" : 400,
          "height": 20,
          "isVertical": true
        },
        {
          "name": "setting2s-controls",
          "color": "#999",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 0,
          "posX": 0,
          "outlineColor": "black"
        },
        {
          "name": "settings2-controls2",
          "color": "darkorange",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 0,
          "posX": 800,
          "outlineColor": "black"
        },
        {
          "name": "settings-2controls3",
          "color": "orangered",
          "width": 800,
          "height" : 400,
          "componentType": "field",
          "posY": 400,
          "posX": 800,
          "outlineColor": "black"
        },
        {
          "name": "settings2image1",
          "width": 0,
          "height" : 0,
          "componentType": "image",
          "posY": 100,
          "posX": 300,
          "fileName": "test.jpg"
        },

      {
        "name": "settings2-btn1",
        "textColor": "white",
        "fontSize": 16,
        "text": "Vertical",
        "width": 40,
        "height": 200,
        "color": "#85a",
        "posX": 1400,
        "posY": 400,
        "isVertical": true,
        "componentType": "button"
      }
      ]
})"};



namespace WebsiteServer {
  
AsyncWebServer server(80);
const uint32_t IMAGE_MAX_SIZE = 100000;

const uint8_t CONNECT_ATTEMPTS_MAX = 10;

const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN PROGMEM = "Access-Control-Allow-Origin";
const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS PROGMEM = "Access-Control-Allow-Methods";
const char* CORS_ALLOWED_METHODS PROGMEM = "POST,GET,OPTIONS";

const char* CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS PROGMEM = "Access-Control-Allow-Headers";
const char* CORS_ALLOWED_HEADERS PROGMEM = "Origin, X-Requested-With, Content-Type, Accept";


namespace HttpCodes {
  const uint16_t OK PROGMEM = 200;
  const uint16_t NO_CONTENT PROGMEM = 204;
  const uint16_t BAD_REQUEST PROGMEM = 400;
  const uint16_t NOT_FOUND PROGMEM = 404;
  const uint16_t PAYLOAD_TOO_LARGE PROGMEM = 413;
  const uint16_t INTERNAL_SERVER_ERROR PROGMEM = 500;
}




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
    const char* Image PROGMEM = "image";
  }
}

// DO NOT MODIFY!
namespace JsonKey {
  const char* Title PROGMEM = "title";
  const char* Body PROGMEM = "body";
  const char* Elements PROGMEM = "elements";
  const char* Tabs PROGMEM = "tabs";

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
  const char* FileName PROGMEM = "fileName";


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
    const char* TabsNotFound PROGMEM = "Json Input - Tabs not found";
    const char* TabsArrayEmpty PROGMEM = "Json Input - Tabs array empty";
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

  struct VisuinoOutputFormatter {
    void format(const JsonObjectConst object) {
      data.clear();
      serializeJson(object, data);
      isDataReady = true;
    }
    const String& getData() { return this->data; }
    bool isDataReady = false;
  private:
    String data;
  };

  class WebsiteComponent {
  public:
    explicit WebsiteComponent(const JsonObjectConst &inputObject);

    virtual ~WebsiteComponent() = default;

    // ** THIS METHOD USES COMMON MEMORY(DOCUMENT) FOR STORING JSON TO AVOID MULTIPLE HEAP ALLOCATIONS **
    // ** WHEN YOU GET OBJECT, THE PREVIOUS ONE IS DELETED AUTOMATICALLY **
    // ** REMEMBER TO CHECK THAT MEMORY IS NOT LOCKED ADN WRAP THIS METHOD IN lock() and release() functions to make it thread safe (ESP32)
    virtual JsonObject toWebsiteJson() = 0;

    virtual void setState(const JsonObjectConst &object) = 0;

    bool isInitializedOK() const { return initializedOK; }

    const String &getName() const { return name; }

    static void setComponentJsonWeakRef(CommonJsonMemory *ref) { componentJsonWeakRef = ref; }
    static CommonJsonMemory *getComponentJsonWeakRef() { return componentJsonWeakRef; };
  protected:
    static CommonJsonMemory *componentJsonWeakRef;  // weak reference to json document stored in ApplicationContext class
    bool initializedOK;
    uint16_t posX;
    uint16_t posY;
    String name;
  };

  CommonJsonMemory *WebsiteComponent::componentJsonWeakRef = nullptr;

  WebsiteComponent::WebsiteComponent(const JsonObjectConst &inputObject) {
    initializedOK = true;
    if (inputObject.containsKey(JsonKey::Name)) {
      this->name = inputObject[JsonKey::Name].as<const char *>();
    } else {
      Log::error("Name not found");
      initializedOK = false;
    }
    if (inputObject.containsKey(JsonKey::PosX)) {
      this->posX = inputObject[JsonKey::PosX];
    } else {
      Log::error("PosX not found");
      initializedOK = false;
    }
    if (inputObject.containsKey(JsonKey::PosY)) {
      this->posY = inputObject[JsonKey::PosY];
    } else {
      Log::error("PosY not found");
      initializedOK = false;
    }

  }

  // ----------------------------------------------------------------------------
  //                         COMPONENTS CLASSES
  // ----------------------------------------------------------------------------


  class InputComponent : public WebsiteComponent {
  public:
    explicit InputComponent(const JsonObjectConst &inputObject)
        : WebsiteComponent(inputObject) {
    }

    virtual JsonObject toVisuinoJson() = 0; //using common memory
  private:
  };

  class OutputComponent : public WebsiteComponent {
  public:
    explicit OutputComponent(const JsonObjectConst &inputObject)
        : WebsiteComponent(inputObject) {}
  };


  class Switch : public InputComponent {
  public:
    explicit Switch(const JsonObjectConst &inputObject)
        : InputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else {
        this->value = DefaultValues::BooleanValue;
      }
      if (inputObject.containsKey(JsonKey::Size)) {
        this->size = inputObject[JsonKey::Size];
      } else this->size = 10;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = componentJsonWeakRef->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Size] = this->size;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::Switch;
      return websiteObj;
    }

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      }
    }
    static VisuinoOutputFormatter& getVisuinoOutputFormatter() { return formatter; }
  private:
    static VisuinoOutputFormatter formatter;
    bool value;
    uint16_t size;
  };
  VisuinoOutputFormatter Switch::formatter;

  class Slider : public InputComponent {
  public:
    explicit Slider(const JsonObjectConst &inputObject)
        : InputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::SliderHeight;
      if (inputObject.containsKey(JsonKey::MinValue)) {
        this->minValue = inputObject[JsonKey::MinValue];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::MaxValue)) {
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = componentJsonWeakRef->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      }
    }
    static VisuinoOutputFormatter& getVisuinoOutputFormatter() { return formatter; }
  private:
    static VisuinoOutputFormatter formatter;
    String color;
    uint16_t width;
    uint16_t height;
    uint32_t value;
    uint32_t minValue;
    uint32_t maxValue;
  };
  VisuinoOutputFormatter Slider::formatter;

  class NumberInput : public InputComponent {
  public:
    explicit NumberInput(const JsonObjectConst &inputObject)
        : InputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0.0f;
      if (inputObject.containsKey(JsonKey::FontSize)) {
        this->fontSize = inputObject[JsonKey::FontSize];
      } else this->fontSize = DefaultValues::FontSize;
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = componentJsonWeakRef->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }


    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::FontSize] = this->fontSize;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::ComponentType] = ComponentType::Input::NumberInput;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      }
    }

    static VisuinoOutputFormatter& getVisuinoOutputFormatter() { return formatter; }
  private:
    static VisuinoOutputFormatter formatter;
    float value;
    uint16_t width;
    uint16_t fontSize;
    String color;
  };
  VisuinoOutputFormatter NumberInput::formatter;


  class Button : public InputComponent {
  public:
    explicit Button(const JsonObjectConst &inputObject)
        : InputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::FontSize)) {
        this->fontSize = inputObject[JsonKey::FontSize];
      } else this->fontSize = DefaultValues::FontSize;
      if (inputObject.containsKey(JsonKey::Text)) {
        this->text = inputObject[JsonKey::Text].as<const char*>();
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char*>();
      } else this->color = DefaultValues::Color;
      if (inputObject.containsKey(JsonKey::TextColor)) {
        this->textColor = inputObject[JsonKey::TextColor].as<const char*>();
      } else this->textColor = DefaultValues::TextColor;
      if (inputObject.containsKey(JsonKey::IsVertical)) {
        this->isVertical = inputObject[JsonKey::IsVertical];
      } else this->isVertical = DefaultValues::IsVertical;
      this->value = false;
    }

    JsonObject toVisuinoJson() override {
      JsonObject outputObj = componentJsonWeakRef->get()->to<JsonObject>();
      outputObj[JsonKey::Name] = this->name;
      outputObj[JsonKey::Value] = this->value;
      return outputObj;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value))
        this->value = object[JsonKey::Value];
    }

    static VisuinoOutputFormatter& getVisuinoOutputFormatter() { return formatter; }
  private:
    static VisuinoOutputFormatter formatter;
    bool value;
    uint16_t width;
    uint16_t height;
    uint16_t fontSize;
    String text;
    String color;
    String textColor;
    bool isVertical;
  };
  VisuinoOutputFormatter Button::formatter;


  class Label : public OutputComponent {
  public:
    explicit Label(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::FontSize)) {
        this->fontSize = inputObject[JsonKey::FontSize];
      } else fontSize = DefaultValues::FontSize;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color;
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value].as<const char *>();
      } else this->value = "";
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::FontSize] = this->fontSize;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Label;
      return websiteObj;
    }

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::FontSize)) {
        this->fontSize = object[JsonKey::FontSize];
      }
      if (object.containsKey(JsonKey::Color)) {
        this->color = object[JsonKey::Color].as<const char *>();
      }
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value].as<const char *>();
      }
    }

  private:
    uint16_t fontSize;
    String color;
    String value;
  };


  class Gauge : public OutputComponent {
  public:
    explicit Gauge(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::MaxValue)) {
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else {
        this->maxValue = 0;
        initializedOK = false;
      }
      if (inputObject.containsKey(JsonKey::MinValue)) {
        this->minValue = inputObject[JsonKey::MinValue];
      } else {
        this->maxValue = 0;
        initializedOK = false;
      }
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color;
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::Height;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      } else this->value = 0;
      if (object.containsKey(JsonKey::Color)) {
        this->color = object[JsonKey::Color].as<const char *>();
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
    explicit LedIndicator(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = DefaultValues::BooleanValue;
      if (inputObject.containsKey(JsonKey::Size)) {
        this->size = inputObject[JsonKey::Size];
      } else this->size = 10;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::LedColor;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Value] = this->value;
      websiteObj[JsonKey::Color] = this->color;
      websiteObj[JsonKey::Size] = this->size;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Indicator;
      return websiteObj;
    }

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      }
      if (object.containsKey(JsonKey::Color)) {
        this->color = object[JsonKey::Color].as<const char *>();
      }
    }

  private:
    bool value;
    uint16_t size;
    String color;
  };


  class ProgressBar : public OutputComponent {
  public:
    explicit ProgressBar(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::MinValue)) {
        this->minValue = inputObject[JsonKey::MinValue];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::MaxValue)) {
        this->maxValue = inputObject[JsonKey::MaxValue];
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::Value)) {
        this->value = inputObject[JsonKey::Value];
      } else this->value = 0;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color2;
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::BarHeight;
      if (inputObject.containsKey(JsonKey::IsVertical)) {
        this->isVertical = inputObject[JsonKey::IsVertical];
      } else this->isVertical = DefaultValues::IsVertical;
    }


    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Value)) {
        this->value = object[JsonKey::Value];
      }
      if (object.containsKey(JsonKey::Color)) {
        this->color.clear();
        this->color = object[JsonKey::Color].as<const char *>();
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

  class ColorField : public OutputComponent {
  public:
    explicit ColorField(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = DefaultValues::Width;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = DefaultValues::Height;
      if (inputObject.containsKey(JsonKey::Color)) {
        this->color = inputObject[JsonKey::Color].as<const char *>();
      } else this->color = DefaultValues::Color;
      if (inputObject.containsKey(JsonKey::FieldOutlineColor)) {
        this->outlineColor = inputObject[JsonKey::FieldOutlineColor].as<const char *>();
      } else this->outlineColor = DefaultValues::FieldOutlineColor;
    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
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

    void setState(const JsonObjectConst &object) override {
      if (object.containsKey(JsonKey::Color)) {
        this->color = object[JsonKey::Color].as<const char *>();
      }
    }

  private:
    uint16_t width;
    uint16_t height;
    String color;
    String outlineColor;
  };

  class BackgroundImage : public OutputComponent {
  public:
    explicit BackgroundImage(const JsonObjectConst &inputObject)
        : OutputComponent(inputObject) {
      if (inputObject.containsKey(JsonKey::FileName)) {
        this->filePath = inputObject[JsonKey::FileName].as<const char *>();
      } else initializedOK = false;
      if (inputObject.containsKey(JsonKey::Width)) {
        this->width = inputObject[JsonKey::Width];
      } else this->width = 0;
      if (inputObject.containsKey(JsonKey::Height)) {
        this->height = inputObject[JsonKey::Height];
      } else this->height = 0;

    }

    JsonObject toWebsiteJson() override {
      JsonObject websiteObj = componentJsonWeakRef->get()->to<JsonObject>();
      websiteObj[JsonKey::Name] = this->name;
      websiteObj[JsonKey::PosX] = this->posX;
      websiteObj[JsonKey::PosY] = this->posY;
      websiteObj[JsonKey::Width] = this->width;
      websiteObj[JsonKey::Height] = this->height;
      websiteObj[JsonKey::FileName] = this->filePath;
      websiteObj[JsonKey::ComponentType] = ComponentType::Output::Image;
      return websiteObj;
    }

    void setState(const JsonObjectConst& object) override {}   // image doesn't have state
  private:
    uint16_t width;
    uint16_t height;
    String filePath;
  };

  class WebsiteTab {
  public:
    enum class ComponentParsingStatus : uint8_t {
      OK,
      OBJECT_NOT_VALID,
      COMPONENT_TYPE_NOT_FOUND,
    };
  public:
    WebsiteTab(const char* name, const JsonArrayConst componentsJson);
    ~WebsiteTab() { this->garbageCollect(); }

    void onUpdate(const JsonArrayConst componentsJson);
    ComponentParsingStatus appendComponent(const JsonObjectConst &object);
    void onApplicationStateHttpRequest(CommonJsonMemory *target);
    bool onComponentInputHttpRequest(CommonJsonMemory *target, const uint8_t *data, size_t len);
    inline const String& getName() { return this->name; }
  private:
    template<typename ComponentType>
    bool parseInputComponentToWebsite(const JsonObjectConst &object);
    template<typename ComponentType>
    bool parseOutputComponentToWebsite(const JsonObjectConst &object);
    template<typename ComponentType>
    bool parseInputComponentToVisuino(const JsonObjectConst &object);
    WebsiteComponent *getComponentByName(const char *name);
    bool componentAlreadyExists(const char *componentName);

    inline void garbageCollect() { for (auto &component : components) delete component; }

  private:
    String name;
    std::vector<WebsiteComponent *> components;
  };

  WebsiteTab::WebsiteTab(const char* name, const JsonArrayConst componentsJson) : name(name) {
    this->onUpdate(componentsJson);
  }

  void WebsiteTab::onUpdate(const JsonArrayConst componentsJson) {
    this->components.reserve(componentsJson.size());
    for (const auto component : componentsJson) {
      this->appendComponent(component);
    }
  }

  void WebsiteTab::onApplicationStateHttpRequest(CommonJsonMemory* target) {
    auto json = target->get();
    JsonArray tabArray = json->createNestedArray(this->name);
    for (auto& component : components) {
      tabArray.add(component->toWebsiteJson());
    }
  }

  WebsiteTab::ComponentParsingStatus WebsiteTab::appendComponent(const JsonObjectConst& object) {
    if (!object.containsKey(JsonKey::Name) || !object.containsKey(JsonKey::ComponentType))
      return ComponentParsingStatus::OBJECT_NOT_VALID;
    const char *componentType = object[JsonKey::ComponentType];
    if (strlen(componentType) < 1) return ComponentParsingStatus::COMPONENT_TYPE_NOT_FOUND;
    using namespace ComponentType;

    if (!strncmp(componentType, Input::Switch, strlen(componentType))) {
      parseInputComponentToWebsite<Switch>(object);
    } else if (!strncmp(componentType, Input::NumberInput, strlen(componentType))) {
      parseInputComponentToWebsite<NumberInput>(object);
    } else if (!strncmp(componentType, Input::Slider, strlen(componentType))) {
      parseInputComponentToWebsite<Slider>(object);
    } else if (!strncmp(componentType, Input::Button, strlen(componentType))) {
      parseInputComponentToWebsite<Button>(object);
    } else if (!strncmp(componentType, Output::Label, strlen(componentType))) {
      parseOutputComponentToWebsite<Label>(object);
    } else if (!strncmp(componentType, Output::Gauge, strlen(componentType))) {
      parseOutputComponentToWebsite<Gauge>(object);
    } else if (!strncmp(componentType, Output::Indicator, strlen(componentType))) {
      parseOutputComponentToWebsite<LedIndicator>(object);
    } else if (!strncmp(componentType, Output::ProgressBar, strlen(componentType))) {
      parseOutputComponentToWebsite<ProgressBar>(object);
    } else if (!strncmp(componentType, Output::Field, strlen(componentType))) {
      parseOutputComponentToWebsite<ColorField>(object);
    } else if (!strncmp(componentType, Output::Image, strlen(componentType))) {
      parseOutputComponentToWebsite<BackgroundImage>(object);
    } else {
      return ComponentParsingStatus::OBJECT_NOT_VALID;
    }
    return ComponentParsingStatus::OK;
  }

  template<typename ComponentType>
  bool WebsiteTab::parseInputComponentToWebsite(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
    if (!componentAlreadyExists(componentName)) {
      auto component = new ComponentType(object);
      if (component->isInitializedOK()) {
        components.push_back(component);
      } else {
        delete component;
        return false;
      }
    }
    return true;
  }

  template<typename ComponentType>
  bool WebsiteTab::parseOutputComponentToWebsite(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
    if (!componentAlreadyExists(componentName)) {
      auto component = new ComponentType(object);
      if (component->isInitializedOK()) {
        components.push_back(component);
      } else {
        delete component;
        return false;
      }
    } else {
      auto component = reinterpret_cast<ComponentType*>(getComponentByName(componentName));
      if (component != nullptr) component->setState(object);
      else return false;
    }
    return true;
  }

  template<typename ComponentType>
  bool WebsiteTab::parseInputComponentToVisuino(const JsonObjectConst& object) {
    const char* componentName = object[JsonKey::Name];
    auto component = reinterpret_cast<InputComponent*>(getComponentByName(componentName));
    if (component == nullptr) return false;
    component->setState(object);
    auto componentJsonWeakRef = WebsiteComponent::getComponentJsonWeakRef();
    if (componentJsonWeakRef->isReadyToUse()) {
      componentJsonWeakRef->lock();
      auto& formatter = ComponentType::getVisuinoOutputFormatter();
      formatter.format(component->toVisuinoJson());
      componentJsonWeakRef->unlock();
    } else return false;
    return true;
  }

  WebsiteComponent* WebsiteTab::getComponentByName(const char* name) {
    for (auto component : this->components) {
      if (component->getName().equals(name)) return component;
    }
    return nullptr;
  }

  bool WebsiteTab::componentAlreadyExists(const char* componentName) {
    bool res = false;
    if (getComponentByName(componentName) != nullptr) res = true;
    return res;
  }

  bool WebsiteTab::onComponentInputHttpRequest(CommonJsonMemory* target, const uint8_t* data, size_t len) {
    deserializeJson(*target->get(), reinterpret_cast<const char*>(data), len);
    auto receivedJson = target->get()->as<JsonObject>();
    const char* componentType = receivedJson[JsonKey::ComponentType];
    using namespace ComponentType;
    if (!strncmp(componentType, Input::Switch, strlen(componentType))) {
      return parseInputComponentToVisuino<Switch>(receivedJson);
    } else if (!strncmp(componentType, Input::Slider, strlen(componentType))) {
      return parseInputComponentToVisuino<Slider>(receivedJson);
    } else if (!strncmp(componentType, Input::NumberInput, strlen(componentType))) {
      return parseInputComponentToVisuino<NumberInput>(receivedJson);
    } else if (!strncmp(componentType, Input::Button, strlen(componentType))) {
      return parseInputComponentToVisuino<Button>(receivedJson);
    }
    return false;
  }

}


  class ApplicationContext {
  public:
    static void parseTabs(const JsonObjectConst& tabsJson);
    static JsonObject onApplicationStateHttpRequest();
    static bool onComponentStatusHTTPRequest(const uint8_t* data, size_t len);
    static inline void garbageCollect() { for (auto& tab : tabs) delete tab; }

    static inline CommonJsonMemory* getStateJsonWeakRef() { return &stateJsonMemory; }
    static inline CommonJsonMemory* getComponentJsonWeakRef() { return &componentJsonMemory; }
    static inline CommonJsonMemory* getVisuinoOutputJsonWeakRef() { return &visuinoOutputJsonMemory; }
    static inline const String& getTtile() { return title; }
    static inline void setTitle(const char* newTitle) { title = newTitle; }

private:
    static Website::WebsiteTab* getTabByName(const char* tabName);
  private:
    static std::vector<Website::WebsiteTab*> tabs;
    static String title;
    static CommonJsonMemory stateJsonMemory;
    static CommonJsonMemory componentJsonMemory;
    static CommonJsonMemory visuinoOutputJsonMemory;

  };
  std::vector<Website::WebsiteTab*> ApplicationContext::tabs;
  String ApplicationContext::title;
  CommonJsonMemory ApplicationContext::stateJsonMemory;
  CommonJsonMemory ApplicationContext::componentJsonMemory;
  CommonJsonMemory ApplicationContext::visuinoOutputJsonMemory;

  void ApplicationContext::parseTabs(const JsonObjectConst& tabsJson) {
    tabs.reserve(tabsJson.size());
    for (JsonPairConst tabJson : tabsJson) {
      const char* tabName = tabJson.key().c_str();
      auto tab = getTabByName(tabName);
      if (tab == nullptr) {
        tabs.push_back(new Website::WebsiteTab(tabName,
                                                tabJson.value().as<JsonArrayConst>()));
      } else {
        tab->onUpdate(tabJson.value().as<JsonArrayConst>());
      }
    }
  }

  JsonObject ApplicationContext::onApplicationStateHttpRequest() {
    stateJsonMemory.get()->clear();
    for (auto& tab : tabs) {
      tab->onApplicationStateHttpRequest(&stateJsonMemory);
    }
    return stateJsonMemory.get()->as<JsonObject>();
  }

  bool ApplicationContext::onComponentStatusHTTPRequest(const uint8_t* data, size_t len) {
    for (auto& tab : tabs) {
      if (tab->onComponentInputHttpRequest(&visuinoOutputJsonMemory, data, len))
        return true;
    }
    return false;
  }

  Website::WebsiteTab* ApplicationContext::getTabByName(const char* tabName) {
    for (const auto& tab : tabs) {
      if (tab->getName().equals(tabName)) return tab;
    }
    return nullptr;
  }



  namespace JsonReader {

  size_t memorySize = 0;
  enum class InputJsonStatus : uint8_t {
    OK,
    JSON_OVERFLOW,
    INVALID_INPUT,
    ALLOC_ERROR,
    TITLE_NOT_FOUND,
    TABS_NOT_FOUND,
    TABS_ARRAY_EMPTY,
    OBJECT_NOT_VALID,
    NAME_NOT_FOUND,
    COMPONENT_TYPE_NOT_FOUND,
  };

  bool validateJson(const String& json){
    return !(json.isEmpty() || json.indexOf(JsonKey::Name) < 0);
  }

  size_t getBiggestObjectSize(const JsonObjectConst& arr){
    size_t biggest = 0;
    for (const JsonPairConst item: arr) {
      if(item.value().memoryUsage() > biggest) biggest = item.value().memoryUsage();
    }
    return biggest;
  }
  
  size_t getBufferSize(size_t memoryUsage) {
    size_t newSize = memoryUsage * 2;
    return newSize;
  }

  // ESP32 needs second JSON document because of multicore architecture and it could not be common with input memory
  bool allocateVisuinoOutputMemory(size_t size) {
    auto visuinoOutputJsonWeakRef = ApplicationContext::getVisuinoOutputJsonWeakRef();
#ifdef ESP32
    if(visuinoOutputJsonWeakRef->allocate(size)) {
      return true;
    } else{
      visuinoOutputJsonWeakRef->garbageCollect();
      return false;
    }
#endif
#ifdef ESP8266
    // ESP8266 is not multithreaded so visuino output can use component output memory
    // It saves few bytes. Anyway, this tool is recommended to work with ESP32 to reach better reliability
    visuinoOutputJsonWeakRef = ApplicationContext::getComponentJsonWeakRef();
    Website::WebsiteComponent::setVisuinoOutputJsonWeakRef(visuinoOutputJsonWeakRef);
    return true;
#endif
  }


  InputJsonStatus readWebsiteComponentsFromJson(const String& json) {
    using namespace Website;
    static bool isValid = false;
    static bool isInitialized = false;
    static bool isElementsInitialized = false;

    isValid = validateJson(json);
    auto stateJsonWeakRef = ApplicationContext::getStateJsonWeakRef();
    if(stateJsonWeakRef == nullptr) return InputJsonStatus::ALLOC_ERROR;
    if (isValid) {
      if (!isInitialized) {
        memorySize = json.length();
        if (stateJsonWeakRef->allocate(getBufferSize(memorySize))) {
          isInitialized = true;
        } else {
          stateJsonWeakRef->garbageCollect();
          return InputJsonStatus::ALLOC_ERROR;
        }
      }
      if (stateJsonWeakRef->isReadyToUse()) {
        // unlock json memory before returning
        auto releaseAndReturn = [&] (InputJsonStatus status) {
          stateJsonWeakRef->unlock();
          return status;
        };
        stateJsonWeakRef->lock();
        deserializeJson(*(stateJsonWeakRef->get()), json);
        if (stateJsonWeakRef->get()->overflowed()) return releaseAndReturn(InputJsonStatus::JSON_OVERFLOW);
        JsonObject inputObject = stateJsonWeakRef->get()->as<JsonObject>();
        if (!isElementsInitialized) {
          size_t biggestObjectSize = getBiggestObjectSize(inputObject);
          if (!allocateVisuinoOutputMemory(getBufferSize(biggestObjectSize))) return releaseAndReturn(InputJsonStatus::ALLOC_ERROR);
          auto componentJsonWeakRef = ApplicationContext::getComponentJsonWeakRef();
          if (componentJsonWeakRef->allocate(getBufferSize(biggestObjectSize))) {
            WebsiteComponent::setComponentJsonWeakRef(componentJsonWeakRef);
            isElementsInitialized = true;
          } else {
            componentJsonWeakRef->garbageCollect();
            return releaseAndReturn(InputJsonStatus::ALLOC_ERROR);
          }
        }
        ApplicationContext::parseTabs(inputObject);
        stateJsonWeakRef->unlock();
      }

    }
    return InputJsonStatus::OK;
  }


  const char* errorHandler(InputJsonStatus status){
    using namespace ErrorMessage::JsonInput;
    const char* statusStr = nullptr;
    switch (status) {
      case InputJsonStatus::TITLE_NOT_FOUND:
        statusStr = TitleNotFound;
        break;
      case InputJsonStatus::TABS_NOT_FOUND:
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
      case InputJsonStatus::TABS_ARRAY_EMPTY:
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
    static auto& switchFormatter = Switch::getVisuinoOutputFormatter();
    static auto& sliderFormatter = Slider::getVisuinoOutputFormatter();
    static auto& numberInputFormatter = NumberInput::getVisuinoOutputFormatter();
    static auto& buttonFormatter = Button::getVisuinoOutputFormatter();

    if(Log::isDataReady){
      //LogOutput.Send(Log::errorStream.c_str());
      Serial.println(Log::errorStream.c_str());
      Log::errorStream.clear();
      Log::isDataReady = false;
    }
    if(switchFormatter.isDataReady){
      Serial.println(switchFormatter.getData());
      //ServerSwitchOutput.Send(Switch::toString());
      switchFormatter.isDataReady = false;
    }
    if(sliderFormatter.isDataReady){
      Serial.println(sliderFormatter.getData());
      //ServerSwitchOutput.Send(Switch::toString());
      sliderFormatter.isDataReady = false;
    }
    if(numberInputFormatter.isDataReady){
      Serial.println(numberInputFormatter.getData());
      //ServerSwitchOutput.Send(Switch::toString());
      numberInputFormatter.isDataReady = false;
    }
    if(buttonFormatter.isDataReady){
      Serial.println(buttonFormatter.getData());
      //ServerSwitchOutput.Send(Switch::toString());
      buttonFormatter.isDataReady = false;
    }
  }
}



void fullCorsAllow(AsyncWebServerResponse* response) {
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_METHODS, CORS_ALLOWED_METHODS);
  response->addHeader(CORS_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, CORS_ALLOWED_HEADERS);
}

void HTTPServeWebsite(AsyncWebServer& webServer) {

  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.html");
  });

  webServer.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });

  webServer.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.js", "application/javascript");
  });

  webServer.on("/tab.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/tab.js", "application/javascript");
  });

  webServer.on("/component.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.css", "text/css");
  });

  webServer.on("/component.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/component.js", "application/javascript");
#ifdef DEBUG_BUILD
    Log::info("Component", Serial);
    Log::memoryInfo(Serial);
#endif
  });

  webServer.on("/Libs/pureknobMin.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Libs/pureknobMin.js", "application/javascript");
#ifdef DEBUG_BUILD
    Log::info("Knob", Serial);
    Log::memoryInfo(Serial);
#endif
  });

  webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/favicon.ico", "image/ico");
  });
}

void HTTPSetMappings(AsyncWebServer& webServer) {

  webServer.on("/init", HTTP_GET, [] (AsyncWebServerRequest* request){
#ifdef DEBUG_BUILD
  request->send(HttpCodes::OK, "text/plain", "Debug Build");
#else
  const String& title = ApplicationContext::getTtile();
  if (!title.isEmpty() || title.equals("")){
    request->send(HttpCodes::OK, "text/plain", title);
  } else request->send(HttpCodes::NO_CONTENT);
#endif
  });

  webServer.on("/state", HTTP_GET, [] (AsyncWebServerRequest* request){
    using namespace Website;
    static String responseBody;   // static to avoid heap allocation in every request - beginResponse takes const reference
    auto stateJsonWeakRef = ApplicationContext::getStateJsonWeakRef();
#ifdef DEBUG_BUILD
    Log::info("Proccessing info request");
#endif
    if (stateJsonWeakRef->isReadyToUse()) {
#ifdef DEBUG_BUILD
      Log::info("mem ok, request resolved");
#endif
      stateJsonWeakRef->lock();
      auto data = ApplicationContext::onApplicationStateHttpRequest();
      responseBody.clear();
      serializeJson(data, responseBody);
      AsyncWebServerResponse* response = request->beginResponse(HttpCodes::OK, "application/json", responseBody);
      fullCorsAllow(response);
      request->send(response);
      stateJsonWeakRef->unlock();
    } else {
#ifdef DEBUG_BUILD
      Log::info("mem locked, no content");
#endif
      request->send(HttpCodes::NO_CONTENT);
    }
  });

  webServer.on("/input", HTTP_POST, [] (AsyncWebServerRequest* request){}, nullptr,
          [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    using namespace Website;
    auto visuinoOutputJsonWeakRef = ApplicationContext::getVisuinoOutputJsonWeakRef();
    if (visuinoOutputJsonWeakRef->isReadyToUse()) {
      visuinoOutputJsonWeakRef->lock();
      if (ApplicationContext::onComponentStatusHTTPRequest(data, len)) {
        request->send(HttpCodes::OK);
      } else {
        Log::error("Error while parsing input component");
        request->send(HttpCodes::BAD_REQUEST);
      }
      visuinoOutputJsonWeakRef->unlock();
    } else {
        request->send(HttpCodes::NO_CONTENT);
    }
  });

  webServer.on("/image", HTTP_GET, [] (AsyncWebServerRequest* request) {
    Log::info("image req", Serial);
    if (request->hasParam("fileName")) {
      Log::info("Has Param", Serial);
      const String& fileName = request->getParam("fileName")->value();
      Log::info(fileName.c_str());
      if (SPIFFS.exists(fileName)) {
        File f = SPIFFS.open(fileName, "r");
        if (f.size() > IMAGE_MAX_SIZE) {
          request->send(HttpCodes::PAYLOAD_TOO_LARGE);  
          Log::error("Requested file is too large or system has not enough memory", Serial);
          f.close();
        } else {
          f.close();
          AsyncWebServerResponse* response = request->beginResponse(SPIFFS, fileName, "image/jpg");
          response->setCode(HttpCodes::OK);
          request->send(response);
        }
      } else {
        Log::error("Requested file not found!", Serial);
        request->send(HttpCodes::NOT_FOUND);
      }
      } else {
        Log::error("Client not provided proper param");
        request->send(HttpCodes::BAD_REQUEST);
      }
  });
}

#if DEBUG_BUILD && ESP32
void registerWiFIDebugEvents() {
  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("AP connected!");
  }, SYSTEM_EVENT_AP_STACONNECTED);

  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("AP_Disconnected!");
  }, SYSTEM_EVENT_AP_STADISCONNECTED);

  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("STA_Connected");
  }, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent([] (WiFiEvent_t event, WiFiEventInfo_t info) {
    Log::info("STA_Disconnected");
  }, SYSTEM_EVENT_STA_DISCONNECTED);
}
#endif


void ServerInit(){
#if DEBUG_BUILD && ESP32
  registerWiFIDebugEvents();
#endif

  HTTPServeWebsite(server);
  HTTPSetMappings(server);
  server.begin();
  Log::errorStream.reserve(100);
}
}

void setup(){
  Serial.begin(115200);
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  if(!SPIFFS.exists("/index.html")) {
    Serial.println("index.html not found");
  }

  WiFi.softAP("esp_ap", "123456789");
  WiFi.begin("NET-MAR_619", "bielaki123424G");
  while (WiFi.status() != WL_CONNECTED) delay(100);
  Serial.println(WiFi.localIP());
  WebsiteServer::ServerInit();
  uint32_t before = millis();
  using namespace WebsiteServer;
  using namespace WebsiteServer::JsonReader;
  InputJsonStatus status = readWebsiteComponentsFromJson(testWebsiteConfigStr);
 // Log::info(errorHandler(status));
  uint32_t after = millis();
  Serial.print("Execution time: ");
  Serial.println(after - before);
}



void loop(){
  uint32_t actual;
  static uint32_t previous = 0;
  uint32_t interval = 500;
  actual = millis();
  if (actual - previous > interval) {
    uint32_t before = millis();
    using namespace WebsiteServer;
    using namespace WebsiteServer::JsonReader;
    InputJsonStatus status = readWebsiteComponentsFromJson(testWebsiteConfigStr);
    uint32_t after = millis();
    previous = actual;
  }

  WebsiteServer::JsonWriter::write();
}

