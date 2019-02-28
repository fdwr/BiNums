2019-02-28 Dwayne Robinson
Little binary number, displaying a number in various formats as binary/hex or vice versa.

Usage examples:
    seebinum 3.14159
    seebinum -13
    seebinum showbinary -13
    seebinum showhex -13
    seebinum 0x4240
    seebinum 0b1101
    seebinum float16 3.14
    seebinum float16 raw 0x4240
    seebinum uint32 mul 3 2 add 3 2 subtract 3 2 dot 1 2 3 4
    seebinum float32 0x2.4p0

Options:
    showbinary showhex - display raw bits as binary or hex (default)
    showhexfloat showdecfloat - display float as hex or decimal (default)
    raw num - treat input as raw bit data or as number (default)
    add subtract multiply dot - apply operation to following numbers
    float16 bfloat16 float32 float64 uint8 uint16 uint32 uint64 int8 int16 int32 int64 - set type
