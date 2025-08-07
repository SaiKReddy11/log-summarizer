# Log Summarizer Bot
A C++ CLI tool that parses JSON syslogs, filters security-critical events, and generates summaries using OLLAMA’s llama3 model for SOC analysts.

## Features
- Parses JSON logs with timestamp and message fields.
- Filters security-critical events like failed logins and unauthorized access.
- Summarizes events using OLLAMA’s llama3 model.
- Supports optional output file for reports.
- Handles errors for file access, JSON parsing, and API calls.

## Installation
1. Install OLLAMA: Download from https://ollama.ai and run `ollama serve`.
2. Pull model: `ollama pull llama3`.
3. Install dependencies: `pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-pkgconf mingw-w64-x86_64-gcc`.
4. Build: `g++ -o log_summarizer main.cpp ollama_api.cpp -I/mingw64/include -L/mingw64/lib -lcurl`.

## Usage
```bash
./log_summarizer.exe logs.json [output.txt]