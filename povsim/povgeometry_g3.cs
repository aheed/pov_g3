

namespace povdata
{

    class Povgeometry
    {
        public const int NOF_SECTORS = 180;
        public const int NOF_LEDS = 64; //32
        public const int LED_DATA_SIZE = 4;
        public const int POV_FRAME_SIZE = (NOF_SECTORS * NOF_LEDS * LED_DATA_SIZE);


        public static readonly int[] povledRadius = 
        {
        -2205,
        -2135,
        -2066,
        -1997,
        -1927,
        -1858,
        -1788,
        -1719,
        -1649,
        -1580,
        -1510,
        -1441,
        -1372,
        -1302,
        -1233,
        -1163,
        -1094,
        -1024,
        -955,
        -885,
        -816,
        -747,
        -677,
        -608,
        -538,
        -469,
        -399,
        -330,
        -260,
        -191,
        -122,
        -52,
        17,
        87,
        156,
        226,
        295,
        365,
        434,
        503,
        573,
        642,
        712,
        781,
        851,
        920,
        990,
        1059,
        1128,
        1198,
        1267,
        1337,
        1406,
        1476,
        1545,
        1615,
        1684,
        1753,
        1823,
        1892,
        1962,
        2031,
        2101,
        2170
        };        
    }
}
