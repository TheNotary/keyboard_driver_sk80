/*
 * Minimal downstream consumer of the installed keylt package, compiled as C99.
 *
 * This is the load-bearing check that keylt.h really is a portable C ABI: a C++
 * compiler would happily accept references, overloads or default arguments that
 * no other language can call. Building this file with a C compiler will not.
 *
 * It exercises only entry points that touch no USB hardware, because CI runners
 * have no keyboard attached.
 */

#include <stdio.h>
#include <string.h>

#include <keylt/keylt.h>

int main(void) {
    uint32_t abi = keylt_abi_version();
    size_t known = 0;
    size_t key_count = 0;
    keylt_status status;

    if (abi != KEYLT_ABI_VERSION) {
        printf("ABI mismatch: header %u, library %u\n", (unsigned)KEYLT_ABI_VERSION,
               (unsigned)abi);
        return 1;
    }

    /* Probing call: no buffer, so this only reports how many entries there are. */
    status = keylt_list_known_keyboards(NULL, 0, &known);
    if (status != KEYLT_OK && status != KEYLT_ERR_BUFFER_TOO_SMALL) {
        printf("keylt_list_known_keyboards failed: %s (%s)\n", keylt_status_name(status),
               keylt_last_error_message());
        return 1;
    }
    if (known == 0) {
        printf("expected at least one known keyboard model\n");
        return 1;
    }

    {
        keylt_keyboard_info infos[8];
        size_t total = 0;
        size_t i;

        if (known > 8) {
            known = 8;
        }
        status = keylt_list_known_keyboards(infos, known, &total);
        if (status != KEYLT_OK) {
            printf("keylt_list_known_keyboards failed: %s (%s)\n", keylt_status_name(status),
                   keylt_last_error_message());
            return 1;
        }
        for (i = 0; i < known; ++i) {
            if (infos[i].display_name[0] == '\0') {
                printf("keyboard %u has an empty display name\n", (unsigned)i);
                return 1;
            }
        }
    }

    status = keylt_model_key_count(KEYLT_MODEL_SK80, &key_count);
    if (status != KEYLT_OK || key_count == 0) {
        printf("keylt_model_key_count failed: %s (%s)\n", keylt_status_name(status),
               keylt_last_error_message());
        return 1;
    }

    {
        const char* name = NULL;
        uint8_t key_id = 0;
        uint8_t looked_up = 0;

        status = keylt_model_key_at(KEYLT_MODEL_SK80, 0, &name, &key_id);
        if (status != KEYLT_OK || name == NULL) {
            printf("keylt_model_key_at failed: %s\n", keylt_status_name(status));
            return 1;
        }
        status = keylt_model_key_id_for_name(KEYLT_MODEL_SK80, name, &looked_up);
        if (status != KEYLT_OK || looked_up != key_id) {
            printf("key id round-trip failed for '%s'\n", name);
            return 1;
        }
    }

    /* Errors must arrive as status codes, never as an exception or a crash. */
    status = keylt_keyboard_open(KEYLT_MODEL_SK80, NULL);
    if (status != KEYLT_ERR_INVALID_ARGUMENT) {
        printf("expected KEYLT_ERR_INVALID_ARGUMENT, got %s\n", keylt_status_name(status));
        return 1;
    }
    if (strlen(keylt_last_error_message()) == 0) {
        printf("expected a message describing the failure\n");
        return 1;
    }

    printf("keylt %s (ABI %u) linked and loaded successfully; %u models, %u SK80 keys\n",
           keylt_version_string(), (unsigned)abi, (unsigned)known, (unsigned)key_count);
    return 0;
}
