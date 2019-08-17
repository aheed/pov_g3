using System;
using System.Linq;

namespace povdata
{
    class LedCoordinate
    {
        public double r;
        public double y;
    }

    class Povgeometry
    {
        public const int NOF_SECTORS = 320;
        public const int NOF_LEDS = 44;
        public const int LED_DATA_SIZE = 4;
        public const int POV_FRAME_SIZE = (NOF_SECTORS * NOF_LEDS * LED_DATA_SIZE);

/*
        private static readonly int[] povledRadius =
        {
        100,
        171,
        242,
        313,
        384,
        455,
        526,
        597,
        668,
        739,
        810,
        881,
        952,
        1023,
        1094,
        1165,
        1235,
        1306,
        1377,
        1448,
        1519,
        1590,
        1661,
        1732,
        1803,
        1874,
        1945,
        2016,
        2087,
        2158,
        2229,
        2300,
        2265,
        2194,
        2123,
        2052,
        1982,
        1911,
        1840,
        1769,
        1698,
        1627,
        1556,
        1485,
        1414,
        1343,
        1272,
        1201,
        1131,
        1060,
        989,
        918,
        847,
        777,
        706,
        635,
        564,
        494,
        424,
        354,
        284,
        215,
        148,
        88
        };

        // radians x10000
        private static readonly int[] povledAngle =
        {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        265,
        273,
        283,
        292,
        303,
        314,
        326,
        339,
        353,
        369,
        386,
        404,
        424,
        447,
        472,
        500,
        531,
        566,
        607,
        654,
        709,
        773,
        851,
        946,
        1065,
        1218,
        1421,
        1705,
        2130,
        2828,
        4169,
        7491
        };
*/

        private static LedCoordinate[] _PovledRadiusDouble
        {
            get
            {
                double leddistance = 70.9677419355;
                double x0;
                var ret = new LedCoordinate[NOF_LEDS];
                int i;
                double x;
                double y;
                for(i=0; i<NOF_LEDS; i++)
                {
                    if(i < (NOF_LEDS / 2))
                    {
                        y = -60;
                        x0 = leddistance / 2;
                        x = x0 + i * leddistance;
                    }
                    else
                    {
                        y = 60;
                        x0 = 0.000000001;
                        x = x0 + (NOF_LEDS - 1 - i) * leddistance;
                    }
                    ret[i] = new LedCoordinate {r = Math.Sqrt(Math.Pow(x, 2) + Math.Pow(y, 2)), y = y};
                    //Console.WriteLine("{0} {1}", ret[i].x, ret[i].y);
                }
                return ret;
            }
        }

        public static int[] PovledRadius
        {
            get 
            {
                var apa = _PovledRadiusDouble.Select(coord => (int)Math.Round(coord.r));
                return apa.ToArray();
            }
        }

        //public static int[] PovledAngle  => povledAngle;

        public static int[] PovledAngle
        {
            get
            {
                return _PovledRadiusDouble.Select((coord) => (int)Math.Round(Math.Atan(coord.y / coord.r) * 10000)).ToArray();
            }
        }
    }
}
