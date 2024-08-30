#include <cstdio>


void PrintBlinkKeysArguments(char* keyIds, int nKeys) {
    printf("Blink keys");
    for (int i = 0; i < nKeys; i++) {
        printf(" 0x%02x", keyIds[i]);
    }
    printf(".  %d keys in total.\n", nKeys);
}
