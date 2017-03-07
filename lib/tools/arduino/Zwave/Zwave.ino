int g_seq = 0;
int last_node = 0;

#define ST_SOF       1
#define ST_LEN       2
#define ST_TYPE      3
#define ST_CMD       4
#define ST_ID        5
#define ST_DATA      6
#define ST_CRC       7
#define ST_DONE      8

class ZWaveClass {
private:  
  byte seq;          // Sequence number which is used to match the callback function
  int state;         // Current state
  int len;           // Length of the returned payload
  int type;          // 0: request 1: resposne 2: timeout
  int cmd;           // the serial api command number of the current payload
  byte payload[64];  // The data of the current packet
  int i;            
  unsigned long expire;  // The expire time of the last command
  void (*f)(byte *,int); // The callback function registered by callback
  void (*nodeinfo)(byte *payload,int len);
public:
  void init() {
    Serial2.begin(115200);
    for(i=0;i<100;i++)
      mainloop();
    randomSeed(analogRead(0));
    seq = random(255);
    state = ST_SOF;
    expire = 0;
    nodeinfo=NULL;
    f=NULL;
  }
  int getType() {
    return type;
  }
  void DisplayNodeInfo() {
    char buf[128];

    snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
    Serial.write(buf);
  }
  // Main event loop of the ZWave SerialAPI
  boolean mainloop() {
    unsigned long now = millis();

    if (expire && (now > expire)) {
      expire = 0;
      type = 2;
      state = ST_SOF;
      if (f!=NULL) f(payload,i);
      Serial.write("timeout...\n");
      return true;
    }
    if (Serial2.available()) {
      expire = now + 1000;
      byte c = Serial2.read();
      char buf[128];
      snprintf(buf,128,"c=%x state=%d\n", c, state);
      Serial.write(buf);
      if (state == ST_SOF) {
        Serial.write("ST_SOF\n");
        if (c == 1) {
          state = ST_LEN;
          len = 0;
        } 
        else if (c == 0x15) {
        }
      } 
      else if (state == ST_LEN) {
        Serial.write("ST_LEN\n");
        len = c-3;
        state = ST_TYPE;
      } 
      else if (state == ST_TYPE) {
        Serial.write("ST_TYPE\n");
        type = c;
        state = ST_CMD;
      } 
      else if (state == ST_CMD) {
        Serial.write("ST_CMD\n");
        cmd = c;
        state = ST_DATA;
        i = 0;
      } 
      else if (state == ST_DATA) {
        Serial.write("ST_DATA\n");
        payload[i++] = c;
        len--;
        if (len == 0) {
          state = ST_CRC;
        }
      } 
      else if (state == ST_CRC) {
        Serial.write("ST_CRC\n");
        Serial2.write(6);
        state = ST_SOF;
        if (f!=NULL) f(payload,i); f = NULL;
        if (cmd == 0x49) {
          if (nodeinfo) nodeinfo(payload,i);
        }
        return true;
      }
    }
    return false;
  }

  // Register for the callback function when we receive packet from the Z-Wave module
  void callback(void (*func)(byte *,int)) {
    f = func;
  }

  void callback_nif(void (*func)(byte *,int)) {
    nodeinfo = func;
  }

  // Send ZWave command to another node. This command can be used as wireless repeater between 
  // two nodes. It has no assumption of the payload sent between them.
  void send(byte id, byte *b,byte l,byte option) {
    int k;
    byte crc;
    byte buf[24];
    byte ll=l+7;

    Serial.write("Send\n");
    buf[0] = 1;
    buf[1] = l+7;
    buf[2] = 0;
    buf[3] = 0x13;
    buf[4] = id;
    buf[5] = l;
    for(k=0;k<l;k++)
      buf[k+6] = b[k];
    buf[l+6] = option;
    buf[l+7] = seq;

    crc = 0xff;
    Serial2.write(buf[0]);
    for(k=0;k<l+7;k++) {
      Serial2.write(buf[k+1]);
      crc = crc ^ buf[k+1];
    }
    seq++;
    Serial2.write(crc);
    expire = millis()+1000;
  }

  // Include or exclude node from the network
  void networkIncludeExclude(byte t,byte m) {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 5;
    b[2] = 0;
    b[3] = t;
    b[4] = m;
    b[5] = seq;
    b[6] = 0xff^5^0^t^m^seq;
    seq++;
    for(k=0;k<7;k++)
      Serial2.write(b[k]);
  }

  // Reset the ZWave module to the factory default value. This must be called carefully since it will make
  // the network unusable.
  void reset() {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 4;
    b[2] = 0;
    b[3] = 0x42;
    b[4] = seq;
    b[5] = 0xff^4^0^0x42^seq;
    seq++;
    for(k=0;k<6;k++)
      Serial2.write(b[k]);

  }

