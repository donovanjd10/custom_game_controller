
/** 
 * Note, this is one way to send controller state data to your server. You can tweak the transmit/receiver sides
 * as desired. Here, we send a packet of a string containing real-time controller state information.
 */



#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_NET_SSID";
const char* password = "YOUR_NET_PW";

typedef struct __attribute__((packed)){
  int8_t x;
  int8_t y;
  uint8_t button;
} ControllerState;

struct ControllerPacket{
  uint16_t signature;
  int8_t xAxis;
  int8_t yAxis;
  uint8_t buttonState;
  uint32_t timestamp;
};


IPAddress remoteIp(255,255,255,255); // YOUR SERVER IP HERE
const unsigned int remotePort = 5000;

WiFiUDP Udp;

const int voltsInputXin = 34;
const int voltsInputYpin = 32;
const int swPin = 35;

const int minIntMap = -1;
const int maxIntMap = 1;

const int OFF = 0;
const int ON = 1;

int fps_to_fpms(int frames_per_sec) {
  const float fpms = frames_per_sec * (1.0 / 1000.0);
  return ceil(fpms);
};

const int HOST_FPS = 60;
int delayMsForSync = fps_to_fpms(HOST_FPS);

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

  // YOUR VALUES HERE MAY VARY DEPENDING ON TOLERANCES -- TWEAK AS DESIRED
  // these values were found from testing with my hardware TODO: add auto calibration to auto determine these values
  float clean_x_value(float y_value){
    if(y_value > 0.22){
      return 1;
    }
    else if(y_value < 0.08){
      return -1;
    }
    else{
      return 0;
    }
  };

float clean_y_value(float x_vlaue){
  if(x_vlaue > - 0.08){
    return 1;
  }
  else if(x_vlaue < -0.20){
    return -1;
  }
  else{
    return 0;
  }
};


void setup() {
  // connect to wifi, set up pins
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
      }
      Serial.println("Connected to WiFi!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

}
  void loop() {
      
      static unsigned long lastSendTime = 0;
      const unsigned long interval = 16; 

      if(millis() - lastSendTime >= interval){
      
        int rawX = analogRead(voltsInputXin);
        int rawY = analogRead(voltsInputYpin);
        
        bool buttonPressed= (digitalRead(swPin) == LOW);

        ControllerPacket packet;

        packet.xAxis = clean_x_value(fmap(rawX, 0, 4095, maxIntMap, minIntMap));
        packet.yAxis = clean_y_value(fmap(rawY, 0, 4095, minIntMap, maxIntMap));
        packet.buttonState = digitalRead(swPin);
        packet.buttonState = buttonPressed ? 1 : 0;
        packet.timestamp = millis();
        packet.signature = 0xAB1D; // identifier - TODO: look for this on server/game side

        Udp.beginPacket(remoteIp, remotePort);
        Udp.write((uint8_t*)&packet, sizeof(packet));
        Udp.endPacket();
      }
  }

