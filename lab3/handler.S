#include "i8042.h"

.global _keyboard_int_handler_assembly

.text

_keyboard_int_handler_assembly:

xor %eax, %eax
inb $OUT_BUF
ret
