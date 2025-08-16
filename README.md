# Log Summarizer Bot
A C++ CLI tool with a web-based GUI that parses JSON syslogs, filters security-critical events, and generates summaries using OLLAMA’s llama3 model for SOC analysts.

## Features
- Parses JSON logs with timestamp, message, and severity fields.
- Filters high-severity events (e.g., failed logins, unauthorized access).
- Summarizes events using OLLAMA’s llama3 model.
- Supports console and file output, plus a web-based GUI with file upload.
- Includes JSON schema validation, unit tests, and structured configuration.

## Installation
1. Install OLLAMA: Download from https://ollama.ai and run `ollama serve`.
2. Pull model: `ollama pull llama3`.
3. Install dependencies: `pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-pkgconf mingw-w64-x86_64-gcc`.
4. Build CLI: `g++ -o log_summarizer main.cpp ollama_api.cpp log_parser.cpp -I/mingw64/include -L/mingw64/lib -lcurl`.
5. Build GUI: `g++ -o web_server web_server.cpp ollama_api.cpp log_parser.cpp -I/mingw64/include -L/mingw64/lib -lcurl -lws2_32`.
6. Build tests: `g++ -o test test.cpp log_parser.cpp -I/mingw64/include`.

## Usage
- CLI: `./log_summarizer.exe logs.json [output.txt]`
- GUI: `./web_server.exe` and access `http://localhost:8080`. Upload a JSON log file via the browser.
- Tests: `./test.exe`

## Sample
- Input: `varied_logs.json`
- Output: `sample_output.txt`

## Dependencies
- g++ 15.1.0
- libcurl 8.15.0
- nlohmann/json 3.12.0
- OLLAMA with llama3 model
