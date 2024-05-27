#include <string>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 100
typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} Line;

typedef struct {
    Line *lines;
    size_t count;
    size_t capacity;
} TextStorage;

void initLine(Line *line) {
    line->capacity = INITIAL_CAPACITY;
    line->length = 0;
    line->text = (char *)malloc(line->capacity * sizeof(char));
    line->text[0] = '\0';
}

void initTextStorage(TextStorage *storage) {
    storage->capacity = INITIAL_CAPACITY;
    storage->count = 0;
    storage->lines = (Line *)malloc(storage->capacity * sizeof(Line));
    initLine(&storage->lines[storage->count++]);
}

void appendText(TextStorage *storage, size_t lineIndex, const char *text) {
    if (lineIndex >= storage->count) {
        printf("Line index out of bounds\n");
        return;
    }

    Line *line = &storage->lines[lineIndex];
    size_t newLength = line->length + strlen(text);

    if (newLength >= line->capacity) {
        line->capacity = newLength + 1;
        line->text = (char *)realloc(line->text, line->capacity * sizeof(char));
    }

    strcat(line->text, text);
    line->length = newLength;
}

void addNewLine(TextStorage *storage) {
    if (storage->count >= storage->capacity) {
        storage->capacity *= 2;
        storage->lines = (Line *)realloc(storage->lines, storage->capacity * sizeof(Line));
    }

    initLine(&storage->lines[storage->count++]);
    printf("New line is started\n");
}

void printText(const TextStorage *storage) {
    for (size_t i = 0; i < storage->count; ++i) {
        printf("%s\n", storage->lines[i].text);
    }
}

void insertText(TextStorage *storage, size_t lineIndex, size_t position, const char *text) {
    if (lineIndex >= storage->count) {
        printf("Line index out of bounds\n");
        return;
    }

    Line *line = &storage->lines[lineIndex];
    if (position > line->length) {
        printf("Position out of bounds\n");
        return;
    }

    size_t newLength = line->length + strlen(text);
    if (newLength >= line->capacity) {
        line->capacity = newLength + 1;
        line->text = (char *)realloc(line->text, line->capacity * sizeof(char));
    }

    memmove(line->text + position + strlen(text), line->text + position, line->length - position + 1);
    memcpy(line->text + position, text, strlen(text));
    line->length = newLength;
}

void saveToFile(const TextStorage *storage, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (!file) {

        file = fopen(filename, "w");
        if (!file) {
            printf("Error opening file for writing\n");
            return;
        }
    }

    for (size_t i = 0; i < storage->count; ++i) {
        fprintf(file, "%s\n", storage->lines[i].text);
    }

    fclose(file);
    printf("Text has been saved successfully\n");
}


void loadFromFile(TextStorage *storage, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file for reading\n");
        return;
    }

    for (size_t i = 0; i < storage->count; ++i) {
        free(storage->lines[i].text);
    }
    free(storage->lines);

    initTextStorage(storage);

    char buffer[INITIAL_CAPACITY];
    while (fgets(buffer, INITIAL_CAPACITY, file)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        appendText(storage, storage->count - 1, buffer);
        addNewLine(storage);
    }

    fclose(file);
    printf("Text has been loaded successfully\n");
}

void searchSubstring(const TextStorage *storage, const char *substring) {
    int found = 0;

    for (size_t i = 0; i < storage->count; ++i) {
        const char *line = storage->lines[i].text;
        const char *pos = strstr(line, substring);
        while (pos) {
            printf("Text is present in this position: %zu %zu\n", i, pos - line);
            found = 1;
            pos = strstr(pos + 1, substring);
        }
    }

    if (!found) {
        printf("Substring not found\n");
    }
}

void clearConsole() {
    printf("\033[2J\033[1;1H");
}

// Enum to represent the commands
typedef enum {
    append_text = 1,
    start_new_line,
    save_to_file,
    load_from_file,
    print_current_text,
    insert_text_by_index,
    search_text,
    exit_program = 0
} Command;

//Function for printing help information
void printHelpInfo()
{
    printf(">Choose the command:\n");
    printf("1. Append text symbols to the end\n");
    printf("2. Start a new line\n");
    printf("3. Save text to file\n");
    printf("4. Load text from file\n");
    printf("5. Print the current text to console\n");
    printf("6. Insert text by line and symbol index\n");
    printf("7. Search text\n");
    printf("0. Exit\n");
}

int main() {
    TextStorage storage;
    initTextStorage(&storage);

    int command;
    char buffer[INITIAL_CAPACITY];

    while (1) {
        clearConsole();
        printHelpInfo();
        printf("> ");
        scanf("%d", &command);
        getchar(); // Consume the newline character left by scanf

        switch (command) {
            case append_text:
                printf("Enter text to append: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                appendText(&storage, storage.count - 1, buffer);
                break;
            case start_new_line:
                addNewLine(&storage);
                break;
            case save_to_file:
                printf("Enter the file name for saving: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                saveToFile(&storage, buffer);
                break;
            case  load_from_file:
                printf("Enter the file name for loading: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                loadFromFile(&storage, buffer);
                break;
            case print_current_text:
                printText(&storage);
                break;
            case insert_text_by_index: {
                size_t lineIndex, position;
                printf("Choose line and index: ");
                scanf("%zu %zu", &lineIndex, &position);
                getchar(); // Consume the newline character left by scanf
                printf("Enter text to insert: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                insertText(&storage, lineIndex, position, buffer);
                break;
            }
            case search_text:
                printf("Enter text to search: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                searchSubstring(&storage, buffer);
                break;
            case  exit_program :
                printf("Exiting the program.\n");
                for (size_t i = 0; i < storage.count; ++i) {
                    free(storage.lines[i].text);
                }
                free(storage.lines);
                return 0;
            default:
                printf("The command is not implemented\n");
        }
    }

    return 0;
}
