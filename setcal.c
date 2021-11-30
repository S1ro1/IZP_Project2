/* TODO
- load sets, display set - Dosilox
- load relations, display relation - Pepa
- command struct - Miski
- funcs over sets - Siro
- funcs over relations - Jakub

- free all - Siro + Dosilox
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_STRING_LENGTH 31
#define DEFAULT_ALLOCATION_SIZE 8

#define UniversumKeyword 'U'
#define SetKeyword 'S'
#define RelationKeyword 'R'
#define CommandKeyword 'C'


// Structs
typedef struct {
    char *data;
    int currentLength;
    int maxLength;
    int rowIndex;
    char keyword;
} DataLine;

typedef struct {
    DataLine *dataLines;
    int rowCount;
    int maxRows;
} LineList;

typedef struct {
    char **items;
    int itemCount;
    int maxItemCount;
} Universum;

typedef struct {
    char keyword[14]; // 14 => longest command +1
    int A;
    int B;
    int C;
} Command;

typedef struct {
    int *items;
    int itemCount;
    int lineNumber;
} Set;

typedef struct {
    int left;
    int right;
} Pair;

typedef struct {
    Pair *pairs;
    int pairCount;
    int LineNumber;
} Relation;


// Prototypes
DataLine DataLineConstructor();
int AddCharToDataLine(DataLine *, char);
int DataLineDtor(DataLine *);
LineList LineListConstructor();
int AddToLineList(LineList *, DataLine);
int FreeLineList(LineList *);
int InitUniversum(Universum *);
int ReallocUniversum(Universum *, int);
int GetDataFromFile(LineList *, char *);
int PopulateUniversum(DataLine *, Universum *);
int AddUniversumItem(Universum *, char *);
int IsKeyword(char *);
int GetItemIndex(Universum *, char *);
void FreeUniversum(Universum *);
int ResolveCommand(Command, Set *, Relation *, Universum);
int UniversumDuplicateCheck(Universum *);
void ClearTempWord(char *);
int GetCommand(char *, Command *);
int CheckCommandArg(int, char);

// --------------------------------------

int main(int argc, char *argv[]) {

    //Set *sets = NULL;
    //Relation *relations = NULL;
    
    if (argc != 2) {
        printf("Invalid arguments");
        return 1;
    }
    LineList lineList = LineListConstructor();
    Universum u = {.items = NULL, .itemCount = 0, .maxItemCount = DEFAULT_ALLOCATION_SIZE};
    int universumResult = InitUniversum(&u);
    if (universumResult != 0) {
        return 1;
    }
    
    int error = GetDataFromFile(&lineList, argv[1]);
    (void) error;

    universumResult = PopulateUniversum(&lineList.dataLines[0], &u) || UniversumDuplicateCheck(&u);

    if (universumResult == 1) {
        fprintf(stderr,"Failed to load universum");
        return 1;
    }


    //Command command = {.keyword = {'\0'}, .A = -1, .B = -1, .C = -1};
    for (int i = 1; i < lineList.rowCount; i++) {
        DataLine currentLine = lineList.dataLines[i];

        switch (currentLine.keyword) {
            case SetKeyword:
            /*  TODO:
                SetCtor()
                PopulateSet()
                AddToSets()
            */    
                break;
            case RelationKeyword:
            /*  TODO:
                RelationCtor()
                PopulateRelation()
                AddToRelations()
            */
                break;
            case CommandKeyword:

                //int CommandResult = GetCommand(currentLine.data, &command);
                //ResolveCommand(command, sets, relations, u);
                break;

            default:
                fprintf(stderr, "Wrong keyword");
                return 1;
        }
    }
    //ResolveCommand(command, sets, relations, u);
    return 0;
}

/*int ResolveCommand(Command command, Set *sets, Relation *relations, Universum universum) {
    char *keyword = command.keyword;
    if (strcmp("empty", keyword) == 0) {
        
    }
    else if (strcmp("card", keyword) == 0) {
        
    }
    else if (strcmp("complement", keyword) == 0) {
        
    }
    else if (strcmp("union", keyword) == 0) {
        
    }
    else if (strcmp("intersect", keyword) == 0) {
        
    }
    else if (strcmp("minus", keyword) == 0) {
        
    }
    else if (strcmp("subseteq", keyword) == 0) {
        
    }
    else if (strcmp("subset", keyword) == 0) {
        
    }
    else if (strcmp("equals", keyword) == 0) {
        
    }
    else if (strcmp("reflexive", keyword) == 0) {
        
    }
    else if (strcmp("symmetric", keyword) == 0) {
        
    }
    else if (strcmp("antisymmetric", keyword) == 0) {
        
    }
    else if (strcmp("transitive", keyword) == 0) {
        
    }
    else if (strcmp("function", keyword) == 0) {
        
    }
    else if (strcmp("domain", keyword) == 0) {
        
    }
    else if (strcmp("codomain", keyword) == 0) {
        
    }
    else if (strcmp("injective", keyword) == 0) {
    
    }
    else if (strcmp("surjective", keyword) == 0) {
        
    }
    else if (strcmp("bijective", keyword) == 0) {
        
    } else {
        return 1;
    }
    return 0;
}*/

