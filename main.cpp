// main.cpp
#include <nlohmann/json.hpp> // Include JSON library
#include <fstream> // Include fstream for file operations
#include <vector> // Include vector for dynamic arrays
#include <iostream> // Include iostream for console output
#include "log_parser.h" // Include log_parser.h
#include "ollama_api.h" // Include ollama_api.h
using json = nlohmann::json; // Use nlohmann::json for JSON manipulation

// output file
void writeReport(const std::string& report, const std::string& outputFile) {
    std::ofstream out(outputFile); // Open output file
    if (!out.is_open()) throw std::runtime_error("Cannot write to output file: " + outputFile); // Check if file is open
    out << "Security Report:\n" << report << std::endl; // Write report to file
}

// Main function
int main(int argc, char* argv[]) {
    ToolConfig config = {"", "", false, 1000};
    if (argc < 2 || argc > 3) { // Check command line arguments
        std::cerr << "Usage: " << argv[0] << " <logfile.json> [output.txt]" << std::endl;
        return 1;
    }
    config.logFilePath = argv[1]; // Set log file path
    config.outputFilePath = (argc == 3) ? argv[2] : "";
    try {
        auto entries = parseLogs(config.logFilePath); // Parse log entries
        if (entries.empty()) { // Check if entries are empty
            std::string msg = "No valid log entries found";
            std::cout << msg << std::endl; // Inform user about no valid entries
            if (!config.outputFilePath.empty()) writeReport(msg, config.outputFilePath);
            return 0;
        }
        for (const auto& entry : entries) { // Iterate through log entries
            std::cout << entry.timestamp << ": " << entry.message << " [Severity: " 
                      << (entry.severity == EventSeverity::HIGH ? "HIGH" : 
                          entry.severity == EventSeverity::MEDIUM ? "MEDIUM" : "LOW") << "]" << std::endl;
        }
        auto securityEntries = filterSecurityEvents(entries); // Filter security events
        if (securityEntries.empty()) {
            std::string msg = "No security-critical events found";
            std::cout << msg << std::endl;
            if (!config.outputFilePath.empty()) writeReport(msg, config.outputFilePath);
            return 0;
        }
        std::string prompt = "Summarize the given security-critical events in easy to understand bullet points for immediate action:\n";
        for (const auto& entry : securityEntries) {
            prompt += "- " + entry.timestamp + ": " + entry.message + " [Severity: HIGH]\n";
        }
        std::string report = sendToOllama(prompt); // Send prompt to Ollama API
        std::cout << "Security Report:\n" << report << std::endl;
        if (!config.outputFilePath.empty()) writeReport(report, config.outputFilePath);
    } catch (const std::exception& e) { // Catch any exceptions
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}