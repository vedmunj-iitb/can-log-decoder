#include <cmath>
#include <cstdio>
#include <sstream>

#include "decoder.hpp"

int failures = 0;
#define CHECK(cond)                                              \
    if (!(cond)) {                                               \
        std::printf("FAIL line %d: %s\n", __LINE__, #cond);      \
        failures++;                                              \
    }

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    Frame f;

    // a normal line
    CHECK(parse_candump("(1699990000.500000) can0 1A0#CC12CBFF57", f));
    CHECK(near(f.time, 1699990000.5) && f.id == 0x1A0 && f.data.size() == 5);
    CHECK(f.data[0] == 0xCC && f.data[4] == 0x57);

    // extended id, empty data
    CHECK(parse_candump("(1.0) can0 18FF50E5#", f));
    CHECK(f.id == 0x18FF50E5 && f.data.empty());

    // lines that must be rejected
    CHECK(!parse_candump("", f));
    CHECK(!parse_candump("can0 1A0#0102", f));                // no time stamp
    CHECK(!parse_candump("(1.0) can0 1A0#012", f));           // half a byte
    CHECK(!parse_candump("(1.0) can0 1A0#R", f));             // remote frame
    CHECK(!parse_candump("(1.0) can0 1A0##10102", f));        // CAN FD
    CHECK(!parse_candump("(1.0) can0 1G0#01", f));            // bad id
    CHECK(!parse_candump("(1.0) can0 1A0#0102030405060708FF", f));  // 9 bytes

    std::istringstream table(
        "# name,id,start,length,signed,scale,offset,unit\n"
        "voltage,0x1A0,0,2,0,0.01,0,V\n"
        "current,0x1A0,2,2,1,0.1,0,A\n"
        "soc,0x1A0,4,1,0,1,0,%\n"
        "temp,0x2B0,0,1,1,1,-40,C\n"
        "broken line\n"
        "bad,0x1A0,x,1,0,1,0,V\n");
    auto sig = load_signals(table);
    CHECK(sig.size() == 4);

    double v;
    CHECK(parse_candump("(2.0) can0 1A0#CC12CBFF57", f));
    CHECK(decode(sig[0], f, v) && near(v, 48.12));  // 0x12CC = 4812 -> 48.12 V
    CHECK(decode(sig[1], f, v) && near(v, -5.3));   // 0xFFCB = -53 -> -5.3 A
    CHECK(decode(sig[2], f, v) && near(v, 87));     // 0x57 = 87 %
    CHECK(!decode(sig[3], f, v));                   // other id

    // offset, and a frame too short for the signal
    CHECK(parse_candump("(3.0) can0 2B0#64", f));
    CHECK(decode(sig[3], f, v) && near(v, 60));     // 100 - 40
    CHECK(parse_candump("(4.0) can0 1A0#CC12", f));
    CHECK(!decode(sig[1], f, v));

    if (failures > 0) {
        std::printf("%d test(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
