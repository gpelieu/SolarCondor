#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 10;

/***********Début code init de pour lire et ecrire des donnée$**********************/
File myFile;

String buffer;
String filename = "test";

/**************Fin Code pour lire et ecrire des donnée$***************/
/**************Début code init Thermométre ********************************/
/* Dépendance pour le bus 1-Wire */
#include <OneWire.h>
float temperature;  
 
/* Broche du bus 1-Wire */
const byte BROCHE_ONEWIRE = 7;

/* Code de retour de la fonction getTemperature() */
enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse reçue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};


/* Création de l'objet OneWire pour manipuler le bus 1-Wire */
OneWire ds(BROCHE_ONEWIRE);
 
/** Fonction de lecture de la température via un capteur DS18B20.*/
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire ci nécessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    return NO_SENSOR_FOUND;
  }
  
  /* Vérifie que l'adresse a été correctement reçue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    return INVALID_ADDRESS;
  }
 
  /* Vérifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  *temperature = ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}
 /*************************Fin code Init Thermométre******************/
 /************************Debut code Init INA3221******************/
#include <Wire.h>
#include "SDL_Arduino_INA3221.h"
float current_mA1 = 0;
float current_mA2 = 0;
float current_mA3 = 0;
SDL_Arduino_INA3221 ina3221;

// the three channels of the INA3221 named for SunAirPlus Solar Power Controller channels (www.switchdoc.com)
#define LIPO_BATTERY_CHANNEL 1
#define SOLAR_CELL_CHANNEL 2
#define OUTPUT_CHANNEL 3

int temps=0;
/************************Fin code Init INA3221************************/
/********************************** Fonction setup() *****************/

void setup() {
  /********************Rien pour le thermométre***********************/
  /**************** Debut code setup  pour récupérer les info d’une carte SD*******/
  Serial.begin(115200);
//  while (!Serial) {
//  } 
 Serial.print("\nInitializing SD card...");

  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }

  uint32_t volumesize;
 // Serial.print("\nVolume type is FAT");
 // Serial.println(volume.fatType(), DEC);
 // Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
 // Serial.print("Volume size (bytes): ");
 // Serial.println(volumesize);
 // Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
 // Serial.println(volumesize);
 // Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
//  Serial.println(volumesize);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  root.ls(LS_R | LS_DATE | LS_SIZE);

/************** Fin Code setup pour récupérer les info d’une carte SD****************************/
/*************** Debut Code setup pour lire et ecrire des donnée*********************************/
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
/***************Fin du code setup pour lire et ecrire des donnée*********************************/
/*************** Debut Code setup INA3221********************************************************/
  //Serial.println("SDA_Arduino_INA3221_Test");
  
//  Serial.println("Measuring voltage and current with ina3221 ...");
  ina3221.begin();

  Serial.print("Manufactures ID=0x");
  int MID;
  MID = ina3221.getManufID();
  Serial.println(MID,HEX);
/*************** Fin Code setup INA3221********************************************************/
delay(1000);

}
void loop(void) {
/****************Début code  loop Luminosité**********************************************************/
temps=temps+1;
  Serial.print(F("Temps : ")); Serial.println(temps);
  int valeur = analogRead(A0);
  float valeurLum=( float (valeur)-770)/(1010-770)*100;
/* Affiche lluminosite */
  Serial.print(F("Luminosite : "));Serial.println(valeurLum); 
/*****************Fin luminosité ********************************************************/
/*****************Début Thermométre loop*************************************************/

/* Lit la température ambiante à ~1Hz */
  if (getTemperature(&temperature, true) != READ_OK) {
    Serial.println(F("Erreur de lecture du capteur"));
    return;
  }

/* Affiche la température */
  Serial.print(F("Temperature : "));Serial.println(temperature, 2);//Serial.println(" Degrés");


/******************Fin Thermométre Loop**************************************************/
/*****************Début INA3221 loop*************************************************/

  current_mA1 = -ina3221.getCurrent_mA(LIPO_BATTERY_CHANNEL);  // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
//Serial.print("LIPO_Battery Current 1:       "); 
  Serial.print(F("Courant 1 : "));Serial.println(current_mA1); 
//Serial.println(" mA");
//Serial.println("");

  current_mA2 = -ina3221.getCurrent_mA(SOLAR_CELL_CHANNEL);
//Serial.print("Solar Cell Current 2:       ");
  Serial.print(F("Courant 2 : "));Serial.println(current_mA2);
//Serial.println(" mA");
//Serial.println("");

  current_mA3 = ina3221.getCurrent_mA(OUTPUT_CHANNEL);
//Serial.print("Output Current 3:       ");
  Serial.print(F("Courant 3 : "));Serial.println(current_mA3*10);
//Serial.println(" mA");
//Serial.println("");
/*****************Fin INA3221 Thermométre loop*************************************************/
/******************Début écriture carte ************************************************/   
  
  myFile = SD.open(filename + ".txt", FILE_WRITE);
  Serial.print(myFile);
delay(1000);
//Serial.print("salut");
  if (myFile) {
        //Serial.print("Ecriture de température et de la luminosité sur la carte " + filename + ".txt...");
   
    
    myFile.print(F("Temperature : "));myFile.println(temperature, 2);//;myFile.println(" Degrés");
    myFile.print(F("Luminosite : "));myFile.println(valeurLum); 
    myFile.print(F("Courant 1 : "));myFile.println(current_mA1); 
    myFile.print(F("Courant 2 : "));myFile.println(current_mA2);
    myFile.print(F("Courant 3 : "));myFile.println(current_mA3*10);
    myFile.close();
    Serial.println(" : Ecriture sur la carte : done.");
  } 
  else {
    Serial.println(" : error opening " + filename + ".txt");
  }
  /******************Fin écriture carte ************************************************/
}
