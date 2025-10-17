#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>

    #define WSAETIMEDOUT ETIMEDOUT 
    #define SOCKET_ERROR -1
#endif
#include <iostream>
#include <string>
#include <regex>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>

#include "../gui/gui.h"

std::atomic<bool> running(true);

std::string ip;
std::string name;
int port = -1;
bool end = false;
std::string userBuffer;
std::vector<std::string> messages;

bool useGui = true;

int getLastError = -1;

#ifdef _WIN32
SOCKET sock;

#else
int sock;

int _kbhit() {
    struct timeval tv = {0, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) == 1;
}

int _getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

sockaddr_in addr{};

void signalHandler(int signalnum) {
    running = false;
}

void updateErrorCode() {
    #ifdef _WIN32
        getLastError = WSAGetLastError();
    #else
        getLastError = errno;
    #endif
}

void start(bool undefined = false) {
    if (undefined) {
        std::cerr << "Invalid input (Not correct IP address or Port, or maybe name is empty)" << std::endl;
        ip = "exit";
        return;
    }
    std::cout << "Enter the server IP: " << std::flush;
    std::getline(std::cin, ip);
    if (ip.empty()) return start(true);
    std::regex pattern(R"(^(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])){3}$)");
    if (!std::regex_match(ip, pattern) && ip != "exit") return start(true);

    std::cout << "Enter the server port: " << std::flush;
    std::cin >> port;
    std::cin.ignore();
    if (port < 0 || port > 65535) return start(true);

    std::cout << "Enter your name: " << std::flush;
    std::getline(std::cin, name);
    if (name.empty()) return start(true);
}

void startWithArgs(int argc, char* argv[]) {
    bool foundIp = false, foundPort = false, foundName = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-nogui") useGui = false;
        else if (arg == "-ip") {
            i++;
            ip = argv[i];
            if (ip.empty()) return start(true);
            std::regex pattern(R"(^(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])){3}$)");
            if (!std::regex_match(ip, pattern) && ip != "exit") return start(true);
            foundIp = true;
        } else if (arg == "-port") {
            i++;
            port = std::stoi(argv[i]);
            if (port < 0 || port > 65535) return start(true);
            foundPort = true;
        } else if (arg == "-name") {
            i++;
            if (i >= argc) return start(true);
            name = argv[i];
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                i++;
                name += " ";
                name += argv[i];
            }
            if (name.empty()) return start(true);
            foundName = true;
        } else {
            std::cerr << "Invalid argument: " << arg << "\nValid arguments:\n -nogui\n -ip <IP address>\n -port <Port>\n -name <Name>\nFor more info please check out:\nhttps://github.com/CennacEh/UDPChatApp/" << std::endl;
        }
    }
    if (!foundIp && !foundPort && !foundName) return start();
    if (!foundIp) {
        std::cout << "Enter the server IP: " << std::flush;
        std::getline(std::cin, ip);
        if (ip.empty()) return start(true);
        std::regex pattern(R"(^(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])){3}$)");
        if (!std::regex_match(ip, pattern) && ip != "exit") return start(true);
    }
    if (!foundPort) {
        std::cout << "Enter the server port: " << std::flush;
        std::cin >> port;
        std::cin.ignore();
        if (port < 0 || port > 65535) return start(true);
    }
    if (!foundName) {
        std::cout << "Enter your name: " << std::flush;
        std::getline(std::cin, name);
        if (name.empty()) return start(true);
    }
}

bool tryConnect() {
    std::string testMes = "CONNECT:" + name;
    int test = sendto(sock, testMes.c_str(), testMes.length(), 0, (sockaddr*)&addr, sizeof(addr));
    updateErrorCode();
    if (test == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server!\nError code: " << getLastError << std::endl;
        return false;
    } else {
        std::cout << "Connecting to server!" << std::endl;
        DWORD timeout = 5000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        char testBuffer[1024];
        sockaddr_in fromAddr{};
        int fromLen = sizeof(fromAddr);
        int recieved = recvfrom(sock, testBuffer, sizeof(testBuffer) - 1, 0, (sockaddr*)&fromAddr, &fromLen);
        updateErrorCode();
        if (recieved == SOCKET_ERROR) {
            int err = getLastError;
            if (err == WSAETIMEDOUT) std::cerr << "Server did not repond in 5 seconds! Is the server even active?\nError code:" << err << std::endl;
            else std::cerr << "Couldn't get a response from server!\nError code: " << err << std::endl;
            return false;
        }
        testBuffer[recieved] = '\0';
        //std::cout << "Response: " << testBuffer << std::endl;
        std::string sBuffer = testBuffer;
        if (sBuffer == "yeah:done") {
            std::cout << "You successfully connected to the server! Say hello!" << std::endl;
        }
    }
    return true;
}

