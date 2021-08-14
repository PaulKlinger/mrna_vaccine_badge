  #include <stdint.h>
#include "sequence.h"

Base read_base(const struct sequence *seq, uint32_t i) {
    // 0th base is encoded in the 2 most significant bits
    uint8_t pos = 3 - i % 4;
    uint8_t byte = seq->data[i / 4];
    
    return (Base) (byte >> (2 * pos)) & 0b00000011;
}
