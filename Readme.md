BiNums - see binary numbers
2019-02-14..2019-03-21  
Dwayne Robinson  
Displays a number in various formats as binary/hex or vice versa.

## Usage examples

    binums 3.14159
    binums -13
    binums showbinary -13
    binums showhex -13
    binums 0x4240
    binums 0b1101
    binums float16 3.14
    binums float16 raw 0x4240
    binums uint32 mul 3 2 add 3 2 subtract 3 2 dot 1 2 3 4
    binums float32 0x2.4p0

## Options

    showbinary showhex - display raw bits as binary or hex (default)
    showhexfloat showdecfloat - display float as hex or decimal (default)
    raw num - treat input as raw bit data or as number (default)
    add subtract multiply divide dot - apply operation to following numbers
    float16 bfloat16 float32 float64 - set floating point data type
    uint8 uint16 uint32 uint64 int8 int16 int32 int64 - set integer data type
    fixed12_12 fixed16_16 fixed8_24 - set fixed precision data type

## Sample output

Display pi:

    BiNums.exe 3.14159
    To binary:
             uint8 3 -> 0x03
            uint16 3 -> 0x0003
            uint32 3 -> 0x00000003
            uint64 3 -> 0x0000000000000003
              int8 3 -> 0x03
             int16 3 -> 0x0003
             int32 3 -> 0x00000003
             int64 3 -> 0x0000000000000003
           float16 3.140625 -> 0x4248
          bfloat16 3.140625 -> 0x4049
           float32 3.1415863037109375 -> 0x40490FC0
           float64 3.1415863037109375 -> 0x400921F800000000
        fixed12_12 3.141357421875 -> 0x003243
        fixed16_16 3.1415863037109375 -> 0x0003243F
         fixed8_24 3.1415863037109375 -> 0x03243F00

    From binary:
             uint8 3 <- 0x03
            uint16 3 <- 0x0003
            uint32 3 <- 0x00000003
            uint64 3 <- 0x0000000000000003
              int8 3 <- 0x03
             int16 3 <- 0x0003
             int32 3 <- 0x00000003
             int64 3 <- 0x0000000000000003
           float16 1.78813934326171875e-07 <- 0x0003
          bfloat16 2.75506488473973634680173e-40 <- 0x0003
           float32 4.20389539297445121277119e-45 <- 0x00000003
           float64 1.48219693752373963252971e-323 <- 0x0000000000000003
        fixed12_12 0.000732421875 <- 0x000003
        fixed16_16 4.57763671875e-05 <- 0x00000003
         fixed8_24 1.78813934326171875e-07 <- 0x00000003

Value 1:

    BiNums.exe 1
    To binary:
             uint8 1 -> 0x01
            uint16 1 -> 0x0001
            uint32 1 -> 0x00000001
            uint64 1 -> 0x0000000000000001
              int8 1 -> 0x01
             int16 1 -> 0x0001
             int32 1 -> 0x00000001
             int64 1 -> 0x0000000000000001
           float16 1 -> 0x3C00
          bfloat16 1 -> 0x3F80
           float32 1 -> 0x3F800000
           float64 1 -> 0x3FF0000000000000
        fixed12_12 1 -> 0x001000
        fixed16_16 1 -> 0x00010000
         fixed8_24 1 -> 0x01000000

    From binary:
             uint8 1 <- 0x01
            uint16 1 <- 0x0001
            uint32 1 <- 0x00000001
            uint64 1 <- 0x0000000000000001
              int8 1 <- 0x01
             int16 1 <- 0x0001
             int32 1 <- 0x00000001
             int64 1 <- 0x0000000000000001
           float16 5.96046e-08 <- 0x0001
          bfloat16 9.18355e-41 <- 0x0001
           float32 1.4013e-45 <- 0x00000001
           float64 4.94066e-324 <- 0x0000000000000001
        fixed12_12 0.000244140625 <- 0x000001
        fixed16_16 1.52587890625e-05 <- 0x00000001
         fixed8_24 5.9604644775390625e-08 <- 0x00000001

Display multiple values in a specific format:

    BiNums.exe float64 1 3.14159 1234
          float64 1(0x3FF0000000000000)
          float64 3.14159(0x400921F9F01B866E)
          float64 1234(0x4093480000000000)

Show as binary rather than hex:

    BiNums.exe float64 showbinary 1 3.14159 1234
          float64 1(0011111111110000000000000000000000000000000000000000000000000000)
          float64 3.14159(0100000000001001001000011111100111110000000110111000011001101110)
          float64 1234(0100000010010011010010000000000000000000000000000000000000000000)

Add values:

    BiNums.exe float64 add 1 3.14159 1234
    Operands:
          float64 1(0x3FF0000000000000)
          float64 3.14159(0x400921F9F01B866E)
          float64 1234(0x4093480000000000)
    Result of add:
          float64 1238.14(0x40935890FCF80DC3)
