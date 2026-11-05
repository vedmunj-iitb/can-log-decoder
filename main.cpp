// Decodes a candump log into CSV: time,signal,value,unit
//   candecode examples/signals.csv examples/sample.log > decoded.csv
#include <cstdio>
#include <fstream>

#include "decoder.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: candecode SIGNALS.csv LOGFILE\n");
        return 1;
    }
    std::ifstream table(argv[1]), log(argv[2]);
    if (!table || !log) {
        std::fprintf(stderr, "cannot open the input files\n");
        return 1;
    }

    std::vector<Signal> signals = load_signals(table);
    std::printf("time,signal,value,unit\n");

    std::string line;
    int frames = 0, skipped = 0;
    while (std::getline(log, line)) {
        Frame f;
        if (!parse_candump(line, f)) {
            skipped++;
            continue;
        }
        frames++;
        for (const Signal& s : signals) {
            double v;
            if (decode(s, f, v)) std::printf("%.6f,%s,%.10g,%s\n", f.time, s.name.c_str(), v, s.unit.c_str());
        }
    }
    std::fprintf(stderr, "%d frames, %d lines skipped, %zu signals\n", frames, skipped, signals.size());
}
