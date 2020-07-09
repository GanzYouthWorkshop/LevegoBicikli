//***************************************************************************************
//Includes
//***************************************************************************************
#include <SPI.h>
#include <MD_MAX72xx.h>

//***************************************************************************************
//Configurations
//***************************************************************************************
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   8
#define CURRENT_SENSOR_PIN A0
#define SS_DISPLAY_PIN 2
#define CURREMT_VALUES_COUNT 32

//***************************************************************************************
//Constants
//***************************************************************************************
#define C_FULL_ROW  0b11111111
#define C_HALF_ROW  0b00111100
#define C_EMPTY_ROW 0b00000000

//***************************************************************************************
//Global variables
//***************************************************************************************
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, SS_DISPLAY_PIN, MAX_DEVICES);

int last_current_value = 0;

int current_values_ring_index = 0;
int current_values[CURREMT_VALUES_COUNT];

byte rs232_buff[6];

bool simulation_enabled = false;
int simulated_value = 0;

//***************************************************************************************
//Setup
//***************************************************************************************
void setup()
{ 
  Serial.begin(9600);
  mx.begin();

  mx.clear();
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);  
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

//***************************************************************************************
// Main loop
//***************************************************************************************
void loop()
{
  //Debug bemenet kezelése
  if(Serial.available())
  {
    Serial.readBytes(rs232_buff, 6);
    if(rs232_buff[0] == 's' && rs232_buff[1] == 'i' && rs232_buff[2] == 'm')
    {
      if(rs232_buff[4] == 'x')
      {
        simulation_enabled = false;
      }
      else
      {
        simulation_enabled = true;

        simulated_value = ((rs232_buff[4] - '0') * 100) + ((rs232_buff[5] - '0') * 10) + (rs232_buff[6] - '0');
      }
    }
  }

  //Aalóg érték olvasása
  last_current_value = 0;
  for(int i = 0; i < CURREMT_VALUES_COUNT; i++)
  {
    int read = abs(analogRead(CURRENT_SENSOR_PIN)) * 5;
    if(last_current_value < read)
    {
      last_current_value = read;
    }
    delay(1);
  }
 // last_current_value /= CURREMT_VALUES_COUNT;

  //Kimenő jel beállítása - valós vagy szimuláció
  int value = simulation_enabled ? simulated_value : last_current_value;

  //Kijelzés
  handle_spi_display();
  Serial.println(value);

  delay(50);
}

void handle_spi_display()
{
  byte col_value = 0;
  int value = simulation_enabled ? simulated_value : last_current_value;
  int check_value = 0;
  
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  for(int module = 0; module < MAX_DEVICES; module++)
  {
    for(int col = 0; col < 8; col++)
    {
      check_value = (module * 8 + col) * 4;
      if(value > check_value)
      {    
        col_value = C_FULL_ROW;
      }
      else if(value > check_value - 2)
      {    
        col_value = C_HALF_ROW;
      }
      else
      {
        col_value = C_EMPTY_ROW;
      }

      //A modulok egynás után jönnek de az oszlopok már fordított sorrendben vannak indexelve :(
      int hwcol = (module * 8) + (7 - col);

      mx.setColumn(hwcol, col_value);
    }
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
