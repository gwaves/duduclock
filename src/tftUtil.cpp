#include <TFT_eSPI.h>
#include "common.h"
#include "PreferencesUtil.h"

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite clk = TFT_eSprite(&tft);
int backColor;
uint16_t backFillColor;
uint16_t penColor;

// 初始化tft
//#define TFT_MOSI	3
//#define TFT_SCLK	2
//#define TFT_CS   	7  // Chip select control pin
//#define TFT_DC		4  // Data Command control pin
//#define TFT_RST		5  // Reset pin (could connect to RST pin)
void tftInit(){
//  Serial.println("before tft.init");
//  sleep(1);
  tft.init();
//  Serial.print("tft.init");
  tft.setSwapBytes(true);
  getBackColor();
  Serial.printf("backcolor is %d\r\n",backColor);
  Serial.printf("mosi:%d,sclk:%d,cs:%d,DC:%d,RST:%d\r\n",TFT_MOSI,TFT_SCLK,TFT_CS,TFT_DC,TFT_RST);
  if(backColor == BACK_BLACK){
    backFillColor = 0x0000;
    penColor = 0xFFFF;
  }else{
    backFillColor = 0xFFFF;
    penColor = 0x0000;
  }
  tft.fillScreen(backFillColor);
}

// 按背景颜色刷新整个屏幕
void reflashTFT(){
  tft.fillScreen(backFillColor);
}

// 在屏幕中间显示文字
void drawText(String text){
  clk.setColorDepth(8);
  clk.setTextDatum(CC_DATUM);
  clk.loadFont(clock_tips_28);
  clk.createSprite(240, 240); 
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.drawString(text,120,120);
  clk.pushSprite(0,0);
  clk.deleteSprite();
  clk.unloadFont(); 
}

// 在屏幕中间显示两行文字
void draw2LineText(String text1, String text2){
  clk.setColorDepth(8);
  clk.setTextDatum(CC_DATUM);
  clk.loadFont(clock_tips_28);
  clk.createSprite(240, 240);
  clk.setTextColor(penColor);
  clk.fillSprite(backFillColor);
  clk.drawString(text1,120,105);
  clk.drawString(text2,120,135);
  clk.pushSprite(0,0);
  clk.deleteSprite();
  clk.unloadFont(); 
}