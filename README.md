# can-log-decoder

Turns a CAN bus log into readable values, in C++17. It reads `candump -l` logs (Linux
SocketCAN) and a small signal table, and writes one CSV row per decoded value.

Written in 2026, separate from my e-bike project, where an ESP32 read battery and
motor-controller frames off the bike's CAN bus. This tool was not used on the bike. It shows how
frames like these can be decoded on a PC, from a standard candump log.

## Input

A log line is the time, the interface, and the ID and data bytes in hex:

```
(1761638400.000000) can0 1A0#CC12CBFF57
```

The signal table (`examples/signals.csv`) says where each value sits in which frame:

```
# name,id,start,length,signed,scale,offset,unit
pack_voltage,0x1A0,0,2,0,0.01,0,V
pack_current,0x1A0,2,2,1,0.1,0,A
```

Signals are whole bytes, little-endian, 1 to 4 bytes long, and `value = raw * scale + offset`.
For example, bytes 2 and 3 of `CC12CBFF57` are `CB FF`. Little-endian that is 0xFFCB, which is
-53 as a signed 16-bit number, and -53 x 0.1 gives -5.3 A.

## Build and run

```bash
cmake -B build
cmake --build build
./build/decoder_test
./build/candecode examples/signals.csv examples/sample.log > decoded.csv
```

```
time,signal,value,unit
1761638400.000000,pack_voltage,48.12,V
1761638400.000000,pack_current,-5.3,A
1761638400.000000,soc,87,%
1761638400.050000,motor_rpm,1200,rpm
1761638400.050000,motor_temp,41,C
...
```

## Limits

- classic CAN only, so CAN FD and remote frames are skipped
- signals must start on a byte boundary and be little-endian (DBC files also allow bit-level and
  big-endian signals)
- the IDs and the layout in `examples/` are made up and do not belong to any real vehicle
