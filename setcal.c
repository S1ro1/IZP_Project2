/* TODO
- load sets, display set - Dosilox - ~~DONE~~
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
    int maxItemCount;
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
    int maxSize;
} Relation;

typedef struct {
    Set *sets;
    int setCount;
    int maxSetCount;
} Sets;


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
int ResolveCommand(Command, Sets *, Universum);
void ClearTempWord(char *);
int GetCommand(char *, Command *);
int CheckCommandArg(int, char);
int SetConstructor(Set *);
int IsSetUnique(Set);
int AddToSet(Set *, int);
int AddToSets(Sets *, Set);
int PopulateSet(DataLine *, Set *, Universum *);
void DisplaySet(Set, Universum);
void FreeSet(Set *);
void* ArrAlloc(void *, size_t, int*, int);
Relation RelationCtor();
void DisplayUniversum(Universum);
int GetUniversumSet(Set *, Universum, DataLine);
int GetSetArrIndex(int, Sets *);

//funcs over sets
void IsEmpty(Set);
void Card(Set);
void SetUnion(Set, Set, Universum *);
void SetMinus(Set, Set, Universum *);
void SetIntersect(Set, Set, Universum *);
void SetEquals(Set, Set);
void IsSubset(Set, Set);
void IsSubsetEq(Set, Set);

int IsInSet(Sets *, int, int);

//funcs over relations
int FindRelIndex(Relation *, int, int);
void IsReflexive(Relation *, int, Universum);
void IsSymetric(Relation *, int);
void IsAntiSymetric(Relation *, int);
void IsTransitive(Relation *, int);
int IsFunction(Relation *, int);
void PrintDomain(Relation *, int, Universum);
void PrintCodomain(Relation *, int, Universum);
int CheckFunctionValidity(Relation *, Sets *, Command, int, int, int);
int IsInjective(Relation *, int, Sets *, Command);
int IsSurjective(Relation *, int, Sets *, Command);
int IsBijective(Relation *, int, Sets *, Command);

// --------------------------------------

int main(int argc, char *argv[]) {
    Sets setCollection = {.sets = NULL, .maxSetCount = 0, .setCount = 0};
    setCollection.sets = (Set*)ArrAlloc(setCollection.sets, sizeof(Set), &setCollection.maxSetCount, 0); // Allocation
    //Relation *relations = NULL;
    
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments");
        return 1;
    }
    LineList lineList = LineListConstructor();

    // Universum allocation
    Universum u = {.items = NULL, .itemCount = 0, .maxItemCount = DEFAULT_ALLOCATION_SIZE};
    int universumResult = InitUniversum(&u);
    if (universumResult != 0) {
        return 1;
    }
    
    // Loading input data
    int error = GetDataFromFile(&lineList, argv[1]);
    if (error != 0) return 1;

    // Load universum and universum set
    Set universumSet = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = 1};
    universumResult = 
            PopulateUniversum(&lineList.dataLines[0], &u) || 
            GetUniversumSet(&universumSet, u, lineList.dataLines[0]);

    if (!IsSetUnique(universumSet)) {
        fprintf(stderr, "Universum contains duplicate items\n");
        return 1;
    }

    if (universumResult == 1) {
        return 1;
    }
    AddToSets(&setCollection, universumSet);
    DisplayUniversum(u);

    // Resolving lines
    //Command command = {.keyword = {'\0'}, .A = -1, .B = -1, .C = -1};
    //int CommandResult = 0;
    for (int i = 1; i < lineList.rowCount; i++) {
        Command command = {.keyword = {'\0'}, .A = -1, .B = -1, .C = -1};
        DataLine currentLine = lineList.dataLines[i];
        int CommandResult = 0;

        Set s = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = currentLine.rowIndex + 1};

        switch (currentLine.keyword) {
            case SetKeyword: {
                int result = 0;

                result = 
                    SetConstructor(&s) ||
                    PopulateSet(&currentLine, &s, &u) ||
                    AddToSets(&setCollection, s);

                if (!IsSetUnique(s)) {
                    fprintf(stderr, "Set on index %d has duplicate items\n", currentLine.rowIndex);
                    return 1;
                }

                if (result != 0) return result;
                DisplaySet(s, u);
                if (result != 0) return result;
                
                break;

            }
                
            case RelationKeyword:
            /*  TODO:
                RelationCtor()
                PopulateRelation()
                AddToRelations()
            */
                break;
            case CommandKeyword:
                CommandResult = GetCommand(currentLine.data, &command);
                if (CommandResult == 1) return 1;
                ResolveCommand(command, &setCollection, u);
                break;

            default:
                fprintf(stderr, "Wrong keyword");
                return 1;
        }
    }
    //ResolveCommand(command, sets, relations, u);
    return 0;
}

