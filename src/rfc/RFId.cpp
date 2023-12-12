#include <SPI.h>
#include <MFRC522.h>

#include "RFId.hpp"

RFId::RFId()
{
    
}

void RFId::setup(const Settings &settings)
{
    SPI.begin();          // Init SPI bus
    Serial.printf("MRFId522_@(%d, %d)\n", settings.MRFC522_SS, settings.MRFC522_RST);
    this->mfrc522.PCD_Init(settings.MRFC522_SS, settings.MRFC522_RST);   // Init MFRC522 module
    this->mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
}

bool RFId::readCard(RFIdTag &tag) 
{
  static auto dump_byte_array = [](byte * buffer, byte bufferSize) -> void 
  {
    for (byte i = 0; i < bufferSize; i++) {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
    }
  };

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < MFRC522::MF_KEY_SIZE; ++i) 
    key.keyByte[i] = 0xFF;

  MFRC522::StatusCode status = MFRC522::StatusCode::STATUS_OK;
  byte blockAddr = 4;
  byte trailerBlock = 7;

  // Show some details of the PICC (that is: the tag/card)
  // Serial.print(F("Card UID:"));
  // dump_byte_array(this->mfrc522.uid.uidByte, this->mfrc522.uid.size);
  // Serial.println();
  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = this->mfrc522.PICC_GetType(this->mfrc522.uid.sak);
  // Serial.println(this->mfrc522.PICC_GetTypeName(piccType));

  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    Serial.println(F("Authenticating Classic using key A..."));
    status = this->mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
    Serial.println(F("Authenticating MIFARE UL..."));
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(this->mfrc522.GetStatusCodeName(status));    
    return false;
  }

  // Show the whole sector as it currently is
  // Serial.println(F("Current data in sector:"));
  // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  // Serial.println();

  // Read data from the block
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(this->mfrc522.GetStatusCodeName(status));
      this->mfrc522.PICC_HaltA(); this->mfrc522.PCD_StopCrypto1();
      return false;
    }
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[18];
    byte size2 = sizeof(buffer2);

    for (int i = 0; i < 4; ++i)
    {
      status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(8 + i, buffer2, &size2);
      if (status != MFRC522::STATUS_OK) 
      {
        Serial.printf("MIFARE_Read_%d() failed: %s\n", i, this->mfrc522.GetStatusCodeName(status));
        this->mfrc522.PICC_HaltA(); this->mfrc522.PCD_StopCrypto1();
        return false;
      }
      memcpy(buffer + (i * 4), buffer2, 4);
    }

    // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(8, buffer2, &size2);
    // if (status != MFRC522::STATUS_OK) {
    //   Serial.print(F("MIFARE_Read_1() failed: "));
    //   Serial.println(this->mfrc522.GetStatusCodeName(status));
    //   return false;
    // }
    // memcpy(buffer, buffer2, 4);

    // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(9, buffer2, &size2);
    // if (status != MFRC522::STATUS_OK) {
    //   Serial.print(F("MIFARE_Read_2() failed: "));
    //   Serial.println(this->mfrc522.GetStatusCodeName(status));
    //   return false;
    // }
    // memcpy(buffer + 4, buffer2, 4);

    // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(10, buffer2, &size2);
    // if (status != MFRC522::STATUS_OK) {
    //   Serial.print(F("MIFARE_Read_3() failed: "));
    //   Serial.println(this->mfrc522.GetStatusCodeName(status));
    //   return false;
    // }
    // memcpy(buffer + 8, buffer2, 4);

    // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Read(11, buffer2, &size2);
    // if (status != MFRC522::STATUS_OK) {
    //   Serial.print(F("MIFARE_Read_4() failed: "));
    //   Serial.println(this->mfrc522.GetStatusCodeName(status));
    //   return false;
    // }
    // memcpy(buffer + 12, buffer2, 4);
  }

  // Serial.print(F("Data on Card "));
  // Serial.println(F(":"));
  // dump_byte_array(buffer, 16);
  // Serial.println();
  // Serial.println();

  if (tag.read((const byte*)buffer, 16))
  {
    Serial.printf("%s@%d: Succeeded to read Card:\n", __PRETTY_FUNCTION__, __LINE__);
    // tag.dump();
  }
  else
    Serial.printf("%s@%d: Failed to read card...\n", __PRETTY_FUNCTION__, __LINE__);

