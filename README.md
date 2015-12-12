# knx-Bus-Simulator

README ME - UNDERSTAND ME !!!
=============================

Before we start:
----------------

Please note that, however you use this toolset, which is provided as is and without any warranty or suitability for a specific purpose,
nothing contained herein has been run through any certification or validation of the KNX organisation.
It is provided as starting point and in the hope that it may be useful to you.

The provided code is nothing for the faint hearted. There are little comments for now.

All intelligence required about the EIB/KNX bus to develop the code contained herein was gained by reading:

"Serial Data Transmission and KNX Protocol", provided by KNX Association,
"EIB-TP-UART-IC", provided by Siemens, Regensburg

Both of these documents are freely available in the internet provided by their respective copyright holders.

!!!!! DO NOT USE ANY OF THIS SOFTWARE IN A PRODUCTIVE EIB/KNX BUS INSTALLATION !!!!!

Now we start:
-------------

This toolset was developed in order to facilitate the development and testing of scripts which run my own home automation, based
on KNX bus. As I'm primarily running Mac OS, at least as long as I don't have to program KNX devices with ETS4, the natural choice became
Mac OS for running this toolset. On the other hand there's the need to hook up to some real H/W, which in my case comes down to a Raspberry
Pi extended with a Busware TPUART interface (which I can highly recommend).
Everything will compile on Mac OS (using __MACH__ pre-compiler setting) and Raspberry Pi (without __MACH__pre-compiler setting).
Nothing else is planned to be supported!

Some of the provided files stem from a former tryout and will not work for now!

'nodedata.h' contains a data table identifiying KNX group objects. But let's start at the beginning.

knxbridge is the core of the toolset. knxbridge simulates the knxbus by providing a couple of message buffers, inbound and outbound.
In simulation mode knxbridge will - simulating the real TPUART - copy everything to be transmitted back to the receive buffer. It
is the central point of communication and the process needs to be started first.

knxmon is a knx monitor and ALL GROUP OBJECT object server. Right now it only suports DPT 1.xxx and DPT.9xxx, but it is planned to
support all known DPTs, which is merely a matter of time to implement than a technical challenge. This one requires a table, nodedata.h, defining all the group addresses supposed to be monitored. Monitored values are required e.g. by the hdlpellet or hdlsolar, which need some temperatures provided by the real temperature sensors. In fact, the simulation allows you to send such values so that the handler, e.g. hdlpellet, can be tested.

sendbit is used for sending a DPT 1.xxx value to some arbitrary receiver (sendbit -s <SENDER ADDRESS> -r <RECEIVING GROUP> -v <VALUE; 0 or
1>)

sendfloat is used for sending a DPT 9.xxx value to some arbitrary receiver (sendfloat -s ... -r ... -v <VALUE>)

hdlpellet is my current - VERY SIMPLE - implementation of handling a pellet stove.

hdlsolar does the same for the solar collectors.


In case you need assistance, please feel free to contact as needed.
