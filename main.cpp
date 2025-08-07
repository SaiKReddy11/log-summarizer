//main.cpp
#include <nlohmann/json.hpp>  // Include the JSON library
#include <fstream> // Include for file operations
#include <vector> // Include for vector operations
#include <iostream> // Include for input/output operations
#include "log_entry.h" // Include the LogEntry struct definition
#include "ollama_api.h" // Include the Ollama API 
using json = nlohmann::json; // Use the nlohmann::json namespace for convenience

// Function to parse logs from a JSON file
std::vector<LogEntry> parseLogs(const std::string& filePath) { 
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("Cannot open log file"); // Check if the file can be opened
    json j;
    try { // Attempt to parse the JSON file
        file >> j;
    } catch (...) { // Catch any exceptions during parsing
        throw std::runtime_error("Invalid JSON format");
    }
    std::vector<LogEntry> entries; // Vector to hold parsed log entries
    for (const auto& item : j) { // Iterate through each item in the JSON array
        LogEntry entry; // Create a LogEntry object
        entry.timestamp = item.value("timestamp", ""); // Extract timestamp, default to empty string if not found
        entry.message = item.value("message", ""); // Extract message, default to empty string if not found
        entries.push_back(entry); // Add the entry to the vector
    }
    return entries;
}

// Filter security-critical logs
std::vector<LogEntry> filterSecurityEvents(const std::vector<LogEntry>& entries) {
    std::vector<LogEntry> filtered;
    std::copy_if(entries.begin(), entries.end(), std::back_inserter(filtered),
                 [](const LogEntry& e) {
                     std::string msg = e.message;
                     std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
                     return msg.find("failed") != std::string::npos;
                 });
    return filtered;
}

int main(int argc, char* argv[]) { // Main function to handle command line arguments and call parseLogs
    if (argc != 2) { // Check if the correct number of arguments is provided
        std::cerr << "Usage: " << argv[0] << " <logfile.json>" << std::endl;
        return 1;
    }
    try { // Try to parse the logs from the provided file path
        auto entries = parseLogs(argv[1]); // Call the parseLogs function with the provided file path
        if (entries.empty()) {
            std::cerr << "No valid log entries found" << std::endl;
            return 1;
        }

        // Test parsing
        for (const auto& entry : entries) { // Iterate through the parsed log entries
            std::cout << entry.timestamp << ": " << entry.message << std::endl;
        }

        // Filter security-critical logs
        auto securityEntries = filterSecurityEvents(entries);
        if (securityEntries.empty()) {
            std::cout << "No security-critical events found" << std::endl;
            return 0;
        }

        // Prompt for Ollama API
        std::string prompt = "Summarize the given security-critical events in easy to understand bullet points for immediate action\n";
        for (const auto& entry : securityEntries) {
            prompt += "- " + entry.timestamp + ": " + entry.message + "\n";
        }

        // Get OLLAMA summary
        std::string report = sendToOllama(prompt);
        std::cout << "Security Report:\n" << report << std::endl;

    } catch (const std::exception& e) { // Catch any exceptions thrown during parsing
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}