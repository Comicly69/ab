#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mongoc/mongoc.h>

// MongoDB connection information
#define MONGO_URI "mongodb+srv://comicly:cheeselover0@about.yaek1bk.mongodb.net/"
#define DB_NAME "user_info"
#define COLLECTION_NAME "users"

// Function to retrieve user information from the database
char* get_user_info(const char* username) {
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_t* query;
    mongoc_cursor_t* cursor;
    const bson_t* doc;
    char* info = NULL;

    mongoc_init();
    client = mongoc_client_new(MONGO_URI);
    collection = mongoc_client_get_collection(client, DB_NAME, COLLECTION_NAME);

    query = BCON_NEW("username", BCON_UTF8(username));
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    if (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init_find(&iter, doc, "info") && BSON_ITER_HOLDS_UTF8(&iter)) {
            info = strdup(bson_iter_utf8(&iter, NULL));
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return info;
}

// Function to add user information to the database
void add_user_info(const char* username) {
    mongoc_client_t* client;
    mongoc_collection_t* collection;
    bson_error_t error;
    bson_t* doc;
    char filename[512];
    char command[512];
    char info[1024];
    FILE* file;

    mongoc_init();
    client = mongoc_client_new(MONGO_URI);
    collection = mongoc_client_get_collection(client, DB_NAME, COLLECTION_NAME);

    // Construct the file path
    snprintf(filename, sizeof(filename), "/tmp/%s", username);

    // Open the file in nano
    snprintf(command, sizeof(command), "nano %s", filename);
    system(command);

    // Read the contents of the file
    file = fopen(filename, "r");
    if (file) {
        char line[256];
        info[0] = '\0';  // Initialize info buffer
        while (fgets(line, sizeof(line), file)) {
            strcat(info, line);
        }
        fclose(file);

        // Remove the trailing newline character, if present
        size_t len = strlen(info);
        if (len > 0 && info[len - 1] == '\n') {
            info[len - 1] = '\0';
        }

        // Create the BSON document and insert into the database
        doc = BCON_NEW("username", BCON_UTF8(username), "info", BCON_UTF8(info));
        if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error)) {
            fprintf(stderr, "Failed to add user information: %s\n", error.message);
        } else {
            printf("User information added successfully.\n");
        }

        bson_destroy(doc);
    } else {
        fprintf(stderr, "Failed to open the file.\n");
    }

    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

// Main function to handle command line arguments
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <username>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) {
            printf("Usage: %s -a <username>\n", argv[0]);
            return 1;
        }
        add_user_info(argv[2]);
    } else {
        char* user_info = get_user_info(argv[1]);
        if (user_info) {
            printf("%s\n", user_info);
            free(user_info); // Free the allocated memory
        } else {
            printf("User not found.\n");
        }
    }

    return 0;
}
