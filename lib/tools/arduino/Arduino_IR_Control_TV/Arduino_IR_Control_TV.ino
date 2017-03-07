#include <IRremote.h>
#include <IRremoteInt.h>
#include <stdlib.h>

#define PanasonicAddress      0x4004     // Panasonic address (Pre data) 
#define PanasonicPower        0x100BCBD  // Panasonic Power button
#define PanasonicVolumeDown   0x1008485
#define PanasonicVolumeUp     0x1000405
#define PanasonicMute         0x1004C4D

#define SPURIOUS 80
#define THESHOLD 20

#define TV_ON_SPAN_MS 5000
#define TV_OFF_SPAN_MS 1000
#define TV_REQUIRED_CHECK_COUNT 0
#define TV_VOLUME_PUSH_DELAY 80
#define TV_VOLUME_DOWN_PUSH 75
#define TV_VOLUME_UP_PUSH 50 + 1

#define TV_ON_INTERRUPT_STATE HIGH
#define TV_MUTE_INTERRUPT_STATE HIGH

#define LED_ON LOW
#define LED_OFF HIGH

IRsend irsend;

int tv_on_off_output_pin = 8;
int tv_on_off_interrupt_pin = 9;
int tv_mute_interrupt_pin = 10;

int tv_current_status_led_pin = A8;
int tv_on_off_interrupt_led_pin = A9;
int tv_check_count_led_pin = A10;
int tv_mute_interrupt_led_pin = A11;

#define PEAK_TO_PEAK_MEDIAN 12
#define WINDOW_LENGTH 24
#define EXP_AVG_ALPHA 0.03
#define EXP_AVG_THRESHOLD 6.5
#define SENSE_SPEED_MS 16

int tv_current_sensor_buf[WINDOW_LENGTH];
bool tv_current_status = false;
int ring_index = 0;
double accum_value = -1.0;
unsigned long last_tv_current_read_time = 0;

bool last_tv_on_off_interrupt = false;
unsigned long last_tv_interrupt_time;
int check_tv_count = 0;
bool last_tv_mute_interrupt = false;


void read_tv_current_status(){
//   int v;
//   int instant_max = 0;
//   int count;
//   for (count = 0; count < 20; count++){
//     v = abs(analogRead(A13) - 512);
//     if (v > instant_max && v < SPURIOUS)
//     {
//       instant_max = v;
//     }
//     delay(50);
//   }
//   if (instant_max > THESHOLD){
//     return true;
//   } else {
//     return false;
//   }
  // Algorithm 2
  unsigned long current_read_time = millis();
  if (current_read_time - last_tv_current_read_time < SENSE_SPEED_MS)
  {
    return;
  }
  else
  {
    last_tv_current_read_time = current_read_time;
  }   
  
  int delta_value, read_value;
  read_value = analogRead(A13);
  if (ring_index < WINDOW_LENGTH)
  {
    tv_current_sensor_buf[ring_index] = read_value;
    if (ring_index >= PEAK_TO_PEAK_MEDIAN)
    {
      delta_value = (read_value - tv_current_sensor_buf[ring_index-PEAK_TO_PEAK_MEDIAN])/PEAK_TO_PEAK_MEDIAN;
      delta_value = delta_value * delta_value;
      tv_current_sensor_buf[ring_index-PEAK_TO_PEAK_MEDIAN] = read_value;
      if (accum_value >= 0)
      {
        accum_value = EXP_AVG_ALPHA * delta_value + (1-EXP_AVG_ALPHA) * accum_value;
        if (accum_value > EXP_AVG_THRESHOLD && !tv_current_status)
        {
          tv_current_status = true; 
          Serial.print(F("Detected: TV is on\n"));
        }
        else if(accum_value <= EXP_AVG_THRESHOLD && tv_current_status)
        {
          tv_current_status = false;
          Serial.print(F("Detected: TV is off\n"));
        }
      }
      else
      {
        accum_value = delta_value;
      }
    }
  }
  ring_index++;
  if (ring_index == WINDOW_LENGTH)
  {
    ring_index = PEAK_TO_PEAK_MEDIAN;
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

  read_tv_current_status();
  digitalWrite(tv_on_off_output_pin, tv_current_status);
  if (tv_current_status)
  {
    digitalWrite(tv_current_status_led_pin, LED_ON);
//    Serial.print(F("Detected: TV is on\n"));
  }
  else
  {
    digitalWrite(tv_current_status_led_pin, LED_OFF);
//    Serial.print(F("Detected: TV is off\n"));
  }
  
  
  if (digitalRead(tv_on_off_interrupt_pin) == TV_ON_INTERRUPT_STATE && !last_tv_on_off_interrupt)
  {
    last_tv_on_off_interrupt = true;
    last_tv_interrupt_time = millis();
    if(!tv_current_status) irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
    digitalWrite(tv_on_off_interrupt_led_pin, LED_ON);
    Serial.print(F("Interrupt: Turn ON TV\n"));
    check_tv_count = TV_REQUIRED_CHECK_COUNT; // to remind system to examine whether TV is truly on
  }
  else if (digitalRead(tv_on_off_interrupt_pin) != TV_ON_INTERRUPT_STATE && last_tv_on_off_interrupt)
  {
    last_tv_on_off_interrupt = false;
    last_tv_interrupt_time = millis();
    if(tv_current_status) irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
    digitalWrite(tv_on_off_interrupt_led_pin, LED_OFF);
    Serial.print(F("Interrupt: Turn OFF TV\n"));
    check_tv_count = TV_REQUIRED_CHECK_COUNT; // to remind system to examine whether TV is truly off
  }
  if (check_tv_count) 
    digitalWrite(tv_check_count_led_pin, LED_ON);
  
  // Examination of TV status once after the interrupt signal
  if(check_tv_count > 0)
  {
    
    if (last_tv_on_off_interrupt && (millis() - last_tv_interrupt_time > TV_ON_SPAN_MS))
    {
      if (!tv_current_status)
      {
        irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
        Serial.print(F("Retry: Turn ON TV\n"));
        last_tv_interrupt_time = millis();
        check_tv_count--;
      }
      else
      {
        check_tv_count = 0;
        digitalWrite(tv_check_count_led_pin, LED_ON);
      }
    }
    else if (!last_tv_on_off_interrupt && (millis() - last_tv_interrupt_time > TV_OFF_SPAN_MS))
    {
      if (tv_current_status)
      {
        irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
        Serial.print(F("Retry: Turn OFF TV\n"));
        last_tv_interrupt_time = millis();
        check_tv_count--;
      }
      else
      {
        check_tv_count = 0;
        digitalWrite(tv_check_count_led_pin, LED_OFF);
      }
    }
  }
}

void checkIfTVMuteTriggered(){
  if (!tv_current_status) return;
  
  if (digitalRead(tv_mute_interrupt_pin) == TV_MUTE_INTERRUPT_STATE && !last_tv_mute_interrupt)
  {
    last_tv_mute_interrupt = true;

    for (int i = 0; i < TV_VOLUME_DOWN_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeDown);
      delay(TV_VOLUME_PUSH_DELAY);
    }
    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);
    Serial.print(F("Interrupt: Mute TV\n"));

    digitalWrite(tv_mute_interrupt_led_pin, LED_ON);
  }
  else if (digitalRead(tv_mute_interrupt_pin) != TV_MUTE_INTERRUPT_STATE && last_tv_mute_interrupt)
  {
    last_tv_mute_interrupt = false;

    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);
    for (int i = 0; i < TV_VOLUME_UP_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeUp);
      delay(TV_VOLUME_PUSH_DELAY);
    }

    digitalWrite(tv_mute_interrupt_led_pin, LED_OFF);
    Serial.print(F("Interrupt: Unmute TV\n"));
  }
}

