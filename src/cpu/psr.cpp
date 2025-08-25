#include "psr.h"
#include "cpu_types.h"
#include "../utils.h"

ProgramStatusRegister::ProgramStatusRegister() : mode(MODE_USER), t(STATE_ARM), f(false), i(false), n(false), z(false), c(false), v(false)
{
};

Word ProgramStatusRegister::value() {
    Word * value = 0;
    Utils::write_bit(value, 31, n);
    Utils::write_bit(value, 30, z);
    Utils::write_bit(value, 29, c);
    Utils::write_bit(value, 28, v);
    Utils::write_bit(value, 7, i);
    Utils::write_bit(value, 6, f);
    Utils::write_bit(value, 5, t);
    Utils::write_bit_range(value, 0, 4, mode);
    return *value;
};

void ProgramStatusRegister::write_value(Word value) {
    n = Utils::read_bit(value, 31);
    z = Utils::read_bit(value, 30);
    c = Utils::read_bit(value, 29);
    v = Utils::read_bit(value, 28);
    i = Utils::read_bit(value, 7);
    f = Utils::read_bit(value, 6);
    t = (CPUState)Utils::read_bit(value, 5);
    mode = (OperatingMode)Utils::read_bit_range(value, 0, 4);
} 
