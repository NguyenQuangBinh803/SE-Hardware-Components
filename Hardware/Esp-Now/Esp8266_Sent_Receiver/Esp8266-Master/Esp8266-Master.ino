//Version 24.9
#include <ESP8266WiFi.h>
#include <espnow.h>

#define GET_ALL_DEVICE_DESCRIPTION 1
#define CHANNEL 3
#define FETCH_SINGLE_DEVICE_STATUS 4
#define FETCH_ALL_DEVICE_STATUS 5
#define SCAN_ALL_DEVICE 6
#define BOARDCAST_SINGLE_DEVICE 8
#define BROADCAST_ALL_DEVICE 9

#define DEBUG_MODE 1

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress_1[] = {0x48, 0x3F, 0xDA, 0x49, 0x48, 0x2A};
uint8_t broadcastAddress_2[] = {0x48, 0x3F, 0xDA, 0x48, 0x7C, 0x4A};
//{0x10, 0x52, 0x1C, 0x01, 0xA0, 0xEE}
//48:3F:DA:49:C1:7B
uint8_t broadcastAddresses[2][6] = {{0x48, 0x3F, 0xDA, 0x49, 0xC1, 0x7B}, {0x48, 0x3F, 0xDA, 0x48, 0x7C, 0x4A}}; //
uint8_t EspAddress[][6] = {};

//int status_received;
const int NUM_DEVICES = 2;
const int NUMBER_OF_STATE = 5;
String device_state[NUMBER_OF_STATE];

unsigned long lastTime = 0;
unsigned long timerDelay = 10;
int slaveCount = 0;

enum STATE {
  ON,
  OFF,
  WHITE,
  YELLOW,
  ORANGE
};

int sent_status;
String serial_in_data;
String list_of_device_name[NUM_DEVICES] = {"PHONG_KHACH", "PHONG_NGU"};//, "TRANG_TRI"
int device_status[NUM_DEVICES] = {0, 0};
// String device_status_definition[NUMBER_OF_STATE] == {"ON","OFF","WHITE","YELLOW","ORANGE"}

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
// Create a struct_message called "slave_status"
struct_status_message slave_status;

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

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&slave_status, incomingData, sizeof(slave_status));
  Serial.print("success,");
  Serial.print(slave_status.device_id); Serial.print(",");
  Serial.print(slave_status.device_name); Serial.print(",");
  Serial.print(slave_status.device_type); Serial.print(",");
  Serial.print(slave_status.device_status); Serial.print(";");

}

//***********************************************************************************//
void define_device_state()
{
  device_state[ON] = "ON";
  device_state[OFF] = "OFF";
  device_state[WHITE] = "WHITE";
  device_state[YELLOW] = "YELLOW";
  device_state[ORANGE] = "ORANGE";
}
//void response_device_status(String device_name, int device_status)
//{
//  Serial.print(device_name);
//  Serial.print(",");
//  Serial.print(device_state[device_status]);
//  Serial.print(";");
//}

//send command to slave after identify
void send_command(uint8_t broadcastAddress[], String device_name, String device_command)
{
  master_command.device_name = device_name;
  master_command.device_command = device_command;
  Serial.println(sizeof(master_command));
//  esp_now_send(broadcastAddress, (uint8_t *)&master_command, sizeof(master_command));
  
  for (int i = 0; i < 10; i++)
  {
    esp_now_send(broadcastAddress, (uint8_t *)&master_command, sizeof(master_command));
    delay(timerDelay);
    //    Serial.println("Loop send status received: " + String(sent_status));
    if (sent_status == 0)
    {
      sent_status == 1;
      break;
    }
  }
}

// Indetify command after receive from serial
void command_identification(String command)
{
  const char *delimiter = ",";
  char *token;
  char *kind_of_command;
  char command_convert_to_char[command.length()];

  //  Command attribute
  int command_kind = 0;
  String command_content;
  String device_name;
  String device_command;

  int command_format_order = 0;

  command.toCharArray(command_convert_to_char, command.length());
  token = strtok(command_convert_to_char, delimiter);
  //  Serial.println(command);
  while (token != NULL)
  {
    if (command_format_order == 0)
    {
      command_kind = String(token).toInt();
    }
    else if (command_format_order == 1)
    {
      command_content = String(token);
      device_name = String(token);
    }
    else if (command_format_order == 2)
    {
      device_command = String(token);
    }
    token = strtok(NULL, delimiter);
    command_format_order++;
    //    Serial.println(command_format_order);
  }

  switch (command_kind)
  {
    //"4,<device_name>_;"
    case FETCH_SINGLE_DEVICE_STATUS:
      Serial.println("Fetch single device status");
      for (int i = 0; i < NUM_DEVICES; i++)
      {
        if (command_content == list_of_device_name[i])
        {
          send_command(broadcastAddresses[i], list_of_device_name[i], "GET_STATUS");
          break;
        }
      }
      break;

    //"5_;"
    case FETCH_ALL_DEVICE_STATUS:
      Serial.println("Fetch all device status");
      for (int i = 0; i < NUM_DEVICES; i++)
      {
        send_command(broadcastAddresses[i], list_of_device_name[i], "GET_STATUS");
      }
      break;

    //"9,<device_command>_;"
    case BROADCAST_ALL_DEVICE:
      Serial.println("Boardcast all devices");
      for (int i = 0; i < NUM_DEVICES; i++)
      {

        if (command_content == "ON")
        {
          send_command(broadcastAddresses[i], list_of_device_name[i], "ON");
        }
        else if (command_content == "OFF")
        {
          send_command(broadcastAddresses[i], list_of_device_name[i], "OFF");
        }
      }
      break;

    //"8,<device_name>,<device_command>_;"
    case BOARDCAST_SINGLE_DEVICE:
      Serial.println("Boardcast single device");
      for (int i = 0; i < NUM_DEVICES; i++)
      {
        if (device_name == list_of_device_name[i])
        {
          send_command(broadcastAddresses[i], device_name, device_command);
          break;
        }
      }
      break;

    //"6_;"
    case SCAN_ALL_DEVICE:
      Serial.println("Scan all devices");
      Serial.println(command_content);
      break;
    default:
      break;
  }
}

//***************************************************************************************************//
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  define_device_state();

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_add_peer(broadcastAddresses[0], ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
//  esp_now_add_peer(broadcastAddress_2, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop()
{
  if ((millis() - lastTime) > 100)
  {
    Serial.println("Send to client");
    send_command(broadcastAddresses[0], "Somethigns", "COMMAND");
    lastTime = millis();
  }
}
