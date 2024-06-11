#include <iostream>
#include <fstream>
#include <cstring>
#include <stack>

#define INITIAL_CAPACITY 100

class Line {
private:
    char *text;
    size_t length;
    size_t capacity;

public:
    Line() {
        capacity = INITIAL_CAPACITY;
        length = 0;
        text = new char[capacity];
        text[0] = '\0';
    }

    ~Line() {
        delete[] text;
    }

    void appendText(const char *str) {
        size_t newLength = length + strlen(str);
        if (newLength >= capacity) {
            capacity = newLength + 1;
            char *newText = new char[capacity];
            strcpy(newText, text);
            delete[] text;
            text = newText;
        }
        strcat(text, str);
        length = newLength;
    }
    void insertText(size_t pos, const char *str) {
        if (pos > length) {
            std::cerr << "Position out of bounds\n";
            return;
        }
        size_t newLength = length + strlen(str);
        if (newLength >= capacity) {
            capacity = newLength + 1;
            char *newText = new char[capacity];
            strncpy(newText, text, pos);
            newText[pos] = '\0';
            strcat(newText, str);
            strcat(newText, text + pos);
            delete[] text;
            text = newText;
        } else {
            memmove(text + pos + strlen(str), text + pos, length - pos + 1);
            memcpy(text + pos, str, strlen(str));
        }
        length = newLength;
    }

    const char* getText() const {
        return text;
    }

};

class TextStorage {
private:
    Line *lines;
    size_t count;
    size_t capacity;
    char *clipboard;
    std::stack<Line *> undoStack;
    std::stack<Line *> redoStack;

    void saveState() {
        Line *currentState = new Line[count];
        for (size_t i = 0; i < count; ++i) {
            currentState[i].appendText(lines[i].getText());
        }
        undoStack.push(currentState);
        while (!redoStack.empty()) {
            delete[] redoStack.top();
            redoStack.pop();
        }
    }

    void deleteLines() {
        delete[] lines;
    }

public:
    TextStorage() {
        capacity = INITIAL_CAPACITY;
        count = 1;
        lines = new Line[capacity];
        clipboard = nullptr;
    }
    ~TextStorage() {
        deleteLines();
        if (clipboard) delete[] clipboard;
        while (!undoStack.empty()) {
            delete[] undoStack.top();
            undoStack.pop();
        }
        while (!redoStack.empty()) {
            delete[] redoStack.top();
            redoStack.pop();
        }
    }
    size_t getLineCount() const {
        return count;
    }
    void appendText(size_t lineIndex, const char *text) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        saveState();
        lines[lineIndex].appendText(text);
    }

    void addNewLine() {
        if (count >= capacity) {
            capacity *= 2;
            Line *newLines = new Line[capacity];
            for (size_t i = 0; i < count; ++i) {
                newLines[i].appendText(lines[i].getText());
            }
            delete[] lines;
            lines = newLines;
        }
        lines[count++] = Line();
    }


    void saveToFile(const char *filename) const {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error opening file for writing\n";
            return;
        }
        for (size_t i = 0; i < count; ++i) {
            outFile << lines[i].getText() << "\n";
        }
        outFile.close();
        std::cout << "Text has been saved successfully\n";
    }
    void printText() const {
        for (size_t i = 0; i < count; ++i) {
            std::cout << lines[i].getText() << std::endl;
        }
    }
    void loadFromFile(const char *filename) {
        std::ifstream inFile(filename);
        if (!inFile) {
            std::cerr << "Error opening file for reading\n";
            return;
        }
        saveState();
        deleteLines();
        capacity = INITIAL_CAPACITY;
        count = 0;
        lines = new Line[capacity];
        char buffer[INITIAL_CAPACITY];
        while (inFile.getline(buffer, INITIAL_CAPACITY)) {
            if (count >= capacity) {
                capacity *= 2;
                Line *newLines = new Line[capacity];
                for (size_t i = 0; i < count; ++i) {
                    newLines[i].appendText(lines[i].getText());
                }
                delete[] lines;
                lines = newLines;
            }
            appendText(count++, buffer);
        }
        inFile.close();
        std::cout << "Text has been loaded successfully\n";
    }

    void insertText(size_t lineIndex, size_t pos, const char *text) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        saveState();
        lines[lineIndex].insertText(pos, text);

    };
    void searchText(const char *substring) const {
        bool found = false;
        for (size_t i = 0; i < count; ++i) {
            const char *pos = strstr(lines[i].getText(), substring);
            while (pos) {
                std::cout << "Text is present in this position: " << i << " " << pos - lines[i].getText() << std::endl;
                found = true;
                pos = strstr(pos + 1, substring);
            }
        }
        if (!found) {
            std::cout << "Substring not found\n";
        }
    }


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

    void printHelpInfo() {
        std::cout << "> Choose the command:\n";
        std::cout << "1. Append text symbols to the end\n";
        std::cout << "2. Start a new line\n";
        std::cout << "3. Save text to file\n";
        std::cout << "4. Load text from file\n";
        std::cout << "5. Print the current text to console\n";
        std::cout << "6. Insert text by line and symbol index\n";
        std::cout << "7. Search text\n";
        std::cout << "0. Exit\n";
    }
};

int main() {
    TextStorage storage;
    int command;
    char buffer[INITIAL_CAPACITY];

    while (true) {
        system("clear");
        storage.printHelpInfo();
        std::cout << "> ";
        std::cin >> command;
        std::cin.ignore();

        switch (command) {
            case TextStorage::append_text:
                std::cout << "Enter text to append: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.appendText(storage.getLineCount() - 1, buffer);
                break;
            case TextStorage::start_new_line:
                storage.addNewLine();
                break;
            case TextStorage::save_to_file:
                std::cout << "Enter the file name for saving: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.saveToFile(buffer);
                break;
            case TextStorage::load_from_file:
                std::cout << "Enter the file name for loading: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.loadFromFile(buffer);
                break;
            case TextStorage::print_current_text:
                storage.printText();
                break;
            case TextStorage::insert_text_by_index: {
                size_t lineIndex, position;
                std::cout << "Choose line and index: ";
                std::cin >> lineIndex >> position;
                std::cin.ignore();
                std::cout << "Enter text to insert: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.insertText(lineIndex, position, buffer);
                break;
            }
            case TextStorage::search_text:
                std::cout << "Enter text to search: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.searchText(buffer);
                break;

            case TextStorage::exit_program:
                std::cout << "Exiting the program.\n";
                return 0;
            default:
                std::cout << "The command is not implemented\n";
        }

    }

    return 0;
}
