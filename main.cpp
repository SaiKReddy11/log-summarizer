// main.cpp
#include <nlohmann/json.hpp>  // Include the JSON library
#include <fstream> // Include for file operations
#include <vector> // Include for vector operations
#include <iostream> // Include for input/output operations
#include <cctype> // Include for character classification functions
#include <string> // Include for string operations
#include "log_entry.h" // Include the LogEntry struct definition
#include "ollama_api.h" // Include the Ollama API
using json = nlohmann::json; // Use the nlohmann::json namespace for convenience

// Function to parse logs from a JSON file
std::vector<LogEntry> parseLogs(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("Cannot open log file: " + filePath);
    json j;
    try { // Attempt to parse the JSON file
        file >> j;
    } catch (...) { // Catch any exceptions during parsing
        throw std::runtime_error("Invalid JSON format in " + filePath);
    }
    std::vector<LogEntry> entries; // Vector to hold parsed log entries
    for (const auto& item : j) { // Iterate through each item in the JSON array
        LogEntry entry; // Create a LogEntry object
        entry.timestamp = item.value("timestamp", ""); // Extract timestamp
        entry.message = item.value("message", ""); // Extract message
        if (!entry.timestamp.empty() && !entry.message.empty()) {
            entries.push_back(entry); // Add valid entries to vector
        }
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

// Write report to optional output file
void writeReport(const std::string& report, const std::string& outputFile) {
    std::ofstream out(outputFile);
    if (!out.is_open()) throw std::runtime_error("Cannot write to output file: " + outputFile);
    out << "Security Report:\n" << report << std::endl;
}

// Main function to handle command line arguments and call parseLogs
int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) { // Check for 1 or 2 arguments
        std::cerr << "Usage: " << argv[0] << " <logfile.json> [output.txt]" << std::endl;
        return 1;
    }
    std::string outputFile = (argc == 3) ? argv[2] : "";
    try { // Try to parse the logs from the provided file path
        auto entries = parseLogs(argv[1]); // Parse logs
        if (entries.empty()) {
            std::string msg = "No valid log entries found";
            std::cout << msg << std::endl;
            if (!outputFile.empty()) writeReport(msg, outputFile);
            return 0;
        }

        // Print all logs
        for (const auto& entry : entries) {
            std::cout << entry.timestamp << ": " << entry.message << std::endl;
        }

        // Filter security-critical logs
        auto securityEntries = filterSecurityEvents(entries);
        if (securityEntries.empty()) {
            std::string msg = "No security-critical events found";
            std::cout << msg << std::endl;
            if (!outputFile.empty()) writeReport(msg, outputFile);
            return 0;
        }

        // Prompt for Ollama API
        std::string prompt = "Summarize the given security-critical events in easy to understand bullet points for immediate action:\n";
        for (const auto& entry : securityEntries) {
            prompt += "- " + entry.timestamp + ": " + entry.message + "\n";
        }

        // Get OLLAMA summary
        std::string report = sendToOllama(prompt);
        std::cout << "Security Report:\n" << report << std::endl;
        // Write to output file if specified
        if (!outputFile.empty()) writeReport(report, outputFile);

    } catch (const std::exception& e) { // Catch any exceptions
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}