#include <WukongVM.h>
Wukong wukong;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wukong.begin();
  Serial.println("init done");
}

extern "C" {
  
  void radio_zwave_learn(void);
  void radio_zwave_reset(void);
};

#define WAIT_PREFIX 1
#define WAIT_CMD 2

int state = WAIT_PREFIX;

void loop() 
{
  // put your main code here, to run repeatedly:
  wukong.loop();
  Serial.println("l");
  if (Serial.available()) {
    byte c = Serial.read();
    if (state == WAIT_PREFIX) {
      if (c == '$') {
        state = WAIT_CMD;
      }
    } else if (state == WAIT_CMD) {
      if (c == 'l') {
        Serial.println("learn");
        radio_zwave_learn();
      } else if (c == 'z') {
        radio_zwave_reset();
      }
      state = WAIT_PREFIX;
    }
  }
}
