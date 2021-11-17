#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_STRING_LENGTH 31
#define DEFAULT_ALLOCATION_SIZE 8

#define UniversumKeyword "U"
#define RelationKeyword "R"
#define CommandKeyword "C"
#define SetKeyword "S"



// Structs
typedef struct {
    char *data;
    int currentLength;
    int maxLength;
} CharList;

typedef struct {
    char keyword;
    CharList data;
    int row;
} DataLine;

typedef struct {
    DataLine *dataLines;
    int currentRow;
    int maxRows;
} DataLineList;

typedef struct {
    char **items;
    int itemCount;
} Universum;

typedef struct {
    int *items;
    int itemCount;
} Set;

typedef struct {
    int left;
    int right;
} Pair;

typedef struct {
    Pair *pairs;
    int pairCount;
} Relation;

// Prototypes

CharList CharlistConstructor();
void AddToCharList(CharList *, char);
void CharlistDtor(CharList *);
DataLineList LinesConstructor();
void AddLine(DataLineList *, DataLine);
void FreeLines(DataLineList *);

Universum UniversumConstructor();
int InitUniversum(Universum *, char *);
int ParseItem(char *);
int GetDataFromFile(DataLineList *, char *);



// --------------------------------------

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    if (argc != 2) {
        printf("Invalid arguments");
        return 1;
    }
    DataLineList lineList = LinesConstructor();

    int error = GetDataFromFile(&lineList, argv[1]);
    (void) error;
    return 0;
}

CharList CharlistConstructor() {
    CharList list = {.data = NULL, .maxLength = DEFAULT_ALLOCATION_SIZE, .currentLength = 0};
    list.data = malloc(list.maxLength * sizeof(char));
    return list;
}

void AddToCharList(CharList *list, char c) {
    if (list->currentLength == list->maxLength) {
        list->data = realloc(list->data, sizeof(list->data) * 2);
        if (list->data == NULL) return; // TODO: fix return value
        
        list->maxLength *= 2;
    }

    list->data[list->currentLength] = c;
    list->currentLength++;
}

void CharlistDtor(CharList *list) {
    if (list->data == NULL) return;
    free(list->data);
}

DataLineList LinesConstructor() {
    DataLineList list = {.dataLines = NULL, .currentRow = 0, .maxRows = DEFAULT_ALLOCATION_SIZE};
    list.dataLines = malloc(list.maxRows * sizeof(DataLine));
    return list;
}

void AddLine(DataLineList *lines, DataLine line) {
    if (lines->currentRow == lines->maxRows) {
        lines->dataLines= realloc(lines->dataLines, sizeof(lines->dataLines) * 2);
        if(lines->dataLines == NULL) return; // TODO: fix return value

        lines->maxRows *= 2;
    }

    lines->dataLines[lines->currentRow] = line;
    lines->currentRow++;
}

void FreeLines(DataLineList *lines) {
    if (lines->dataLines == NULL) return;
    free(lines->dataLines);
}


int GetDataFromFile(DataLineList *dataLineList, char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) return 1;

    CharList charList = CharlistConstructor();
    DataLine dataLine = {.data = charList, .keyword = '\0'};
    

    int currentIndex = 0;
    int charInt = fgetc(file);
    while(charInt != EOF) { 
        char currentChar = (char)charInt;

        // Adding non-keyword char to char list
        if (dataLine.keyword != '\0' && currentChar != '\n') {
            AddToCharList(&charList, currentChar);
        }

        // Getting Keyword
        if (dataLine.keyword == '\0') {
            dataLine.keyword = currentChar;
        }

        // Keyword is not single char
        if (dataLine.keyword != '\0' && currentIndex == 1 && currentChar != ' ') {
            return 1;
        }

        // At the end of line
        if (currentChar == '\n') {
            AddLine(dataLineList, dataLine);
            charList = CharlistConstructor();
            dataLine.data = charList;
            dataLine.keyword = '\0';
        }
        
        
        charInt = fgetc(file); // for checking EOF
        currentIndex++;
        
    }
    
    fclose(file);
    // Empty file
    if (dataLineList->currentRow == 0) {
        return 2;
    } 

    return 0;
}







/*



Universum UniversumConstructor() {
    Universum u = {.items = NULL, .itemCount = DEFAULT_ALLOCATION_SIZE};
    u.items = malloc(DEFAULT_ALLOCATION_SIZE * MAX_STRING_LENGTH * sizeof(char));
    return u;
}



int InitUnversum(Universum *universum, char *filePath) {
    FILE *file = fopen(filePath, "r");

    if (file == NULL) {
        return 2;
    }
    if ((char)fgetc(file) != UniversumKeyword) return 1;
    
    int currentWord = 0;
    int currentCharNumber;
    char currentChar;
    int itemIndex = 0;
    char item[MAX_STRING_LENGTH] = {'\0'};
    do {
        currentCharNumber = fgetc(file);
        currentChar = (char)currentCharNumber;

        if(itemIndex > MAX_STRING_LENGTH - 1) return 1;

        if (currentChar != " ") {
            item[itemIndex] = currentChar;
            itemIndex++;
        } else {
            itemIndex = 0;
            
            if (IsKeyword(item)) {
                return 1; 
            } else {
                if (universum->size < currentWord) {
                    universum->size *= 2;
                    realloc(universum->items, universum->size * MAX_STRING_LENGTH * sizeof(char));
                }
                *universum->items[currentWord] = item;
            }
            char item[MAX_STRING_LENGTH] = {'\0'};
        }

    } while(currentChar != '\n');

    return 0;
    
}


bool IsKeyword(char *element) {

    char keywords[] = {"true", "false", "empty", "card", "complement", "union",
    "intersect", "minus", "subseteq", "subset", "equals", "reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain",
    "codomain", "injective", "surjective", "bijective"};

    for (int i = 0; i < 25; i++) {
        if (strcmp(keywords[i], element) == 0) {
            return true;
        }
    }
    return false;
}
*/