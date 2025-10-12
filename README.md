# An unsecure UDP chat app!

## How to use
Start up Server and Clients can connect to your server, however keep in mind that this will allow any udp request to go through, I'm not even expecting people to use this.
For a Client to connect, the IP is the IP of the device where the Server is hosted in, for localhost use the ip 127.0.0.1 and port is by default 4218.
## Build
Run the build.sh file (For g++ users)
Or you can run the g++ command
```bash
build src/client.cpp -o client.exe -lws2_32
build src/server.cpp -o server.exe -lws2_32
```
This code is made and tested on Windows 10.