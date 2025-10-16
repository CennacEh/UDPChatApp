#include <fstream>
#include <iostream>

void createLogFile() {
    std::ofstream file("log.txt");
    file.close();
}

void logMessage(std::string message) {
    std::ofstream file("log.txt", std::ios::app);
    file << message << std::endl;
    file.close();
}