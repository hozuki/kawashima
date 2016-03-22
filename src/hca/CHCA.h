#ifndef KAWASHIMA_HCA_H
#define KAWASHIMA_HCA_H

#include "../kstypedef.h"
#include "./internal/CHcaAth.h"
#include "./internal/CHcaCipher.h"
#include "./internal/CHcaData.h"
#include "internal/HcaChannel.h"
#include "../ksapi.h"

typedef struct _KS_DECODE_STATUS KS_DECODE_STATUS;

class CHCA {

public:

    typedef void (*DecodeFunc)(float data, uint8 *buffer, uint32 *cursor);

    CHCA(uint32 ciphKey1, uint32 ciphKey2);

    static uint16 Checksum(void *data, uint32 size, uint16 sum);

    KS_RESULT ReadHeader(uint8 *data, uint32 size, KS_DECODE_STATUS *status);

    KS_RESULT GetWaveHeader(uint8 *pBuffer, uint32 *pdwDataSize);

    KS_RESULT DecodeData(uint8 *pData, uint32 dwDataSize, KS_DECODE_STATUS *status, uint8 *pWaveData,
                         uint32 *pdwWaveDataSize, ubool *pbHasMore);

    KS_RESULT GenerateWaveDataBlock(uint8 *pData, uint32 dwDataSize, uint32 *pDataCursor, uint8 *pBuffer,
                                    uint32 *pBufferCursor, DecodeFunc pfnDecodeFunc);

    KS_RESULT DecodeBlock(uint8 *pData, uint32 dwDataSize, uint32 *pDataCursor);

    const HCA_INFO *GetInfo();

private:

    uint32 _ciph_key1;
    uint32 _ciph_key2;

    CHcaAth _ath;
    CHcaCipher _cipher;
    HcaChannel _channels[0x10];
    HCA_INFO hcaInfo;

};

#endif
