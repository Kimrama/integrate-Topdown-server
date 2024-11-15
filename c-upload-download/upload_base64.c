#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t mod_table[] = {0, 2, 1};
    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t combined = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(combined >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(combined >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(combined >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(combined >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = '\0';
    return encoded_data;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    return size * nmemb;
}

int upload_file(const char *url, const char *filename) {
    CURL *curl;
    CURLcode res;
    FILE *file;
    size_t file_size;
    unsigned char *file_data;
    char *base64_data;
    size_t base64_length;

    file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_data = malloc(file_size);
    if (file_data == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return 1;
    }
    fread(file_data, 1, file_size, file);
    fclose(file);

    base64_data = base64_encode(file_data, file_size, &base64_length);
    free(file_data);

    if (base64_data == NULL) {
        fprintf(stderr, "Failed to encode file to Base64\n");
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        free(base64_data);
        return 1;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char post_data[base64_length + 100];  // Ensure the buffer is large enough
    snprintf(post_data, sizeof(post_data), "{\"file_name\":\"%s\",\"data\":\"%s\"}", filename, base64_data);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(base64_data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    free(base64_data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <url> <file_path>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];       // URL to upload the file
    const char *filename = argv[2];  // Path to the file to be uploaded

    if (upload_file(url, filename) == 0) {
        printf("File uploaded successfully!\n");
    } else {
        printf("File upload failed.\n");
    }

    return 0;
}