// ================= DYNAMIC STRUCTURE MANIPULATION =================

DataLine DataLineConstructor() {
    DataLine list = {.data = NULL, .maxLength = DEFAULT_ALLOCATION_SIZE, .currentLength = 0, .rowIndex = 0, .keyword = '\0'};
    list.data = malloc(list.maxLength * sizeof(char));
    return list;
}
int AddCharToDataLine(DataLine *list, char c) {
    if (list->currentLength == list->maxLength) {
        char *tmp = realloc(list->data, sizeof(char) * 2* list->maxLength);
        if (tmp == NULL){
            fprintf(stderr, "Allocation unsuccessful");
            return 1;
        }  // TODO: fix return value
        list->data = tmp;
        list->maxLength *= 2;
    }

    list->data[list->currentLength] = c;
    list->currentLength++;
    return 0;
}

int DataLineDtor(DataLine *list) {
    if (list->data == NULL) { 
        fprintf(stderr, "Wrong pointer");
        return 1;
    }
    free(list->data);
    list->data = NULL;
    return 0;
}

LineList LineListConstructor() {
    LineList list = {.dataLines = NULL, .rowCount = 0, .maxRows = DEFAULT_ALLOCATION_SIZE};
    list.dataLines = malloc(list.maxRows * sizeof(DataLine));
    return list;
}

int AddToLineList(LineList *lines, DataLine line) {
    if (lines->rowCount == lines->maxRows) {
        DataLine *tmp= realloc(lines->dataLines, sizeof(DataLine) * 2* lines->maxRows);
        if (tmp == NULL) {
            fprintf(stderr, "Allocation unsuccessful");
            return 1;
        }
        lines->dataLines= tmp;
        lines->maxRows *= 2;
    }

    lines->dataLines[lines->rowCount] = line;
    lines->rowCount++;
    return 0;
}

int FreeLineList(LineList *lines) {
    if (lines->dataLines == NULL){
        fprintf(stderr, "Wrong pointer");
        return 1;
    }
    
    free(lines->dataLines);
    lines->dataLines = NULL;\
    return 0;
}

int ReallocUniversum(Universum *universum, int newSize) {
    if (universum->itemCount == 0) {
        universum->items = (char **)malloc(DEFAULT_ALLOCATION_SIZE * sizeof(char *));
        if (universum->items == NULL) {
            fprintf(stderr, "Allocation unsuccessful");
            return 1;
        }
    } else {
        char **tmp = realloc(universum->items, newSize * sizeof(char *));

        if (tmp == NULL) {
            fprintf(stderr, "Allocation unsuccessful");
            return 1;
        }
        universum->items = tmp;
    }

    //columns allocation
    for (int i = universum->itemCount; i < universum->maxItemCount; i++) {
        universum->items[i] = (char *)malloc(MAX_STRING_LENGTH);
        if (universum->items[i] == NULL) {
            fprintf(stderr, "Allocation unsuccessful");
            return 1;
        }
    }
    return 0;
}

void FreeUniversum(Universum *universum) {
    for (int i = 0; i < universum->maxItemCount; i++) {
        free(universum->items[i]);
    }
    free(universum->items);
    universum->items = NULL;
    universum->maxItemCount = 0;
    universum->itemCount = 0;
}

    // ==============================================================

int GetItemIndex(Universum *universum, char *item) {
    int index = -1;

    for (int i = 0; i < universum->itemCount; i++) {
        if (strcmp(universum->items[i], item) == 0) {
            index = i;
            break;
        }
    }

    return index;
}

//duplicate check in universum
int UniversumDuplicateCheck(Universum *universum){
    for (int i = 0; i < universum->itemCount; i++) {
        char *currentItem = universum->items[i];

        for (int j = 0; j < universum->itemCount; j++) {
            if (j == i) continue;
            char *comparingItem = universum->items[j];

            if (strcmp(currentItem, comparingItem) == 0) return 1;
        }
    }
    return 0;
}

int GetDataFromFile(LineList *LineList, char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Wrong file path");
    }

    DataLine line = DataLineConstructor();
    
    int currentIndex = 0;
    int charInt = fgetc(file);
    int currentRow = 0;
    while(charInt != EOF) { 
        char currentChar = (char)charInt;

        // Adding non-keyword char to char list
        if (line.keyword != '\0' && currentChar != '\n' && currentIndex != 1) {
            AddCharToDataLine(&line, currentChar);
        }

        // Getting Keyword
        if (line.keyword == '\0') {
            line.keyword = currentChar;
        }

        // Keyword is not single char
        if (line.keyword != '\0' && currentIndex == 1 && currentChar != ' ' && currentChar != '\n') {
            fprintf(stderr, "Keyword is not a single char");
            return 1;
        }

        // At the end of line
        if (currentChar == '\n') {
            AddCharToDataLine(&line, '\0'); // To end the string
            line.rowIndex = currentRow;
            AddToLineList(LineList, line);
            line = DataLineConstructor();
            currentIndex = -1; // below is incrementation
            currentRow++;
        }
        
        
        charInt = fgetc(file); // for checking EOF
        currentIndex++;
        
    }
    
    fclose(file);
    // Empty file
    if (LineList->rowCount == 0) {
        fprintf(stderr, "The file is empty");
        return 1;
    } 

    return 0;
}

