#pragma once
#include "constants_sk80.h"

// constexpr auto MESSAGE_LENGTH_SK80 = 65;

// extern unsigned char BULK_LED_HEADER_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
// extern const size_t  BULK_LED_HEADER_MESSAGES_COUNT_SK80;

// extern unsigned char BULK_LED_FOOTER_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
// extern const size_t  BULK_LED_FOOTER_MESSAGES_COUNT_SK80;

// extern unsigned char BULK_LED_VALUE_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
// extern const size_t  BULK_LED_VALUE_MESSAGES_COUNT_SK80;

namespace sk80 {

const unsigned char BULK_LED_HEADER_MESSAGES[BULK_LED_HEADER_MESSAGES_COUNT][MESSAGE_LENGTH] = {
    {
        0x00,
        0x04, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x00,
        0x04, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // the 0x09... don't change it.  I'm not sure what it does but it's not good...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};


const unsigned char BULK_LED_FOOTER_MESSAGES[BULK_LED_FOOTER_MESSAGES_COUNT][MESSAGE_LENGTH] = {
    {
        0x00,
        0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x00,
        0x04, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    /*{ // These look like they're kicking off a new message type...
        0x00,
        0x04, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // I really think this was setting us up to set overall brightness or something great...
        0x00,
        0x04, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // This one is some kind of payload packet, not a header
        0x00,
        0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x05, 0x05, 0x00, 0x00, 0x00, 0xaa, 0x55,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // And heres an absolute footer message pair
        0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x04, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
    */
};



// Format:
// 65 bytes per packet
// 16 keys per packet
// For each packet:
//   (byte) report_type = 0x00
//   For each key:
//     (byte) KeyId (value 0x00 - 0x79, though some are absent)
//     (byte) Red value (Value 0x00 - 0xff)
//     (byte) Green value (Value 0x00 - 0xff)
//     (byte) Blue value (Value 0x00 - 0xff)
const unsigned char BULK_LED_VALUE_MESSAGES[BULK_LED_VALUE_MESSAGES_COUNT][MESSAGE_LENGTH] = {
    { // 1 - Top Row, esc - f12
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // KeyId 0x00 - 0x0f
        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // 2
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // KeyId 0x10 - 0x1f
        0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
        0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
        0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
        0x1a, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00,
        0x1c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
        0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00
    },
    { // 3
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00,
        0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00,
        0x28, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00,
        0x2a, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00,
        0x2e, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00
    },
    { // 4
        0x00,
        0x30, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00,
        0x38, 0x00, 0x00, 0x00, 0x39, 0x00, 0x00, 0x00,
        0x3a, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00,
        0x3c, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00, 0x00,
        0x3e, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00
    },
    { // 5
        0x00,
        0x40, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
        0x42, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x49, 0x00, 0x00, 0x00,
        0x4a, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00,
        0x4c, 0x00, 0x00, 0x00, 0x4d, 0x00, 0x00, 0x00,
        0x4e, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00
    },

    { // 6
        0x00,
        0x50, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00,
        0x52, 0x00, 0x00, 0x00, 0x53, 0x00, 0x00, 0x00,
        0x54, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x5b, 0x00, 0x00, 0x00,
        0x5c, 0x00, 0x00, 0x00, 0x5d, 0x00, 0x00, 0x00,
        0x5e, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00
    },
    { // 7
        0x00,
        0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x62, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00,
        0x64, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00,
        0x66, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // 8
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x76, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x79, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // 9
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x55  // Lighting levels?
    }
};

}
