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
    int command;

    while (true) {
        printHelpInfo();
        printf("> ");
        scanf("%d", &command);

        switch (command) {
            case  append_text:
                printf("Append text symbols to the end \n");
                break;
            case start_new_line:
                printf("Start a new line \n");
                break;
            case save_to_file:
                printf("Save text to file \n");
                break;
            case  load_from_file:
                printf("Load text from file \n");
                break;
            case  print_current_text:
                printf("Print the current text to console \n");
                break;
            case insert_text_by_index:
                printf("Insert text by line and symbol index \n");
                break;
            case search_text:
                printf("Search text \n");
                break;
            case exit_program:
                printf("Exiting the program.\n");
                return 0;
            default:
                printf("The command is not implemented\n");
        }
    }

    return 0;
}
