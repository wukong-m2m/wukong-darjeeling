#include <WukongVM.h>
Wukong wukong;
int g_port = 0;
int g_port1 = 0;
int g_port2 = 0;
bool sound_enable= false;
unsigned long sound_freq=0;
void sound_update(wuobject_t *wuobject)
{
    Serial.print("sound become ");
    bool b;
    int freq;
    wukong.getPropertyBool(wuobject, wukong.SOUND_ON_OFF,&b);
    //wukong.getPropertyBool(g_port1, wukong.SOUND_ON_OFF,&b);

    Serial.println(b);
    sound_enable = b;
    wukong.getPropertyShort(wuobject,wukong.SOUND_FREQ, &freq);
    //wukong.getPropertyShort(g_port1,wukong.SOUND_FREQ, &freq);

    sound_freq = freq;    
    Serial.print("freq=");Serial.println(sound_freq);
}

void binary_sensor_update(wuobject_t *wuobject)
{
    static bool last_st;
    Serial.println("beat");
    bool b = digitalRead(5);
    if (b == last_st) return;
    last_st = b;
    if (b) {
      Serial.println("released");
    } else {
      Serial.println("pressed");
    }
    wukong.setPropertyBool(wuobject, wukong.BINARY_SENSOR_CURRENT_VALUE, last_st);
    //wukong.setPropertyBool(g_port, wukong.BINARY_SENSOR_CURRENT_VALUE, last_st);
}  




void slider_update(wuobject_t *wuobject)
{
    short v = analogRead(A2)/50*50;
    Serial.println(v);
    wukong.setPropertyShort(wuobject, wukong.SLIDER_OUTPUT, v);
   // wukong.setPropertyShort(g_port2, wukong.SLIDER_OUTPUT, v);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wukong.begin();
  Serial.println("done");
  int i=0;
  
  wukong.enableWuClass(wukong.BINARY_SENSOR,binary_sensor_update, true);
  wukong.enableWuClass(wukong.SLIDER,slider_update, true);
  g_port = wukong.addWuObject(wukong.BINARY_SENSOR);
  g_port2 = wukong.addWuObject(wukong.SLIDER);
  wukong.setPropertyRefreshRate(g_port, wukong.BINARY_SENSOR_REFRESH_RATE, 1000);
  
  wukong.enableWuClass(wukong.SOUND,sound_update, true);
  g_port1 = wukong.addWuObject(wukong.SOUND);
  
  
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
