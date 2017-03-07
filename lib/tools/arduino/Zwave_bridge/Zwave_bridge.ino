void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
}
void loop()
{
  if(Serial.available())
  {
    char read_data;
    read_data=Serial.read();
    Serial2.write(read_data);
  }

  if(Serial2.available())
  {
    char read_data;
    read_data=Serial2.read();
    Serial.write(read_data);
  }
}
