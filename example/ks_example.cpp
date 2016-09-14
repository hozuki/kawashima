/*
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <iostream>
#include "../src/ksapi.h"

using namespace std;

void PrintHelp();

uint32 hatoui(const char *a) {
    uint32 val = 0;
    while (a && *a) {
        if ('0' <= *a && *a <= '9') {
            val = (val << 4) + (*a - '0');
        } else if ('a' <= *a && *a <= 'f') {
            val = (val << 4) + (*a - 'a' + 10);
        } else if ('A' <= *a && *a <= 'F') {
            val = (val << 4) + (*a - 'A' + 10);
        } else {
            break;
        }
        ++a;
    }
    return val;
}

int main(int argc, const char *argv[]) {
    if (argc != 5) {
        PrintHelp();
        return 0;
    }

    HKDECODE hDecode = NULL;
    KS_RESULT result = KS_ERR_OK;
    uint8 *buffer = NULL;
    uint32 bufferSize = 0;
    uint32 dataSize = 0;
    FILE *fp = NULL;

    // Open the file and get a handle.
    result = KsOpenFile(argv[1], &hDecode);
    if (!KS_CALL_SUCCESSFUL(result)) {
        cout << "Load failed." << endl;
        return 1;
    }

    // Set parameters before beginning to decode.
    if (argc >= 5) {
        KsSetParamI32(hDecode, KS_PARAM_KEY1, hatoui(argv[3]));
        KsSetParamI32(hDecode, KS_PARAM_KEY2, hatoui(argv[4]));
    }
    KsBeginDecode(hDecode);
    fp = fopen(argv[2], "wb");

    // Write the WAVE header.
    KsGetWaveHeader(hDecode, NULL, &bufferSize);
    buffer = new uint8[bufferSize];
    KsGetWaveHeader(hDecode, buffer, &bufferSize);
    fwrite(buffer, bufferSize, 1, fp);
    delete[] buffer;

    // Write WAVE data blocks.
    KsDecodeData(hDecode, NULL, &bufferSize);
    if (bufferSize > 0) {
        result = KS_OP_HAS_MORE_DATA;
        while (result > 0) {
            // 10 blocks per time
            dataSize = bufferSize * 10;
            buffer = new uint8[dataSize];
            result = KsDecodeData(hDecode, buffer, &dataSize);
            fwrite(buffer, dataSize, 1, fp);
            delete[] buffer;
        }
    }

    // Clean up.
    fclose(fp);
    KsEndDecode(hDecode);
    KsCloseHandle(hDecode);
}

void PrintHelp() {
    cout << "Usage:\r\nks-example <in file> <out file> <key1> <key2>" << endl;
}
