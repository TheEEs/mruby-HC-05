#include <Arduino.h>
#include <Servo.h>

#define MODE 0
#define DIGITAL_WRITE 1
#define ANALOG_WRITE 2
#define DIGITAL_READ 3
#define ANALOG_READ 4

Servo servo9, servo10;

void setup() {
  Serial.begin(9600);
  servo9.attach(9);
  servo10.attach(10);
}

void loop() {
  int bytes_avaiable = Serial.available();
  uint8_t bytes[3] = {0};
  uint8_t value;
  if(bytes_avaiable == 3){
    Serial.readBytes(bytes,3);
    uint8_t opcode = bytes[0];
    switch(opcode){
      case MODE: {
        uint8_t mode = (bytes[2] == INPUT ? INPUT : OUTPUT);
        pinMode(bytes[1],mode);
      }
      break;
      case DIGITAL_WRITE:
      {
        value = bytes[2];
        digitalWrite((uint8_t)bytes[1],value);
      }
      break;
      case DIGITAL_READ:
      {
        value =  (uint8_t)digitalRead(bytes[1]);
        Serial.write(&value,1);
      }
      break;
      case ANALOG_WRITE:
      {
        value = bytes[2];
        uint8_t pin = bytes[1];
        if (pin == 9){
          servo9.write(value);
        }else if(pin == 10)
        {
          servo10.write(value);
        }
        else{
          analogWrite(pin,value);
        }
      }
      break;
      case ANALOG_READ:
      {
        value = (uint8_t) analogRead(bytes[1]);
        Serial.write(&value,1);
      }
        break;
    }
  }
}