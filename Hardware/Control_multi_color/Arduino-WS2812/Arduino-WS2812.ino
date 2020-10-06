// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h>
#endif

#define OFF       0
#define ON        1
#define PIN       6 
#define NUMPIXELS 300 // Popular NeoPixel ring size
#define DELAY_TIME 500

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Indetify command after receive from serial
//"0,__;" ==> OFF
//"1,<hex-color>_;" ==> control-color
void command_identification(String command)
{
  String command_kind = getValue(command,';',0);
  String command_content = getValue(command,',',1);
  
  //"1,<hex_value>;"
  if (command_kind.toInt() == ON) {
    Serial.println("Control led RGB");
    int R, G, B;
    char *hex = command_content.c_str();
    sscanf(hex, "%02x%02x%02x", &R, &G, &B);
    Serial.print(R);Serial.print("--");Serial.print(G);Serial.print("--");Serial.println(B);
    control_led(R, G, B);
  }
  
  //"0,_;"
  else if (command_kind.toInt() == OFF){
    Serial.println("OFF led");
    control_led(0, 0, 0);
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void control_led(int R, int G, int B){
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(R, G, B));
      pixels.show();   // Send the updated pixel colors to the hardware.
//      delay(DELAYVAL); // Pause before next pass through loop
    }
  }


void setup() {
  Serial.begin(115200);
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  control_led(0, 0, 0);
}

void loop() {
  if (Serial.available())
    {
      String serial_in_data = Serial.readStringUntil('\n');
      command_identification(serial_in_data);
    }
}
