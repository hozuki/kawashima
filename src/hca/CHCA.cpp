//--------------------------------------------------
// インクルード
//--------------------------------------------------
#include <stdio.h>
#include <memory.h>
#include "CHCA.h"
#include "HcaNative.h"
#include "../wave/WaveNative.h"
#include "../api/ks_decode.h"
#include "Magic.h"

static const uint32 WAVE_BIT_PER_CHANNEL = 16;
static const ubool WAVE_USER_LOOP = FALSE;

void Decode16Bit(float data, uint8 *buffer, uint32 *cursor) {
    int16 i = (int16)(data * 0x7fff);
    *(int16 *)(buffer + *cursor) = i;
    *cursor += 2;
}

template<typename T>
T clamp(T value, T min, T max) {
    return value < min ? min : (value > max ? max : value);
}

inline int16 bswap(int16 v) {
    int16 r = (int16)(v & 0xFF);
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline uint16 bswap(uint16 v) {
    uint16 r = (uint16)(v & 0xFF);
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline int32 bswap(int32 v) {
    int32 r = v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline uint32 bswap(uint32 v) {
    uint32 r = v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline int64 bswap(int64 v) {
    int64 r = v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline uint64 bswap(uint64 v) {
    uint64 r = v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    r <<= 8;
    v >>= 8;
    r |= v & 0xFF;
    return r;
}

inline float bswap(float v) {
    uint32 i = bswap(*(uint32 *)&v);
    return *(float *)&i;
}

inline uint32 ceil2(uint32 a, uint32 b) {
    return (b > 0) ? (a / b + ((a % b) ? 1 : 0)) : 0;
}

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
CHCA::CHCA(uint32 ciphKey1, uint32 ciphKey2) :
        _ath(), _cipher() {
    _ciph_key1 = ciphKey1;
    _ciph_key2 = ciphKey2;
    memset(&hcaInfo, 0, sizeof(HCA_INFO));
}

const HCA_INFO *CHCA::GetInfo() {
    return &hcaInfo;
}

//--------------------------------------------------
// チェックサム
//--------------------------------------------------
uint16 CHCA::Checksum(void *data, uint32 size, uint16 sum) {
    static uint16 checksumTable[] = {
            0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
            0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
            0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
            0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
            0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
            0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
            0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
            0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
            0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
            0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
            0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
            0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
            0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
            0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
            0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
            0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
            0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
            0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
            0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
            0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
            0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
            0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
            0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
            0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
            0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
            0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
            0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
            0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
            0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
            0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
            0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
            0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202,
    };
    for (uint8 *s = (uint8 *)data, *e = s + size; s < e; s++) {
        sum = (sum << 8) ^ checksumTable[(sum >> 8) ^ *s];
    }
    return sum;
}

KS_RESULT CHCA::ReadHeader(uint8 *data, uint32 size, KS_DECODE_STATUS *status) {
    if (!data || !status) {
        return KS_ERR_INVALID_PARAMETER;
    }
    if (size < sizeof(HCA_HEADER)) {
        return KS_ERR_INVALID_PARAMETER;
    }
    uint32 cursor = status->dataCursor;

    // Check HCA header (of the whole file).
    HCA_HEADER *hca = (HCA_HEADER *)(data + cursor);
    if (!Magic::Match(hca->hca, Magic::HCA)) {
        return KS_ERR_MAGIC_NOT_MATCH;
    }
    // The file must be bigger than a raw HCA header.
    uint32 dataOffset = bswap(hca->dataOffset);
    if (size < dataOffset) {
        return KS_ERR_INVALID_PARAMETER;
    }
    uint16 fileVersion = bswap(hca->version);
    hcaInfo.versionMajor = (uint16)(fileVersion >> 8);
    hcaInfo.versionMinor = (uint16)(fileVersion & 0xff);
    if (Checksum(hca, dataOffset, 0) != 0) {
        // File may be broken.
        return KS_ERR_CHECKSUM_NOT_MATCH;
    }
    cursor += sizeof(HCA_HEADER);

    // FMT
    HCA_FORMAT *fmt = (HCA_FORMAT *)(data + cursor);
    if (!Magic::Match(fmt->fmt, Magic::FORMAT)) {
        return KS_ERR_MAGIC_NOT_MATCH;
    }
    hcaInfo.channelCount = fmt->channelCount;
    hcaInfo.samplingRate = bswap((uint32)(fmt->samplingRate << 8));
    hcaInfo.blockCount = bswap(fmt->blockCount);
    hcaInfo.fmtR01 = bswap(fmt->r01);
    hcaInfo.fmtR02 = bswap(fmt->r02);
    if (!(hcaInfo.channelCount >= 1 && hcaInfo.channelCount <= 16)) {
        return KS_ERR_INVALID_INTERNAL_STATE;
    }
    if (!(hcaInfo.samplingRate >= 1 && hcaInfo.samplingRate <= 0x7fffff)) {
        return KS_ERR_INVALID_INTERNAL_STATE;
    }
    cursor += sizeof(HCA_FORMAT);

    // COMP or DEC
    if (Magic::Match(*((uint32 *)(data + cursor)), Magic::COMPRESS)) {
        HCA_COMPRESS *comp = (HCA_COMPRESS *)(data + cursor);
        hcaInfo.blockSize = bswap(comp->blockSize);
        hcaInfo.compR01 = comp->r01;
        hcaInfo.compR02 = comp->r02;
        hcaInfo.compR03 = comp->r03;
        hcaInfo.compR04 = comp->r04;
        hcaInfo.compR05 = comp->r05;
        hcaInfo.compR06 = comp->r06;
        hcaInfo.compR07 = comp->r07;
        hcaInfo.compR08 = comp->r08;
        if (!((hcaInfo.blockSize >= 8 && hcaInfo.blockSize <= 0xFFFF) || (hcaInfo.blockSize == 0))) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        if (!(hcaInfo.compR01 >= 0 && hcaInfo.compR01 <= hcaInfo.compR02 && hcaInfo.compR02 <= 0x1f)) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        cursor += sizeof(HCA_COMPRESS);
    } else if (Magic::Match(*((uint32 *)(data + cursor)), Magic::DECODE)) {
        HCA_DECODE *dec = (HCA_DECODE *)(data + cursor);
        hcaInfo.blockSize = bswap(dec->blockSize);
        hcaInfo.compR01 = dec->r01;
        hcaInfo.compR02 = dec->r02;
        hcaInfo.compR03 = dec->r04;
        hcaInfo.compR04 = dec->r03;
        hcaInfo.compR05 = (uint16)(dec->count1 + 1);
        hcaInfo.compR06 = (uint16)((dec->enableCount2 ? dec->count2 : dec->count1) + 1);
        hcaInfo.compR07 = hcaInfo.compR05 - hcaInfo.compR06;
        hcaInfo.compR08 = 0;
        if (!((hcaInfo.blockSize >= 8 && hcaInfo.blockSize <= 0xFFFF) || (hcaInfo.blockSize == 0))) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        if (!(hcaInfo.compR01 >= 0 && hcaInfo.compR01 <= hcaInfo.compR02 && hcaInfo.compR02 <= 0x1f)) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        if (!hcaInfo.compR03) {
            hcaInfo.compR03 = 1;
        }
        cursor += sizeof(HCA_DECODE);
    } else {
        return KS_ERR_MAGIC_NOT_MATCH;
    }

    // VBR
    HCA_VBR *vbr = (HCA_VBR *)(data + cursor);
    if (Magic::Match(vbr->vbr, Magic::VBR)) {
        hcaInfo.vbrR01 = bswap(vbr->r01);
        hcaInfo.vbrR02 = bswap(vbr->r02);
        cursor += sizeof(HCA_VBR);
    } else {
        hcaInfo.vbrR01 = hcaInfo.vbrR02 = 0;
    }

    // ATH
    HCA_ATH *ath = (HCA_ATH *)(data + cursor);
    if (Magic::Match(ath->ath, Magic::ATH)) {
        hcaInfo.athType = ath->type;
        cursor += sizeof(HCA_ATH);
    } else {
        hcaInfo.athType = (uint16)(hcaInfo.versionMajor < 2 ? 1 : 0);
    }

    // LOOP
    HCA_LOOP *loop = (HCA_LOOP *)(data + cursor);
    if (Magic::Match(loop->loop, Magic::LOOP)) {
        hcaInfo.loopExists = TRUE;
        hcaInfo.loopStart = bswap(loop->loopStart);
        hcaInfo.loopEnd = bswap(loop->loopEnd);
        hcaInfo.loopR01 = bswap(loop->r01);
        hcaInfo.loopR02 = bswap(loop->r02);
        if (!(0 <= hcaInfo.loopStart && hcaInfo.loopStart <= hcaInfo.loopEnd &&
              hcaInfo.loopEnd < hcaInfo.blockCount)) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        cursor += sizeof(HCA_LOOP);
    } else {
        hcaInfo.loopStart = hcaInfo.loopEnd = 0;
        hcaInfo.loopR01 = 0;
        hcaInfo.loopR02 = 0x400;
        hcaInfo.loopExists = FALSE;
    }

    // CIPH
    HCA_CIPHER *ciph = (HCA_CIPHER *)(data + cursor);
    if (Magic::Match(ciph->ciph, Magic::CIPHER)) {
        hcaInfo.cipherType = (HCA_CIPHER_TYPE)bswap(ciph->type);
        if (!(hcaInfo.cipherType == HCA_CIPHER_TYPE_NO_CIPHER || hcaInfo.cipherType == HCA_CIPHER_TYPE_STATIC ||
              hcaInfo.cipherType == HCA_CIPHER_TYPE_WITH_KEY)) {
            return KS_ERR_INVALID_INTERNAL_STATE;
        }
        cursor += sizeof(HCA_CIPHER);
    } else {
        hcaInfo.cipherType = HCA_CIPHER_TYPE_NO_CIPHER;
    }

    // RVA
    HCA_RVA *rva = (HCA_RVA *)(data + cursor);
    if (Magic::Match(rva->rva, Magic::RVA)) {
        hcaInfo.rvaVolume = bswap(rva->volume);
        cursor += sizeof(HCA_RVA);
    } else {
        hcaInfo.rvaVolume = 1.0f;
    }

    // COMM
    HCA_COMMENT *comment = (HCA_COMMENT *)(data + cursor);
    if (Magic::Match(comment->comm, Magic::COMMENT)) {
        hcaInfo.commentLength = comment->len;
        memset(hcaInfo.comment, 0, 0x100);
        strcpy(hcaInfo.comment, comment->comment);

    } else {
        hcaInfo.commentLength = 0;
        memset(hcaInfo.comment, 0, 0x100);
    }

    // Initialize adjustment and cipher tables.
    if (!_ath.Init(hcaInfo.athType, hcaInfo.samplingRate)) {
        return KS_ERR_ATH_INIT_FAILED;
    }
    if (!_cipher.Init((HCA_CIPHER_TYPE)hcaInfo.cipherType, _ciph_key1, _ciph_key2)) {
        return KS_ERR_CIPH_INIT_FAILED;
    }

    if (!hcaInfo.compR03) {
        hcaInfo.compR03 = 1;
    }

    // Prepare the channel decoders.
    memset(_channels, 0, sizeof(_channels));
    if (!(hcaInfo.compR01 == 1 && hcaInfo.compR02 == 0xf)) {
        return KS_ERR_INVALID_INTERNAL_STATE;
    }
    hcaInfo.compR09 = ceil2(hcaInfo.compR05 - (hcaInfo.compR06 + hcaInfo.compR07), hcaInfo.compR08);
    uint8 r[0x10];
    memset(r, 0, 0x10);
    uint32 b = hcaInfo.channelCount / hcaInfo.compR03;
    if (hcaInfo.compR07 && b > 1) {
        uint8 *c = r;
        for (auto i = 0; i < hcaInfo.compR03; ++i, c += b) {
            switch (b) {
                case 2:
                case 3:
                    c[0] = 1;
                    c[1] = 2;
                    break;
                case 4:
                    c[0] = 1;
                    c[1] = 2;
                    if (hcaInfo.compR04 == 0) {
                        c[2] = 1;
                        c[3] = 2;
                    }
                    break;
                case 5:
                    c[0] = 1;
                    c[1] = 2;
                    if (hcaInfo.compR04 <= 2) {
                        c[3] = 1;
                        c[4] = 2;
                    }
                    break;
                case 6:
                case 7:
                    c[0] = 1;
                    c[1] = 2;
                    c[4] = 1;
                    c[5] = 2;
                    // Fall through
                case 8:
                    c[6] = 1;
                    c[7] = 2;
                    break;
                default:
                    return KS_ERR_STATE_OUT_OF_RANGE;
            }
        }
    }
    for (auto i = 0; i < hcaInfo.channelCount; ++i) {
        _channels[i].type = r[i];
        _channels[i].value3 = &_channels[i].value[hcaInfo.compR06 + hcaInfo.compR07];
        _channels[i].count = hcaInfo.compR06 + ((r[i] != 2) ? hcaInfo.compR07 : 0);
    }

    status->dataCursor = dataOffset;
    return KS_ERR_OK;
}

KS_RESULT CHCA::GetWaveHeader(uint8 *pBuffer, uint32 *pdwDataSize) {
    if (!pdwDataSize) {
        return KS_ERR_INVALID_PARAMETER;
    }

    WaveRiffSection wavRiff = {'R', 'I', 'F', 'F', 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 0x10, 0, 0, 0, 0, 0, 0};
    WaveSampleSection wavSmpl = {'s', 'm', 'p', 'l', 0x3C, 0, 0, 0, 0x3C, 0, 0, 0, 1, 0x18, 0, 0, 0, 0, 0, 0};
    WaveNoteSection wavNote = {'n', 'o', 't', 'e', 0, 0};
    WaveDataSection wavData = {'d', 'a', 't', 'a', 0};
    wavRiff.fmtType = (uint16)((WAVE_BIT_PER_CHANNEL > 0) ? 1 : 3);
    wavRiff.fmtChannelCount = (uint16)hcaInfo.channelCount;
    wavRiff.fmtBitCount = (uint16)((WAVE_BIT_PER_CHANNEL > 0) ? WAVE_BIT_PER_CHANNEL : 32);
    wavRiff.fmtSamplingRate = hcaInfo.samplingRate;
    wavRiff.fmtSamplingSize = (uint16)(wavRiff.fmtBitCount / 8 * wavRiff.fmtChannelCount);
    wavRiff.fmtSamplesPerSec = wavRiff.fmtSamplingRate * wavRiff.fmtSamplingSize;
    if (hcaInfo.loopExists) {
        wavSmpl.samplePeriod = (uint32)(1 / (double)wavRiff.fmtSamplingRate * 1000000000);
        wavSmpl.loopStart = hcaInfo.loopStart * 0x80 * 8 * wavRiff.fmtSamplingSize;
        wavSmpl.loopEnd = hcaInfo.loopEnd * 0x80 * 8 * wavRiff.fmtSamplingSize;
        wavSmpl.loopPlayCount = (hcaInfo.loopR01 == 0x80) ? 0 : hcaInfo.loopR01;
    } else if (WAVE_USER_LOOP) {
        wavSmpl.loopStart = 0;
        wavSmpl.loopEnd = hcaInfo.blockCount * 0x80 * 8 * wavRiff.fmtSamplingSize;
        hcaInfo.loopStart = 0;
        hcaInfo.loopEnd = hcaInfo.blockCount;
    }
    if (hcaInfo.commentLength > 0) {
        wavNote.noteSize = 4 + hcaInfo.commentLength + 1;
        if (wavNote.noteSize & 3) {
            wavNote.noteSize += 4 - (wavNote.noteSize & 3);
        }
    }
    wavData.dataSize = hcaInfo.blockCount * 0x80 * 8 * wavRiff.fmtSamplingSize +
                       (wavSmpl.loopEnd - wavSmpl.loopStart) * WAVE_USER_LOOP;
    wavRiff.riffSize = (uint32)(0x1C + ((hcaInfo.loopExists && !WAVE_USER_LOOP) ? sizeof(wavSmpl) : 0) +
                                (hcaInfo.commentLength > 0 ? 8 + wavNote.noteSize : 0) + sizeof(wavData) +
                                wavData.dataSize);

    uint32 sizeNeeded = (uint32)sizeof(WaveRiffSection);
    if (hcaInfo.loopExists && !WAVE_USER_LOOP) {
        sizeNeeded += sizeof(WaveSampleSection);
    }
    if (hcaInfo.commentLength > 0) {
        sizeNeeded += 8 + wavNote.noteSize;
    }
    sizeNeeded += sizeof(WaveDataSection);

    // Only computes the needed size of the buffer, and returns.
    if (pBuffer == NULL) {
        *pdwDataSize = sizeNeeded;
        return KS_ERR_OK;
    } else {
        ubool bufferLargeEnough = (ubool)(*pdwDataSize >= sizeNeeded);
        if (bufferLargeEnough) {
            uint32 cursor = 0;
#define WRITE_STRUCT(src, size) memcpy(pBuffer + cursor, src, size); cursor += size
            WRITE_STRUCT(&wavRiff, sizeof(WaveRiffSection));
            if (hcaInfo.loopExists && !WAVE_USER_LOOP) {
                WRITE_STRUCT(&wavSmpl, sizeof(WaveSampleSection));
            }
            if (hcaInfo.commentLength > 0) {
                uint32 address = cursor;
                WRITE_STRUCT(&wavNote, sizeof(WaveNoteSection));
                strcpy((char *)(pBuffer + cursor), hcaInfo.comment);
                pBuffer[cursor + hcaInfo.commentLength] = '\0';
                cursor = address + 8 + wavNote.noteSize;
            }
            WRITE_STRUCT(&wavData, sizeof(WaveDataSection));
#undef WRITE_STRUCT
        }
        *pdwDataSize = sizeNeeded;
        return bufferLargeEnough ? KS_ERR_OK : KS_ERR_BUFFER_TOO_SMALL;
    }

    return KS_ERR_OK;
}

KS_RESULT CHCA::DecodeData(uint8 *pData, uint32 dwDataSize, KS_DECODE_STATUS *status, uint8 *pWaveData,
                           uint32 *pdwWaveDataSize, ubool *pbHasMore) {
    if (!pData || !status || !pdwWaveDataSize || !pbHasMore) {
        return KS_ERR_INVALID_PARAMETER;
    }
    uint32 waveBlockSize = 8 * 0x80 * (WAVE_BIT_PER_CHANNEL / sizeof(uint8)) * hcaInfo.channelCount;

    // The consumer just want to check the minimum acceptable buffer size.
    if (pWaveData == NULL) {
        *pdwWaveDataSize = waveBlockSize;
        *pbHasMore = FALSE;
        return KS_ERR_OK;
    }

    if (*pdwWaveDataSize <= waveBlockSize) {
        return KS_ERR_BUFFER_TOO_SMALL;
    }
    if (status->blockIndex >= hcaInfo.blockCount) {
        return KS_ERR_DECODE_ALREADY_COMPLETED;
    }
    uint32 blocksProcessableThisRound = *pdwWaveDataSize / waveBlockSize;

    if (!WAVE_USER_LOOP && !hcaInfo.loopExists) {
        uint32 bufferCursor = 0;
        if (status->blockIndex + blocksProcessableThisRound >= hcaInfo.blockCount) {
            blocksProcessableThisRound = hcaInfo.blockCount - status->blockIndex;
            *pbHasMore = FALSE;
        } else {
            *pbHasMore = TRUE;
        }
        KS_RESULT result;
        for (auto i = 0; i < blocksProcessableThisRound; ++i) {
            result = GenerateWaveDataBlock(pData, hcaInfo.blockSize, &status->dataCursor, pWaveData, &bufferCursor,
                                           Decode16Bit);
            if (!KS_CALL_SUCCESSFUL(result)) {
                return result;
            }
        }
        status->blockIndex += blocksProcessableThisRound;
        *pdwWaveDataSize = bufferCursor;
    } else {
        return KS_ERR_NOT_IMPLEMENTED;
    }

    return KS_ERR_OK;
}

KS_RESULT CHCA::GenerateWaveDataBlock(uint8 *pData, uint32 dwDataSize, uint32 *pDataCursor, uint8 *pBuffer,
                                      uint32 *pBufferCursor, DecodeFunc pfnDecodeFunc) {
    if (!pData || !pDataCursor || !pBuffer || !pBufferCursor || !pfnDecodeFunc) {
        return KS_ERR_INVALID_PARAMETER;
    }
    KS_RESULT result = DecodeBlock(pData, dwDataSize, pDataCursor);
    if (!KS_CALL_SUCCESSFUL(result)) {
        return result;
    }
    float f;
    for (auto i = 0; i < 8; ++i) {
        for (auto j = 0; j < 0x80; ++j) {
            for (auto k = 0; k < hcaInfo.channelCount; ++k) {
                f = _channels[k].wave[i][j] * hcaInfo.rvaVolume;
                f = clamp(f, -1.0f, 1.0f);
                pfnDecodeFunc(f, pBuffer, pBufferCursor);
            }
        }
    }
    return KS_ERR_OK;
}

KS_RESULT CHCA::DecodeBlock(uint8 *pData, uint32 dwDataSize, uint32 *pDataCursor) {
    if (!pData || !pDataCursor) {
        return KS_ERR_INVALID_PARAMETER;
    }
    if (dwDataSize != hcaInfo.blockSize) {
        return KS_ERR_INVALID_PARAMETER;
    }
    if (Checksum(pData + *pDataCursor, dwDataSize, 0) != 0) {
        return KS_ERR_CHECKSUM_NOT_MATCH;
    }
    _cipher.Mask(pData + *pDataCursor, dwDataSize);
    CHcaData data(pData + *pDataCursor, dwDataSize);
    int32 magic = data.GetBit(16);
    if (magic == 0xffff) {
        int32 a = (data.GetBit(9) << 8) - data.GetBit(7);
        for (auto i = 0; i < hcaInfo.channelCount; ++i) {
            _channels[i].Decode1(&data, hcaInfo.compR09, a, _ath.GetTable());
        }
        for (auto i = 0; i < 8; ++i) {
            for (auto j = 0; j < hcaInfo.channelCount; ++j) {
                _channels[j].Decode2(&data);
            }
            for (auto j = 0; j < hcaInfo.channelCount; ++j) {
                _channels[j].Decode3(hcaInfo.compR09, hcaInfo.compR08, hcaInfo.compR07 + hcaInfo.compR06,
                                     hcaInfo.compR05);
            }
            for (auto j = 0; j < hcaInfo.channelCount - 1; ++j) {
                _channels[j].Decode4(i, hcaInfo.compR05 - hcaInfo.compR06, hcaInfo.compR06, hcaInfo.compR07);
            }
            for (auto j = 0; j < hcaInfo.channelCount; ++j) {
                _channels[j].Decode5(i);
            }
        }
    } else {
        return KS_ERR_INVALID_INTERNAL_STATE;
    }
    *pDataCursor += dwDataSize;
    return KS_ERR_OK;
}
