#include "BluetoothSerial.h"
#include <Arduino.h>
#include <ELMduino.h>


BluetoothSerial SerialBT;
ELM327 elm327;
uint8_t elm327MacAddress[6] = {0x00, 0x1D, 0xA5, 0x68, 0x98, 0x8A};
#define DEBUG_PORT Serial
#define ELM_PORT   SerialBT
static bool elm327Connected = false;
static bool bluetoothConnected = false;

uint32_t rpm = 0;

float kph, coolant;

bool connectBluetooth()
{
  while (!bluetoothConnected)
  {
    if (SerialBT.connect(elm327MacAddress))
    {
      bluetoothConnected = true;
    }
    else
    {
      delay(200);
    }
  }
  
  return bluetoothConnected;
}

bool connectElm327()
{
  while (!elm327Connected)
  {
    if (elm327.begin(SerialBT, false, 2000))
    {
      elm327Connected = true;
    }
    else
    {
      delay(200);
    }
  }

  return elm327Connected;
}

void setup()
{

  DEBUG_PORT.begin(115200);

  SerialBT.begin("Koci", true);
  SerialBT.setPin("1234");

  DEBUG_PORT.println("Attempting to connect to ELM327...");

  connectBluetooth();
  connectElm327();

  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner");
    while(1);
  }

  DEBUG_PORT.println("Connected to ELM327");
  DEBUG_PORT.println("Ensure your serial monitor line ending is set to 'Carriage Return'");
}


void loop()
{
  while (elm327Connected && bluetoothConnected )
  {
    float tempRPM = elm327.rpm();

    if (elm327.nb_rx_state == ELM_SUCCESS)
    {
      rpm = (uint32_t)tempRPM;
      Serial.print("RPM: "); Serial.println(rpm);
    }
    else if (elm327.nb_rx_state != ELM_GETTING_MSG)
      elm327.printError();
  }
}