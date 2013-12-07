////////////////////////////////////////////////////////////////////////////////
//
//  3D Printer Driver - Definition of LCD utility functions
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef LCD_UTILS_H
#define LCD_UTILS_H

#include <Arduino.h>
#include <String>

extern void configDisplay();
extern void clearDisplay();
extern void clearLine1();
extern void clearLine2();
extern void writeBanner(const String& the_appName, const String& the_appVer);
extern void writeLine1(const String&);
extern void writeLine2(const String&);
extern void writeLines(const String&, const String&);

////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef LCD_UTILS_H

