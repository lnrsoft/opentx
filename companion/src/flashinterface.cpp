/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 * 
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <QtGui>
#include "hexinterface.h"
#include "splash.h"
#include "flashinterface.h"

int getFileType(const QString &fullFileName)
{
  QString suffix = QFileInfo(fullFileName).suffix().toUpper();
  if (suffix == "HEX")
    return FILE_TYPE_HEX;
  else if (suffix == "BIN")
    return FILE_TYPE_BIN;
  else if (suffix == "EEPM")
    return FILE_TYPE_EEPM;
  else if (suffix == "EEPE")
    return FILE_TYPE_EEPE;
  else if (suffix == "XML")
    return FILE_TYPE_XML;
  else
    return 0;
}

FlashInterface::FlashInterface(QString fileName):
  flash(MAX_FSIZE, 0),
  flash_size(0),
  isValidFlag(false)
{
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) { //reading HEX TEXT file
    QTextStream inputStream(&file);
    flash_size = HexInterface(inputStream).load((uint8_t *)flash.data(), MAX_FSIZE);
    file.close();
    if (flash_size == 0) {
      file.open(QIODevice::ReadOnly);
      flash_size = file.read((char *)flash.data(), MAX_FSIZE);
    }
    if (flash_size > 0) {
      svn = seekLabel(SVN_MARK);
      version = seekLabel(VERS_MARK);
      date = seekLabel(DATE_MARK);
      time = seekLabel(TIME_MARK);
      eeprom = seekLabel(EEPR_MARK);
      SeekSplash();
      isValidFlag = true;
    }
  }
}

QString FlashInterface::seekString(const QString & string)
{
  QString result = "";

  int start = flash.indexOf(string);
  if (start > 0) {
    start += string.length();
    int end = -1;
    for (int i=start; i<start+20; i++) {
      char c = flash.at(i);
      if (c == '\0' || c == '\036') {
        end = i;
        break;
      }
    }
    if (end > 0) {
      result = flash.mid(start, (end - start)).trimmed();
    }
  }

  return result;
}

QString FlashInterface::seekLabel(const QString & label)
{
  QString result = seekString(label + "\037\033:");
  if (!result.isEmpty())
    return result;

  return seekString(label + ":");
}

void FlashInterface::SeekSplash(void) 
{
  QByteArray splash,spe;
  splash_offset=0;
  splash.clear();
  splash.append((char *)gr9x_splash, sizeof(gr9x_splash));
  int start = flash.indexOf(splash);
  if (start>0) {
    splash_offset=start;
    splash_type=1;
    splash_size=sizeof(gr9x_splash);
  } 
  if (start==-1) {
    splash.clear();
    splash.append((char *)er9x_splash, sizeof(er9x_splash));
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start;
      splash_type=2;
      splash_size=sizeof(er9x_splash);
    }
  }
  if (start==-1) {
    splash.clear();
    splash.append((char *)opentx_splash, sizeof(opentx_splash));
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start;
      splash_type=3;
      splash_size=sizeof(opentx_splash);
    }
  }
  if (start==-1) {
    splash.clear();
    splash.append((char *)opentxtaranis_splash, sizeof(opentxtaranis_splash));
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start;
      splash_type=5;
      splash_size=sizeof(opentxtaranis_splash);
    }
  }
  if (start==-1) {
    splash.clear();
    splash.append((char *)gr9xv4_splash, sizeof(gr9xv4_splash));
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start;
      splash_type=1;
      splash_size=sizeof(gr9xv4_splash);
    }
  }
  if (start==-1) {
    splash.clear();
    splash.append((char *)ersky9x_splash, sizeof(ersky9x_splash));
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start;
      splash_type=4;
      splash_size=sizeof(ersky9x_splash);
    }
  }
  if (start==-1) {
    start=0;
    splash.clear();
    splash.append(OTX_SPS);
    splash.append('\0');
    spe.clear();
    spe.append(OTX_SPE);
    spe.append('\0');   
    int diff=0;
    int end=0;
    while (start>=0) {
      start = flash.indexOf(splash,start+1);
      if (start>0) {
        end=flash.indexOf(spe,start+1);
        if (end>0) {
          diff=end-start;
          while (diff<1030 && end>0) {
            end=flash.indexOf(spe,end+1);
            diff=end-start;
          }
          if (end>0) {
            diff=end-start;
            if (diff==1030) {
              splash_offset=start+OTX_OFFSET;
              splash_type=2;
              splash_size=sizeof(opentx_splash);
              break;
            }
            if (diff==6790) {
              splash_offset=start+OTX_OFFSET;
              splash_type=5;
              splash_size=sizeof(opentxtaranis_splash);
              break;
            }            
          }
        }
      }
    }
  }
  if (start==-1) {
    start=0;
    splash.clear();
    splash.append(ERSKY9X_SPS);
    splash.append('\0');
    spe.clear();
    spe.append(ERSKY9X_SPE);
    spe.append('\0');   
    int diff=0;
    int end=0;
    while (start>=0) {
      start = flash.indexOf(splash,start+1);
      if (start>0) {
        end=flash.indexOf(spe,start+1);
        if (end>0) {
          diff=end-start;
          while (diff<1031 && end>0) {
            end=flash.indexOf(spe,end+1);
            diff=end-start;
          }
          if (end>0) {
            diff=end-start;
            if (diff==1031) {
              splash_offset=start+ERSKY9X_OFFSET;
              splash_type=4;
              splash_size=sizeof(ersky9x_splash);
              break;
            }
          }
        }
      }
    }
  }
  if (start==-1) {
    splash.clear();
    splash.append(ERSPLASH_MARKER);
    splash.append('\0');
    start = flash.indexOf(splash);
    if (start>0) {
      splash_offset=start+ERSPLASH_OFFSET;
      splash_type=2;
      splash_size=sizeof(er9x_splash);
    }
  }
  if (splash_offset>0) {
    if (splash_type==5) {
      splash_width=SPLASHX9D_WIDTH;
      splash_height=SPLASHX9D_HEIGHT;
      splash_colors=16;
      splash_format=QImage::Format_Indexed8;
    } else {
      splash_width=SPLASH_WIDTH;
      splash_height=SPLASH_HEIGHT;
      splash_colors=1;
      splash_format=QImage::Format_Mono;
    }
  }
}

