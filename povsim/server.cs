using System;
using System.Threading.Tasks;
using Microsoft.AspNetCore.SignalR.Client;

/*
Handled incoming POV LED data update requests. 
Acts as SignalR client. It is assumed the SignalR hub is located on localhost.
Received LED data is passed on to the hub.

 */

 namespace PovSim
 {
     class Server
     {
        protected HubConnection hubConn;

        protected async void HandlePovDataUpdate(object sender, PovDataUpdateEventArgs e)
        {
            Console.WriteLine("SignalR client: Received this message: {0}", e.tmp);

            try
            {
                Console.WriteLine("SignalR client: sending UpdatePovData to Hub");
                await hubConn.InvokeAsync("UpdatePovData", 
                    "fakePovData");
            }
            catch (Exception ex)
            {        
                Console.WriteLine("SignalR client: failed to send to Hub");
                Console.WriteLine(ex.Message);                
            }

        }

        protected async Task RetryConnection()
        {
            try
            {
                await hubConn.StartAsync();
                Console.WriteLine("SignalR client: Connection started");
            }
            catch (Exception ex)
            {
                Console.WriteLine("SignalR client: Connection failed");
                Console.WriteLine(ex.Message);
                Console.WriteLine("Retrying...");
                await Task.Delay(new Random().Next(0,5) * 1000);
                await RetryConnection();
            }
        }

        public async void Start()
        {
            hubConn = new HubConnectionBuilder()
                .WithUrl("http://localhost:5201/povsim")
                .Build();

            hubConn.Closed += async (error) =>
            {
                Console.WriteLine("SignalR client:  closed");
                await RetryConnection();
            };

            hubConn.On<string, string>("ReceiveMessage", (user, message) =>
            {
                var newMessage = $"SignalR Client received from {user}: {message}";
                Console.WriteLine(newMessage);
            });

            hubConn.On<string>("UpdatePOV", (message) =>
            {
                var newMessage = $"SignalR Client: UpdatePOV : {message}";
                Console.WriteLine(newMessage);
            });

            await RetryConnection();            

            SynchronousSocketListener sockListener = new SynchronousSocketListener();
            sockListener.PovDataUpdate += HandlePovDataUpdate;
            await Task.Run(() => {sockListener.StartListening();});
        }
     }
         
 }