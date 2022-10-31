#include <Arduino.h>
#include <SPI.h>

#define P1 17
#define LedTest 2

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_CS   15

//Variables y constantes
static const int DELAY_SPI = 10;   // 10us
static const int spiClk = 2000000; // 1 MHz

//Punteros no inicializados a objetos SPIN
SPIClass * hspi = NULL;

void IRAM_ATTR ObtenerOperacion()
{
  //digitalWrite(LedTest, !digitalRead(LedTest));
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  //pull SS slow to prep other end for transfer
  digitalWrite(hspi->pinSS(), LOW); 
  //Envia la solicitud 
  hspi->transfer(0xA0);
  delayMicroseconds(DELAY_SPI);
  byte bufferSPI = hspi->transfer(0x00);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0xF0);
  //pull ss high to signify end of data transfer
  digitalWrite(hspi->pinSS(), HIGH); 
  hspi->endTransaction();

  if (bufferSPI == 0xEF){
    digitalWrite(LedTest, !digitalRead(LedTest));
  }
  
}

void setup()
{
  //Inicializa una instancia de la clase SPIClass adjunta a VSPI
  hspi = new SPIClass(HSPI);
  hspi->begin();
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS); //SCLK, MISO, MOSI, SS
  pinMode(hspi->pinSS(), OUTPUT); //VSPI SS

  //Define los pines  LedTest como salida y P1 como entrada
  pinMode(LedTest, OUTPUT);
  pinMode(P1, INPUT);
  attachInterrupt(P1, ObtenerOperacion, RISING);
}

void loop()
{
  // put your main code here, to run repeatedly:
}