void getMessage() {
    char mbuffer[1024];
    sockaddr_in fromAddr{};
    int len = sizeof(fromAddr);
    DWORD timeout = 10;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    int mess = recvfrom(sock, mbuffer, sizeof(mbuffer) - 1, 0, (sockaddr*)&fromAddr, &len);
    updateErrorCode();
    if (mess == SOCKET_ERROR) {
        int err = getLastError;
        if (err != WSAETIMEDOUT) std::cerr << "Message recieve failed!\nError code: " << err << ", " << mess << ", " << mbuffer << std::endl;
        timeout = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    }
    timeout = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    mbuffer[mess] = '\0';
    std::string mBufferString = mbuffer;
    if (mess > 0 && mBufferString != "pong") {
        std::cout << mbuffer << std::endl;
        messages.push_back(mBufferString);
        if (messages.size() >= 20) {
            messages.erase(messages.begin());
        }
    } else if (mBufferString == "pong") {
        messages.push_back("This is an error.");
    }
}

bool ping() {
    std::string toSend = "ping";
    int sendm = sendto(sock, toSend.c_str(), toSend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    updateErrorCode();
    if (sendm == SOCKET_ERROR) {
        std::cerr << "Couldn't ping Server, Error code: " << getLastError << std::endl;
        return false;
    }
    DWORD timeout = 5000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    char pingBuf[1024];
    sockaddr_in fromAddr{};
    int fromlen = sizeof(fromAddr);
    int recieved = recvfrom(sock, pingBuf, sizeof(pingBuf) - 1, 0, (sockaddr*)&fromAddr, &fromlen);
    updateErrorCode();
    if (recieved == SOCKET_ERROR) {
        int err = getLastError;
        if (err == WSAETIMEDOUT) std::cerr << "Server did not repond in 5 seconds! Is the server even active?\nError code:" << err << std::endl;
        else std::cerr << "Couldn't get a response from server!\nError code: " << err << std::endl;
        return false;
    }
    pingBuf[recieved] = '\0';
    timeout = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    return true;
}

std::string cinNonBlocking() {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == '\r') {
            std::string result = userBuffer;
            userBuffer.clear();
            std::cout << '\n' << std::flush;
            return result;
        } else if (ch == '\b') {
            if (!userBuffer.empty()) {
                userBuffer.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else {
            userBuffer += ch;
            std::cout << ch << std::flush;
        }
    }
    return "";
}

bool sendMessageToSer(std::string msg) {
    std::string tosend = "message:" + msg;
    int sendm = sendto(sock, tosend.c_str(), tosend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    updateErrorCode();
    if (sendm == SOCKET_ERROR) {
        std::cerr << "Failed to send message to server.\nError code: " << getLastError << std::endl;
        return false;
    }
    return true;
}

int discoAttempt = 0;
bool disconnectme() {
    std::string tosend = "disconnectme";
    int sendm = sendto(sock, tosend.c_str(), tosend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    updateErrorCode();
    if (sendm == SOCKET_ERROR) {
        updateErrorCode(); 
        if (discoAttempt > 5) {
            std::cerr << "Tried to disconnect 5 times and failed!\nError code: " << getLastError << std::endl;
            return false;
        }
        disconnectme();
        discoAttempt++;
        std::cerr << "Failed to disconnect, Trying to disconnect again! Attempt: " << discoAttempt << "\nError code: " << getLastError << std::endl;
    }
    std::cout << "Disconnected from server!" << std::endl;
    return true;
}
int main(int argc, char** argv) {
    std::signal(SIGINT, signalHandler);

    std::cout << "Client started!" << std::endl;
    if (argc > 1) startWithArgs(argc, argv); else start();
    if (ip == "exit") return 0;

    #ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    #endif

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    updateErrorCode();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Couldn't create Socket!\nError code: " << getLastError << std::endl;
        return 1;
    }

    addr.sin_family = AF_INET;
    #ifdef _WIN32
        InetPtonA(AF_INET, ip.c_str(), &addr.sin_addr);
    #else
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    #endif

    addr.sin_port = htons(port);

    bool connected = tryConnect();
    if (!connected) {
        std::cerr << "Couldn't connect to " << ip << ":" << port << ", Server may be offline." << std::endl;
        return 1;
    }
    auto last = std::chrono::steady_clock::now();
    if (useGui) initGui();
    while (running) {
        updateErrorCode(); 
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (ms >= 5000 && !ping()) {
            disconnectme();
            break;
        }
        if (ms >= 5000) last = now;
        if (end) {
            disconnectme();
            break;
        }
        std::string toSend = cinNonBlocking();
        if (!toSend.empty()) sendMessageToSer(toSend);
        if (ms >= 1000) {
            getMessage();
        }
        if (useGui) renderGui();
    }
    std::cout << "Closing app :D." << std::endl;
    disconnectme();
    if (useGui) deinitGui();
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
    return 0;
}