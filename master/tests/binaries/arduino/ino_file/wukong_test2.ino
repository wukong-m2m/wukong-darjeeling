#include <WukongVM.h>
Wukong wukong;
int g_port = 0;
int g_port1 = 0;
int g_port2 = 0;
bool sound_enable= false;
unsigned long sound_freq=0;

void binary_testsensor_update(wuobject_t *wuobject) {}  

void integer_testsensor_update(wuobject_t *wuobject) {}

void sound_update(wuobject_t *wuobject)
{
    Serial.print("sound become ");
    bool b;
    int freq;
    wukong.getPropertyBool(wuobject, wukong.SOUND_ON_OFF,&b);

    Serial.println(b);
    sound_enable = b;
    wukong.getPropertyShort(wuobject,wukong.SOUND_FREQ, &freq);

    sound_freq = freq;    
    Serial.print("freq=");Serial.println(sound_freq);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wukong.begin();
  Serial.println("done");
  int i=0;
  
  wukong.enableWuClass(wukong.BINARY_TESTSENSOR,binary_testsensor_update, true);
  g_port = wukong.addWuObject(wukong.BINARY_TESTSENSOR);
  wukong.enableWuClass(wukong.INTEGER_TESTSENSOR,integer_testsensor_update, true);
  g_port1 = wukong.addWuObject(wukong.INTEGER_TESTSENSOR);  
  wukong.enableWuClass(wukong.SOUND,sound_update, true);
  g_port2 = wukong.addWuObject(wukong.SOUND);
  
  pinMode(5,INPUT);
  digitalWrite(5, HIGH);
  Serial.println("init done");
  pinMode(6,OUTPUT);
  wukong.start();
  Serial.println("start");
}

extern "C" {
  void radio_zwave_learn(void);
  void radio_zwave_reset(void);
};

#define WAIT_PREFIX 1
#define WAIT_CMD 2

int state = WAIT_PREFIX;

void loop() {
  // put your main code here, to run repeatedly:
  wukong.loop();
  if (sound_enable) {
    digitalWrite(6, HIGH);
    delayMicroseconds(sound_freq);
    digitalWrite(6, LOW);
    delayMicroseconds(sound_freq);
  }
  waitZwaveMessage();
}

void waitZwaveMessage(){
//  Serial.println("l");
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
      } else if (c == 'r') {
        radio_zwave_reset();
      }
      state = WAIT_PREFIX;
    }
  }
}
