#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <libSD.h>


//**************************************************************************************************************************************

#define LedTest 2
#define P1 17
#define PushButtom 16
#define MCLR 21

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_CS 15

// Variables y constantes
static const int DELAY_SPI = 10;   // 10us
static const int spiClk = 1000000; // 1 MHz

short banInicio;
short banInitGPS;
short banPrueba;
byte tiempoPIC[8];
// short fuenteTiempoPic;

String tiempoString;
short banWriteSD;

// Punteros no inicializados a objetos SPIN
SPIClass *hspi = NULL;

// Metodos para la comunicacion con el dsPIC
void IniciarGPS();                            // C:0xA2	F:0xF2
void ObtenerTiempoPIC();                      // C:0xA5	F:0xF5
void ObtenerReferenciaTiempo(int referencia); // C:0xA6	F:0xF6

//**************************************************************************************************************************************

//**************************************************************************************************************************************
// Interrupciones
//**************************************************************************************************************************************

void IRAM_ATTR ObtenerOperacion()
{
  // digitalWrite(LedTest, !digitalRead(LedTest));
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  // pull SS slow to prep other end for transfer
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la solicitud
  hspi->transfer(0xA0);
  delayMicroseconds(DELAY_SPI);
  byte bufferSPI = hspi->transfer(0x00);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0xF0);
  // pull ss high to signify end of data transfer
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();

  // Aqui se selecciona el tipo de operacion que se va a ejecutar
  if (bufferSPI == 0xB1)
  {
    // Serial.print("Interrupcion P1: 0xB1\n");
    //  NuevoCiclo();
  }
  if (bufferSPI == 0xB2)
  {
    digitalWrite(LedTest, !digitalRead(LedTest));
    // Serial.print("Interrupcion P1: 0xB2\n");
    ObtenerTiempoPIC();
  }

  /*
  if (bufferSPI == 0xEF){
    digitalWrite(LedTest, !digitalRead(LedTest));
  }
  */
}

void IRAM_ATTR EnviarSolicitud()
{
  // Serial.print("Enviando solicitud...\n");

  if (banInitGPS == 0)
  {
    // IniciarGPS();
    ObtenerReferenciaTiempo(1);
    banInitGPS = 1;
  }
  else
  {
    ObtenerReferenciaTiempo(2);
  }

  // ObtenerReferenciaTiempo(1);
}

//**************************************************************************************************************************************

//**************************************************************************************************************************************
// Metodos para la comunicacion ESP32-dsPIC
//**************************************************************************************************************************************

// C:0xA2 F:0xF2
void IniciarGPS()
{

  Serial.print("Iniciando GPS...\n");
  // Activa la comunicacion SPI:
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la solicitud:
  hspi->transfer(0xA2);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0x00);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0xF2);
  // Desactiva la comunicacion SPI:
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();
}

// C:0xA5	F:0xF5
void ObtenerTiempoPIC()
{

  // Serial.print("Hora dsPIC: ");
  //  Activa la comunicacion SPI
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la cabecera de trama:
  hspi->transfer(0xA5);
  delayMicroseconds(DELAY_SPI);
  // Recibe el byte que indica la fuente de tiempo del PIC
  byte fuenteTiempoPic = hspi->transfer(0);
  delayMicroseconds(DELAY_SPI);
  // Recibe la trama de tiempo:
  for (short i = 0; i < 6; i++)
  {
    byte bufferSPI = hspi->transfer(0x00);
    tiempoPIC[i] = bufferSPI; // Guarda la hora y fecha devuelta por el dsPIC
    delayMicroseconds(DELAY_SPI);
  }
  // Envia el final de trama:
  hspi->transfer(0xF5);
  delayMicroseconds(DELAY_SPI);
  // Desactiva la comunicacion SPI:
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();

  // Imprime la fuente de tiempo:
  switch (fuenteTiempoPic)
  {
  case 0:
    Serial.print("RPi ");
    break;
  case 1:
    Serial.print("GPS ");
    break;
  case 2:
    // Serial.print("RTC ");
    break;
  default:
    Serial.print("E");
    Serial.print(fuenteTiempoPic);
    Serial.print(" ");
    break;
  }
  // Imprime la trama de tiempo:

  /*Serial.print(tiempoPIC[3]);
  Serial.print(":");
  Serial.print(tiempoPIC[4]);
  Serial.print(":");
  Serial.print(tiempoPIC[5]);
  Serial.print(" ");
  Serial.print(tiempoPIC[0]);
  Serial.print("/");
  Serial.print(tiempoPIC[1]);
  Serial.print("/");
  Serial.print(tiempoPIC[2]);
  Serial.print("\n");
  */

  // Graba en la SD el tiempo recuperado:
  //tiempoString = String(tiempoPIC[3]) + ":" + String(tiempoPIC[4]) + ":" + String(tiempoPIC[5]) + " " + String(tiempoPIC[0]) + "/" + String(tiempoPIC[1]) + "/" + String(tiempoPIC[2]) + "\n";
  // Serial.print(tiempoString);
  banWriteSD = 1;
  // Serial.print("\n");
  // appendFile(SD, "/Tiempo.txt", tiempoString);

  // prueba trama tiempo
  // tiempoPIC[0] = 77;
  // tiempoPIC[1] = 56;
  // tiempoPIC[2] = 55;
  // tiempoPIC[3] = 0;
  // tiempoPIC[4] = 1;
  // tiempoPIC[5] = 2;

}

