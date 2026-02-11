#!/usr/bin/env python3
"""Convert x.out (Z8000 executable) to flat binary.

Reads the x.out header and segment table, concatenates initialized segment
data (code, const, data) and emits zeros for BSS.  Outputs a flat binary
suitable for loading into an emulator.

x.out on-disk format (all fields big-endian):

  Header (16 bytes):
    uint16  x_magic     magic number (0xEE03 = non-segmented executable)
    int16   x_nseg      number of segments
    int32   x_init      length of initialized data following segment table
    int32   x_reloc     length of relocation table
    int32   x_symb      length of symbol table

  Per-segment descriptor (4 bytes each, x_nseg entries):
    uint8   x_sg_no     segment number
    uint8   x_sg_typ    segment type (see below)
    uint16  x_sg_len    segment length

  Segment types:
    1 = BSS   (non-initialized, no data in file)
    2 = stack (no data in file)
    3 = code
    4 = const
    5 = data
    6 = mixed unprotectable
    7 = mixed protectable
"""

import struct
import sys

# Segment types
SG_BSS = 1
SG_STK = 2
SG_COD = 3
SG_CON = 4
SG_DAT = 5

# Types that have initialized data in the file
INITIALIZED_TYPES = {SG_COD, SG_CON, SG_DAT, 6, 7}

HDR_FMT = '>HhiiI'   # magic, nseg, init, reloc, symb (16 bytes)
HDR_SIZE = struct.calcsize(HDR_FMT)

SG_FMT = '>BBH'      # sg_no, sg_typ, sg_len (4 bytes)
SG_SIZE = struct.calcsize(SG_FMT)


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input.out> <output.bin>", file=sys.stderr)
        sys.exit(1)

    infile, outfile = sys.argv[1], sys.argv[2]

    with open(infile, 'rb') as f:
        data = f.read()

    if len(data) < HDR_SIZE:
        print("Error: file too small for x.out header", file=sys.stderr)
        sys.exit(1)

    magic, nseg, init_len, reloc_len, symb_len = struct.unpack(HDR_FMT, data[:HDR_SIZE])

    if magic not in (0xEE00, 0xEE01, 0xEE02, 0xEE03, 0xEE06, 0xEE07, 0xEE0A, 0xEE0B):
        print(f"Error: bad magic 0x{magic:04X}", file=sys.stderr)
        sys.exit(1)

    # Read segment descriptors
    segments = []
    off = HDR_SIZE
    for i in range(nseg):
        if off + SG_SIZE > len(data):
            print(f"Error: truncated segment table at segment {i}", file=sys.stderr)
            sys.exit(1)
        sg_no, sg_typ, sg_len = struct.unpack(SG_FMT, data[off:off+SG_SIZE])
        segments.append((sg_no, sg_typ, sg_len))
        off += SG_SIZE

    # Initialized data starts right after segment table
    init_start = off

    # Build output: iterate segments in order, concatenate data or zeros
    output = bytearray()
    data_off = init_start

    for sg_no, sg_typ, sg_len in segments:
        if sg_typ in INITIALIZED_TYPES:
            # This segment has data in the file
            chunk = data[data_off:data_off + sg_len]
            if len(chunk) < sg_len:
                print(f"Warning: segment {sg_no} truncated (expected {sg_len}, got {len(chunk)})",
                      file=sys.stderr)
                chunk += b'\x00' * (sg_len - len(chunk))
            output.extend(chunk)
            data_off += sg_len
        elif sg_typ == SG_BSS:
            # BSS: emit zeros
            output.extend(b'\x00' * sg_len)
        # else: stack or unknown — skip

    with open(outfile, 'wb') as f:
        f.write(output)

    print(f"{infile} -> {outfile}: {len(output)} bytes "
          f"({nseg} segments, {init_len} initialized)", file=sys.stderr)


if __name__ == '__main__':
    main()
