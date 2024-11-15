#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Function to read file into memory
unsigned char* read_file(const char* file_path, size_t* file_size) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file %s for reading\n", file_path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* file_data = (unsigned char*)malloc(*file_size);
    if (!file_data) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(file_data, 1, *file_size, file);
    fclose(file);

    return file_data;
}

// Base64 encoding function
char* base64_encode(unsigned char* input, size_t length) {
    static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_size = 4 * ((length + 2) / 3);
    char* encoded_data = (char*)malloc(output_size + 1);
    size_t i, j = 0;

    for (i = 0; i < length; i += 3) {
        unsigned int value = (input[i] << 16) | (input[i + 1] << 8) | (input[i + 2]);
        encoded_data[j++] = base64_table[(value >> 18) & 0x3F];
        encoded_data[j++] = base64_table[(value >> 12) & 0x3F];
        encoded_data[j++] = base64_table[(value >> 6) & 0x3F];
        encoded_data[j++] = base64_table[value & 0x3F];
    }

    // Add padding
    for (i = 0; i < length % 3; ++i) {
        encoded_data[output_size - 1 - i] = '=';
    }

    encoded_data[output_size] = '\0';
    return encoded_data;
}

// Function to upload base64-encoded file data using a raw HTTP POST request
int upload_file_to_api(const char* url, const char* base64_data, const char* filename) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char *body;
    size_t body_size;

    // Prepare the multipart body
    const char* boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    body_size = strlen(boundary) * 2 + 2 + strlen(filename) + 4 + strlen(base64_data) + 4 + 1;
    body = (char*)malloc(body_size);
    if (!body) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Create multipart/form-data body
    snprintf(body, body_size,
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
             "Content-Type: application/octet-stream\r\n\r\n"
             "%s\r\n"
             "--%s--\r\n", 
             boundary, filename, base64_data, boundary);

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl initialization failed\n");
        free(body);
        curl_global_cleanup();
        return 1;
    }

    // Set up HTTP headers
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the HTTP POST request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Curl request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(body);
        curl_global_cleanup();
        return 1;
    }

    printf("File uploaded successfully!\n");

    // Clean up
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(body);
    curl_global_cleanup();
    return 0;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <wav_file_path>\n", argv[0]);
        return 1;
    }

    const char* file_path = argv[1];
    size_t file_size;

    // Read file into memory
    unsigned char* file_data = read_file(file_path, &file_size);
    if (!file_data) {
        return 1;
    }

    // Encode the file data to base64
    char* base64_encoded = base64_encode(file_data, file_size);
    free(file_data); // Free the raw file data after encoding

    if (!base64_encoded) {
        return 1;
    }

    // Upload the base64-encoded file to the API
    const char* api_url = "http://192.168.43.3:5000/upload"; // Change the URL if necessary
    if (upload_file_to_api(api_url, base64_encoded, file_path) != 0) {
        free(base64_encoded);
        return 1;
    }

    // Free the base64-encoded string
    free(base64_encoded);
    return 0;
}