int inByte;
bool inLedStatus;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupIRLED();
  pinMode(tv_on_off_interrupt_pin, INPUT);
  pinMode(tv_mute_interrupt_pin, INPUT);
  pinMode(tv_on_off_output_pin, OUTPUT);
  
  pinMode(tv_current_status_led_pin, OUTPUT);
  digitalWrite(tv_current_status_led_pin, LED_OFF);
  pinMode(tv_on_off_interrupt_led_pin, OUTPUT);
  digitalWrite(tv_on_off_interrupt_led_pin, LED_OFF);
  pinMode(tv_mute_interrupt_led_pin, OUTPUT);
  digitalWrite(tv_mute_interrupt_led_pin, LED_OFF);
  pinMode(tv_check_count_led_pin, OUTPUT);
  digitalWrite(tv_check_count_led_pin, LED_OFF);
  inLedStatus = false;
  Serial.print(F("init done\n"));
}

void loop() {
  
  inByte = Serial.read();
  if (inByte == 'O')
  {
    irsend.sendPanasonic(PanasonicAddress, PanasonicPower);
    if(!inLedStatus) digitalWrite(tv_on_off_interrupt_led_pin, LED_ON);
    else digitalWrite(tv_on_off_interrupt_led_pin, LED_OFF);
    inLedStatus = !inLedStatus;
    Serial.print(F("Backdoor: turn On/off TV\n"));
  }
  else if(inByte == 'M')
  {
    for (int i = 0; i < TV_VOLUME_DOWN_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeDown);
      delay(TV_VOLUME_PUSH_DELAY);
    }
    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);
    
    digitalWrite(tv_mute_interrupt_led_pin, LED_ON);
    Serial.print(F("Backdoor: Mute TV\n"));
  }
  else if(inByte == 'U')
  {
    irsend.sendPanasonic(PanasonicAddress, PanasonicMute);
    for (int i = 0; i < TV_VOLUME_UP_PUSH; ++i){
      irsend.sendPanasonic(PanasonicAddress, PanasonicVolumeUp);
      delay(TV_VOLUME_PUSH_DELAY);
    }
    digitalWrite(tv_mute_interrupt_led_pin, LED_OFF);
    Serial.print(F("Backdoor: Unmute TV\n"));
  }
  checkIfTVOnOffTriggered();  
  checkIfTVMuteTriggered();
}