//  tempCard.nfcFolderSettings.folder = buffer[5];
//  tempCard.nfcFolderSettings.mode = buffer[6];
//  tempCard.nfcFolderSettings.special = buffer[7];
//  tempCard.nfcFolderSettings.special2 = buffer[8];
//
//  if (tempCard.cookie == cardCookie) {
//
//    if (activeModifier != NULL && tempCard.nfcFolderSettings.folder != 0) {
//      if (activeModifier->handleRFID(&tempCard) == true) {
//        return false;
//      }
//    }
//
//    if (tempCard.nfcFolderSettings.folder == 0) {
//      if (activeModifier != NULL) {
//        if (activeModifier->getActive() == tempCard.nfcFolderSettings.mode) {
//          activeModifier = NULL;
//          Serial.println(F("modifier removed"));
//          if (isPlaying()) {
//            mp3.playAdvertisement(261);
//          }
//          else {
//            mp3.start();
//            delay(100);
//            mp3.playAdvertisement(261);
//            delay(100);
//            mp3.pause();
//          }
//          delay(2000);
//          return false;
//        }
//      }
//      if (tempCard.nfcFolderSettings.mode != 0 && tempCard.nfcFolderSettings.mode != 255) {
//        if (isPlaying()) {
//          mp3.playAdvertisement(260);
//        }
//        else {
//          mp3.start();
//          delay(100);
//          mp3.playAdvertisement(260);
//          delay(100);
//          mp3.pause();
//        }
//      }
//      switch (tempCard.nfcFolderSettings.mode ) {
//        case 0:
//        case 255:
//          mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); adminMenu(true);  break;
//        case 1: activeModifier = new SleepTimer(tempCard.nfcFolderSettings.special); break;
//        case 2: activeModifier = new FreezeDance(); break;
//        case 3: activeModifier = new Locked(); break;
//        case 4: activeModifier = new ToddlerMode(); break;
//        case 5: activeModifier = new KindergardenMode(); break;
//        case 6: activeModifier = new RepeatSingleModifier(); break;
//
//      }
//      delay(2000);
//      return false;
//    }
//    else {
//      memcpy(nfcTag, &tempCard, sizeof(RFIdTag));
//      Serial.println( nfcTag->nfcFolderSettings.folder);
//      myFolder = &nfcTag->nfcFolderSettings;
//      Serial.println( myFolder->folder);
//    }
//    return true;
//  }
//  else {
//    memcpy(nfcTag, &tempCard, sizeof(RFIdTag));
//    return true;
//  }
  this->mfrc522.PICC_HaltA(); this->mfrc522.PCD_StopCrypto1();
  return true;
}


