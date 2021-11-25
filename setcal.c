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
    char keyword;
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

DataLine DataLineConstructor();
void AddCharToDataLine(DataLine *, char);
void CharlistDtor(DataLine *);
DataLineList DataLineListConstructor();
void AddToDataLineList(DataLineList *, DataLine);
void FreeDataLineList(DataLineList *);

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
    DataLineList lineList = DataLineListConstructor();

    int error = GetDataFromFile(&lineList, argv[1]);
    (void) error;

    // DEBUG TEMP CODE
    for (int i= 0; i<lineList.currentRow; i++){
        printf("%c", lineList.dataLines[i].keyword);
        for (int j= 0; lineList.dataLines[i].data[j]!='\0';j++){

            printf("%c", lineList.dataLines[i].data[j]);
        }
        printf("\n");
    }
    // ==================
    
    return 0;
}

// ================= DYNAMIC STRUCTURE MANIPULATION =================

DataLine DataLineConstructor() {
    DataLine list = {.data = NULL, .maxLength = DEFAULT_ALLOCATION_SIZE, .currentLength = 0, .keyword = '\0'};
    list.data = malloc(list.maxLength * sizeof(char));
    return list;
}
void AddCharToDataLine(DataLine *list, char c) {
    if (list->currentLength == list->maxLength) {
        char *tmp = realloc(list->data, sizeof(char) * 2* list->maxLength);
        if (tmp == NULL){
            printf("wrong allocation %d", list->maxLength); //test print
            return;
        }  // TODO: fix return value
        list->data = tmp;
        list->maxLength *= 2;
    }

    list->data[list->currentLength] = c;
    list->currentLength++;
}

void CharlistDtor(DataLine *list) {
    if (list->data == NULL) return;
    free(list->data);
    list->data = NULL;
}

DataLineList DataLineListConstructor() {
    DataLineList list = {.dataLines = NULL, .currentRow = 0, .maxRows = DEFAULT_ALLOCATION_SIZE};
    list.dataLines = malloc(list.maxRows * sizeof(DataLine));
    return list;
}

void AddToDataLineList(DataLineList *lines, DataLine line) {
    if (lines->currentRow == lines->maxRows) {
        DataLine *tmp= realloc(lines->dataLines, sizeof(DataLine) * 2* lines->maxRows);
        if (tmp == NULL) {
            printf("wrong allocation %d", lines->maxRows); //test print
            return;
            }// TODO: fix return value
        lines->dataLines= tmp;
        lines->maxRows *= 2;
    }

    lines->dataLines[lines->currentRow] = line;
    lines->currentRow++;
}

void FreeDataLineList(DataLineList *lines) {
    if (lines->dataLines == NULL) return;
    free(lines->dataLines);
    lines->dataLines = NULL;
}

// ==============================================================

int GetDataFromFile(DataLineList *dataLineList, char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) return 1;

    DataLine charList = DataLineConstructor();
    

    int currentIndex = 0;
    int charInt = fgetc(file);
    while(charInt != EOF) { 
        char currentChar = (char)charInt;

        // Adding non-keyword char to char list
        if (charList.keyword != '\0' && currentChar != '\n' && currentIndex != 1) {
            AddCharToDataLine(&charList, currentChar);
        }

        // Getting Keyword
        if (charList.keyword == '\0') {
            charList.keyword = currentChar;
        }

        // Keyword is not single char
        if (charList.keyword != '\0' && currentIndex == 1 && currentChar != ' ' && currentChar != '\n') {
            return 1;
        }

        // At the end of line
        if (currentChar == '\n') {
            AddCharToDataLine(&charList, '\0'); // To end the string
            AddToDataLineList(dataLineList, charList);
            charList = DataLineConstructor();
            currentIndex = -1; // bellow is incrementation
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