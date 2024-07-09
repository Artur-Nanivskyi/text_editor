#include <iostream>
#include <fstream>
#include <cstring>
#include <stack>
#include <cctype>
#include <dlfcn.h>
#define INITIAL_CAPACITY 100

typedef char* (*EncryptFunc)(char*, int);
typedef char* (*DecryptFunc)(char*, int);


class CaesarLib {
private:
    void* handle;
    EncryptFunc encrypt;
    DecryptFunc decrypt;

public:
    CaesarLib(const char* libPath) {
        try {
            handle = dlopen(libPath, RTLD_LAZY);
            if (!handle) {
                throw std::runtime_error(dlerror());
            }

            encrypt = (EncryptFunc)dlsym(handle, "encrypt");
            decrypt = (DecryptFunc)dlsym(handle, "decrypt");

            char* error;
            if ((error = dlerror()) != NULL) {
                throw std::runtime_error(error);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading library or symbols: " << e.what() << std::endl;
            std::cerr << "Path attempted: " << libPath << std::endl;
            exit(1);
        }
    }

    ~CaesarLib() {
        if (handle) {
            dlclose(handle);
        }
    }

    char* encryptText(char* text, int shift) {
        return encrypt(text, shift);
    }

    char* decryptText(char* text, int shift) {
        return decrypt(text, shift);
    }
};

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

    Line(const Line &other) {
        capacity = other.capacity;
        length = other.length;
        text = new char[capacity];
        std::strcpy(text, other.text);
    }

    Line &operator=(const Line &other) {
        if (this != &other) {
            delete[] text;
            capacity = other.capacity;
            length = other.length;
            text = new char[capacity];
            std::strcpy(text, other.text);
        }
        return *this;
    }

    ~Line() {
        delete[] text;
    }

    void appendText(const char *str) {
        size_t newLength = length + std::strlen(str);
        if (newLength >= capacity) {
            capacity = newLength + 1;
            char *newText = new char[capacity];
            std::strcpy(newText, text);
            delete[] text;
            text = newText;
        }
        std::strcat(text, str);
        length = newLength;
    }

    void insertText(size_t pos, const char *str) {
        if (pos > length) {
            std::cerr << "Position out of bounds\n";
            return;
        }
        size_t newLength = length + std::strlen(str);
        if (newLength >= capacity) {
            capacity = newLength + 1;
            char *newText = new char[capacity];
            std::strncpy(newText, text, pos);
            newText[pos] = '\0';
            std::strcat(newText, str);
            std::strcat(newText, text + pos);
            delete[] text;
            text = newText;
        } else {
            std::memmove(text + pos + std::strlen(str), text + pos, length - pos + 1);
            std::memcpy(text + pos, str, std::strlen(str));
        }
        length = newLength;
    }

    void deleteText(size_t pos, size_t len) {
        if (pos >= length || pos + len > length) {
            std::cerr << "Position and length out of bounds\n";
            return;
        }
        std::memmove(text + pos, text + pos + len, length - pos - len + 1);
        length -= len;
    }

    void insertWithReplace(size_t pos, const char *str) {
        if (pos > length) {
            std::cerr << "Position out of bounds\n";
            return;
        }
        size_t strLength = std::strlen(str);
        size_t newLength = pos + strLength;

        if (newLength >= capacity) {
            capacity = newLength + 1;
            char *newText = new char[capacity];
            std::strncpy(newText, text, pos);
            newText[pos] = '\0';
            std::strcat(newText, str);
            if (pos + strLength < length) {
                std::strcat(newText, text + pos + strLength);
            }
            delete[] text;
            text = newText;
        } else {
            std::strncpy(text + pos, str, strLength);
            if (newLength < length) {
                std::memmove(text + newLength, text + pos + strLength, length - pos - strLength + 1);
            } else {
                text[newLength] = '\0';
            }
        }

        length = newLength;
    }

    const char* getText() const {
        return text;
    }

    size_t getTextLength() const {
        return length;
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
                newLines[i] = lines[i];
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

    void printText() const {
        for (size_t i = 0; i < count; ++i) {
            std::cout << lines[i].getText() << std::endl;
        }
    }

    void insertText(size_t lineIndex, size_t pos, const char *text) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        saveState();
        lines[lineIndex].insertText(pos, text);
    }

    void searchText(const char *substring) const {
        bool found = false;
        for (size_t i = 0; i < count; ++i) {
            const char *pos = std::strstr(lines[i].getText(), substring);
            while (pos) {
                std::cout << "Text is present in this position: " << i << " " << pos - lines[i].getText() << std::endl;
                found = true;
                pos = std::strstr(pos + 1, substring);
            }
        }
        if (!found) {
            std::cout << "Substring not found\n";
        }
    }

