#include "SPI.h"
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <GxGDEH029A1/GxGDEH029A1.h> // 2.9" b/w 296 x 128 Pixel

#include <U8g2_for_Adafruit_GFX.h>
#include <u8g2_fonts.h>

#define SPI_MOSI        23
#define SPI_MISO        2
#define SPI_CLK         18

#define ELINK_SS        5
#define ELINK_BUSY      4
#define ELINK_RESET     12
#define ELINK_DC        19

#define coord           uint16_t
#define D_WIDTH  296
#define D_HEIGHT 128

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

U8G2_FOR_ADAFRUIT_GFX fonts;

void sc_setup() {
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
  
  display.init();
  display.setRotation(1);
  fonts.begin(display);
  Serial.println("Pantalla Lista");
}

void sc_iniciando() {
  display.fillScreen(GxEPD_WHITE);
  fonts.setFont(u8g2_font_luBS12_te);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  fonts.setCursor(10,40);
  fonts.println("Iniciando.");
  fonts.setCursor(10,80);
  fonts.println("Por favor espere ...");
  display.update();
}


void sc_configure() {
  display.fillScreen(GxEPD_WHITE);
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
  
  display.update();
}
