#pragma once

void PrintMessageInBuffer(unsigned char* buffer, size_t i, size_t message_length);
void PrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length);

extern const size_t BEGIN_BULK_UPDATE_MESSAGE_LENGTH;
extern const size_t BEGIN_BULK_UPDATE_MESSAGE_COUNT;
extern unsigned char BEGIN_BULK_UPDATE_MESSAGES[][65];

extern const size_t END_BULK_UPDATE_MESSAGE_LENGTH;
extern const size_t END_BULK_UPDATE_MESSAGE_COUNT;
extern unsigned char END_BULK_UPDATE_MESSAGES[][65];

extern const size_t TURN_F12_RED_MESSAGE_LENGTH;
extern const size_t TURN_F12_RED_MESSAGE_COUNT;
extern unsigned char TURN_F12_RED_MESSAGES[][65];

extern const size_t TURN_F12_OFF_MESSAGE_LENGTH;
extern const size_t TURN_F12_OFF_MESSAGE_COUNT;
extern unsigned char TURN_F12_OFF_MESSAGES[][65];

extern unsigned char TEST_SLIM_MESSAGES[][65];
extern unsigned char TEST_SLIM_HEADER_MESSAGES[][65];

constexpr auto MESSAGE_LENGTH = 65;
