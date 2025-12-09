#include "../include/filesystem.h"
#include <iostream>
#include <sstream>
#include <vector>

std::vector<std::string> parseCommand(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void printHelp() {
    std::cout << "\nAvailable commands:\n"
              << "  format                  - Initialize filesystem\n"
              << "  mkdir <path>            - Create directory\n"
              << "  ls <path>               - List directory contents\n"
              << "  write <path> <data...>  - Write file\n"
              << "  read <path>             - Read file\n"
              << "  rm <path>               - Delete file/directory\n"
              << "  fsck                    - Check and repair all blocks\n"
              << "  recover <block_id>      - Recover specific block\n"
              << "  help                    - Show this help\n"
              << "  exit                    - Exit shell\n" << std::endl;
}

int main() {
    std::cout << "=================================\n"
              << "  Self-Healing File System v1.0\n"
              << "=================================\n" << std::endl;
    
    FileSystem fs("./data/fs_storage");
    
    std::cout << "Type 'help' for commands, 'exit' to quit\n" << std::endl;
    
    std::string line;
    while (true) {
        std::cout << "shfs> ";
        std::getline(std::cin, line);
        
        if (line.empty()) continue;
        
        std::vector<std::string> tokens = parseCommand(line);
        if (tokens.empty()) continue;
        
        std::string cmd = tokens[0];
        
        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        else if (cmd == "help") {
            printHelp();
        }
        else if (cmd == "format") {
            fs.format();
        }
        else if (cmd == "mkdir" && tokens.size() >= 2) {
            fs.mkdir(tokens[1]);
        }
        else if (cmd == "ls") {
            std::string path = tokens.size() >= 2 ? tokens[1] : "/";
            auto entries = fs.ls(path);
            
            if (entries.empty()) {
                std::cout << "(empty)" << std::endl;
            } else {
                for (const auto& entry : entries) {
                    std::cout << "  " << entry << std::endl;
                }
            }
        }
        else if (cmd == "write" && tokens.size() >= 3) {
            std::string path = tokens[1];
            std::string data;
            for (size_t i = 2; i < tokens.size(); i++) {
                data += tokens[i];
                if (i < tokens.size() - 1) data += " ";
            }
            fs.writeFile(path, data);
        }
        else if (cmd == "read" && tokens.size() >= 2) {
            std::string data;
            if (fs.readFile(tokens[1], data)) {
                std::cout << "Content: " << data << std::endl;
            }
        }
        else if (cmd == "rm" && tokens.size() >= 2) {
            fs.deleteFile(tokens[1]);
        }
        else if (cmd == "fsck") {
            fs.fsck();
        }
        else if (cmd == "recover" && tokens.size() >= 2) {
            size_t block_id = std::stoul(tokens[1]);
            fs.recover(block_id);
        }
        else {
            std::cout << "Unknown command or invalid arguments. Type 'help' for usage." << std::endl;
        }
    }
    
    return 0;
}