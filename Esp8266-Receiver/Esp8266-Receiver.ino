//Version 24.9
#include <ESP8266WiFi.h>
#include <espnow.h>

#define CONTROL_PIN 0
#define DEVICE_ID 1
#define DEVICE_NAME "PHONG_KHACH"
#define DEVICE_TYPE "ONOFF"
#define TIMER_DELAY 10

uint8_t MASTER_ADDRESS[] = {0x10, 0x52, 0x1C, 0x01, 0xA0, 0xEE};
String DEVICE_STATUS = "OFF";
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

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&master_command, incomingData, sizeof(master_command));

  Serial.print("Bytes received: ");
  Serial.println(sizeof(master_command));
  Serial.print("Device: ");
  Serial.println(master_command.device_name);
  Serial.print("Command: ");
  Serial.println(master_command.device_command);
  Serial.println();
  command_from_master = master_command.device_command;
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

//send command to slave after identify
void send_status(uint8_t masterAddress[], int device_id, String device_name, String device_type, String device_status)
{
  slave_status.device_id = device_id;
  slave_status.device_name = device_name;
  slave_status.device_type = device_type;
  slave_status.device_status = device_status;
  Serial.println("It gét in to this function");
  for (int i = 0; i < 50; i++)
  {
    esp_now_send(masterAddress, (uint8_t *)&slave_status, sizeof(slave_status));
    delay(TIMER_DELAY);
    //    Serial.println("Loop send status received: " + String(sent_status));
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
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  //  esp_now_add_peer(MASTER_ADDRESS, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  //  if (command_from_master != DEVICE_STATUS && command_from_master != ""){
  //    if (command_from_master == "ON"){
  //      digitalWrite(CONTROL_PIN, HIGH);
  //      DEVICE_STATUS = "ON";
  //      command_from_master = "";
  //      Serial.print("ON LED DONE ...");
  //      }
  //    else if (command_from_master == "OFF"){
  //      digitalWrite(CONTROL_PIN, LOW);
  //      DEVICE_STATUS = "OFF";
  //      command_from_master = "";
  //      Serial.print("OFF LED DONE ...");
  //      }
  //    else if (command_from_master == "GET_STATUS"){
  //      send_status(MASTER_ADDRESS, DEVICE_ID, DEVICE_NAME, DEVICE_TYPE, DEVICE_STATUS);
  //      command_from_master = "";
  //      Serial.print("REPORT STATUS DONE ...");
  //      }
  //    }
}
