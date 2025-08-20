// test.cpp
#include <cassert> // Include assertions for test cases
#include <vector> // Include vector for dynamic arrays
#include <string> // Include string for text manipulation
#include <fstream> // Include fstream for file operations
#include <iostream> // Include iostream for console output
#include <nlohmann/json.hpp> // Include JSON library for parsing
#include "log_parser.h" // Include log_parser.h for function declarations
using json = nlohmann::json; // Use nlohmann::json for JSON manipulation

void testParseLogs() { // Test log parsing functionality
    std::ofstream out("test.json");
    out << R"([{"timestamp":"2025-08-07T08:00:00Z","message":"Failed password for admin"},{"timestamp":"2025-08-07T08:01:00Z","message":"User login successful"}])";
    out.close();
    auto entries = parseLogs("test.json");
    assert(entries.size() == 2);
    assert(entries[0].timestamp == "2025-08-07T08:00:00Z");
    assert(entries[0].message == "Failed password for admin");
    assert(entries[0].severity == EventSeverity::HIGH);
    assert(entries[1].severity == EventSeverity::LOW);
}

void testFilterSecurityEvents() { // Test filtering of security events
    std::vector<LogEntry> entries = { // Create test log entries
        {"2025-08-07T08:00:00Z", "Failed password for admin", EventSeverity::HIGH},
        {"2025-08-07T08:01:00Z", "User login successful", EventSeverity::LOW}
    };
    auto filtered = filterSecurityEvents(entries); // Call filtering function
    assert(filtered.size() == 1); // Check filtered size
    assert(filtered[0].message == "Failed password for admin"); // Check filtered message
}

int main() { // Main function to run tests
    testParseLogs();
    testFilterSecurityEvents();
    std::cout << "All tests passed!" << std::endl; // Print success message
    return 0;
}