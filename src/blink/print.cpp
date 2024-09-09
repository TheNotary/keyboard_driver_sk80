#include <cstdio>


void PrintBlinkKeysArguments(char* key_ids, int n_keys) {
    printf("Blink keys");
    for (int i = 0; i < n_keys; i++) {
        printf(" 0x%02x", key_ids[i]);
    }
    printf(".  %d keys in total.\n", n_keys);
}