    void deleteText(size_t lineIndex, size_t pos, size_t len) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        saveState();
        lines[lineIndex].deleteText(pos, len);
    }

    void undo() {
        if (undoStack.empty()) {
            std::cerr << "No more undo steps available\n";
            return;
        }
        redoStack.push(lines);
        lines = undoStack.top();
        undoStack.pop();
    }

    void redo() {
        if (redoStack.empty()) {
            std::cerr << "No more redo steps available\n";
            return;
        }
        undoStack.push(lines);
        lines = redoStack.top();
        redoStack.pop();
    }

    void cutText(size_t lineIndex, size_t pos, size_t len) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }

        if (pos + len > lines[lineIndex].getTextLength()) {
            std::cerr << "Position and length out of bounds\n";
            return;
        }
        copyText(lineIndex, pos, len);
        saveState();
        lines[lineIndex].deleteText(pos, len);
    }

    void pasteText(size_t lineIndex, size_t pos) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        if (!clipboard) {
            std::cerr << "Clipboard is empty\n";
            return;
        }
        saveState();
        lines[lineIndex].insertText(pos, clipboard);
    }

    void copyText(size_t lineIndex, size_t pos, size_t len) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        const char *lineText = lines[lineIndex].getText();
        if (pos + len > std::strlen(lineText)) {
            std::cerr << "Position and length out of bounds\n";
            return;
        }
        if (clipboard) {
            delete[] clipboard;
        }
        clipboard = new char[len + 1];
        std::strncpy(clipboard, lineText + pos, len);
        clipboard[len] = '\0';
    }

    void insertWithReplace(size_t lineIndex, size_t pos, const char *text) {
        if (lineIndex >= count) {
            std::cerr << "Line index out of bounds\n";
            return;
        }
        saveState();
        lines[lineIndex].insertWithReplace(pos, text);
    }
    void encryptText(int shift, CaesarLib& caesarLib) {
        for (size_t i = 0; i < count; ++i) {
            const char* currentText = lines[i].getText();
            char* textCopy = new char[strlen(currentText) + 1];
            strcpy(textCopy, currentText);

            char* encrypted = caesarLib.encryptText(textCopy, shift);
            lines[i].insertWithReplace(0, encrypted);

            delete[] encrypted;
            delete[] textCopy;
        }
    }

    void decryptText(int shift, CaesarLib& caesarLib) {
        for (size_t i = 0; i < count; ++i) {
            const char* currentText = lines[i].getText();
            char* textCopy = new char[strlen(currentText) + 1];
            strcpy(textCopy, currentText);

            char* decrypted = caesarLib.decryptText(textCopy, shift);
            lines[i].insertWithReplace(0, decrypted);

            delete[] decrypted;
            delete[] textCopy;
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
        delete_text,
        undo_action,
        redo_action,
        cut_text,
        paste_text,
        copy_text,
        insert_with_replace,
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
        std::cout << "8. Delete text\n";
        std::cout << "9. Undo\n";
        std::cout << "10. Redo\n";
        std::cout << "11. Cut text\n";
        std::cout << "12. Paste text\n";
        std::cout << "13. Copy text\n";
        std::cout << "14. Insert text by line and symbol index with replacement\n";
        std::cout << "0. Exit\n";
    }
};

int main() {
    TextStorage storage;
    int command;
    char buffer[INITIAL_CAPACITY];

    while (true) {
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
            case TextStorage::delete_text: {
                size_t lineIndex, position, length;
                std::cout << "Choose line, index and length: ";
                std::cin >> lineIndex >> position >> length;
                std::cin.ignore();
                storage.deleteText(lineIndex, position, length);
                break;
            }
            case TextStorage::undo_action:
                storage.undo();
                break;
            case TextStorage::redo_action:
                storage.redo();
                break;
            case TextStorage::cut_text: {
                size_t lineIndex, position, length;
                std::cout << "Choose line, index and length: ";
                std::cin >> lineIndex >> position >> length;
                std::cin.ignore();
                storage.cutText(lineIndex, position, length);
                break;
            }
            case TextStorage::paste_text: {
                size_t lineIndex, position;
                std::cout << "Choose line and index: ";
                std::cin >> lineIndex >> position;
                std::cin.ignore();
                storage.pasteText(lineIndex, position);
                break;
            }
            case TextStorage::copy_text: {
                size_t lineIndex, position, length;
                std::cout << "Choose line, index and length: ";
                std::cin >> lineIndex >> position >> length;
                std::cin.ignore();
                storage.copyText(lineIndex, position, length);
                break;
            }
            case TextStorage::insert_with_replace: {
                size_t lineIndex, position;
                std::cout << "Choose line and index: ";
                std::cin >> lineIndex >> position;
                std::cin.ignore();
                std::cout << "Enter text to replace: ";
                std::cin.getline(buffer, sizeof(buffer));
                storage.insertWithReplace(lineIndex, position, buffer);
                break;
            }
            case TextStorage::exit_program:
                std::cout << "Exiting the program.\n";
                return 0;
            default:
                std::cout << "The command is not implemented\n";
        }
    }

    return 0;
}