// C:0xA6	F:0xF6
void ObtenerReferenciaTiempo(int referencia)
{

  // referencia = 0 -> RPi
  // referencia = 1 -> GPS
  // referencia = 2 -> RTC

  if (referencia == 1)
  {
    // Serial.print("Obteniendo hora del GPS...\n");
  }
  else
  {
    // Serial.print("Obteniendo hora del RTC...\n");
  }

  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  // pull SS slow to prep other end for transfer
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la solicitud
  hspi->transfer(0xA6);
  delayMicroseconds(DELAY_SPI);
  byte bufferSPI = hspi->transfer(referencia);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0xF6);
  // pull ss high to signify end of data transfer
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();
}

//**************************************************************************************************************************************

//**************************************************************************************************************************************
// Main
//**************************************************************************************************************************************

void setup()
{
  // Inicio de variables
  banInicio = 0;
  banInitGPS = 0;
  digitalWrite(MCLR, 1);
  banPrueba = 0;
  banWriteSD = 0;

  // Define los pines  LedTest como salida y P1 como entrada
  pinMode(LedTest, OUTPUT);
  pinMode(MCLR, OUTPUT);
  pinMode(P1, INPUT);
  pinMode(PushButtom, INPUT);

  // Inicializa el puerto serial:
  Serial.begin(9600);

  // Inicializa la tarjeta SD:
  Serial.printf("\n**************************************\n");
  if (!SD.begin(5))
  {
    Serial.println("Fallo el montaje de la tarjeta");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No hay tarjeta SD conectada");
    return;
  }
  Serial.print("Tipo de tarjeta SD: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
    banInicio = 1;
  }
  else
  {
    Serial.println("Desconocida");
  }

  Serial.printf("**************************************\n");

  // Crea un archivo para guardar el tiempo:
  //writeFile(SD, "/Tiempo.txt", "0");
  crearArchivo(SD, "/Tiempo.txt");

  // Inicializa una instancia de la clase SPIClass adjunta a hspi para la comunicacion con el dsPIC:
  hspi = new SPIClass(HSPI);
  hspi->begin();
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS); // SCLK, MISO, MOSI, SS
  pinMode(HSPI_CS, OUTPUT);                              // HSPI SS

  // Define las interrupciones
  attachInterrupt(P1, ObtenerOperacion, RISING);
  attachInterrupt(PushButtom, EnviarSolicitud, RISING);
}

void loop()
{
  // put your main code here, to run repeatedly:
  // Serial.print("Hola mundo");

  if (banInicio == 1)
  {
    if (banPrueba == 0)
    {
      Serial.print("\n***************************\n");
      Serial.print("Iniciando toma de tiempo...\n");
      ObtenerReferenciaTiempo(2);
      banPrueba = 1;
    }

    if (banWriteSD == 1)
    {
      escribirArchivo(SD, "/Tiempo.txt", tiempoPIC, 6);
      banWriteSD = 0;
    }
    // IniciarGPS();
  }

  delay(100);
  // ObtenerReferenciaTiempo(2);
}

//**************************************************************************************************************************************