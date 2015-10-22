#include <IRremote.h>
#include <IRremoteInt.h>

#define PanasonicAddress      0x4004     // Panasonic address (Pre data) 
#define PanasonicPower        0x100BCBD  // Panasonic Power button
#define PanasonicVolumeDown   0x1008485
#define PanasonicVolumeUp     0x1000405
#define PanasonicMute         0x1004C4D

#define SPURIOUS 80
#define THESHOLD 20

#define TV_ON_SPAN_MS 5000
#define TV_OFF_SPAN_MS 1000
#define TV_REQUIRED_CHECK_COUNT 1
#define TV_VOLUME_PUSH_DELAY 10
#define TV_VOLUME_DOWN_PUSH 100
#define TV_VOLUME_UP_PUSH 50

#define TV_ON_INTERRUPT_STATE HIGH
#define TV_MUTE_INTERRUPT_STATE HIGH

IRsend irsend;

int tv_on_off_interrupt_pin = 3;
int tv_mute_interrupt_pin = 4;
int tv_on_off_output_pin = 5;

int tv_current_status_led_pin = A8;
int tv_on_off_interrupt_led_pin = A9;
int tv_check_count_led_pin = A10;
int tv_mute_interrupt_led_pin = A11;

bool last_tv_on_off_interrupt = false;
unsigned long last_tv_interrupt_time;
int check_tv_count = 0;
bool last_tv_mute_interrupt = false;


bool read_tv_current_status(){
   int v;
   int instant_max = 0;
   int count;
   for (count = 0; count < 20; count++){
     v = abs(analogRead(A13) - 512);
     if (v > instant_max && v < SPURIOUS)
     {
       instant_max = v;
     }
     delay(50);
   }
   if (instant_max > THESHOLD){
     return true;
   } else {
     return false;
   }
}

void setupIRLED()
{
  // Turn on IR LED
  DDRA |= (1<<4);
  PORTA |= (1<<4);
  delay(10);
  DDRJ |= (1<<4);
  PORTJ &= ~(1<<4);
  delay(10);
  PORTA &= ~(1<<4);
  
  DDRE |= (1<<2);
  PORTE &= ~(1<<2);
}


void checkIfTVOnOffTriggered(){
  if (digitalRead(tv_on_off_interrupt_pin) == TV_ON_INTERRUPT_STATE && !last_tv_on_off_interrupt)
  {
    last_tv_on_off_interrupt = true;
    last_tv_interrupt_time = millis();
    irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
    analogWrite(tv_on_off_interrupt_led_pin, 255);
    check_tv_count = TV_REQUIRED_CHECK_COUNT; // to remind system to examine whether TV is truly on
  }
  else if (digitalRead(tv_on_off_interrupt_pin) != TV_ON_INTERRUPT_STATE && last_tv_on_off_interrupt)
  {
    last_tv_on_off_interrupt = false;
    last_tv_interrupt_time = millis();
    irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
    analogWrite(tv_on_off_interrupt_led_pin, 0);
    check_tv_count = TV_REQUIRED_CHECK_COUNT; // to remind system to examine whether TV is truly off
  }
  if (check_tv_count) 
    digitalWrite(tv_check_count_led_pin, HIGH);
  
  bool tv_current_status = read_tv_current_status();
  digitalWrite(tv_on_off_output_pin, tv_current_status);
  if (tv_current_status)
  {
    analogWrite(tv_current_status_led_pin, 255);
  }
  else
  {
    analogWrite(tv_current_status_led_pin, 0);
  }
  
  // Examination of TV status once after the interrupt signal
  if(check_tv_count > 0)
  {
    
    if (last_tv_on_off_interrupt && (millis() - last_tv_interrupt_time > TV_ON_SPAN_MS))
    {
      if (!tv_current_status)
      {
        irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
        last_tv_interrupt_time = millis();
        check_tv_count--;
      }
      else
      {
        check_tv_count = 0;
        digitalWrite(tv_check_count_led_pin, LOW);
      }
    }
    else if (!last_tv_on_off_interrupt && (millis() - last_tv_interrupt_time > TV_OFF_SPAN_MS))
    {
      if (tv_current_status)
      {
        irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
        last_tv_interrupt_time = millis();
        check_tv_count--;
      }
      else
      {
        check_tv_count = 0;
        digitalWrite(tv_check_count_led_pin, LOW);
      }
    }
  }
}

void checkIfTVMuteTriggered(){
  if (digitalRead(tv_mute_interrupt_pin) == TV_MUTE_INTERRUPT_STATE && !last_tv_mute_interrupt)
  {
    last_tv_mute_interrupt = true;
  
    for (int i = 0; i < TV_VOLUME_DOWN_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeDown);
      delay(TV_VOLUME_PUSH_DELAY);
    }
    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);

    analogWrite(tv_mute_interrupt_led_pin, 255);
  }
  else if (digitalRead(tv_mute_interrupt_pin) != TV_MUTE_INTERRUPT_STATE && last_tv_mute_interrupt)
  {
    last_tv_mute_interrupt = false;

    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);
    for (int i = 0; i < TV_VOLUME_UP_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeUp);
      delay(TV_VOLUME_PUSH_DELAY);
    }

    analogWrite(tv_mute_interrupt_led_pin, 0);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupIRLED();
  pinMode(tv_on_off_interrupt_pin, INPUT);
  pinMode(tv_mute_interrupt_pin, INPUT);
  pinMode(tv_on_off_output_pin, OUTPUT);
  
  pinMode(tv_current_status_led_pin, OUTPUT);
  pinMode(tv_on_off_interrupt_led_pin, OUTPUT);
  pinMode(tv_mute_interrupt_led_pin, OUTPUT);
  pinMode(tv_check_count_led_pin, OUTPUT);
  Serial.println("init done");
}

void loop() {
  
  checkIfTVOnOffTriggered();  
  checkIfTVMuteTriggered();
}