  // Reset the ZWave module to the factory default value. This must be called carefully since it will make
  // the network unusable.
  void learn(int onoff) {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 5;
    b[2] = 0;
    b[3] = 0x50;
    b[4] = onoff;
    b[5] = seq;
    b[6] = 0xff^5^0^0x50^onoff^seq;
    seq++;
    for(k=0;k<7;k++)
      Serial2.write(b[k]);

  }
  
  void getId() {
    byte b[5];
    int k;

    b[0] = 1;
    b[1] = 3;
    b[2] = 0x00;
    b[3] = 0x20;
    b[4] = 0xff^3^0x00^0x20;
    for(k=0;k<5;k++)
      Serial2.write(b[k]);

  }

  // Start inclusion procedure to add a new node
  void includeAny() {
    networkIncludeExclude(0x4A,1);
  }

  // Stop inclusion/exclusion procedure
  void LearnStop() {
    networkIncludeExclude(0x4A,5);
  }

  // Start exclusion procedure
  void excludeAny() {
    networkIncludeExclude(0x4B,1);
  }

  // Set the value of a node
  void set(byte id,byte v,byte option) {
    byte b[3];

    b[0] = 0x20;
    b[1] = 1;
    b[2] = v;
    send(id,b,3,option);
  }
};

ZWaveClass ZWave;

void displayStatus(byte *ret,int len)
{
  char buf[64];


  snprintf(buf,64,"Status=%d Node=%d\n", ret[1],ret[2]);
  Serial.write(buf);
}
void include1(byte *ret,int len)
{
  char buf[64];


  snprintf(buf,64,"Status=%d Node=%d\n", ret[1],ret[2]);
  Serial.write(buf);
  ZWave.callback(displayStatus);
  ZWave.includeAny();
}

void next(byte *ret,int len)
{
  Serial.write("Done\n");
  if (ZWave.getType() == 0) {
    if (ret[1] == 0) {
      Serial.write("ACK\n");
    } 
    else {
      Serial.write("No ACK\n");
    }
    ZWave.excludeAny();
    ZWave.callback(include1);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.excludeAny();
    ZWave.callback(include1);
  }
}

void offack(byte *b,int len)
{
  if (ZWave.getType() == 0) {
    delay(500);
    ZWave.callback(onack);
    ZWave.set(last_node,255,5);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.callback(onack);
    ZWave.set(last_node,255,5);
  } 
}

void onack(byte *b,int len)
{
  if (ZWave.getType() == 0) {
    delay(500);
    ZWave.callback(offack);

    ZWave.set(last_node,0,5);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.callback(offack);
    ZWave.set(last_node,0,5);
  }
}

void display_nif(byte *payload,int len) {
  char buf[128];
  Serial.write('display_nif');
  snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
  Serial.write(buf);
  last_node = payload[1];
}



void setup()
{
  byte b[1] = {
    0  };
  Serial.begin(115200);

  ZWave.init();
  Serial.write("Start\n");
  //ZWave.reset();
  //ZWave.callback(include1);
  //ZWave.excludeAny();
  //ZWave.LearnStop();
  //ZWave.includeAny();
  //ZWave.send(2,b,1);
  //offack(NULL,0);
  ZWave.callback_nif(display_nif);
}

unsigned char hex[] = { 
  '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void getId_cb(byte *b,int len)
{
  char buf[64];

  snprintf(buf,64,"HomeId: %02x%02x%02x%02x Node=%x\n", b[0],b[1],b[2],b[3],b[4]);
  Serial.write(buf);
}

void include_cb(byte *b,int len)
{
  char buf[64];

  snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
}

void exclude_cb(byte *b,int len)
{
  char buf[64];

  snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
}

void help()
{
  Serial.write("a: Include a new node\n");
  Serial.write("d: Exclude a node\n");
  Serial.write("s: stop inclusion/exclusion\n");
  Serial.write("t: test the current node\n");
  Serial.write("l: turn on learn mode\n");
  Serial.write("L: turn off learn mode\n");
  Serial.write("r: reset the node\n");
  Serial.write("m: get node id\n");
  Serial.write("Press the program button of the node to change the current node\n");
}

void loop()
{
  if (ZWave.mainloop()) {

  }
  if (Serial.available()) {
    byte c = Serial.read();
    if (c == 'a') {
      ZWave.callback(include_cb);
      ZWave.includeAny();
    } 
    else if (c == 'd') {
      ZWave.callback(exclude_cb);
      ZWave.excludeAny();
    } 
    else if (c == 's') {
      ZWave.callback(0);
      ZWave.LearnStop();
    } 
    else if (c == 't') {
      ZWave.callback(offack);
      ZWave.set(last_node,0,5);
    } 
    else if (c == 'l') {
      ZWave.callback(0);
      ZWave.learn(1);      
    } 
    else if (c == 'L') {
      ZWave.callback(0);
      ZWave.learn(0);      
    } 
    else if (c == 'r') {
      ZWave.callback(0);
      ZWave.reset();      
    } 
    else if (c == 'm') {
      ZWave.callback(getId_cb);
      ZWave.getId();
    }
    else {
      help();
    }
  }
}



