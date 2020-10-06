//Version 24.9
#include <ESP8266WiFi.h>
#include <espnow.h>

#define CHANNEL 1
const int control_pin = 0;
unsigned long timerDelay = 1000;
unsigned long lastTime = 0;

String state_array[] = {"WHITE", "WHITE-OFF", "ORANGE", "ORANGE-OFF", "YELLOW", "YELLOW-OFF",
                        "WHITE", "WHITE-OFF", "ORANGE", "ORANGE-OFF", "YELLOW", "YELLOW-OFF"
                       };
String state = "YELLOW-OFF";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  char a[32];
  String b;
} struct_message;

// Create a struct_message called myData
struct_message myData;

//Config AP
void configDeviceAP()
{
  String Prefix = "Slave:COLOR:";
  String Mac = WiFi.macAddress();
  String SSID = Prefix + Mac;
  String Password = "123456789";
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
  if (!result)
  {
    Serial.println("AP Config failed.");
  }
  else
  {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}

//check in array
bool in_array(String value, String *array)
{
  int size = (*array).length();
  for (int i = 0; i < size; i++)
  {
    if (value == array[i])
    {
      return true;
    }
  }
  return false;
}

String predict_state(String serial_received) {
  if (serial_received == "OFF") {
    if (state == "WHITE") {
      return "WHITE-OFF";
    }
    else if (state == "ORANGE") {
      return "ORANGE-OFF";
    }
    else if (state == "YELLOW") {
      return "YELLOW-OFF";
    }
  }
  else {
    return serial_received;
  }
}

//find num step to gaint option state
uint8_t find_num_step(String current_state, String option_state) {
  //    List<int> current_state_index_list =new List<int>();
  //    List<int> option_state_index_list =new List<int>();

  std::vector<std::uint8_t> current_state_index_list = {};
  std::vector<std::uint8_t> option_state_index_list = {};
  Serial.print("Max size and current size")
  Serial.println(current_state_index_list.max_size());
  Serial.println(current_state_index_list.size());
  
  //    uint8_t current_state_index_list[] = {};
  //    uint8_t option_state_index_list[] = {};
  int index_current_state;
  int index_option_state;

  for (int i = 0; i < sizeof(state_array); i++)
  {
    if (current_state == state_array[i])
    {
      current_state_index_list.push_back(i);
    }
    if (option_state == state_array[i])
    {
      option_state_index_list.push_back(i);
    }
  }
  index_current_state = current_state_index_list[0];
  index_option_state = option_state_index_list[0];
  if (index_option_state < index_current_state) {
    index_option_state = option_state_index_list[1];
  }

  // Need to remove element to ignore stack overflow
  option_state_index_list.pop_back()
  current_state_index_list.pop_back();
  return index_option_state - index_current_state;
}

//control lamp to option state
void control_color_lamp(String current_state, uint8_t num_control) {
  if (current_state == "WHITE" || current_state == "ORANGE" || current_state == "YELLOW") {
    bool control_level = LOW;
    int i = 0;
    while (i < num_control) {
      if ((millis() - lastTime) > timerDelay) {
        digitalWrite(control_pin, control_level);
        Serial.print("control_level: ");
        Serial.println(control_level);
        control_level = !control_level;
        i++;
        lastTime = millis();
      }
    }
  }
  else {
    bool control_level = HIGH;
    int i = 0;
    while (i < num_control) {
      if ((millis() - lastTime) > timerDelay) {
        digitalWrite(control_pin, control_level);
        Serial.print("control_level: ");
        Serial.println(control_level);
        control_level = !control_level;
        i++;
        lastTime = millis();
      }
    }
  }
}

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println("============================================");
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Device: ");
  Serial.println(myData.a);
  Serial.print("Message: ");
  Serial.println(myData.b);
  String option_state;
  option_state = predict_state(myData.b);
  if (in_array(option_state, state_array)) {
    Serial.print("Predict option state: ");
    Serial.println(option_state);
    int num_step = find_num_step(state, option_state);
    Serial.print("CONTROL STEP: ");
    Serial.println(num_step);
    control_color_lamp(state, num_step);
    state = option_state;
    Serial.print("CONTROL LAMP DONE, CURRENT STATE: ");
    Serial.println(state);
  }
}

void setup() {
  //set pin control
  pinMode(control_pin, OUTPUT);
  digitalWrite(control_pin, LOW);
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  configDeviceAP();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}
