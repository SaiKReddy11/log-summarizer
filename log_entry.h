// log_entry.h
#ifndef LOG_ENTRY_H // Header guard
#define LOG_ENTRY_H // Header guard

#include <string> // Include for string operations

enum class EventSeverity { LOW, MEDIUM, HIGH }; // Enum for event severity levels

struct LogEntry { // Struct for log entries
    std::string timestamp;
    std::string message;
    EventSeverity severity;
};

struct ToolConfig { // Struct for tool configuration
    std::string logFilePath;
    std::string outputFilePath;
    bool enableGUI;
    int maxEvents;
};

#endif