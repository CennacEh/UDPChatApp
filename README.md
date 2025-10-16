# An unsecure UDP chat app!

## How to use
### Server
Simply run the server.exe file and input a port that will be used, make sure it's not already used by another app!
### Client
For a Client to connect, Use the IP of the device where the Server is hosted in, for localhost use the IP `127.0.0.1`, Port is `4218` by default but is changeable!
Name can be anything but an empty value.
Keep in mind that this will allow any udp request to go through, I'm not even expecting people to use this.
I will try to add a thing to prevent that.
You can use the client in either the GUI of the app or Console.
If you do not want to show the GUI then run this command.
```bash
client.exe -nogui
```
If you want to pre-enter the values then you may do this. (Experimental, not 100% completed!)
```bash
client.exe -ip <ip address> -port <port> -name <name>
client.exe -ip 127.0.0.1 -port 4218 -name cennac
```

## Build
### Make
```bash
make
```
### GNU C++ Compiler (g++)
```bash
build src/client/client.cpp -o build/client.exe -lws2_32
build src/server/server.cpp -o build/server.exe -lws2_32
```
### Note
The code will be compiled inside the build folder of the project directory. this only works for Windows 10/11! (Only tested in Windows 10)