#include <WukongVM.h>
Wukong wukong;
int g_port = 0;

void binary_sensor_update(wuobject_t *wuobject)
{
    static bool last_st;
    Serial.println("beat");
    char cmd[] = {0x20,1,0xff};
    wukong.send(255,cmd,3);
    bool b = digitalRead(2);
    if (b == last_st) return;
    last_st = b;
    if (b)
      Serial.println("released");
    else
      Serial.println("pressed");
}  


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wukong.begin();
  wukong.enableWuClass(wukong.BINARY_SENSOR,binary_sensor_update);
  g_port = wukong.addWuObject(wukong.BINARY_SENSOR);
  Serial.print("port is ");Serial.println(g_port);
  wukong.setPropertyRefreshRate(g_port, wukong.BINARY_SENSOR_REFRESH_RATE, 1000);
  pinMode(2,INPUT);
  digitalWrite(2, HIGH);
  Serial.println("init done");
}

void loop() {
  // put your main code here, to run repeatedly:
  wukong.loop();
}
