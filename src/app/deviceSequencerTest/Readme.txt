This is a test app for nmb_DeviceSequencer which is for imposing a 
(possibly partial) order on commands and data acquisition among 
multiple devices.

There are separate client and server programs compiled using "gmake client"
and "gmake server".

The device code is in testDevice.[Ch] and it is designed with two types of
messages. One is a request/send pair of messages that represents a modification
command and response. The other is a periodically sent data message representing
something which just samples as fast as it can such (e.g., french ohmmeter).
It is important to recognize that the modify response message is not actually
necessary in order for this code to serialize rpc calls properly.

The server sends periodic data once per second no matter what the client does.
The client requests modifications every 0.7 seconds.
The sequence is specified in client.C and basically says that the program
should alternate between receiving 10 data messages and doing whatever
modifications have been requested. Since modifications occur at about 1.4 times
the rate of periodic data acquisition we expect to get about 14 modification
steps performed per cycle. We can trade off between getting more data per
modification cycle and getting less data per cycle but doing fewer modification
steps per cycle by changing the number of data message repeats per cycle or
by somehow changing the rate at which modification requests are made. It is
up to the user to determine what the most appropriate balance is between
modification and data acquisition.

The .dsw/.dsp files are for compiling the server on a pc in MSVC++ 5.0

Note: the server machine is hard-coded in client.C and will need to be
changed depending on where you run the server.
