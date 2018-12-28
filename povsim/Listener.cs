using System;  
using System.Net;  
using System.Net.Sockets;  
using System.Text;  

namespace PovSim
{
    public class PovDataUpdateEventArgs {
        public string tmp;
    }

    public class SynchronousSocketListener {  


        public event EventHandler<PovDataUpdateEventArgs> PovDataUpdate;  


        protected virtual void OnPovDataUpdate(PovDataUpdateEventArgs e)
        {
            Console.WriteLine("SocketListener: OnPovDataUpdate fired");
            // Make a temporary copy of the event to avoid possibility of
            // a race condition if the last subscriber unsubscribes
            // immediately after the null check and before the event is raised.
            EventHandler<PovDataUpdateEventArgs> handler = PovDataUpdate;

            // Event will be null if there are no subscribers
            if (handler != null)
            {
                // Format the string to send inside the CustomEventArgs parameter
                e.tmp += $"POV data update at {DateTime.Now}";

                // Use the () operator to raise the event.
                handler(this, e);
            }
        }

        // Incoming data from the client.  
        public string data = null;  
        povdata.PovFramePayload recPayload = new povdata.PovFramePayload();

        public void StartListening() {  
            // Data buffer for incoming data.  
            byte[] bytes = new Byte[1024];  

            // Establish the local endpoint for the socket.  
            // Dns.GetHostName returns the name of the   
            // host running the application.  
            /*IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());  
            IPAddress ipAddress = ipHostInfo.AddressList[0];  */

            string ipAddr = "127.0.0.1";
            IPAddress ipAddress = null;
            try {
                ipAddress = IPAddress.Parse(ipAddr);
            }
            catch(Exception e)
            {
                Console.WriteLine("SocketListener: Failed to parse IP address " + ipAddr);
                Console.WriteLine(e);
            }
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 10013);  

            // Create a TCP/IP socket.  
            Socket listener = new Socket(ipAddress.AddressFamily,  
                SocketType.Stream, ProtocolType.Tcp );  

            // Bind the socket to the local endpoint and   
            // listen for incoming connections.  
            try {  
                listener.Bind(localEndPoint);  
                listener.Listen(10);

                // Start listening for connections.  
                while (true) {  
                    Console.WriteLine("SocketListener: Waiting for a connexion...");  
                    // Program is suspended while waiting for an incoming connection.  
                    Socket handler = listener.Accept();  
                    handler.ReceiveTimeout = 1000;
                    data = null;  

                    Console.WriteLine("SocketListener: Connected");  

                    // An incoming connection needs to be processed.  
                    int bytesRec = -1;
                    int totalBytesRec = 0;

                    while ((bytesRec != 0) && (totalBytesRec != recPayload.leddata.Length)) {  

                        try {
                            Console.WriteLine("SocketListener: Receiving {0} bytes...", recPayload.leddata.Length - totalBytesRec);  
                            bytesRec = handler.Receive(recPayload.leddata, totalBytesRec, recPayload.leddata.Length - totalBytesRec, 0); 
                            totalBytesRec += bytesRec;
                            Console.WriteLine("SocketListener: Done receiving...");  
                        }
                        catch(Exception e) {
                            Console.WriteLine(e);
                            break;
                        }
                        finally {
                            Console.WriteLine("SocketListener: Got {0} bytes...", bytesRec);
                        }

                        data += Encoding.ASCII.GetString(recPayload.leddata,0,bytesRec);  
                        if (data.IndexOf("<EOF>") > -1) {  
                            break;  
                        }  
                    }  

                    /*if(data != null)
                    {
                        // Show the data on the console.  
                        Console.WriteLine( "Text received : {0}", data);  
                    }*/

                    if(recPayload.leddata.Length == totalBytesRec)
                    {
                        // Got a complete frame
                        // Notify any observer
                        OnPovDataUpdate(new PovDataUpdateEventArgs {tmp = "got something\n"});
                    }

                    handler.Shutdown(SocketShutdown.Both);  
                    handler.Close();  
                }  

            } catch (Exception e) {  
                Console.WriteLine(e.ToString());  
            }  

            Console.WriteLine("\nPress ENTER to continue...");  
            Console.Read();  

        }  

        /*public static int Main(String[] args) {  
            StartListening();  
            return 0;  
        }  */
    }
}