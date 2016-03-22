#ifndef KAWASHIMA_KSAPI_H
#define KAWASHIMA_KSAPI_H

#define BUILDING_DLL

#include "kstypedef.h"
#include "hca_info.h"

typedef void *HKDECODE;

typedef enum {
    KS_PARAM_BUFFER_SIZE,
    KS_PARAM_KEY1,
    KS_PARAM_KEY2,
    KS_PARAM_KEY,
    KS_PARAM_FORCE_DWORD = (uint32)0xffffffff
} KS_PARAM_TYPE;

typedef enum {
    KS_OP_HAS_MORE_DATA = 1,
    KS_ERR_OK = 0,
    KS_ERR_INVALID_HANDLE = (int32)-1,
    KS_ERR_MAGIC_NOT_MATCH = (int32)-2,
    KS_ERR_ALREADY_CLOSED = (int32)-3,
    KS_ERR_INVALID_PARAMETER = (int32)-4,
    KS_ERR_INVALID_STAGE = (int32)-5,
    KS_ERR_FILE_OP_FAILED = (int32)-6,
    KS_ERR_INVALID_FILE_PROP = (int32)-7,
    KS_ERR_INVALID_OPERATION = (int32)-8,
    KS_ERR_DECODE_FAILED = (int32)-9,
    KS_ERR_BUFFER_TOO_SMALL = (int32)-10,
    KS_ERR_CHECKSUM_NOT_MATCH = (int32)-11,
    KS_ERR_INVALID_INTERNAL_STATE = (int32)-12,
    KS_ERR_ATH_INIT_FAILED = (int32)-13,
    KS_ERR_CIPH_INIT_FAILED = (int32)-14,
    KS_ERR_STATE_OUT_OF_RANGE = (int32)-15,
    KS_ERR_NOT_IMPLEMENTED = (int32)-16,
    KS_ERR_DECODE_ALREADY_COMPLETED = (int32)-17,
    KS_ERR_FORCE_DWORD = (int32)0xffffffff
} KS_RESULT;

#define KS_CALL_SUCCESSFUL(x) ((ubool)(((int32)x) >= 0))

KS_API KS_RESULT KsOpenFile(const char *pFileName, HKDECODE *ppHandle);
KS_API KS_RESULT KsOpenBuffer(uint8 *pData, uint32 dwDataSize, ubool bClone, HKDECODE *ppHandle);
KS_API KS_RESULT KsSetParamI32(HKDECODE hDecode, KS_PARAM_TYPE dwParamType, uint32 dwParam);
KS_API KS_RESULT KsSetParamI64(HKDECODE hDecode, KS_PARAM_TYPE dwParamType, uint64 qwParam);
KS_API KS_RESULT KsBeginDecode(HKDECODE hDecode);
KS_API KS_RESULT KsGetWaveHeader(HKDECODE hDecode, uint8 *pBuffer, uint32 *pdwDataSize);
KS_API KS_RESULT KsDecodeData(HKDECODE hDecode, uint8 *pBuffer, uint32 *pdwDataSize);
KS_API KS_RESULT KsEndDecode(HKDECODE hDecode);
KS_API KS_RESULT KsCloseHandle(HKDECODE hDecode);
KS_API KS_RESULT KsGetHcaInfo(HKDECODE hDecode, HCA_INFO *pInfo);
KS_API ubool KsIsActiveHandle(HKDECODE hDecode);
KS_API ubool KsIsHcaCheckPassed(HKDECODE hDecode);
KS_API KS_RESULT KsHasMoreData(HKDECODE hDecode, ubool *pbHasMore);
KS_API void KsTest();

#endif //KAWASHIMA_API_H
