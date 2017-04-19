using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace CSharpTcpClient
{
    class Program
    {
        public const int PORT = 8080;
        public static void Main()
        {
            TcpClient client = null;
            Console.WriteLine("\nHit enter to start...");
            Console.Read();
            try
            {
                // Set the TcpListener on port 13000.
                // TcpListener server = new TcpListener(port);
                client = new TcpClient(AddressFamily.InterNetwork);


                client.Connect("localhost", PORT);
                Byte[] bytes = new Byte[256];
                int i;
                using (var stream = client.GetStream())
                {
                    using (var output = new FileStream(@"C:/Users/Public/transferred.asf", FileMode.Create))
                    {
                        while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            Console.WriteLine("Received: {0}", bytes.Length);
                            output.Write(bytes, 0, bytes.Length);
                        }
                    }

                }
            }
            catch (SocketException e)
            {
                Console.WriteLine("SocketException: {0}", e);
            }
            finally
            {
                // Stop listening for new clients.
                client.Close();
            }


            Console.WriteLine("\nHit enter to continue...");
            Console.ReadLine();
        }
    }
}
