#pragma once
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

// One classic CAN frame from a candump log.
struct Frame {
    double time = 0;
    uint32_t id = 0;
    std::vector<uint8_t> data;
};

// A signal inside a frame: `length` bytes from `start`, little-endian,
// value = raw * scale + offset.
struct Signal {
    std::string name;
    uint32_t id = 0;
    int start = 0;
    int length = 1;  // 1 to 4 bytes
    bool is_signed = false;
    double scale = 1, offset = 0;
    std::string unit;
};

// Parses a `candump -l` line such as "(1699990000.123456) can0 1A0#0102030405060708".
// Returns false for anything else (remote frames, CAN FD, broken lines).
bool parse_candump(const std::string& line, Frame& frame);

// Reads a signal table, one signal per line:
//   name,id,start,length,signed,scale,offset,unit
// Lines that are empty or start with # are skipped.
std::vector<Signal> load_signals(std::istream& in);

// Decodes one signal from a frame. Returns false if the frame does not carry it.
bool decode(const Signal& s, const Frame& f, double& value);
