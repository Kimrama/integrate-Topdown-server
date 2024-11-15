#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>

struct MemoryChunk {
    char *memory;
    size_t size;
};


size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct MemoryChunk *mem = (struct MemoryChunk *)userp;

    char *ptr = realloc(mem->memory, mem->size + real_size + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Not enough memory to allocate buffer\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->memory[mem->size] = 0;

    return real_size;
}


int download_file(const char *url, const char *output_filename) {
    CURL *curl;
    CURLcode res;
    struct MemoryChunk chunk = {malloc(1), 0}; 

    if (chunk.memory == NULL) {
        fprintf(stderr, "Failed to allocate initial memory\n");
        return 1;
    }


    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        free(chunk.memory);
        return 1;
    }

  
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);


    res = curl_easy_perform(curl);

 
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        
        FILE *output_file = fopen(output_filename, "wb");
        if (output_file == NULL) {
            fprintf(stderr, "Failed to open file for writing: %s\n", output_filename);
            free(chunk.memory);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 1;
        }
        fwrite(chunk.memory, 1, chunk.size, output_file);
        fclose(output_file);
        printf("File downloaded successfully and saved as %s\n", output_filename);
    }


    free(chunk.memory);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return (res == CURLE_OK) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <url> <output_file_path>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];         
    const char *output_filename = argv[2];  

    return download_file(url, output_filename);
}
