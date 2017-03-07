void setup()
{
  Serial.begin(9600);
  Serial3.begin(9600);
}
void loop()
{
  if(Serial.available())
  {
    char read_data;
    read_data=Serial.read();
    Serial3.write(read_data);
  }

  if(Serial3.available())
  {
    char read_data;
    read_data=Serial3.read();
    Serial.write(read_data);
  }
}