int InitUniversum(Universum *universum) {
    return ReallocUniversum(universum, 0);
}

int PopulateUniversum(DataLine *source, Universum *universum) {
    if (source->keyword != UniversumKeyword) return 1;

    char *data = source->data;

    int wordLength = 0;
    char tmpWord[31] = {'\0'};
    
    for (int i = 0; data[i] != '\0'; i++) {
        char currentChar = data[i];
        if (wordLength > 30) return 1;
        if (currentChar != ' ') {
            if (!isalpha(currentChar)){
                fprintf(stderr, "Universum element contains unsupported char!");
                return 1;
            }
            tmpWord[wordLength] = currentChar;
            wordLength++;
        } else {
            if (wordLength == 0) {
                fprintf(stderr, "Universum element separated by 2 spaces");
            return 1;
            }
            if (IsKeyword(tmpWord)) {
                fprintf(stderr, "Universum element is a forbidden word");
            }
            wordLength = 0;
            AddUniversumItem(universum, tmpWord);
            ClearTempWord(tmpWord);
        }
    }
    return 0;
}

int AddUniversumItem(Universum *universum, char* word) {
    // Realloc checking 
    if (universum->itemCount == universum->maxItemCount) {
        universum->maxItemCount *= 2;
        ReallocUniversum(universum, universum->maxItemCount);
    }
    strcpy(universum->items[universum->itemCount], word);
    universum->itemCount++;
    return 0;
}

int IsKeyword(char *element) {

    char *keywords[] = {"true", "false", "empty", "card", "complement", "union",
    "intersect", "minus", "subseteq", "subset", "equals", "reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain",
    "codomain", "injective", "surjective", "bijective"};

    for (int i = 0; i < 21; i++) {
        if (strcmp(keywords[i], element) == 0) {
            return 1;
        }
    }
    return 0;
}
void ClearTempWord(char *tmpWord) {
    for (int i = 0; tmpWord[i] !='\0';i++) {
        tmpWord[i]='\0';
    }
}

//function check arguments in commands
int CheckCommandArg(int number, char symbol){
    if (number >= 100){ //100 * 10 more symbol wanted to be added
        fprintf(stderr, ("Too long command number argument identifier\n"));
        return 1;
    }
    if (!isdigit(symbol)){
        fprintf(stderr, "Number identifier contains alpha symbols\n");
        return 1;
    }
    return 0;
}

//function load command to struct and return 1 if some error appear
int GetCommand(char line[], Command *command){
    int SpaceCount = 0;
    int SpaceIdentifier = false;
    for(int index_1 = 0, index_2 = 0; line[index_1] != '\0'; index_1++){

        if (line[index_1] == ' '){
            //check if whether there are 2 spaces in a row
            if (SpaceIdentifier){
                fprintf(stderr, "Command contains 2 spaces in row\n");
                return 1;
            }
            SpaceCount++;
            index_2 = 0;
            SpaceIdentifier = true;
            continue;
        }

        SpaceIdentifier = false; //if symbol is not 'space'

        //populating command->keyword
        if (SpaceCount == 0){
            //check if keyword is no longer than 13 + 1 length
            if (index_2 >= 13){
                fprintf(stderr, "Incorrect command\n");
                return 1;
            }

            command->keyword[index_2] = line[index_1];
            index_2++;
        }
        //add to command->A
        else if (SpaceCount == 1){

            if (CheckCommandArg(command->A, line[index_1]) == 1) return 1;

            if (command->A == -1){
                command->A = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->A *= index_2;
                command->A += (line[index_1] - '0');
            }
        }
        //add to command->B
        else if (SpaceCount == 2){

            if (CheckCommandArg(command->B, line[index_1]) == 1) return 1;

            if (command->B == -1){
                command->B = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->B *= index_2;
                command->B += (line[index_1] - '0');
            }
        }
        //add to command->C
        else if (SpaceCount == 3){

            if (CheckCommandArg(command->C, line[index_1]) == 1) return 1;

            if (command->C == -1){
                command->C = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->C *= index_2;
                command->C += (line[index_1] - '0');
            }
        }
        else if (SpaceCount > 3){
            fprintf(stderr, "Too many arguments in command line\n");
            return 1;
        }
        //add to command A, B, C if not given
        else{
            command->A = -1;
            command->B = -1;
            command->C = -1;
        }
    }
    //check if command->keyword is correct
    if (IsKeyword(command->keyword) == 0){
        fprintf(stderr, "Wrong command keyword\n");
        return 1;
    }
    return 0;
}