# kawashima Example

This example shows how to use kawashima to convert an HCA file to a WAV file, in C++.

```cpp
#include <iostream>
#include "ksapi.h"

using namespace std;

// Usage: <exec-file> <hca-in> <wav-out> <key1> <key2>
// Set key1 and key2 to 0 if not needed.

int main(int argc, const char *argv[]) {
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
    KsSetParamI32(hDecode, KS_PARAM_KEY1, (uint32)atoi(argv[2]));
    KsSetParamI32(hDecode, KS_PARAM_KEY2, (uint32)atoi(argv[3]));
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
    return 0;
}
```
