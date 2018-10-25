using System;  
using System.Net;  
using System.Net.Sockets;  
using System.Text;  
  
// Client app is the one sending messages to a Server/listener.   
// Both listener and client can send messages back and forth once a   
// communication is established.  
public class SocketClient  
{  
  
    public static int Tx(string serverIp, int serverPort, byte[] txData, out byte[] rxData)
    {  
        byte[] bytes = new byte[1024];
        int bytesRec = 0;
  
        try  
        {  
            // Connect to a Remote server  
            // Get Host IP Address that is used to establish a connection  
            // In this case, we get one IP address of localhost that is IP : 127.0.0.1  
            // If a host has multiple addresses, you will get a list of addresses
            Console.WriteLine(serverIp);
            IPAddress[] addresses = Dns.GetHostAddresses(serverIp);  
            IPAddress ipAddress = addresses[0];  
            IPEndPoint remoteEP = new IPEndPoint(ipAddress, serverPort);  
  
            // Create a TCP/IP  socket.    
            Socket sender = new Socket(ipAddress.AddressFamily,  
                SocketType.Stream, ProtocolType.Tcp);  
  
            // Connect the socket to the remote endpoint. Catch any errors.    
            try  
            {  
                // Connect to Remote EndPoint  
                sender.Connect(remoteEP);  
  
                Console.WriteLine("Socket connected to {0}",  
                    sender.RemoteEndPoint.ToString());  
  
                // Encode the data string into a byte array.    
                //byte[] msg = Encoding.ASCII.GetBytes("This is a testtttt<EOF>");  
  
                // Send the data through the socket.    
                int bytesSent = sender.Send(txData);  
  
                // Receive the response from the remote device.    
                bytesRec = sender.Receive(bytes);  
                Console.WriteLine("Echoed test = {0}",  
                    Encoding.ASCII.GetString(bytes, 0, bytesRec));  
  
                // Release the socket.    
                sender.Shutdown(SocketShutdown.Both);  
                sender.Close();  
  
            }  
            catch (ArgumentNullException ane)  
            {  
                Console.WriteLine("ArgumentNullException : {0}", ane.ToString());  
            }  
            catch (SocketException se)  
            {  
                Console.WriteLine("SocketException : {0}", se.ToString());  
            }  
            catch (Exception e)  
            {  
                Console.WriteLine("Unexpected exception : {0}", e.ToString());  
            }  
  
        }  
        catch (Exception e)  
        {  
            Console.WriteLine(e.ToString());  
        }  

        rxData = bytes;
        return bytesRec;
    }  
} 