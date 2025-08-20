// log_parser.h
#ifndef LOG_PARSER_H  
#define LOG_PARSER_H  

#include <vector>
#include <string>
#include "log_entry.h"

std::vector<LogEntry> parseLogs(const std::string& filePath); // Parses the log file at the given path
std::vector<LogEntry> filterSecurityEvents(const std::vector<LogEntry>& entries); // Filters security events from the log entries

#endif