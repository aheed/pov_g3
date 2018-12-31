using System;
using System.IO;
using povdata;

namespace PovSim
{
    public class HubPovData
    {
        public int nofSectors = Povgeometry.NOF_SECTORS;
        public int nofLeds = Povgeometry.NOF_LEDS;

        public int[] povledRadius = Povgeometry.povledRadius;

        public int[] ledColors;

        public void Init(PovFramePayload payload)
        {
            if(payload == null)
            {
                Console.WriteLine("No payload!!!");
            }
            else
            {
                //Console.WriteLine("zzz1b");
            }

            if(payload.leddata == null)
            {
                Console.WriteLine("No leddata !!!");
            }
            else
            {
                //Console.WriteLine("{0}", payload.leddata.Length);
            }

            ledColors = new int[Povgeometry.NOF_SECTORS * Povgeometry.NOF_LEDS];

            int sector;
            for(sector = 0; sector < Povgeometry.NOF_SECTORS; ++sector)
            {
                int led;
                for(led = 0; led < Povgeometry.NOF_LEDS; ++led)
                {
                    int ledIndex = sector * Povgeometry.NOF_LEDS + led; 
                    int byteIndex = ledIndex * 4;
                    
                    ledColors[ledIndex] =
                        payload.leddata[byteIndex + 1] |       //R
                        payload.leddata[byteIndex + 2] << 8 |  //G
                        payload.leddata[byteIndex + 3] << 16;  //B    
                }
            }
        }
    }
}
