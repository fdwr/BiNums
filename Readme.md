BiNums - see binary numbers
2019-02-14..2019-07-11  
Dwayne Robinson  
Displays a number in various formats as binary/hex or vice versa.  
https://github.com/fdwr/BiNums

## Usage examples

    binums 12.75  // floating point value in various formats\n"
    binums 0b1101  // read binary integer\n"
    binums float32 raw 0x40490FDB  // read raw floating point bits\n"
    binums float16 raw 0x5140  // read raw floating point bits\n"
    binums fields hex 7 12.75 -13 bin 7 12.75 -13  // see fields of numbers\n"
    binums int8 fields 13 -13  // see fields of numbers\n"
    binums uint32 add 1.5 3.25  // perform operation\n"
    binums float32 add float16 2 3  // read float16, compute in float32\n"
    binums uint32 mul 3 2 add 3 2 subtract 3 2 dot 1 2 3 4\n"
    binums 0x1.5p5  // floating point hexadecimal\n"
    binums fixed12_12 sub 3.5 2  // fixed point arithmetic\n"

## Options

    showbinary showhex - display raw bits as binary or hex (default)
    showhexfloat showdecfloat - display float as hex or decimal (default)
    raw num - treat input as raw bit data or as number (default)
    add subtract multiply divide dot - apply operation to following numbers
    float16 bfloat16 float32 float64 - set floating point data type
    uint8 uint16 uint32 uint64 int8 int16 int32 int64 - set integer data type
    fixed12_12 fixed16_16 fixed8_24 - set fixed precision data type

## Sample output

Display integer:

    BiNums.exe 123
    Representations:
              type int32
           decimal 123
          floathex 123
               hex 0x0000007B
               oct 0o00000000173
               bin 0b00000000000000000000000001111011
        fields bin int:0b0000000000000000000000001111011 sign:0b0

    To binary:
             uint8 123 -> 0x7B
            uint16 123 -> 0x007B
            uint32 123 -> 0x0000007B
            uint64 123 -> 0x000000000000007B
              int8 123 -> 0x7B
             int16 123 -> 0x007B
     ->      int32 123 -> 0x0000007B
             int64 123 -> 0x000000000000007B
           float16 123 -> 0x57B0
          bfloat16 123 -> 0x42F6
           float32 123 -> 0x42F60000
           float64 123 -> 0x405EC00000000000
        fixed12_12 123 -> 0x07B000
        fixed16_16 123 -> 0x007B0000
         fixed8_24 123 -> 0x7B000000

    From binary:
             uint8 123 <- 0x7B
            uint16 123 <- 0x007B
            uint32 123 <- 0x0000007B
            uint64 123 <- 0x000000000000007B
              int8 123 <- 0x7B
             int16 123 <- 0x007B
     ->      int32 123 <- 0x0000007B
             int64 123 <- 0x000000000000007B
           float16 7.331371307373046875e-06 <- 0x007B
          bfloat16 1.12957660274329190218871e-38 <- 0x007B
           float32 1.72359711111952499723619e-43 <- 0x0000007B
           float64 6.0770074438473324933718e-322 <- 0x000000000000007B
        fixed12_12 0.030029296875 <- 0x00007B
        fixed16_16 0.0018768310546875 <- 0x0000007B
         fixed8_24 7.331371307373046875e-06 <- 0x0000007B

Display floating point value:

    BiNums.exe 12.75
    Representations:
              type float64
           decimal 12.75
          floathex 0x1.9800000000000p+3
               hex 0x4029800000000000
               oct 0o0400514000000000000000
               bin 0b0100000000101001100000000000000000000000000000000000000000000000
        fields bin frac:0b1001100000000000000000000000000000000000000000000000 exp:0b10000000010 sign:0b0

    To binary:
             uint8 12 -> 0x0C
            uint16 12 -> 0x000C
            uint32 12 -> 0x0000000C
            uint64 12 -> 0x000000000000000C
              int8 12 -> 0x0C
             int16 12 -> 0x000C
             int32 12 -> 0x0000000C
             int64 12 -> 0x000000000000000C
           float16 12.75 -> 0x4A60
          bfloat16 12.75 -> 0x414C
           float32 12.75 -> 0x414C0000
     ->    float64 12.75 -> 0x4029800000000000
        fixed12_12 12.75 -> 0x00CC00
        fixed16_16 12.75 -> 0x000CC000
         fixed8_24 12.75 -> 0x0CC00000

    From binary:
             uint8 0 <- 0x00
            uint16 0 <- 0x0000
            uint32 0 <- 0x00000000
            uint64 4623367229960880128 <- 0x4029800000000000
              int8 0 <- 0x00
             int16 0 <- 0x0000
             int32 0 <- 0x00000000
             int64 4623367229960880128 <- 0x4029800000000000
           float16 0 <- 0x0000
          bfloat16 0 <- 0x0000
           float32 0 <- 0x00000000
     ->    float64 12.75 <- 0x4029800000000000
        fixed12_12 0 <- 0x000000
        fixed16_16 0 <- 0x00000000
         fixed8_24 0 <- 0x00000000

Display multiple values in a specific format:

    BiNums.exe float64 1 3.14159 1234
          float64 1 (0x3FF0000000000000)
          float64 3.14159 (0x400921F9F01B866E)
          float64 1234 (0x4093480000000000)

Show as binary rather than hex:

    BiNums.exe float64 bin 1 3.14159 1234
          float64 1 (0b0011111111110000000000000000000000000000000000000000000000000000)
          float64 3.14159 (0b0100000000001001001000011111100111110000000110111000011001101110)
          float64 1234 (0b0100000010010011010010000000000000000000000000000000000000000000)

Add values:

    BiNums.exe float64 add 1 3.14159 1234
    Operands:
          float64 1 (0x3FF0000000000000)
          float64 3.14159 (0x400921F9F01B866E)
          float64 1234 (0x4093480000000000)
    Result of add:
          float64 1238.14 (0x40935890FCF80DC3)
