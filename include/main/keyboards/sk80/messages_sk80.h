#pragma once

constexpr auto MESSAGE_LENGTH_SK80 = 65;

extern unsigned char BULK_LED_HEADER_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
extern const size_t  BULK_LED_HEADER_MESSAGES_COUNT_SK80;

extern unsigned char BULK_LED_FOOTER_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
extern const size_t  BULK_LED_FOOTER_MESSAGES_COUNT_SK80;

extern unsigned char BULK_LED_VALUE_MESSAGES_SK80[][MESSAGE_LENGTH_SK80];
extern const size_t  BULK_LED_VALUE_MESSAGES_COUNT_SK80;