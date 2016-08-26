#ifndef __METER_CRC__
#define __METER_CRC__

#include "base_type.h"


extern s32 Crc16(unsigned char *crc,unsigned char *puchMsg, unsigned long usDataLen);
extern s32 Crc16_HL(unsigned char *crc,unsigned char *puchMsg, unsigned long usDataLen);

extern s32 CHK_Crc16(u8 *crc,  u8 *puchMsg,  s32 usDataLen);
extern s32 CHK_Crc16HL(u8 *crc,  u8 *puchMsg,  s32 usDataLen);

#endif
