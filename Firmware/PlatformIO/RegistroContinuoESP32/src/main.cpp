#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <libSD.h>

//**************************************************************************************************************************************

#define LedTest 2
#define P1 17
#define P2 16
#define MCLR 21

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_CS 15

// Variables y constantes
static const int DELAY_SPI = 10;   // 10us
static const int spiClk = 1000000; // 1 MHz

boolean banInicioSistema = false;
short banInitGPS;
boolean banNuevoCiclo = false;
boolean banTiempPIC = false;
boolean banWriteSD = false;

boolean banPrimerEncendido = true;

byte tiempoPIC[8];
byte tiempoLocal[8];
byte tramaAceleracion[2506];
// short fuenteTiempoPic;

String fechaPIC;
String horaPIC;
String pathDirectorio;
String pathArchivo;

char bufferFecha[8] = " ";
char bufferHora[8] = " ";

// Punteros no inicializados a objetos SPIN
SPIClass *hspi = NULL;

// Metodos para la comunicacion con el dsPIC
void IniciarMuestreo();                       // C:0xA1	F:0xF1
void IniciarGPS();                            // C:0xA2	F:0xF2
void ObtenerTramaAceleracion();               // C:0xA3	F:0xF3
void EnviarTiempoLocal();                     // C:0xA4	F:0xF4
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
    digitalWrite(LedTest, !digitalRead(LedTest));
    ObtenerTramaAceleracion();
  }
  if (bufferSPI == 0xB2)
  {
    //Serial.print("Interrupcion P1: 0xB2\n");
    digitalWrite(LedTest, !digitalRead(LedTest));
    ObtenerTiempoPIC();
  }
}

void IRAM_ATTR NuevoCiclo()
{
  //Serial.print("Interrupcion P2\n");
  banNuevoCiclo = true;
}

//**************************************************************************************************************************************

//**************************************************************************************************************************************
// Metodos para la comunicacion ESP32-dsPIC
//**************************************************************************************************************************************

// C:0xA1 F:0xF1
void IniciarMuestreo()
{
  Serial.print("Iniciando Muestreo...\n");
  // Activa la comunicacion SPI:
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la solicitud:
  hspi->transfer(0xA1);
  delayMicroseconds(DELAY_SPI);
  hspi->transfer(0xF1);
  // Desactiva la comunicacion SPI:
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();
}

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

// C:0xA3	F:0xF3
void ObtenerTramaAceleracion()
{
  // Serial.print("Hora dsPIC: ");
  //  Activa la comunicacion SPI
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la cabecera de trama:
  hspi->transfer(0xA3);
  delayMicroseconds(DELAY_SPI);
  // Recibe la trama de tiempo:
  for (int i = 0; i < 2506; i++)
  {
    byte bufferSPI = hspi->transfer(0x00);
    tramaAceleracion[i] = bufferSPI; // Guarda la hora y fecha devuelta por el dsPIC
    delayMicroseconds(DELAY_SPI);
  }
  // Envia el final de trama:
  hspi->transfer(0xF3);
  delayMicroseconds(DELAY_SPI);
  // Desactiva la comunicacion SPI:
  digitalWrite(hspi->pinSS(), HIGH);
  hspi->endTransaction();
  // Activa la bandera para guardar los datos en la SD:
  banWriteSD = true;
}

// C:0xA4	F:0xF4
void EnviarTiempoLocal()
{
  //Establece el tiempo local que se enviara al dsPIC:
  tiempoLocal[0] = 22; //anio
  tiempoLocal[1] = 11; //mes
  tiempoLocal[2] = 10; //dia
  tiempoLocal[3] = 23; //hora
  tiempoLocal[4] = 58; //minuto
  tiempoLocal[5] = 0; //segundo
  //  Activa la comunicacion SPI
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
  digitalWrite(hspi->pinSS(), LOW);
  // Envia la cabecera de trama:
  hspi->transfer(0xA4);
  delayMicroseconds(DELAY_SPI);
  // Recibe la trama de tiempo:
  for (short i = 0; i < 6; i++)
  {
    hspi->transfer(tiempoLocal[i]);
    delayMicroseconds(DELAY_SPI);
  }
  // Envia el final de trama:
  hspi->transfer(0xF4);
  delayMicroseconds(DELAY_SPI);
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

  // Guarda la fecha y hora en formato String:
  sprintf(bufferFecha, "%0.2d%0.2d%0.2d", tiempoPIC[0], tiempoPIC[1], tiempoPIC[2]);
  sprintf(bufferHora, "%0.2d%0.2d%0.2d", tiempoPIC[3], tiempoPIC[4], tiempoPIC[5]);
  fechaPIC = String(bufferFecha);
  horaPIC = String(bufferHora);

  // Activa la bandera para indicar que se recupero la hora del dsPIC:
  banTiempPIC = true;

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
  banInitGPS = 0;
  digitalWrite(MCLR, 1);

  // Define los pines de entrada y salida:  
  pinMode(LedTest, OUTPUT);
  pinMode(MCLR, OUTPUT);
  pinMode(P1, INPUT);
  pinMode(P2, INPUT);

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
  } else 
  {
    banInicioSistema = true;
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
    }
    else
    {
      Serial.println("Desconocida");
    }
  }
  Serial.printf("**************************************\n");

  // Inicializa una instancia de la clase SPIClass adjunta a hspi para la comunicacion con el dsPIC:
  hspi = new SPIClass(HSPI);
  hspi->begin();
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS); // SCLK, MISO, MOSI, SS
  pinMode(HSPI_CS, OUTPUT);                              // HSPI SS

  // Define las interrupciones
  attachInterrupt(P1, ObtenerOperacion, RISING);
  attachInterrupt(P2, NuevoCiclo, RISING);
}

void loop()
{
  if (banInicioSistema == true)
  {
    if (banWriteSD == true)
    {
      escribirArchivo(SD, pathArchivo, tramaAceleracion, 2506);
      banWriteSD = false;
    }
    if (banTiempPIC == true)
    {
      // Crea un directorio con la fecha por nombre:
      pathDirectorio = "/" + fechaPIC;
      pathArchivo = pathDirectorio + "/" + fechaPIC + horaPIC + ".dat";
      createDir(SD, pathDirectorio);
      crearArchivo(SD, pathArchivo);
      IniciarMuestreo();
      banTiempPIC = false;
    }
    if (banNuevoCiclo == true)
    {
      Serial.print("\n***************************\n");
      Serial.print("Iniciando un nuevo ciclo de muestreo...\n");
      if (banPrimerEncendido==true)
      {
        //Envia el tiempo local en el primer encendido:
        EnviarTiempoLocal();  
        //ObtenerReferenciaTiempo(2); 
        banPrimerEncendido = false;
      } 
      else 
      {
        //Obtiene el tiempo del dsPIC en los siguientes ciclos de muestreo:
        Serial.print("Obteniendo tiempo del RTC\n");
        ObtenerReferenciaTiempo(2); 
      }
      banNuevoCiclo = false;
    }
  }

}

//**************************************************************************************************************************************