void IsEmpty(Set set) {
    if (set.itemCount == 0) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

void Card(Set set) {
    printf("%d\n", set.itemCount);
}

void SetEquals(Set a, Set b) {
    if (a.itemCount != b.itemCount) {
        printf("false\n");
        return;
    }
    bool found = false;

    for (int i = 0; i < a.itemCount; i++) {
        found = false;
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found = true;
            }
        }
        if (found == false) {
            printf("false\n");
            return;
        }
    }
    printf("true\n");
}

void SetsUnion(Set a, Set b, Universum *u) {
    bool found = false;
    
    Set s = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = -1};
    SetConstructor(&s);
    for (int i = 0; i < a.itemCount; i++){
        AddToSet(&s, a.items[i]);
    }
    for (int index_1 = 0; index_1 < b.itemCount; index_1++) {
        found = false;
        for (int index_2 = 0; index_2 < s.itemCount; index_2++) {
            if (b.items[index_1] == s.items[index_2]) {
                found = true;
            }
        }
        if (found == false) {
            AddToSet(&s, b.items[index_1]);
        }
    }

    DisplaySet(s, *u);
}

void SetMinus(Set a, Set b, Universum *u) {

    Set s = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = -1};
    SetConstructor(&s);

    for (int i = 0; i < a.itemCount; i++) {
        bool found = false;
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found = true;
            }
        }
        if (found == false) {
            AddToSet(&s, a.items[i]);
        }
    }
    DisplaySet(s, *u);
}

void SetIntersect(Set a, Set b, Universum *u) {

    Set s = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = -1};
    SetConstructor(&s);

    for (int i = 0; i < a.itemCount; i++) {
        bool found = false;
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found = true;
            }
        }
        if (found == true) {
            AddToSet(&s, a.items[i]);
        }
    }
    DisplaySet(s, *u);
}



void IsSubset(Set a, Set b) {
    int DuplCount = 0;
    for (int i = 0; i < a.itemCount; i++) {
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                DuplCount++;
                break;
            }
        }
    }

    bool equals = false;

    //to check if they're equal
    for (int i = 0; i < a.itemCount; i++) {
        bool found = false;
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found = true;
            }
        }
        if (found == false) {
            equals = false;
            break;
        }
        equals = true;
    }
    
    if (DuplCount == a.itemCount && !equals) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

void IsSubsetEq(Set a, Set b) {
    int DuplCount = 0;
    for (int i = 0; i < a.itemCount; i++) {
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                DuplCount++;
                break;
            }
        }
    }
    if (DuplCount == a.itemCount) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

int GetSetArrIndex(int index, Sets *sets) {
    for (int i = 0; i < sets->setCount; i++) {
        if (sets->sets[i].lineNumber == index) {
            return i;
        }
    }
    return -1;
}

