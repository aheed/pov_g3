using System;
using System.IO;
using Microsoft.AspNetCore.SignalR;
using System.Threading.Tasks;

namespace PovSim.Hubs
{
    public class PovSimHub : Hub
    {
        public async Task SendMessage(string user, string message)
        {
            await Clients.All.SendAsync("ReceiveMessage", user, message);
        }


        /*[HubMethodName("Test")]
        public async Task Test(string tmp)
        {
            Console.WriteLine("hub: sending update");
            await Clients.All.SendAsync("Test2", tmp);
        }*/

        [HubMethodName("UpdatePovData")]
        public async Task UpdatePovData(HubPovData hubPovData)
        {
            Console.WriteLine("hub: sending update");
            await Clients.All.SendAsync("UpdatePOV", hubPovData);
        }
    }
}
