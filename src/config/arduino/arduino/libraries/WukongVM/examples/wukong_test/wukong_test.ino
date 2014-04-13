#include <WukongVM.h>

Wukong wukong;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wukong.begin();
  Serial.println("init done");
}

void loop() {
  // put your main code here, to run repeatedly:
  wukong.loop();
}
