#include <stdlib.h> // atoi, getenv, exit, EXIT_SUCCESS, EXIT_FAILURE
#include <stdbool.h> // bool, true, false
#include <stdio.h> // fread, printf
#include <string.h> // strcmp

#include "../vendor/sqlite3.h"

void httpError(int status, const char *reason) {
    printf("Status: %d %s\n", status, reason);
    printf("Content-type: text/plain\n\n");
    printf("HTTP %d %s\n", status, reason);
}

void jsonResponse(const char *jsonString) {
    printf("Content-type: application/json\n\n");
    if (jsonString != NULL) {
        printf("%s\n", jsonString);
    }
}

int callback(void *isFirstRow, int columnCount, char **columns, char **columnNames) {
    bool *isFirstRowp = (bool *) isFirstRow;

    // Array element separator: add comma before every row except the first.
    if (!*isFirstRowp) {
        printf(",");
    }
    *isFirstRowp = false;

    printf("\n  {");

    // Property separator: add comma before every column except the first.
    for (int column = 0; column < columnCount; column++) {
        printf("%s\n    \"%s\": %s", column > 0 ? "," : "", columnNames[column], columns[column]);
    }

    printf("\n  }");

    return 0;
}

int main(void) {
    const char *contentLength = getenv("CONTENT_LENGTH");
    const char *contentType = getenv("CONTENT_TYPE");
    const char *requestPath = getenv("REQUEST_URI");
    const char *requestMethod = getenv("REQUEST_METHOD");
    bool routeMatched = false;
    char buffer[512] = "";
    sqlite3 *db;
    int rc;
    size_t readBytes;
    const char *step;
    bool isFirstRow;

    // Check necessary environment variables.
    if (requestPath == NULL) {
        httpError(500, "Internal Server Error");
        printf("REQUEST_URI not given.\n");
        exit(EXIT_FAILURE);
    }
    if (requestMethod == NULL) {
        httpError(500, "Internal Server Error");
        printf("REQUEST_METHOD not given.\n");
        exit(EXIT_FAILURE);
    }

    // GET/POST /api/check - print content headers and body.
    if (strcmp(requestPath, "/api/check") == 0) {
        printf("Content-type: text/plain\n\n");
        printf("Content-length: %s\n", contentLength);
        printf("Content-type: %s\n", contentType);

        if (contentLength == NULL) {
            printf("No body.\n");
            exit(EXIT_SUCCESS);
        }

        readBytes = (size_t) atoi(contentLength);
        if (sizeof(buffer) - 1 < readBytes) {
            readBytes = sizeof(buffer) - 1;
        }
        readBytes = fread(buffer, 1, readBytes, stdin);
        printf("Body:\n%s\n", buffer);

        if (readBytes < (size_t) atoi(contentLength)) {
            printf("Failed to read entire body. %zu vs. %d\n", readBytes, atoi(contentLength));
        }

        exit(EXIT_SUCCESS);
    }

    // Open DB connection and create table if necessary.
    step = "open database";
    rc = sqlite3_open("./db.sqlite", &db);
    if (rc == SQLITE_OK) {
        step = "create table";
        rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS task (taskId INTEGER);", NULL, NULL, NULL);
    }
    if (rc != SQLITE_OK) {
        httpError(500, "Internal Server Error");
        printf("Error during step \"%s\": %s\n", step, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    // GET /api/task - return task list as JSON.
    if (strcmp(requestMethod, "GET") == 0 && strcmp(requestPath, "/api/task") == 0) {
        routeMatched = true;

        jsonResponse(NULL);
        printf("[");
        isFirstRow = true;

        step = "select row";
        rc = sqlite3_exec(db, "SELECT taskId FROM task;", &callback, &isFirstRow, NULL);

        printf("\n]\n");

        if (rc != SQLITE_OK) {
            httpError(500, "Internal Server Error");
            printf("Error during step \"%s\": %s\n", step, sqlite3_errmsg(db));
        }
    }

    // POST /api/task - insert a task into the DB, creating the DB and table if needed.
    if (strcmp(requestMethod, "POST") == 0 && strcmp(requestPath, "/api/task") == 0) {
        routeMatched = true;

        step = "insert row";
        rc = sqlite3_exec(db, "INSERT INTO task VALUES (456);", NULL, NULL, NULL);

        if (rc == SQLITE_OK) {
            jsonResponse("{\"ok\": true}");
        } else {
            httpError(500, "Internal Server Error");
            printf("Error during step \"%s\": %s\n", step, sqlite3_errmsg(db));
        }
    }

    sqlite3_close(db);

    if (!routeMatched) {
        httpError(404, "Not Found");
    }

    exit(EXIT_SUCCESS);
}
