#include "decoder.hpp"

#include <sstream>

bool parse_candump(const std::string& line, Frame& f) {
    std::istringstream in(line);
    std::string stamp, iface, body;
    if (!(in >> stamp >> iface >> body)) return false;
    if (stamp.size() < 3 || stamp.front() != '(' || stamp.back() != ')') return false;

    size_t hash = body.find('#');
    if (hash == std::string::npos || hash == 0) return false;
    std::string id = body.substr(0, hash), hex = body.substr(hash + 1);
    if (hex.size() % 2 != 0 || hex.size() > 16) return false;  // whole bytes, at most 8

    try {
        f.time = std::stod(stamp.substr(1, stamp.size() - 2));
        size_t used = 0;
        f.id = std::stoul(id, &used, 16);
        if (used != id.size()) return false;
        f.data.clear();
        for (size_t i = 0; i < hex.size(); i += 2) {
            f.data.push_back(std::stoi(hex.substr(i, 2), &used, 16));
            if (used != 2) return false;
        }
    } catch (const std::exception&) {  // stod / stoul / stoi on bad text
        return false;
    }
    return true;
}

std::vector<Signal> load_signals(std::istream& in) {
    std::vector<Signal> signals;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::vector<std::string> col;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) col.push_back(field);
        if (col.size() != 8) continue;

        Signal s;
        try {
            s.name = col[0];
            s.id = std::stoul(col[1], nullptr, 0);  // base 0 accepts 0x1A0 or 416
            s.start = std::stoi(col[2]);
            s.length = std::stoi(col[3]);
            s.is_signed = col[4] == "1";
            s.scale = std::stod(col[5]);
            s.offset = std::stod(col[6]);
            s.unit = col[7];
        } catch (const std::exception&) {
            continue;  // a number that is not a number
        }
        if (s.length >= 1 && s.length <= 4 && s.start >= 0 && s.start <= 8 - s.length) signals.push_back(s);
    }
    return signals;
}

bool decode(const Signal& s, const Frame& f, double& value) {
    if (f.id != s.id || s.start < 0 || s.start + s.length > (int)f.data.size()) return false;

    // little-endian: the first byte is the lowest
    uint32_t raw = 0;
    for (int i = s.length - 1; i >= 0; i--) raw = (raw << 8) | f.data[s.start + i];

    int64_t v = raw;
    int bits = 8 * s.length;
    if (s.is_signed && ((raw >> (bits - 1)) & 1)) v -= int64_t(1) << bits;  // two's complement

    value = v * s.scale + s.offset;
    return true;
}
