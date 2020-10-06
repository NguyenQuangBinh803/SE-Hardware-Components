//Version 24.9
#include <ESP8266WiFi.h>
#include <espnow.h>
#include<SoftwareSerial.h>

#define CHANNEL 1
#define CONTROL_PIN 0
#define DEVICE_ID 1
#define DEVICE_NAME "TRANG_TRI"
#define DEVICE_TYPE "MULTI_COLOR"
#define TIMER_DELAY 700

SoftwareSerial Arduino_Serial(3, 1);

uint8_t MASTER_ADDRESS[] = {0x10, 0x52, 0x1C, 0x01, 0xA0, 0xEE};
unsigned long lastTime = 0;
String DEVICE_STATUS = "000000";
int sent_status;

//define variable receive from master
String command_from_master = "000000";

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
  if (master_command.device_name == DEVICE_NAME && command_from_master.length() > 5){
    if (command_from_master == "GET_STATUS"){
      send_status(MASTER_ADDRESS, DEVICE_ID, DEVICE_NAME, DEVICE_TYPE, DEVICE_STATUS);
      DEVICE_STATUS = command_from_master;
      command_from_master = "";
      }
    else if (command_from_master == "OFF"){
      Arduino_Serial.print("0,OFF");
      DEVICE_STATUS = command_from_master;
      command_from_master = "";
      }
    else {
      Arduino_Serial.print("1,");Arduino_Serial.println(command_from_master);
      Serial.println("Sent command color to Arduino");
      DEVICE_STATUS = command_from_master;
      command_from_master = "";
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
  Arduino_Serial.begin(9600);
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
