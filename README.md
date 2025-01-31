# Academic linux rootkit with a Command & Control
![License](https://img.shields.io/badge/License-GPL%20V3-purple?labelColor=cian&style=for-the-badge)
![Version](https://img.shields.io/badge/Version-1.0-purple?labelColor=cyan&style=for-the-badge)
![Visual Studio Code](https://img.shields.io/badge/Visual%20Studio%20Code-0078d7.svg?style=for-the-badge&logo=visual-studio-code&logoColor=white)
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C#](https://img.shields.io/badge/C%23-239120?style=for-the-badge&logo=csharp&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Arch](https://img.shields.io/badge/Arch_Linux-1793D1?style=for-the-badge&logo=arch-linux&logoColor=white)

<p align="center">
<img src="https://upload.wikimedia.org/wikipedia/commons/d/d6/Linux_mascot_tux.png" alt="rootkit" width="20%" height="20%" />
</p>
<h2 style="text-align:center;">This is an academic rootkit, as a final degree project.</h2>

## 🔍​ Features

- Keylogger.
- Binary loader in memory with super user permissions that can run a payload were sent from Command & Control.
- Sniffing and capturing network packets and sending to Command & Control.
- Capture of TLS encryption keys from the browser and send to Command & Control.
- Hijacking system calls to hide rootkit.
- Privilege escalation using a device in /dev
- Proof of concept of how to sniff information from the process memory.

    * This proof of concept consists of stealing a private key from an ethereum wallet (InifityWallet)
 

## 📝 Requirements
The following packages are required to compile:
 - makefile
 - build-essential
 - kernel headers
 - dotnet-sdk

## ⚙️ Configuring
- To compile the project it is necessary to specify the full path of the `reflectiveLoader` file in `binaryBlob.S` as shown in the block below.
 ~~~
.incbin "/xxxx/xxxx/xxxx/Rootkit/build/reflectiveLoader"
~~~

- To configure which functions will be available, edit the variables in the `/src/main.c` file.
 ~~~
#define verbose 0

#define keylogger 1 /*Requiere sendNetworkPackets*/ /*Requiere socket*/
#define binatyLoader 1 
#define SSLKeys 1 /*Requiere charRoot*/ /*Requiere socket*/
#define charRoot 1 /* Char device*/
#define sendNetworkPackets 1 
#define networkCC 1 /* Network socket*/
#define hideDentry 1 /* Hiding you in the file system */
#define hideModuleList 1 /* Hides you from the list of kernel modules (but can't be unloaded) */
#define pocInifinityWallet 1
 ~~~

- To use qemu in a simple way you also have to configure the config.sh file  and then load it with `source ./config.sh`


 ~~~
hardDisk="path to linux qemu disk"
~~~


## 🚀 Building

- Run make all will compile and copy all binaries files in /build folder.
these files are:
    * ***CheckRoot*** -> This program check that is running in root mode. 
    * ***Command_and_Control*** -> Command & Control binary.
    * ***reflectiveLoader*** -> this program loads a payload that was sended from the command and control.
    * ***reverseShell*** -> This program open a reverse shell that can be listened from netcat program.
    ***roortkit.ko*** -> The roorkit kernel module file.
- Run make also will copy few files to /qemu/ isshared (this folder shared between the virtual machine and host)
    * ***load.sh*** -> This script simulates a roorkit loading.
    * ***rootkit.ko*** -> The roorkit kernel module file.
- Run clean will delete all compiled files.
- You can also compile the different parts, such as the tools or Command & Control, for that see the file makefile

## 💻​ How to use

- First run the Command & Control .

- A script is included that simulate the rootkit kernel module load, the first load implant the persistence in the system, and patch the systemd variables for the all childs process export the ssl keys.

- Next, the session must then be closed, to simulate a kernel module load with persistence. This also activates all other functions.

The script guides in this process.

The `config.sh` file allows to launch a qemu virtual machine in a simpler way (with the configuration in a single command).

The command & Control generates some files where it stores the data received from the roorkit. ***there are example files***
 * ***keyLogger.txt***
 * ***log.txt***
 * ***networkData.txt***
 * ***SSLKEYLOGFILE.txt***
 * ***ethereumPrivateKey.txt***

To convert the data packets to .pcap files use the command
 ~~~
 cat xx.x.xxxx_networkData.txt | text2pcap - dump.pcap
~~~

To create a server for the reverse shell use the following command.
~~~
nc -lvnp 9001 
~~~


## 📷​​ Screen Shots
</p>
<p style="text-align:center;">Command & Control main window</p>
<p align="center">
<img src="./resources/captura.png" alt="rootkit" />

</p>
<p style="text-align:center;">SSL firefox keys</p>
<p align="center">
<img src="./resources/ssl.png" alt="rootkit" />

</p>
<p style="text-align:center;">Ethereum private key whit wallet password "gatoloco12345"</p>
<p align="center">
<img src="./resources/Ethereum.png" alt="rootkit" />

</p>
<p style="text-align:center;">Ethereum private key InfinityWallet</p>
<p align="center">
<img src="./resources/ethereum 2.png" alt="rootkit" />

</p>
<p style="text-align:center;">Send payload</p>
<p align="center">
<img src="./resources/payload.png" alt="rootkit" />

</p>
<p style="text-align:center;">Reverse Shell Netcat (you can see the root user with whoami command) </p>
<p align="center">
<img src="./resources/reverseshell.png" alt="rootkit" />


## License

[GPLv3](https://opensource.org/licenses/)



