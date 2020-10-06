//Version 24.9
#include <ESP8266WiFi.h>
#include <espnow.h>

#define CHANNEL 1
#define CONTROL_PIN 0
#define DEVICE_ID 2
#define DEVICE_NAME "PHONG_NGU"
#define DEVICE_TYPE "COLOR"
#define TIMER_DELAY 700

uint8_t MASTER_ADDRESS[] = {0x10, 0x52, 0x1C, 0x01, 0xA0, 0xEE};
unsigned long lastTime = 0;
String command_master[] = {"ON", "WHITE", "ORANGE", "YELLOW", "OFF", "GET_STATUS"};
String state_array[] = {"WHITE", "WHITE-OFF", "ORANGE", "ORANGE-OFF", "YELLOW", "YELLOW-OFF",
                        "WHITE", "WHITE-OFF", "ORANGE", "ORANGE-OFF", "YELLOW", "YELLOW-OFF"
                       };
String DEVICE_STATUS = "YELLOW-OFF";
int sent_status;

//define variable receive from master
String command_from_master = "OFF";

// Structure to sent data, must match the slave receive info structure
typedef struct struct_command_message 
{
  String device_name;
  String device_command;
} struct_command_message;
// Create a struct_message called "master_command"
struct_command_message master_command;

// Structure to receive data, must match the slave sent info structure
typedef struct struct_status_message
{ 
  int    device_id;
  String device_name;
  String device_type;
  String device_status;
} struct_status_message;
// Create device_name struct_message called "slave_status"
struct_status_message slave_status;

//********************************************************************************//
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
    if (DEVICE_STATUS == "WHITE") {
      return "WHITE-OFF";
    }
    else if (DEVICE_STATUS == "ORANGE") {
      return "ORANGE-OFF";
    }
    else if (DEVICE_STATUS == "YELLOW") {
      return "YELLOW-OFF";
    }
  }
  else if (DEVICE_STATUS == "ON"){
    if (DEVICE_STATUS == "WHITE-OFF"){
      return "ORANGE-ON";
      }
    if (DEVICE_STATUS == "ORANGE-OFF"){
      return "YELLOW-ON";
      }
    if (DEVICE_STATUS == "YELLOW-OFF"){
      return "WHITE-ON";
      }
    }
  else {
    return serial_received;
  }
}

//find num step to gaint option state
uint8_t find_num_step(String current_state, String option_state) {
  std::vector<std::uint8_t> current_state_index_list = {};
  std::vector<std::uint8_t> option_state_index_list = {};
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
  return index_option_state - index_current_state;
}

//control lamp to option state
void control_color_lamp(String current_state, uint8_t num_control) {
  if (current_state == "WHITE" || current_state == "ORANGE" || current_state == "YELLOW") {
    bool control_level = LOW;
    int i = 0;
    while (i < num_control) {
      if ((millis() - lastTime) > TIMER_DELAY) {
        digitalWrite(CONTROL_PIN, control_level);
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
      if ((millis() - lastTime) > TIMER_DELAY) {
        digitalWrite(CONTROL_PIN, control_level);
        Serial.print("control_level: ");
        Serial.println(control_level);
        control_level = !control_level;
        i++;
        lastTime = millis();
      }
    }
  }
}

//control lamp to option state Fuction 2
void control_color_lamp_1(String current_state, uint8_t num_control) {
  if (current_state == "WHITE" || current_state == "ORANGE" || current_state == "YELLOW") {
    bool control_level = LOW;
    for (int i= 0; i < num_control; i++) {
        digitalWrite(CONTROL_PIN, control_level);
        Serial.print("control_level: ");
        Serial.println(control_level);
        control_level = !control_level;
        delay(1000);
      }
  }
  else {
    bool control_level = HIGH;
    for (int i = 0; i < num_control; i++) {
        digitalWrite(CONTROL_PIN, control_level);
        Serial.print("control_level: ");
        Serial.println(control_level);
        control_level = !control_level;
        delay(1000);
      }
    }
}


// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&master_command, incomingData, sizeof(master_command));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Device: ");
  Serial.println(master_command.device_name);
  Serial.print("Command: ");
  Serial.println(master_command.device_command);
  Serial.println();
  command_from_master = master_command.device_command;
  if (in_array(command_from_master, command_master)){
    if (command_from_master == "GET_STATUS"){
      send_status(MASTER_ADDRESS, DEVICE_ID, DEVICE_NAME, DEVICE_TYPE, DEVICE_STATUS);
      command_from_master = "";
      }
    else {
      String option_state = predict_state(command_from_master);
      if (in_array(option_state, state_array)) {
        Serial.print("Predict option state: ");
        Serial.println(option_state);
        int num_step = find_num_step(DEVICE_STATUS, option_state);
        Serial.print("CONTROL STEP: ");
        Serial.println(num_step);
        control_color_lamp(DEVICE_STATUS, num_step);
        DEVICE_STATUS = option_state;
        Serial.print("CONTROL LAMP DONE, CURRENT STATE: ");
        Serial.println(DEVICE_STATUS);
        command_from_master = "";
      }
    }
  }
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  sent_status = sendStatus;
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery fail");
  }
}


//send status to master
void send_status(uint8_t masterAddress[], int device_id, String device_name, String device_type, String device_status)
{
  slave_status.device_id = device_id;
  slave_status.device_name = device_name;
  slave_status.device_type = device_type;
  slave_status.device_status = device_status;
  for (int i = 0; i < 50; i++)
  {
    esp_now_send(masterAddress, (uint8_t *)&slave_status, sizeof(slave_status));
    delay(TIMER_DELAY);
    if (sent_status == 0)
    {
      sent_status == 1;
      break;
    }
  }
}


void setup() {
  //set pin control
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(MASTER_ADDRESS, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
}
