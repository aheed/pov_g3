using System;
using System.Linq;
using System.Net;

namespace monpov
{
    class Program
    {
        static private int[] BytesToInts(byte[] bytes, int nofInts)
        {
            int resElems = Math.Min(bytes.Length / sizeof(int), nofInts);
            int[] res = new int[resElems];
            Buffer.BlockCopy(bytes, 0, res, 0, res.Length * sizeof(int));
            if( BitConverter.IsLittleEndian)
            {
                // Flip byte order
                Console.WriteLine("Flipping byte order");
                res = res.Select(x => IPAddress.NetworkToHostOrder(x)).ToArray();
            }

            return res;
        }

        static void Main(string[] args)
        {
            Console.WriteLine("Connecting...");
            byte[] tx = {0x88}; // Anything but 0xFF will do
            byte[] rx;
            int recBytes = SocketClient.Tx(args[0], 10013, tx, out rx);

            Console.WriteLine("Got {0} bytes", recBytes);
            var intRx = BytesToInts(rx, 5);
            foreach (var item in intRx)
            {
                Console.WriteLine(item.ToString());
            }

            /*Console.WriteLine("");
            for(int i=0; i < 16; i++)
            {
                Console.WriteLine("{0}: {1}", i, rx[i]);
            }*/
        }
    }
}
