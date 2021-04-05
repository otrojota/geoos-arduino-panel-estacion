//* Usar Tarjeta (Arduino Board Manager)
// ESP3w Arduino / Adafruit ESP32 Feather

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_EPD.h>

#include <U8g2_for_Adafruit_GFX.h>
#include <u8g2_fonts.h>

#define coord           uint16_t
#define D_WIDTH  250
#define D_HEIGHT 122

#define GxEPD_BLACK EPD_BLACK
#define GxEPD_WHITE EPD_WHITE


// ESP32 settings
#define SD_CS       14

#define SRAM_CS     32
#define EPD_CS      15
#define EPD_DC      33
#define EPD_RESET   -1 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    -1 // can set to -1 to not use a pin (will wait a fixed delay)

Adafruit_SSD1675 display(250, 122, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

U8G2_FOR_ADAFRUIT_GFX fonts;

void sc_setup() {  
  display.begin();
  //display.setRotation(2);
  fonts.begin(display);
  Serial.println("Pantalla Lista");
}

void sc_iniciando() {
  display.clearBuffer();
  //display.fillScreen(GxEPD_WHITE);
  fonts.setFont(u8g2_font_luBS12_te);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  fonts.setCursor(10,40);
  fonts.println("[1.0.3] Iniciando.");
  fonts.setCursor(10,80);
  fonts.println("Por favor espere ...");
  //display.update();
  display.display();
}


void sc_configure() {
  display.clearBuffer();
  //display.fillScreen(GxEPD_WHITE);
  fonts.setFont(u8g2_font_luBS12_te);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  fonts.setCursor(5,20);
  fonts.println("Requiere Configurar");
  fonts.setFont(u8g2_font_courB10_tr);
  fonts.setCursor(5,50);
  fonts.println("Conectar a la red WIFI:");
  fonts.setCursor(5,70);  
  fonts.println(wi_ap_name);
  fonts.setCursor(5,90);
  fonts.println("y seguir instrucciones");
  
  //display.update();
  display.display();
}