bool FlashInterface::setSplash(const QImage & newsplash)
{
  char b[SPLASH_SIZE_MAX] = {0};
  QColor color;
  QByteArray splash;
  if (splash_offset == 0) {
    return false;
  }
  else {
    if (splash_colors == 16) {
      unsigned int idx = 0;
      for (unsigned int y=0; y<splash_height; y+=8) {
        for (unsigned int x=0; x<splash_width; x++) {
          unsigned int values[] = { 255, 255, 255, 255 };
          for (unsigned int z=0; z<8; z++) {
            if (y+z < splash_height) {
              QRgb gray = qGray(newsplash.pixel(x, y+z));
              for (unsigned int i=0; i<4; i++) {
                if (gray & (1<<(4+i)))
                  values[i] -= 1 << z;
              }
            }
          }
          for (int i=0; i<4; i++)
            b[idx++] = values[i];
        }
      }
    }
    else {
      QColor black = QColor(0,0,0);
      QImage blackNwhite = newsplash.convertToFormat(QImage::Format_MonoLSB);
      for (uint y=0; y<splash_height; y++) {
        for (uint x=0; x<splash_width; x++) {
          color = QColor(blackNwhite.pixel(x,y));
          b[splash_width*(y/8) + x] |=((color==black ? 1: 0)<<(y % 8));
        }
      }
    }
    splash.clear();
    splash.append(b, splash_size);
    flash.replace(splash_offset, splash_size, splash);
    return true;
  }
}

int FlashInterface::getSplashWidth()
{
  return splash_width;
}

uint FlashInterface::getSplashHeight()
{
  return splash_height;
}

uint FlashInterface::getSplashColors()
{
  return splash_colors;
}

QImage::Format FlashInterface::getSplashFormat()
{
  return splash_format;
}

QImage FlashInterface::getSplash()
{
  if (splash_colors == 16) {
    QImage image(splash_width, splash_height, QImage::Format_RGB888);
    if (splash_offset > 0) {
      for (unsigned int y=0; y<splash_height; y++) {
        unsigned int idx = (y/8)*splash_width*4;
        unsigned int mask = (1 << (y%8));
        for (unsigned int x=0; x<splash_width; x++, idx+=4) {
          unsigned int z = ((flash.at(splash_offset+idx) & mask) ? 0x1 : 0) + ((flash.at(splash_offset+idx+1) & mask) ? 0x2 : 0) + ((flash.at(splash_offset+idx+2) & mask) ? 0x4 : 0) + ((flash.at(splash_offset+idx+3) & mask) ? 0x8 : 0);
          z = 255-(z*255)/15;
          QRgb rgb = qRgb(z, z, z);
          image.setPixel(x, y, rgb);
        }
      }
    }
    return image;
  }
  else {
    QImage image(splash_width, splash_height, QImage::Format_Mono);
    if (splash_offset > 0) {
      for (unsigned int y=0; y<splash_height; y++)
        for(unsigned int x=0; x<splash_width; x++)
          image.setPixel(x, y, (flash.at(splash_offset+(splash_width*(y/8)+x)) & (1<<(y % 8))) ? 0 : 1);
    }
    return image;
  }
}

bool FlashInterface::hasSplash()
{
  return (splash_offset > 0 ? true : false);
}

bool FlashInterface::isValid()
{
  return isValidFlag;
}


uint FlashInterface::saveFlash(QString fileName)
{
  uint8_t binflash[MAX_FSIZE];
  memcpy(&binflash, flash.constData(), flash_size);
  QFile file(fileName);
  
  int fileType = getFileType(fileName);

  if (fileType == FILE_TYPE_HEX) {
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) { //reading HEX TEXT file
      return -1;
    }
    QTextStream outputStream(&file);
    HexInterface hex=HexInterface(outputStream);
    hex.save(binflash, flash_size);
  }
  else {
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) { //reading HEX TEXT file
      return -1;
    }
    file.write((char*)binflash, flash_size);
  }

  file.close();

  return flash_size;
}
