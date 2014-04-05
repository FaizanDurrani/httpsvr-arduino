////////////////////////////////////////////////////////////////////////////////
//
//  Definition of LCD utility functions
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <String>

////////////////////////////////////////////////////////////////////////////////
// Initialize the LCD library with the numbers of the interface pins

static const uint8_t LCD_RS = 8;
static const uint8_t LCD_EN = 7;
static const uint8_t LCD_D0 = 6;
static const uint8_t LCD_D1 = 5;
static const uint8_t LCD_D2 = 3;
static const uint8_t LCD_D3 = 2;

LiquidCrystal LCD_this(LCD_RS, LCD_EN, LCD_D0, LCD_D1, LCD_D2, LCD_D3);

////////////////////////////////////////////////////////////////////////////////

static const int LCD_NROWS = 2;
static const int LCD_NCOLS = 16;

void configDisplay()
{
  // Set up the LCD's number of columns and rows: 
  LCD_this.begin(LCD_NCOLS, LCD_NROWS);
}

////////////////////////////////////////////////////////////////////////////////

String paddedString(uint8_t u)
{
  if (u < 10) return String("0") + String(u, DEC);
  else return String(u, DEC);
}

String numString(float the_v)
{
  String sInt(static_cast<int>(floor(the_v)), DEC);
  String sDec(static_cast<int>(10.0*the_v)%10, DEC);
  return sInt + "." + sDec;
}

void writeBanner(const String& the_appName, const String& the_appVer)
{
  String sFirstLine  = the_appName + " v." + the_appVer;
  String sSecondLine = String(__DATE__);
  LCD_this.setCursor(0, 0); LCD_this.print(sFirstLine);
  LCD_this.setCursor(0, 1); LCD_this.print(sSecondLine);
}

////////////////////////////////////////////////////////////////////////////////

static char * fullLine(const String& s)
{
  static char cs[LCD_NCOLS+1];
  
  uint8_t len = s.length();
  for (uint8_t i=0; i<LCD_NCOLS; ++i)
    cs[i] = (i<len ? s[i] : ' ');
  cs[LCD_NCOLS] = 0;
  return cs;
}

void writeLine1(const String& s)
{  LCD_this.setCursor(0, 0); LCD_this.print(fullLine(s)); }

void writeLine2(const String& s)
{ LCD_this.setCursor(0, 1); LCD_this.print(fullLine(s)); }

void writeLines(const String& s1, const String& s2)
{ writeLine1(s1); writeLine2(s2); }

////////////////////////////////////////////////////////////////////////////////

void clearDisplay() { LCD_this.clear(); }
void clearLine1()   { writeLine1(""); }
void clearLine2()   { writeLine2(""); }

////////////////////////////////////////////////////////////////////////////////

