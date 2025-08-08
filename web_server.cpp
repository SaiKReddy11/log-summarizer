// web_server.cpp
#include <iostream> // Include iostream for console output
#include <string> // Include string for string manipulation
#include <sstream> // Include sstream for string stream
#include <fstream> // Include fstream for file operations
#include <vector> // Include vector for dynamic arrays
#include <algorithm> // Include algorithm for std::sort
#include <winsock2.h> // Include winsock2.h for Windows socket programming
#include <ws2tcpip.h> // Include ws2tcpip.h for TCP/IP socket programming
#include <nlohmann/json.hpp> // Include nlohmann/json.hpp for JSON manipulation
#include "log_parser.h" // Include log_parser.h
#include "ollama_api.h" // Include ollama_api.h
using json = nlohmann::json; // Use nlohmann::json for JSON manipulation

#pragma comment(lib, "Ws2_32.lib")

// Read file content
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Generate HTML response with log analysis
std::string generateLogAnalysis(const std::string& inputFile) {
    std::string response = "<script>document.getElementById('results').innerHTML = `";
    try {
        auto entries = parseLogs(inputFile); // Parse log entries from input file
        if (entries.empty()) {
            response += "<p style=\\\"color: #ff0080;\\\">No valid log entries found</p>";
        } else { // If log entries are found
            response += "<h2 style=\\\"color: #00ff99;\\\">All Logs</h2><ul style=\\\"color: #66ffcc; list-style-position: inside;\\\">";
            for (const auto& entry : entries) {
                response += "<li>" + entry.timestamp + ": " + entry.message + " [Severity: " +
                            (entry.severity == EventSeverity::HIGH ? "HIGH" :
                             entry.severity == EventSeverity::MEDIUM ? "MEDIUM" : "LOW") + "]</li>";
            }
            response += "</ul>"; // Close the list of all logs
            auto securityEntries = filterSecurityEvents(entries);
            if (securityEntries.empty()) {
                response += "<h2 style=\\\"color: #00ff99;\\\">Security Report</h2><p style=\\\"color: #66ffcc;\\\">No security-critical events found</p>";
            } else {
                std::string prompt = "Summarize the given security-critical events in easy to understand bullet points for immediate action:\n";
                for (const auto& entry : securityEntries) {
                    prompt += "- " + entry.timestamp + ": " + entry.message + " [Severity: HIGH]\n";
                }
                std::string report = sendToOllama(prompt);
                response += "<h2 style=\\\"color: #00ff99;\\\">Security Report</h2><pre style=\\\"color: #66ffcc; white-space: pre-wrap;\\\">" + report + "</pre>";
            }
        }
    } catch (const std::exception& e) { // Catch any exceptions
        response += "<p style=\\\"color: #ff0080;\\\">Error: " + std::string(e.what()) + "</p>";
    }
    response += "`;</script>";
    return response;
}

// Parse HTTP POST request to extract uploaded file
std::string extractUploadedFile(const std::string& request) { // Extract uploaded file from HTTP request
    std::string boundary;
    std::string content;
    size_t pos = request.find("boundary="); // Find boundary in the request
    if (pos != std::string::npos) {
        boundary = request.substr(pos + 9, request.find("\r\n", pos) - pos - 9);
    }
    if (!boundary.empty()) { // If boundary is found
        std::string fileStart = "--" + boundary;
        pos = request.find(fileStart);
        if (pos != std::string::npos) { // If file start is found
            pos = request.find("filename=\"", pos);
            if (pos != std::string::npos) { // If filename is found
                pos = request.find("\r\n\r\n", pos) + 4;
                size_t endPos = request.find(fileStart, pos);
                if (endPos != std::string::npos) { // If file end is found
                    content = request.substr(pos, endPos - pos - 2);
                }
            }
        }
    }
    if (!content.empty()) { // If content is found
        std::ofstream out("uploaded_logs.json"); // Open output file
        if (out.is_open()) { // Check if file is open
            out << content;
            out.close();
            return "uploaded_logs.json";
        }
    }
    return "varied_logs.json"; // Fallback
}

int main() { // Main function
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (server_fd == INVALID_SOCKET) { // Check if socket creation failed
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in address; // Create address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address
    address.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) { // Bind socket to address
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR) { // Listen for incoming connections
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Server running at http://localhost:8080\n"; // Inform user about server status
    std::cout << "Access the GUI in your browser and upload a JSON log file.\n"; // Prompt user to access GUI

    std::string indexHtml;
    try {
        indexHtml = readFile("index.html");
    } catch (const std::exception& e) {
        std::cerr << "Error loading index.html: " << e.what() << std::endl;
        indexHtml = "<html><body><h1>Error</h1><p>Cannot load index.html</p></body></html>";
    }

    while (true) { // Main server loop
        SOCKET client_socket = accept(server_fd, nullptr, nullptr); // Accept incoming connections
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        char buffer[4096] = {0}; // Buffer for incoming data
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) { // If data is received
            buffer[bytes_received] = '\0';
            std::string request(buffer);
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
            if (request.find("POST /upload") != std::string::npos) {
                std::string inputFile = extractUploadedFile(request);
                response += indexHtml;
                response += generateLogAnalysis(inputFile);
            } else {
                response += indexHtml;
                response += generateLogAnalysis("varied_logs.json");
            }
            send(client_socket, response.c_str(), response.length(), 0);
        }
        closesocket(client_socket); // Close client socket
    }

    closesocket(server_fd); // Close server socket
    WSACleanup(); // Cleanup Winsock
    return 0;
}