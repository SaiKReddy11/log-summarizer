// ollama_api.cpp
#include <curl/curl.h> // Include the cURL library for HTTP requests
#include <nlohmann/json.hpp> // Include the JSON library
#include <string> // Include string library for handling strings
#include "ollama_api.h" // Include the header file for the Ollama API
using json = nlohmann::json; // Use nlohmann::json for JSON handling

// Callback for HTTP response
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data) { // Function to handle the data received from the HTTP request
    data->append(ptr, size * nmemb); // Append the received data to the string
    return size * nmemb;
}
// Function to send a prompt to OLLAMA and get the response
std::string sendToOllama(const std::string& prompt) { 
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("CURL initialization failed"); // Initialize cURL
    std::string response;
    json request = {{"model", "llama3"}, {"prompt", prompt}, {"stream", false}}; // Create a JSON request with model, prompt, and stream options
    std::string requestBody = request.dump(); // Convert the JSON request to a string
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) { // Check if the cURL request was successful
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed");
    }
    curl_easy_cleanup(curl); // Clean up cURL resources
    try { // Parse the JSON response
        json responseJson = json::parse(response);
        return responseJson.value("response", "No valid response from OLLAMA");
    } catch (...) { // Handle any parsing errors
        return "Error parsing OLLAMA response";
    }
}