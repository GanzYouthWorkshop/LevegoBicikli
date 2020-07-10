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
#define SS_DISPLAY_PIN 2

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

//***************************************************************************************
//Setup
//***************************************************************************************
void setup()
{ 
  mx.begin();
  delay(1000);
  Serial.begin(9600);
  

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
    last_current_value = Serial.parseInt();
  }

  Serial.println(last_current_value);
  //Kijelzés
  handle_spi_display();
}

void handle_spi_display()
{
  byte col_value = 0;
  int value = last_current_value;
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

