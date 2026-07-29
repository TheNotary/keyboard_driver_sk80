/*
 * Minimal downstream consumer of the installed blink package, compiled as C99.
 *
 * This is the load-bearing check that blink.h really is a portable C ABI: a C++
 * compiler would happily accept references, overloads or default arguments that
 * no other language can call. Building this file with a C compiler will not.
 *
 * It exercises only entry points that touch no USB hardware, because CI runners
 * have no keyboard attached.
 */

#include <stdio.h>
#include <string.h>

#include <blink/blink.h>

int main(void) {
    uint32_t abi = blink_abi_version();
    size_t known = 0;
    size_t key_count = 0;
    blink_status status;

    if (abi != BLINK_ABI_VERSION) {
        printf("ABI mismatch: header %u, library %u\n", (unsigned)BLINK_ABI_VERSION,
               (unsigned)abi);
        return 1;
    }

    /* Probing call: no buffer, so this only reports how many entries there are. */
    status = blink_list_known_keyboards(NULL, 0, &known);
    if (status != BLINK_OK && status != BLINK_ERR_BUFFER_TOO_SMALL) {
        printf("blink_list_known_keyboards failed: %s (%s)\n", blink_status_name(status),
               blink_last_error_message());
        return 1;
    }
    if (known == 0) {
        printf("expected at least one known keyboard model\n");
        return 1;
    }

    {
        blink_keyboard_info infos[8];
        size_t total = 0;
        size_t i;

        if (known > 8) {
            known = 8;
        }
        status = blink_list_known_keyboards(infos, known, &total);
        if (status != BLINK_OK) {
            printf("blink_list_known_keyboards failed: %s (%s)\n", blink_status_name(status),
                   blink_last_error_message());
            return 1;
        }
        for (i = 0; i < known; ++i) {
            if (infos[i].display_name[0] == '\0') {
                printf("keyboard %u has an empty display name\n", (unsigned)i);
                return 1;
            }
        }
    }

    status = blink_model_key_count(BLINK_MODEL_SK80, &key_count);
    if (status != BLINK_OK || key_count == 0) {
        printf("blink_model_key_count failed: %s (%s)\n", blink_status_name(status),
               blink_last_error_message());
        return 1;
    }

    {
        const char* name = NULL;
        uint8_t key_id = 0;
        uint8_t looked_up = 0;

        status = blink_model_key_at(BLINK_MODEL_SK80, 0, &name, &key_id);
        if (status != BLINK_OK || name == NULL) {
            printf("blink_model_key_at failed: %s\n", blink_status_name(status));
            return 1;
        }
        status = blink_model_key_id_for_name(BLINK_MODEL_SK80, name, &looked_up);
        if (status != BLINK_OK || looked_up != key_id) {
            printf("key id round-trip failed for '%s'\n", name);
            return 1;
        }
    }

    /* Errors must arrive as status codes, never as an exception or a crash. */
    status = blink_keyboard_open(BLINK_MODEL_SK80, NULL);
    if (status != BLINK_ERR_INVALID_ARGUMENT) {
        printf("expected BLINK_ERR_INVALID_ARGUMENT, got %s\n", blink_status_name(status));
        return 1;
    }
    if (strlen(blink_last_error_message()) == 0) {
        printf("expected a message describing the failure\n");
        return 1;
    }

    printf("blink %s (ABI %u) linked and loaded successfully; %u models, %u SK80 keys\n",
           blink_version_string(), (unsigned)abi, (unsigned)known, (unsigned)key_count);
    return 0;
}
