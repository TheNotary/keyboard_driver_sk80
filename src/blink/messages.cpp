#include "messages_data.h"

///////////////////////
// Begin Bulk Update //
///////////////////////

const size_t BEGIN_BULK_UPDATE_MESSAGE_LENGTH = 65;

unsigned char BEGIN_BULK_UPDATE_MESSAGES[][BEGIN_BULK_UPDATE_MESSAGE_LENGTH] = {
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
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};

const size_t BEGIN_BULK_UPDATE_MESSAGE_COUNT = sizeof(BEGIN_BULK_UPDATE_MESSAGES) / BEGIN_BULK_UPDATE_MESSAGE_LENGTH;


/////////////////////
// End Bulk Update //
/////////////////////

const size_t END_BULK_UPDATE_MESSAGE_LENGTH = 65;

unsigned char END_BULK_UPDATE_MESSAGES[][END_BULK_UPDATE_MESSAGE_LENGTH] = {
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
    /*{ // These look like they're kicking off a new bulk message...
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
    }*/
};

const size_t END_BULK_UPDATE_MESSAGE_COUNT = sizeof(END_BULK_UPDATE_MESSAGES) / END_BULK_UPDATE_MESSAGE_LENGTH;


//////////////
// Turn Red //
//////////////
const size_t TURN_F12_RED_MESSAGE_LENGTH = 65;

unsigned char TURN_F12_RED_MESSAGES[][TURN_F12_RED_MESSAGE_LENGTH] = {
    { // 1
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0x00, 0x00,  // I'm hacking this up to be white, RGB
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    { // 2
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x55
    }
};

const size_t TURN_F12_RED_MESSAGE_COUNT = sizeof(TURN_F12_RED_MESSAGES) / TURN_F12_RED_MESSAGE_LENGTH;


//////////////
// Turn off //
//////////////


const size_t TURN_F12_OFF_MESSAGE_LENGTH = 65;

unsigned char TURN_F12_OFF_MESSAGES[][TURN_F12_OFF_MESSAGE_LENGTH] = {
    {
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,  // This line differs! is the reddness moved to 0x00?
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
        0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
        0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
        0x1a, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00,
        0x1c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
        0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00
    },
    {
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
    {
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
    {
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
    {
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
    {
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
    {
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
    {
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x55
    }
};

const size_t TURN_F12_OFF_MESSAGE_COUNT = sizeof(TURN_F12_OFF_MESSAGES) / TURN_F12_OFF_MESSAGE_LENGTH;


unsigned char TEST_SLIM_MESSAGES[][TURN_F12_OFF_MESSAGE_LENGTH] = {
    {
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0x00, 0x00,  // This line differs! is the reddness moved to 0x00?
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};

unsigned char TEST_SLIM_HEADER_MESSAGES[][BEGIN_BULK_UPDATE_MESSAGE_LENGTH] = {
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