bool RFId::writeCard(const RFIdTag &nfcTag, bool force) 
{
  bool retval = (bool)nfcTag || force;
  if (retval)
  {
    if (!(retval = this->mfrc522.PICC_IsNewCardPresent() && this->mfrc522.PICC_ReadCardSerial()))
    {
      mfrc522.PCD_Init();
      delay(40);
      retval = this->mfrc522.PICC_IsNewCardPresent() && this->mfrc522.PICC_ReadCardSerial();
    }
  
    if (retval)
    {
      byte buffer[16];
      bzero(buffer, 16);
      nfcTag.write(buffer, 16);
    
      MFRC522::MIFARE_Key key;
      for (byte i = 0; i < 6; i++) 
      {
        key.keyByte[i] = 0xFF;
      }
    
      static auto dump_byte_array = [](const byte *buffer, byte bufferSize) -> void 
        {
          for (byte i = 0; i < bufferSize; i++) {
            Serial.print(buffer[i] < 0x10 ? " 0" : " ");
            Serial.print(buffer[i], HEX);
          }
        };
        
      MFRC522::StatusCode status = MFRC522::StatusCode::STATUS_OK;
      byte sector = 1;
      byte blockAddr = 4;
      byte trailerBlock = 7;
      
      // byte size = sizeof(buffer);
  
      Serial.print(F("Card UID:"));
      dump_byte_array(this->mfrc522.uid.uidByte, this->mfrc522.uid.size);
      Serial.print(" (");
      Serial.print(this->mfrc522.uid.size);
      Serial.println(")");
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type mifareType = this->mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(this->mfrc522.PICC_GetTypeName(mifareType));
  
      // Authenticate using key A
      if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
          (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
          (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
      {
        Serial.println(F("Authenticating Classic using key A..."));
        status = this->mfrc522.PCD_Authenticate(
                   MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      }
      else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
      {
        byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard
    
        // Authenticate using key A
        Serial.println(F("Authenticating MIFARE UL..."));
        status = this->mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
      }
    
      if (status != MFRC522::STATUS_OK) 
      {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(this->mfrc522.GetStatusCodeName(status));
        return false;
      }
    
      // Show the whole sector as it currently is
      Serial.println(F("Current data in sector:"));
      this->mfrc522.PICC_DumpMifareClassicSectorToSerial(&(this->mfrc522.uid), &key, sector);
      Serial.println();
     
      // Write data to the block
      Serial.print(F("Writing data into block "));
      Serial.print(blockAddr);
      Serial.println(F(" ..."));
      dump_byte_array(buffer, 16);
      Serial.println();
    
      if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
          (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
          (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
      {
        status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(blockAddr, buffer, 16);
      }
      else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
      {
        byte buffer2[16];
        byte size2 = sizeof(buffer2);

        for (int i = 0; i < 4; ++i)
        {
          memset(buffer2, 0, size2);
          memcpy(buffer2, buffer + (i * 4), 4);
          status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(8 + i, buffer2, 16);
        }   

        // memset(buffer2, 0, size2);
        // memcpy(buffer2, buffer, 4);
        // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(8, buffer2, 16);
    
        // memset(buffer2, 0, size2);
        // memcpy(buffer2, buffer + 4, 4);
        // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(9, buffer2, 16);
    
        // memset(buffer2, 0, size2);
        // memcpy(buffer2, buffer + 8, 4);
        // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(10, buffer2, 16);
    
        // memset(buffer2, 0, size2);
        // memcpy(buffer2, buffer + 12, 4);
        // status = (MFRC522::StatusCode)this->mfrc522.MIFARE_Write(11, buffer2, 16);
      }
    
      if (status != MFRC522::STATUS_OK) 
      {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(this->mfrc522.GetStatusCodeName(status));
      }
    
      retval = (status == MFRC522::STATUS_OK);
      this->mfrc522.PICC_HaltA(); this->mfrc522.PCD_StopCrypto1();
    }  
  }
  return retval;
}

bool RFId::clearCard()
{
  return this->writeCard(RFIdTag {}, true);
}

bool RFId::checkCardPresent()
{
  static unsigned long lastCall = 0;
  static bool lastResult = true;

  bool retval = true;
  
  if (millis() - lastCall > 100)
  {
//    byte bufferATQA[10];
//    byte bufferSize = sizeof(bufferATQA);
    byte buffer[2];
    byte bufferSize = sizeof(buffer);
    bool currentResult = (this->mfrc522.PICC_REQA_or_WUPA(this->mfrc522.PICC_CMD_WUPA, buffer, &bufferSize) == mfrc522.STATUS_OK);
    //bool currentResult = mfrc522.PICC_WakeupA(bufferATQA, &bufferSize) == mfrc522.STATUS_OK;
    // Serial.printf("%s: lastResult=%d currentResult=%d\n", __PRETTY_FUNCTION__, lastResult, currentResult);
    retval = lastResult || currentResult;
    lastCall = millis();
    lastResult = currentResult;
  }
  if (!retval)
    lastResult = true;
  this->mfrc522.PICC_HaltA();

  return retval;
}

// bool RFId::waitCard(int timeout, Menu *menu, const String &title, uint16_t glyph, const String &text)
// {
//   bool retval = false;

//   byte buffer[2];
//   byte bufferSize = sizeof(buffer);

//   int textOffset = 0;
//   int textWidth = 0;
//   int lastValue = -1;
  
//   for (uint32_t start = millis(), current = start; !retval && current - start < (timeout * 1000); current = millis())
//   // for (uint32_t start = millis(), current = start, end = start + timeout * 1000; !retval && current < end; current = millis())
//   {
//     if ((retval = (mfrc522.PICC_REQA_or_WUPA(mfrc522.PICC_CMD_WUPA, buffer, &(bufferSize = sizeof(buffer))) == mfrc522.STATUS_OK)))
// //    if ((retval = mfrc522.PICC_IsNewCardPresent()))
//     {
//       continue;
//     }
      
//     if (menu && textOffset == 0)
//     {
//       menu->u8g2.clearBuffer();
//       menu->drawTitle(title.c_str(), glyph);
//       menu->u8g2.setFont(u8g2_font_6x12_tf);
//       int titleWidth = 0;
//       if (text.length() > 0)
//       {
//         titleWidth = menu->u8g2.getUTF8Width(text.c_str());
//         textOffset = titleWidth + 5;
//       }
//       textOffset += (textWidth = menu->u8g2.getUTF8Width(String(timeout).c_str()));
//       textOffset = 64 - (textOffset / 2);
//       menu->u8g2.drawUTF8(textOffset, 30, text.c_str());
//       if (text.length() > 0)
//       {
//         textOffset += titleWidth;
//         textOffset += 5;
//       }
//     }
//     if (textOffset > 0)
//     {
//       int currentValue = ((current - start) / 1000);
//       if (currentValue != lastValue)
//       {
//         menu->u8g2.setDrawColor(0);
//         menu->u8g2.drawBox(textOffset, 20, textOffset + textWidth, 30);
//         menu->u8g2.setDrawColor(1);

//         menu->u8g2.drawUTF8(textOffset, 30, String(timeout - currentValue).c_str());
//         currentValue = lastValue;
//         menu->u8g2.sendBuffer();
//       }    
//     }
//     delay(100);
//   }
//   return retval;
// }

bool RFId::waitCard(int timeout, ProgressListener progressListener)
{
  bool retval = false;

  byte buffer[2];
  byte bufferSize = sizeof(buffer);

  int lastValue = -1;
  for (uint32_t start = millis(), current = start; !retval && ((current - start) < (timeout * 1000)); current = millis())
  {
    if ((retval = (this->mfrc522.PICC_REQA_or_WUPA(mfrc522.PICC_CMD_WUPA, buffer, &(bufferSize = sizeof(buffer))) == mfrc522.STATUS_OK)))
      continue;
      
    int currentValue = ((current - start) / 1000);
    if (currentValue != lastValue)
    {
      lastValue = currentValue;
      if (progressListener)
        progressListener(currentValue);
    }
    else
    if (lastValue < 0 && progressListener)
    {
      progressListener(0);
      lastValue = 0;
    }    
    delay(250);
  }
  return retval;
}

// bool RFId::waitCardLeft(int timeout, Menu *menu, const String &title, uint16_t glyph, const String &text)
// {
//   bool retval = false;

//   // byte buffer[2];
//   // byte bufferSize = sizeof(buffer);

//   int textOffset = 0;
//   int textWidth = 0;
//   int lastValue = -1;

//   for (uint32_t start = millis(), current = start; !retval && (!timeout || ((current - start) < (timeout * 1000))); current = millis())
//   // for (uint32_t start = millis(), current = start, end = start + timeout * 1000; !retval && (!timeout || current < end); current = millis())
//   {
//     if ((retval = !checkCardPresent()))
//       continue;
      
//     if (menu && textOffset == 0)
//     {
//       menu->drawTitle(title.c_str(), glyph);
//       menu->tft
//       menu->u8g2.setFont(u8g2_font_6x12_tf);
//       int titleWidth = 0;
//       if (text.length() > 0)
//       {
//         titleWidth = menu->u8g2.getUTF8Width(text.c_str());
//         textOffset = titleWidth + (timeout > 0 ? 5 : 0);
//       }
//       textOffset += (textWidth = menu->u8g2.getUTF8Width(String(timeout).c_str()));
//       textOffset = 64 - (textOffset / 2);
//       menu->u8g2.drawUTF8(textOffset, 30, text.c_str());
//       if (text.length() > 0)
//       {
//         textOffset += titleWidth;
//         if (timeout)
//           textOffset += 5;
//       }
//       if (!timeout)
//         menu->u8g2.sendBuffer();
//     }
//     if (timeout && textOffset > 0)
//     {
//       int currentValue = ((current - start) / 1000);
//       if (currentValue != lastValue)
//       {
//         menu->u8g2.setDrawColor(0);
//         menu->u8g2.drawBox(textOffset, 20, textOffset + textWidth, 30);
//         menu->u8g2.setDrawColor(1);

//         menu->u8g2.drawUTF8(textOffset, 30, String(timeout - currentValue).c_str());
//         currentValue = lastValue;
//         menu->u8g2.sendBuffer();
//       }

//     }
//     delay(100);
//   }
//   return retval;
// }

bool RFId::waitCardLeft(int timeout, ProgressListener progressListener)
{
  bool retval = false;

  int lastValue = -1;
  for (uint32_t start = millis(), current = start; !retval && (!timeout || ((current - start) < (timeout * 1000))); current = millis())
  {
    if ((retval = !checkCardPresent()))
      continue;
      
    int currentValue = ((current - start) / 1000);
    if (timeout && progressListener)
    {
      if (currentValue != lastValue)
      {
        lastValue = currentValue;
        if (progressListener)
          progressListener(currentValue);
      }
    }
    else
    if (lastValue < 0 && progressListener)
    {
      progressListener(0);
      lastValue = 0;
    }

    delay(250);
  }
  return retval;
}