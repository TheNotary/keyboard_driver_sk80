#pragma once
#include <array>
#include <misc.h>
#include <keyboards/keyboard_base.h>


TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId_RK84(UINT8 active_key);


// TODO: 
//   - Create a base class and inherit from that
//   - Inject this into the main Keyboard class during the initializer.

class RK84 : public KeyboardBase {
public:
	 void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const;

private:

};