int ResolveCommand(Command command, Sets *setCollection, Universum universum) {
    char *keyword = command.keyword;

    Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
    Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];

    if (strcmp("empty", keyword) == 0) {
        IsEmpty(A);   
    }
    else if (strcmp("card", keyword) == 0) {
        Card(A);
    }
    else if (strcmp("complement", keyword) == 0) {
        SetMinus(setCollection->sets[0], A, &universum);
    }
    else if (strcmp("union", keyword) == 0) {
        SetsUnion(A, B, &universum);
    }
    else if (strcmp("intersect", keyword) == 0) {
        SetIntersect(A, B, &universum);
    }
    else if (strcmp("minus", keyword) == 0) {
        SetMinus(A, B, &universum);
    }
    else if (strcmp("subseteq", keyword) == 0) {
        IsSubset(A, B);
    }
    else if (strcmp("subset", keyword) == 0) {
        IsSubset(A, B);
    }
    else if (strcmp("equals", keyword) == 0) {
        SetEquals(A, B);
    }
    /*else if (strcmp("reflexive", keyword) == 0) { // TODO: command A == -1
        if (!(command.B != -1 || command.C != -1)) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        IsReflexive(relations, relindex, universum);
    }
    else if (strcmp("symmetric", keyword) == 0) { 
        if (!(command.B != -1 || command.C != -1)) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        IsSymetric(relations, relindex);
    }
    else if (strcmp("antisymmetric", keyword) == 0) {
        if (!(command.B != -1 || command.C != -1) && command.A == -1) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        IsAntiSymetric(relations, relindex);
        }
    else if (strcmp("transitive", keyword) == 0) {
        if (!(command.B != -1 || command.C != -1)) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        IsTransitive(relations, relindex);
    }
    else if (strcmp("function", keyword) == 0) {
        if (!(command.B != -1 || command.C != -1)) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        if (IsFunction(relations, relindex)) {
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("domain", keyword) == 0) {
        if (!(command.B != -1 || command.C != -1)) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        PrintDomain(relations, relindex, universum);
    }
    else if (strcmp("codomain", keyword) == 0) {
        if (!(command.B != -1 || command.C != -1 )) {
            return 1;
        }
        int relindex = FindRelIndex(relations, relcount, command.A);
        if (relindex == -1) {
            return 1; // TODO: errorhandling
        }
        PrintCodomain(relations, relindex, universum);
    }
    else if (strcmp("injective", keyword) == 0) {
        if (IsInjective(relations, relcount, setCollection, command)){
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("surjective", keyword) == 0) {
        if (IsSurjective(relations, relcount, setCollection, command)){
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("bijective", keyword) == 0) {
        if (IsBijective(relations, relcount, setCollection, command)){
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }*/ else {    
        return 1;
    }
    return 0;
}

// ================= DYNAMIC STRUCTURE MANIPULATION =================

void* ArrAlloc(void *target, size_t typeSize, int *maxSize_p, int currentSize) {
    void* tmp = target;

    // New allocation
    if (*maxSize_p == 0 && currentSize == 0) {
       *maxSize_p = DEFAULT_ALLOCATION_SIZE;
       tmp = malloc(typeSize * *maxSize_p);
    }

    // Realloc
    if (*maxSize_p == currentSize) {
        *maxSize_p *= 2;
        tmp = realloc(target, *maxSize_p);
    }

    // Check for allocation
    if (tmp == NULL) {
        fprintf(stderr, "Cannot allocate array\n");
    }
    return tmp;
}

int AddToSets(Sets *setColl, Set set) {
    // Ensure collection capacity
    setColl->sets = ArrAlloc(setColl->sets, sizeof(Set), &setColl->maxSetCount, setColl->setCount);
    if (setColl == NULL) return 1;

    setColl->sets[setColl->setCount] = set;
    setColl->setCount++;

    return 0;
}

DataLine DataLineConstructor() {
    DataLine list = {.data = NULL, .maxLength = DEFAULT_ALLOCATION_SIZE, .currentLength = 0, .rowIndex = 0, .keyword = '\0'};
    list.data = malloc(list.maxLength * sizeof(char));
    return list;
}

int SetConstructor(Set *set) {
    if(set == NULL) return 1;

    set->items = malloc(DEFAULT_ALLOCATION_SIZE * sizeof(int));

    if(set->items == NULL) return 1;
    set->maxItemCount = DEFAULT_ALLOCATION_SIZE;
    set->itemCount = 0;
    return 0;
}

int AddToSet(Set *set, int item) {
    // Realloc check
    if (set->maxItemCount == set->itemCount) {
        set->maxItemCount *= 2;
        int* tmp = realloc(set->items, set->maxItemCount * sizeof(int));
        if (tmp == NULL) return 1;
        set->items = tmp;
    }

    set->items[set->itemCount] = item;
    set->itemCount++;
    
    return 0;
}

void FreeSet(Set *set) {
    if (set == NULL) return;

    free(set->items);
    set->maxItemCount = 0;
    set->itemCount = 0;
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
    lines->dataLines = NULL;
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
    // Inner arrays cleanup
    for (int i = 0; i < universum->maxItemCount; i++) {
        free(universum->items[i]);
    }

    // Outer cleanup
    free(universum->items);
    universum->items = NULL;
    universum->maxItemCount = 0;
    universum->itemCount = 0;
}

// ==============================================================

