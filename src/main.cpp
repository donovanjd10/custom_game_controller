
/** 
 * Note, this is one way to send controller state data to your server. You can tweak the transmit/receiver sides
 * as desired. Here, we send a packet of a string containing real-time controller state information.
 */



#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_NET_SSID";
const char* password = "YOUR_NET_PW";

IPAddress remoteIp(255,255,255,255); // YOUR SERVER IP HERE
const unsigned int remotePort = 5000;

WiFiUDP Udp;

const int volts_x_pin = 34;
const int volts_y_pin = 32;
const int SwPin = 35;

const int min_int_map = -1;
const int max_int_map = 1;

const int OFF = 0;
const int ON = 1;

int fps_to_fpms(int frames_per_sec) {
  const float fpms = frames_per_sec * (1 / 1000);
  return ceil(fpms);
}

const int HOST_FPS = 60;
int delay_ms_for_sync = fps_to_fpms(HOST_FPS);

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

  // YOUR VALUES HERE MAY VARY DEPENDING ON TOLERANCES -- TWEAK AS DESIRED
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
      
      int x_pos = analogRead(volts_x_pin);
      int y_pos = analogRead(volts_y_pin);
      float swState = digitalRead(SwPin);

      float mappedX = fmap(x_pos, 0, 4095, max_int_map, min_int_map);
      float mappedY = fmap(y_pos, 0, 4095, min_int_map, max_int_map);
      float mappedSw = fmap(swState, 0, 4095, -1, ON);

      float switched_x = clean_x_value(mappedX);
      float switched_y = clean_y_value(mappedY);

      Serial.println(String(mappedX) + ", " + String(mappedY) + ", " + String(swState));
      String message = String(switched_x) + ", " +  String(switched_y) + ", " + String(mappedSw);

      Serial.println(message);
      Udp.beginPacket(remoteIp, remotePort);
      Udp.print(message);
      Udp.endPacket();
      delay(17);
      
  }

