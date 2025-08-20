// log_parser.cpp
#include "log_parser.h" // Include necessary headers
#include <nlohmann/json.hpp> // Include JSON library for parsing
#include <fstream> // Include fstream for file operations
#include <algorithm> // Include algorithm for std::transform
#include <cctype> // Include cctype for character classification
using json = nlohmann::json; // Use nlohmann::json for JSON manipulation

// Parse JSON logs with schema validation
std::vector<LogEntry> parseLogs(const std::string& filePath) { // Function to parse logs
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("Cannot open log file: " + filePath); // Check if file is open
    json j;
    try { file >> j; } catch (...) { throw std::runtime_error("Invalid JSON format in " + filePath); }
    json schema = R"({
        "type": "array",
        "items": {
            "type": "object",
            "required": ["timestamp", "message"],
            "properties": {
                "timestamp": {"type": "string"},
                "message": {"type": "string"}
            }
        }
    })"_json; // Define JSON schema
    if (!j.is_array()) throw std::runtime_error("JSON must be an array");
    for (const auto& item : j) {
        if (!item.is_object() || !item.contains("timestamp") || !item.contains("message") ||
            !item["timestamp"].is_string() || !item["message"].is_string()) {
            throw std::runtime_error("JSON does not match required schema");
        }
    }
    std::vector<LogEntry> entries; // Create a vector to hold log entries
    for (const auto& item : j) {
        LogEntry entry;
        entry.timestamp = item.value("timestamp", "");
        entry.message = item.value("message", "");
        if (!entry.timestamp.empty() && !entry.message.empty()) {
            std::string msg = entry.message;
            std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
            if (msg.find("failed") != std::string::npos || msg.find("unauthorized") != std::string::npos) {
                entry.severity = EventSeverity::HIGH;
            } else if (msg.find("suspicious") != std::string::npos) {
                entry.severity = EventSeverity::MEDIUM;
            } else {
                entry.severity = EventSeverity::LOW;
            }
            entries.push_back(entry);
        }
    }
    return entries; // Return parsed log entries
}

// Filter security-critical logs
std::vector<LogEntry> filterSecurityEvents(const std::vector<LogEntry>& entries) { // Function to filter security events
    std::vector<LogEntry> filtered; // Create a vector to hold filtered log entries
    std::copy_if(entries.begin(), entries.end(), std::back_inserter(filtered),
                 [](const LogEntry& e) { return e.severity == EventSeverity::HIGH; });
    return filtered;  // Return filtered log entries
}