int GetUniversumSet(Set *set, Universum universum, DataLine universumLine) {
    int result;
    result = 
        SetConstructor(set) ||
        PopulateSet(&universumLine, set, &universum);
    if (result != 0) {
        fprintf(stderr, "Error loading the universum"); 
        return 1;
    }
    return 0;
}

void DisplayUniversum(Universum universum) {
    if (universum.itemCount == 0) {
        printf("U");
    } else {
        printf("U "); //needs redoing
    }
    for (int i = 0; i < universum.itemCount; i++) {
        printf("%s", universum.items[i]);

        if (i < universum.itemCount - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

int PopulateSet(DataLine *line, Set *set, Universum *universum) {
    // Values init
    char word[MAX_STRING_LENGTH] = {'\0'};
    int wordIndex = 0;
    if (set->items == NULL || line->data == NULL || universum->items == NULL) {
        fprintf(stderr, "Array is not initialized");
        return 1;
    }
    for (int i = 0; i < line->currentLength; i++) { 
        char currentChar = line->data[i];

        // Adding item's char to word array
        if (currentChar != ' ' && currentChar != '\n') {
            word[wordIndex] = currentChar;
            wordIndex++;

            if (wordIndex == MAX_STRING_LENGTH) {
                fprintf(stderr, "Universum element exceeds the maximal length\n");
                return 1;
            }
            continue;
        }

        // Double space
        if (wordIndex == 0 && set->itemCount > 0) {
            fprintf(stderr, "Syntax error in set on index %d\n", line->rowIndex);
            return 1;
        }

        // Ignore empty sets
        if (word[0] == '\0') continue;

        int wordId = GetItemIndex(universum, word);

        // Invalid item
        if (wordId == -1) {
            fprintf(stderr, "Item is not present in universum");
            return 1;
        }

        // Adding item to set
        int result = AddToSet(set, wordId);
        if (result != 0) {
            fprintf(stderr, "Cannot add item to the set");
            return result;
        }

        ClearTempWord(word);
        wordIndex = 0;
        
    }    
    return 0;
}

int IsSetUnique(Set set) {
    for (int i = 0; i < set.itemCount; i++) {
        for (int j = 0; j < set.itemCount; j++) {
            if (i == j) continue; // Avoid comparing item with itself

            if (set.items[i] == set.items[j]) return 0;
        }
    }

    return 1;
}

void DisplaySet(Set set, Universum universum) {
    if (set.itemCount == 0) {
        printf("S");
    } else {
        printf("S ");
    }
    for (int i = 0; i < set.itemCount; i++) {
        int currentItemId = set.items[i];
        printf("%s", universum.items[currentItemId]);

        if (i < set.itemCount - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

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

int GetDataFromFile(LineList *LineList, char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Wrong file path");
        return 1;
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
            AddCharToDataLine(&line, currentChar);
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
        if (currentChar != ' ') {
            if (currentChar == '\n') {
                if (IsKeyword(tmpWord)) {
                    fprintf(stderr, "Universum element is a forbidden word");
                    return 1;
                } else {
                    if (strlen(tmpWord) != 0){
                        AddUniversumItem(universum, tmpWord);
                    }
                    
                    break;
                }
            }
            if (!isalpha(currentChar)){
                fprintf(stderr, "Universum element contains unsupported char!");
                return 1;
            }
            tmpWord[wordLength] = currentChar;
            wordLength++;
        } else {
            
            if (wordLength == 0) {
                fprintf(stderr, "Universum element is separated by 2 or more spaces\n");
                return 1;
            }
            if (IsKeyword(tmpWord)) {
                fprintf(stderr, "Universum element is a forbidden word");
                return 1;
            }
            if (tmpWord[31] != '\0') {
                fprintf(stderr, "Password exceeds the maximal length\n");
                return 1;
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
    (void) symbol;
    /*
    if (!isdigit(symbol)){
        fprintf(stderr, "Command number identifier contains alpha symbols\n");
        return 1;
    }*/
    return 0;
}
//endregion
//function load command to struct and return 1 if some error appear
int GetCommand(char line[], Command *command){
    int SpaceCount = 0;
    int SpaceIdentifier = false;
    memset(command->keyword, 0, 14); //reset command
    for(int index_1 = 0, index_2 = 0; line[index_1] != '\n'; index_1++){
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
    if (IsKeyword(command->keyword) != 1){
        fprintf(stderr, "Wrong command keyword\n");
        return 1;
    }
    return 0;
}
Relation RelationCtor(){
    Relation relation = {.pairs = NULL, .pairCount = 0, .maxSize = 0, .LineNumber = 0};
    relation.pairs = malloc(relation.pairCount * sizeof(Pair));
    return relation;
}

// operations with relations 
int FindRelIndex(Relation *relationArr, int relcount, int command){
    for (int relindex = 0; relindex < relcount;relindex++){
        if (relationArr[relindex].LineNumber== command){
            return relindex;
        }
    }
    return -1;
}

void IsReflexive(Relation *relationArr, int relindex, Universum universum) {
    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    int uniqueItems = 0;
    int count = 0;

    for (int i = 0; i < universum.itemCount; i++) {
        haveSeen[i] = false;
    }
    for(int i = 0;  i<relationArr[relindex].pairCount; i++) {
        haveSeen[relationArr[relindex].pairs[i].right] = true;
        haveSeen[relationArr[relindex].pairs[i].left] = true;
    
    }
    for(int i = 0;  i<universum.itemCount; i++) {
        if (haveSeen == true) {
            uniqueItems++;
        }
    
    }
    free(haveSeen);

    for (int i = 0; i < universum.itemCount; i++) {
        if (relationArr[relindex].pairs[i].right == relationArr[relindex].pairs[i].left) {
            count++;
            }
    }
    if (count != uniqueItems) {
        printf("false\n");
        }
    else {
        printf("true\n");
    }

}

void IsSymetric(Relation *relationArr, int relindex) {
    int count = 0;
    for (int i = 0; i < relationArr[relindex].pairCount; i++) {
        for (int j = 0; j < relationArr[relindex].pairCount; j++) {
            if (relationArr[relindex].pairs[i].left == relationArr[relindex].pairs[j].right) {
                if (relationArr[relindex].pairs[i].right == relationArr[relindex].pairs[j].left) {
                    count++;
                    break;
                }
                
            }

        }
        if (count != relationArr[relindex].pairCount) {
            printf("false\n");
            return;
            }
        
        }
    printf("true\n");
}

void IsAntiSymetric(Relation *relationArr, int relindex) {
    for (int i = 0; i < relationArr[relindex].pairCount; i++) {
        if (relationArr[relindex].pairs[i].right == relationArr[relindex].pairs[i].left) continue;
        for (int j = 0; j < relationArr[relindex].pairCount; j++) {
            if (relationArr[relindex].pairs[j].right == relationArr[relindex].pairs[j].left) continue;
            
            if (relationArr[relindex].pairs[i].left == relationArr[relindex].pairs[j].right){
                if (relationArr[relindex].pairs[i].right == relationArr[relindex].pairs[j].left){
                    if (relationArr[relindex].pairs[i].right != relationArr[relindex].pairs[i].left){
                        printf("false\n");
                        return;
                    }
                }

            }
            
        }
    }
    printf("true\n");
}

void IsTransitive(Relation *relationArr, int relindex) {
    for (int i = 0; i < relationArr[relindex].pairCount; i++) {
        for (int j = 0; j < relationArr[relindex].pairCount; j++) {
            if (relationArr[relindex].pairs[i].right == relationArr[relindex].pairs[j].left){
                bool found = false;
                int tmpRight = relationArr[relindex].pairs[j].right;
                int tmpLeft = relationArr[relindex].pairs[i].left;
                for (int k = 0; k < relationArr[relindex].pairCount; k++) {
                    if (relationArr[relindex].pairs[k].right == tmpRight && relationArr[relindex].pairs[k].left == tmpLeft) {
                        found = true;
                    }
                    if (k == relationArr[relindex].pairCount-1 && found == false) {
                        printf("false\n");
                        return;
                    }
                }
            }
        }
    }
    printf("true\n");
}

int IsFunction(Relation *relationArr, int relindex) {
    for (int i = 0; i<relationArr[relindex].pairCount; i++) {
        int count = 0;
        for (int j = i; j<relationArr[relindex].pairCount; j++) {
            if (relationArr[relindex].pairs[i].left == relationArr[relindex].pairs[j].left) {
                count++;
                if (count != 1) {
                    return 0;
                }
            }
        }

    }
    return 1;
}

void PrintDomain(Relation *relationArr, int relindex, Universum universum) {

    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    for (int i = 0; i < universum.itemCount; i++) {
        haveSeen[i] = false;
    }
    printf("S");
    for(int i = 0;  i<relationArr[relindex].pairCount; i++) {
        haveSeen[relationArr[relindex].pairs[i].left] = true;
    
    }
    for(int i = 0;  i< universum.itemCount; i++) {
        if (haveSeen[i] == true) {
            printf(" %s", universum.items[i]);

        }
        
    
    }
    printf("\n");
    free(haveSeen);

}

void PrintCodomain(Relation *relationArr, int relindex, Universum universum) {

    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    for (int i = 0; i < universum.itemCount; i++) {
        haveSeen[i] = false;
    }
    printf("S");
    for(int i = 0;  i<relationArr[relindex].pairCount; i++) {
        haveSeen[relationArr[relindex].pairs[i].right] = true;
    
    }
    for(int i = 0;  i< universum.itemCount; i++) {
        if (haveSeen[i] == true) {
            printf(" %s", universum.items[i]);

        }
        
    
    }
    printf("\n");
    free(haveSeen);

}

int IsInSet(Sets *sets, int elementIndex, int setIndex) {
    for (int i = 0; i < sets->sets[setIndex].itemCount; i++) {
        if (sets->sets[setIndex].items[i] == elementIndex) {
            return 1;
        }
    }
    return 0;

}

// function for last 3 operations
int CheckFunctionValidity(Relation *relationArr, Sets *sets, Command command, int relIndex, int set1Index, int set2Index) {
    if (!IsFunction(relationArr, relIndex)){
        return 0;
    }
    for (int i = 0; i < relationArr[relIndex].pairCount; i++) {
        if (!IsInSet(sets, relationArr[relIndex].pairs[i].left, set1Index)) {
            return 0;
            }
        }
    for (int i = 0; i < relationArr[relIndex].pairCount; i++) {
        if (!IsInSet(sets, relationArr[relIndex].pairs[i].right, set2Index)) {
            return 0;
        }
    }
    return 1;
}

int IsInjective(Relation *relationArr, int relcount, Sets *sets, Command command) { // TODO: error handling
    int relIndex = FindRelIndex(relationArr, relcount, command.A);
    int set1Index = GetSetArrIndex(command.B, sets);
    int set2Index = GetSetArrIndex(command.C, sets);
    if (!CheckFunctionValidity(relationArr, sets, command, relIndex, set1Index, set2Index)) {
        return 0;
    }
    bool *haveSeen = malloc(sizeof(bool) * sets->sets[set2Index].itemCount);
    for (int i = 0; i < sets->sets[set2Index].itemCount; i++) {
        haveSeen[i] = false;
    }
    for(int i = 0;  i<relationArr[relIndex].pairCount; i++) {
        if (haveSeen[relationArr[relIndex].pairs[i].right] != true) {
            haveSeen[relationArr[relIndex].pairs[i].right] = true;
        }
        else {
            free(haveSeen);
            return 0;
        }
    
    }
    free(haveSeen);
    return 1;
}

int IsSurjective(Relation *relationArr, int relcount, Sets *sets, Command command) {
    int relIndex = FindRelIndex(relationArr, relcount, command.A);
    int set1Index = GetSetArrIndex(command.B, sets);
    int set2Index = GetSetArrIndex(command.C, sets);
    int count = 0;
    if (!CheckFunctionValidity(relationArr, sets, command, relIndex, set1Index, set2Index)) {
        return 0;
    }
    bool *haveSeen = malloc(sizeof(bool) * sets->sets[set2Index].itemCount);
    for (int i = 0; i < sets->sets[set2Index].itemCount; i++) {
        haveSeen[i] = false;
    }
    for(int i = 0;  i<relationArr[relIndex].pairCount; i++) {
        haveSeen[relationArr[relIndex].pairs[i].right] = true;
    }
    
    for(int i = 0;  i<relationArr[relIndex].pairCount; i++) {
        if (haveSeen[relationArr[relIndex].pairs[i].right] == true) {
            count++;
        }
    }
    free(haveSeen);
    if (count != sets->sets[set2Index].itemCount) {
        return 0;
    }

    return 1;
}

int IsBijective(Relation *relationArr, int relcount, Sets *sets, Command command) {
    if (IsInjective(relationArr, relcount, sets, command) && IsSurjective(relationArr, relcount, sets, command)) {
        return 1;
    }
    return 0;
}
    
