#include <ESP8266WiFi.h>
#include <espnow.h>

#define FETCH_ALL_DEVICE_STATUS 5
#define FETCH_SINGLE_DEVICE_STATUS 4
#define GET_ALL_DEVICE_DESCRIPTION 1
#define BROADCAST_COMMAND 9
#define BOARDCAST_SINGLE_DEVICE 8

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x48, 0x3F, 0xDA, 0x49, 0x48, 0x2A};
uint8_t broadcastAddress_1[] = {0x48, 0x3F, 0xDA, 0x48, 0x7C, 0x4A};

String inData;
int status_received;

typedef struct struct_message {
  char a[32];
  String b;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 500;
unsigned long wait = 500;

void send_command(uint8_t broadcastAddress[], const char* device, String command)
{ 
  strcpy(myData.a, device);
  myData.b = command;
  for(int i=0; i<50; i++){
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      delay(wait);
      Serial.println("Loop send status received: "+ String(status_received));
      if (status_received == 0){
        status_received = 1;
        break;
        }
      }
  }

void command_identification(String command)
{
  const char* delimiter = ",";
  char *token;
  char* kind_of_command;
  char command_convert_to_char[command.length()];

  //  Command attribute
  int command_kind = 0;
  String command_content;
  int command_format_order = 0;


  command.toCharArray(command_convert_to_char, command.length());
  token = strtok(command_convert_to_char, delimiter);


  while ( token != NULL ) {
    if (command_format_order == 0) {
      command_kind = String(token).toInt();
    }
    else if (command_format_order == 1) {
      command_content = String(token);
    }
    token = strtok(NULL, delimiter);
    command_format_order ++;
    Serial.println(command_format_order);
  }

  switch (command_kind) {

    case FETCH_ALL_DEVICE_STATUS:
      Serial.print("device_name: ABC, device_status: ON; device_name: ABC, device_status: ON; device_name: ABC, device_status: ON; device_name: ABC, device_status: ON;");
      break;

    case BROADCAST_COMMAND:

      Serial.println("Boardcast");
      if (command_content == "ON") {
        Serial.println(command_content);
        send_command(broadcastAddress, "LED 1", "ON");
        send_command(broadcastAddress_1, "LED 2", "ON");
        
      }
      else if (command_content == "OFF") {
        Serial.println(command_content);
        send_command(broadcastAddress, "LED 1", "OFF");
        send_command(broadcastAddress_1, "LED 2", "OFF");
        
      }

    case BOARDCAST_SINGLE_DEVICE:
      if (command_content == "LED_1_ON") {
        Serial.println(command_content);
        send_command(broadcastAddress, "LED 1", "ON");
      }

      else if (command_content == "LED_2_ON") {
        Serial.println(command_content);
        send_command(broadcastAddress_1, "LED 2", "ON");
        
      }
      else if (command_content == "LED_1_OFF") {
        Serial.println(command_content);
        send_command(broadcastAddress, "LED 1", "OFF");
        
      }
      else if (command_content == "LED_2_OFF") {
        Serial.println(command_content);
        send_command(broadcastAddress_1, "LED 2", "OFF");
      }
      break;
    default:
      break;
  }

}
// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  status_received = sendStatus;
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_add_peer(broadcastAddress_1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  server.client().print()
  if ((millis() - lastTime) > timerDelay) {
    if (Serial.available())
    {
      inData = Serial.readStringUntil(';');
      command_identification(inData);
    }
    lastTime = millis();
  }
}
