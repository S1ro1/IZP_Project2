/*
* IZP - Projekt 2 - Prace s datovymi strukturami
***************************************************
* Matej Sirovatka - xsirov00@stud.fit.vutbr.cz - xsirov00
* Josef Uncovsky - xuncov00@stud.fit.vutbr.cz - xuncov00
* Michal Las - xlasmi00@stud.fit.vutbr.cz - xlasmi00
* Petr Vecera - xvecer29@stud.fit.vutbr.cz - xvecer29
* Jakub Kontrik - xkontr02@stud.fit.vutbr.cz - xkontr02
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_STRING_LENGTH 31
#define DEFAULT_ALLOCATION_SIZE 8
#define MAX_COMMAND_LENGTH 13

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

typedef struct {
    Relation *relations;
    int relCount;
    int maxRelCount;
} Relations;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//                      Prototypes
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// Parsing the file
DataLine DataLineConstructor();
int AddCharToDataLine(DataLine *, char);
void DataLineDtor(DataLine *);
LineList LineListConstructor();
int AddToLineList(LineList *, DataLine);
int GetDataFromFile(LineList *, char *);

// General funcs
int IsKeyword(char *);
int GetItemIndex(Universum *, char *);
void *ArrAlloc(void *, size_t, int *, int);
void ClearTempWord(char *);
bool ValidNumOfparams(Command, int);

// Universum
int InitUniversum(Universum *);
int ReallocUniversum(Universum *, int);
int PopulateUniversum(DataLine *, Universum *);
int AddUniversumItem(Universum *, char *);

// Commands
int ResolveCommand(Command, Sets *, Universum, Relations *);
int GetCommand(char *, Command *);
int CheckCommandArg(int);
int IsCommand(char *);

// Sets
int SetConstructor(Set *);
int AddToSet(Set *, int);
int AddToSets(Sets *, Set);
int PopulateSet(DataLine *, Set *, Universum *);
void DisplaySet(Set, Universum);
int GetUniversumSet(Set *, Universum, DataLine);
int GetSetArrIndex(int, Sets *);
int IsSetUnique(Set);

// Relations
int RelationCtor(Relation *);
int AddToRelation(int, int, Relation *);
int AddToRelations(Relations *, Relation);
int PopulateRelation(DataLine *, Relation *, Universum *);
void DisplayRelation(Universum, Relation);
void DisplayUniversum(Universum);
int IsRelationUnique(Relation);

// Command funcions over sets
void IsEmpty(Set);
void Card(Set);
void SetUnion(Set, Set, Universum *);
void SetMinus(Set, Set, Universum *);
void SetIntersect(Set, Set, Universum *);
void SetEquals(Set, Set);
void IsSubset(Set, Set);
void IsSubsetEq(Set, Set);
int IsInSet(Set, int);

// Command functions over relations
int FindRelIndex(int, Relations *);
void IsReflexive(Relation, Universum);
void IsSymetric(Relation);
void IsAntiSymetric(Relation);
void IsTransitive(Relation);
int IsFunction(Relation);
void PrintDomain(Relation, Universum);
void PrintCodomain(Relation, Universum);
int CheckFunctionValidity(Relation, Set, Set);
int IsInjective(Relation, Set, Set, Universum);
int IsSurjective(Relation, Set, Set, Universum);
int IsBijective(Relation, Set, Set, Universum);

// Free functions
void FreeAll(LineList *, Sets *, Relations *, Universum *);
void FreeRelationCollection(Relations *);
void FreeRelation(Relation *);
void FreeSetCollection(Sets *);
void FreeLineList(LineList *);
void FreeUniversum(Universum *);
void FreeSet(Set *);

// --------------------------------------

int main(int argc, char *argv[]) {
    // Exception handling
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments");
        return 1;
    }

    // Sets init
    Set universumSet = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = 1};

    Sets setCollection = {.sets = NULL, .maxSetCount = 0, .setCount = 0};
    setCollection.sets = (Set*)ArrAlloc(setCollection.sets, sizeof(Set), &setCollection.maxSetCount, 0); // Allocation

    // Relations init
    Relations relationCollection = {.relations = NULL, .maxRelCount = 0, .relCount = 0};
    relationCollection.relations = (Relation*)ArrAlloc(relationCollection.relations, sizeof(Relation), &relationCollection.maxRelCount, 0); // Allocation

    // To handle correct file order
    bool SetRelFound = false;
    bool CommandFound = false;

    int error = 0;
    
    // struct containing all the data
    LineList lineList = LineListConstructor();

    // Universum allocation
    Universum universum = {.items = NULL, .itemCount = 0, .maxItemCount = DEFAULT_ALLOCATION_SIZE};
    error = InitUniversum(&universum);
    if (error != 0) {
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        return 1;
    }
    
    // Loading input data
    error = GetDataFromFile(&lineList, argv[1]);
    if (error != 0) {
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        return 1;
    }
    // Load universum and universum set
    error = 
            PopulateUniversum(&lineList.dataLines[0], &universum) || 
            GetUniversumSet(&universumSet, universum, lineList.dataLines[0]) || !IsSetUnique(universumSet);
    
    AddToSets(&setCollection, universumSet);

    if (error == 1) {
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        return 1;
    }
    DisplayUniversum(universum);

    if (lineList.rowCount == 1) {
        fprintf(stderr, "Only universum\n");
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        return 1;
    }

    if (lineList.rowCount > 1000) {
        fprintf(stderr, "File exceeds the maximal length\n");
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        return 1;
    }

    // load the lines 1 by 1 and sort them into correct structs
    for (int i = 1; i < lineList.rowCount; i++) {
        Command command = {.keyword = {'\0'}, .A = -1, .B = -1, .C = -1};
        DataLine currentLine = lineList.dataLines[i];
        error = 0;

        switch (currentLine.keyword) {
            case SetKeyword: {

                if (CommandFound == true) {
                    fprintf(stderr, "Wrong file format\n");
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;
                }
                Set s = {.items = NULL, .itemCount = 0, .maxItemCount = 0, .lineNumber = currentLine.rowIndex + 1};
                error = 
                    SetConstructor(&s) ||
                    PopulateSet(&currentLine, &s, &universum);
                    
                if ((AddToSets(&setCollection, s) || !IsSetUnique(s)) == 1) {
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;
                }

                DisplaySet(s, universum);
                SetRelFound = true;
                break;
            }
                
            case RelationKeyword: {
                if (CommandFound == true) {
                    fprintf(stderr, "Wrong file format\n");
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;
                }
                Relation r = {.pairs = NULL, .pairCount = 0, .maxSize = 0, .LineNumber = currentLine.rowIndex + 1};
                error = 
                    RelationCtor(&r) || 
                    PopulateRelation(&currentLine, &r, &universum);
                
                if ((AddToRelations(&relationCollection, r) || !IsRelationUnique(r)) == 1) {
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;             
                }

                DisplayRelation(universum, r);
                SetRelFound = true;
                break;
            }
                
            case CommandKeyword:
                if (SetRelFound != true) {
                    fprintf(stderr, "Wrong file format\n");
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;
                }
                error = GetCommand(currentLine.data, &command);
                if (ResolveCommand(command, &setCollection, universum, &relationCollection) == 1) {
                    fprintf(stderr, "Invalid command\n");
                    FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                    return 1;
                }
                CommandFound = true;
                break;

            default:
                FreeAll(&lineList, &setCollection, &relationCollection, &universum);
                fprintf(stderr, "Wrong keyword");
                return 1;
        }
        if (error != 0) {
            FreeAll(&lineList, &setCollection, &relationCollection, &universum);
            return 1;
        }
        
    }
    if (CommandFound == false) {
        FreeAll(&lineList, &setCollection, &relationCollection, &universum);
        fprintf(stderr, "No commands\n");
        return 1;
    }

    FreeAll(&lineList, &setCollection, &relationCollection, &universum);

    return 0;
}

void FreeAll(LineList *list, Sets *sets, Relations *relations, Universum *universum) {
    FreeLineList(list);
    FreeSetCollection(sets);
    FreeRelationCollection(relations);
    FreeUniversum(universum);
}

void FreeRelationCollection(Relations *relations) {
    if (relations == NULL) return;
    // Freeing pairs
    for (int i = 0; i < relations->relCount; i++) {
        FreeRelation(&relations->relations[i]);
    }

    free(relations->relations);

    relations->relations = NULL;
    relations = NULL;
    
}

void FreeSetCollection(Sets *sets) {
    if (sets == NULL) return;
    // Freeing sets arrays
    for (int i =0; i < sets->setCount; i++) {
        FreeSet(&sets->sets[i]);
    }

    free(sets->sets);

    sets->sets = NULL;
    sets = NULL;
}

void FreeRelation(Relation *relation) {
    if (relation == NULL) return;

    free(relation->pairs);
    relation = NULL;
}

// handles params for every needed command
bool ValidNumOfParams(Command command, int num) {
    if (num == 1) {
        return (command.A != -1 && command.B == -1 && command.C == -1);
    }
    else if (num == 2) {
        return (command.A != -1 && command.B != -1 && command.C == -1);
    }
    else if (num == 3) {
        return (command.A != -1 && command.B != -1 && command.C != -1);
    }
    return false;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//                      Functions over sets
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
    for (int i = 0; i < a.itemCount; i++) {
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
    FreeSet(&s);
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
    FreeSet(&s);
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
    FreeSet(&s);
}

void IsSubset(Set a, Set b) {
    int found = 0;
    for (int i = 0; i < a.itemCount; i++) {
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found++;
                break;
            }
        }
    }
    if (found == a.itemCount && b.itemCount != a.itemCount) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

void IsSubsetEq(Set a, Set b) {
    int found = 0;
    for (int i = 0; i < a.itemCount; i++) {
        for (int j = 0; j < b.itemCount; j++) {
            if (a.items[i] == b.items[j]) {
                found++;
                break;
            }
        }
    }
    if (found == a.itemCount) {
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
    fprintf(stderr, "Not a set\n");
    return -1;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//                    Functions over relations
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int IsInjective(Relation relation, Set setA, Set setB, Universum u)
{
    if (!CheckFunctionValidity(relation, setA, setB))
    {
        return 0;
    }
    bool *haveSeen = malloc(sizeof(bool) * u.itemCount);
    for (int i = 0; i < u.itemCount; i++)
    {
        haveSeen[i] = false;
    }
    for (int i = 0; i < relation.pairCount; i++)
    {
        if (haveSeen[relation.pairs[i].right] != true)
        {
            haveSeen[relation.pairs[i].right] = true;
        }
        else
        {
            free(haveSeen);
            return 0;
        }
    }
    free(haveSeen);
    return 1;
}

int IsSurjective(Relation relation, Set setA, Set setB, Universum universum)
{
    int count = 0;
    if (!CheckFunctionValidity(relation, setA, setB))
    {
        return 0;
    }
    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    for (int i = 0; i < universum.itemCount; i++)
    {
        haveSeen[i] = false;
    }
    for (int i = 0; i < relation.pairCount; i++)
    {
        haveSeen[relation.pairs[i].right] = true;
    }

    for (int i = 0; i < setB.itemCount; i++)
    {
        if (haveSeen[setB.items[i]] == true)
        {
            count++;
        }
    }
    free(haveSeen);
    if (count != setB.itemCount)
    {
        return 0;
    }

    return 1;
}

int IsBijective(Relation relation, Set setA, Set setB, Universum universum)
{
    if (IsInjective(relation, setA, setB, universum) && IsSurjective(relation, setA, setB, universum))
    {
        return 1;
    }
    return 0;
}

void IsReflexive(Relation relation, Universum universum) {
    int reflexiveElemets = 0;
    for (int i = 0; i < relation.pairCount; i++) {
        if (relation.pairs[i].right == relation.pairs[i].left) {
            reflexiveElemets++;
        }
    }
    if (reflexiveElemets != universum.itemCount) {
        printf("false\n");
    }
    else {
        printf("true\n");
    }
}

void IsSymetric(Relation relation) {
    int count = 0;
    for (int i = 0; i < relation.pairCount; i++) {
        for (int j = 0; j < relation.pairCount; j++) {
            if (relation.pairs[i].left == relation.pairs[j].right) {
                if (relation.pairs[i].right == relation.pairs[j].left) {
                    count++;
                    break;
                } 
            }
        }
    }
    if (count != relation.pairCount) {
        printf("false\n");
        return;
        }    
    printf("true\n");
}

void IsAntiSymetric(Relation relation) {
    for (int i = 0; i < relation.pairCount; i++) {
        if (relation.pairs[i].right == relation.pairs[i].left) continue;
        for (int j = 0; j < relation.pairCount; j++) {
            if (relation.pairs[j].right == relation.pairs[j].left) continue;
            
            if (relation.pairs[i].left == relation.pairs[j].right) {
                if (relation.pairs[i].right == relation.pairs[j].left) {
                    if (relation.pairs[i].right != relation.pairs[i].left) {
                        printf("false\n");
                        return;
                    }
                }
            }
        }
    }
    printf("true\n");
}

void IsTransitive(Relation relation) {
    for (int i = 0; i < relation.pairCount; i++) {
        for (int j = 0; j < relation.pairCount; j++) {
            if (relation.pairs[i].right == relation.pairs[j].left) {
                bool found = false;
                int tmpRight = relation.pairs[j].right;
                int tmpLeft = relation.pairs[i].left;
                for (int k = 0; k < relation.pairCount; k++) {
                    if (relation.pairs[k].right == tmpRight && relation.pairs[k].left == tmpLeft) {
                        found = true;
                    }
                    if (k == relation.pairCount-1 && found == false) {
                        printf("false\n");
                        return;
                    }
                }
            }
        }
    }
    printf("true\n");
}

int IsFunction(Relation relation) {
    for (int i = 0; i<relation.pairCount; i++) {
        int count = 0;
        for (int j = i; j<relation.pairCount; j++) {
            if (relation.pairs[i].left == relation.pairs[j].left) {
                count++;
                if (count != 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void PrintDomain(Relation relation, Universum universum) {

    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    for (int i = 0; i < universum.itemCount; i++) {
        haveSeen[i] = false;
    }
    printf("S");
    for (int i = 0;  i<relation.pairCount; i++) {
        haveSeen[relation.pairs[i].left] = true;
    }
    for (int i = 0;  i< universum.itemCount; i++) {
        if (haveSeen[i] == true) {
            printf(" %s", universum.items[i]);
        }
    }
    printf("\n");
    free(haveSeen);
}

void PrintCodomain(Relation relation, Universum universum) {
    bool *haveSeen = malloc(sizeof(bool) * universum.itemCount);
    for (int i = 0; i < universum.itemCount; i++) {
        haveSeen[i] = false;
    }
    printf("S");
    for (int i = 0;  i<relation.pairCount; i++) {
        haveSeen[relation.pairs[i].right] = true;
    
    }
    for (int i = 0;  i< universum.itemCount; i++) {
        if (haveSeen[i] == true) {
            printf(" %s", universum.items[i]);
        }
    }
    printf("\n");
    free(haveSeen);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//                      Command func
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// Function to call functions depending on command

int ResolveCommand(Command command, Sets *setCollection, Universum universum, Relations * relationCollection) {
    char *keyword = command.keyword;

    if (strcmp("empty", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        IsEmpty(A);   
    }
    else if (strcmp("card", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Card(A);
    }
    else if (strcmp("complement", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        SetMinus(setCollection->sets[0], A, &universum);
    }
    else if (strcmp("union", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        SetsUnion(A, B, &universum);
    }
    else if (strcmp("intersect", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        SetIntersect(A, B, &universum);
    }
    else if (strcmp("minus", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        SetMinus(A, B, &universum);
    }
    else if (strcmp("subseteq", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        IsSubsetEq(A, B);
    }
    else if (strcmp("subset", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        IsSubset(A, B);
    }
    else if (strcmp("equals", keyword) == 0) {
        if (ValidNumOfParams(command, 2) == false) return 1;
        Set A = setCollection->sets[(GetSetArrIndex(command.A, setCollection))];
        Set B = setCollection->sets[(GetSetArrIndex(command.B, setCollection))];
        if (GetSetArrIndex(command.A, setCollection) == -1 || GetSetArrIndex(command.B, setCollection) == -1) return 1;
        SetEquals(A, B);
    }
    else if (strcmp("reflexive", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1; 
        Relation R = relationCollection->relations[relindex];
        IsReflexive(R, universum);
    }
    else if (strcmp("symmetric", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        IsSymetric(R);
    }
    else if (strcmp("antisymmetric", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        IsAntiSymetric(R);
    }
    else if (strcmp("transitive", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        IsTransitive(R);
    }
    else if (strcmp("function", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        if (IsFunction(R)) {
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("domain", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        PrintDomain(R, universum);
    }
    else if (strcmp("codomain", keyword) == 0) {
        if (ValidNumOfParams(command, 1) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        if (relindex == -1) return 1;
        Relation R = relationCollection->relations[relindex];
        PrintCodomain(R, universum);
    }
    else if (strcmp("injective", keyword) == 0) {
        if (ValidNumOfParams(command, 3) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        int setIndexA = GetSetArrIndex(command.B, setCollection);
        int setIndexB = GetSetArrIndex(command.C, setCollection);
        if (relindex == -1 || setIndexA == -1 || setIndexB == -1) return 1;
        Set A = setCollection->sets[setIndexA];
        Set B = setCollection->sets[setIndexB];
        Relation R = relationCollection->relations[relindex];
        if (IsInjective(R, A, B, universum)) {
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("surjective", keyword) == 0) {
        if (ValidNumOfParams(command, 3) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        int setIndexA = GetSetArrIndex(command.B, setCollection);
        int setIndexB = GetSetArrIndex(command.C, setCollection);
        if (relindex == -1 || setIndexA == -1 || setIndexB == -1) return 1;
        Set A = setCollection->sets[setIndexA];
        Set B = setCollection->sets[setIndexB];
        Relation R = relationCollection->relations[relindex];
        if (IsSurjective(R, A, B, universum)) {
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    }
    else if (strcmp("bijective", keyword) == 0) {
        if (ValidNumOfParams(command, 3) == false) return 1;
        int relindex = FindRelIndex(command.A, relationCollection);
        int setIndexA = GetSetArrIndex(command.B, setCollection);
        int setIndexB = GetSetArrIndex(command.C, setCollection);
        if (relindex == -1 || setIndexA == -1 || setIndexB == -1) return 1;
        Set A = setCollection->sets[setIndexA];
        Set B = setCollection->sets[setIndexB];
        Relation R = relationCollection->relations[relindex];
        if (IsBijective(R, A, B, universum)) {
            printf("true\n");
        }
        else {
            printf("false\n");
        }
    } else {    
        return 1;
    }
    return 0;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//          DYNAMIC STRUCTURE MANIPULATION
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void* ArrAlloc(void *target, size_t typeSize, int *maxSize_p, int currentSize) {
    void* tmp = target;

    // New allocation
    if (*maxSize_p == 0 && currentSize == 0) {
       *maxSize_p = DEFAULT_ALLOCATION_SIZE;
       tmp = malloc(*maxSize_p * typeSize);
    }

    // Realloc
    if (*maxSize_p == currentSize) {
        *maxSize_p *= 2;
        tmp = realloc(target, *maxSize_p * typeSize);
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
    if (set == NULL) return 1;

    set->items = malloc(DEFAULT_ALLOCATION_SIZE * sizeof(int));

    if (set->items == NULL) return 1;
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
    set = NULL;
}

int AddCharToDataLine(DataLine *list, char c) {
    if (list->currentLength == list->maxLength) {
        char *tmp = realloc(list->data, sizeof(char) * 2* list->maxLength);
        if (tmp == NULL) {
            fprintf(stderr, "Allocation unsuccessful\n");
            return 1;
        }
        list->data = tmp;
        list->maxLength *= 2;
    }

    list->data[list->currentLength] = c;
    list->currentLength++;
    return 0;
}

void DataLineDtor(DataLine *list) {
    if (list->data == NULL) return;
    free(list->data);
    list->data = NULL;
}

LineList LineListConstructor() {
    LineList list = {.dataLines = NULL, .rowCount = 0, .maxRows = DEFAULT_ALLOCATION_SIZE};
    list.dataLines = malloc(list.maxRows * sizeof(DataLine));
    return list;
}

int AddToLineList(LineList *lines, DataLine line) {
    // realloc incase of full capacity
    if (lines->rowCount == lines->maxRows) {
        DataLine *tmp= realloc(lines->dataLines, sizeof(DataLine) * 2 * lines->maxRows);
        if (tmp == NULL) {
            fprintf(stderr, "Allocation unsuccessful\n");
            return 1;
        }
        lines->dataLines= tmp;
        lines->maxRows *= 2;
    }

    lines->dataLines[lines->rowCount] = line;
    lines->rowCount++;
    return 0;
}

void FreeLineList(LineList *lines) {
    if (lines->dataLines == NULL) return;

    for (int i = 0; i < lines->rowCount; i++) {
        DataLineDtor(&lines->dataLines[i]);
    }
    
    free(lines->dataLines);
    lines->dataLines = NULL;
    lines = NULL;
}

int ReallocUniversum(Universum *universum, int newSize) {
    // 
    if (universum->itemCount == 0) {
        universum->items = (char **)malloc(DEFAULT_ALLOCATION_SIZE * sizeof(char *));
        if (universum->items == NULL) {
            fprintf(stderr, "Allocation unsuccessful\n");
            return 1;
        }
    } else {
        char **tmp = realloc(universum->items, newSize * sizeof(char *));

        if (tmp == NULL) {
            fprintf(stderr, "Allocation unsuccessful\n");
            return 1;
        }
        universum->items = tmp;
    }

    //columns allocation
    for (int i = universum->itemCount; i < universum->maxItemCount; i++) {
        universum->items[i] = (char *)malloc(MAX_STRING_LENGTH);
        if (universum->items[i] == NULL) {
            fprintf(stderr, "Allocation unsuccessful\n");
            return 1;
        }
    }
    return 0;
}

void FreeUniversum(Universum *universum) {
    if (universum == NULL) return;
    
    // Inner arrays cleanup
    for (int i = 0; i < universum->maxItemCount; i++) {
        free(universum->items[i]);
    }

    // Outer cleanup
    free(universum->items);
    universum->items = NULL;
    universum->maxItemCount = 0;
    universum->itemCount = 0;
    universum = NULL;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// Creates a set from universum to access it as line 1
int GetUniversumSet(Set *set, Universum universum, DataLine universumLine) {
    int result = 
        SetConstructor(set) ||
        PopulateSet(&universumLine, set, &universum);

    if (result != 0) {
        fprintf(stderr, "Error loading the universum\n"); 
        return 1;
    }
    return 0;
}

void DisplayUniversum(Universum universum) {
    if (universum.itemCount == 0) {
        printf("U");
    } else {
        printf("U "); //TODO: needs redoing
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
        fprintf(stderr, "Array is not initialized\n");
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
            fprintf(stderr, "Item is not present in universum\n");
            return 1;
        }

        // Adding item to set
        int result = AddToSet(set, wordId);
        if (result != 0) {
            fprintf(stderr, "Cannot add item to the set\n");
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

            if (set.items[i] == set.items[j]) {
                fprintf(stderr, "Set has duplicate items\n");
                return 0;
            }
        }
    }

    return 1;
}

int IsRelationUnique(Relation relation) {
    for (int i = 0; i <relation.pairCount; i++) {
        for (int j = 0; j < relation.pairCount; j++) {
            if (i == j) continue;
            if (relation.pairs[i].left == relation.pairs[j].left && relation.pairs[i].right == relation.pairs[j].right) {
                fprintf(stderr, "Relation has duplicate items\n");
                return 0;
            }
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

// Gets the index of the word in the universum to save it in set as int
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

// Loads the whole file into 1 datastruct that gets processed later
int GetDataFromFile(LineList *LineList, char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Wrong file path\n");
        return 1;
    }

    DataLine line = DataLineConstructor();
    
    int currentIndex = 0;
    int charInt = fgetc(file);
    int currentRow = 0;
    while (charInt != EOF) { 
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
            fprintf(stderr, "Keyword is not a single char\n");
            DataLineDtor(&line);
            fclose(file);
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
    
    DataLineDtor(&line);
    fclose(file);
    // Empty file
    if (LineList->rowCount == 0) {
        fprintf(stderr, "The file is empty\n");
        return 1;
    }

    return 0;
}

// Allocates memory to universum
int InitUniversum(Universum *universum) {
    return ReallocUniversum(universum, 0);
}

// Loads values from the DataLine struct into universum
int PopulateUniversum(DataLine *source, Universum *universum) {
    if (source->keyword != UniversumKeyword) {
        fprintf(stderr, "No universum\n");
        return 1;
    }
    char *data = source->data;

    int wordLength = 0;
    char tmpWord[MAX_STRING_LENGTH + 1] = {'\0'};
    
    for (int i = 0; data[i] != '\0'; i++) {
        char currentChar = data[i];

        if (tmpWord[MAX_STRING_LENGTH - 1] != '\0') {
            fprintf(stderr, "Universum element exceeds the maximal length\n");
            return 1;
        }
        
        // Adding char to temp
        if (currentChar != ' ') {
            if (currentChar == '\n') {
                if (IsKeyword(tmpWord)) {
                    fprintf(stderr, "Universum element is a forbidden word\n");
                    return 1;
                } else {
                    if (tmpWord[0] != '\0') {
                        AddUniversumItem(universum, tmpWord);
                        ClearTempWord(tmpWord);
                    }
                    break;
                }
            }
            if (!isalpha(currentChar)) {
                fprintf(stderr, "Universum element contains unsupported char!\n");
                return 1;
            }
            tmpWord[wordLength] = currentChar;
            wordLength++;
            continue;
        }
        
        // Exception handling    
        if (wordLength == 0) {
            fprintf(stderr, "Universum element is separated by 2 or more spaces\n");
            return 1;
        }
        if (IsKeyword(tmpWord)) {
            fprintf(stderr, "Universum element is a forbidden word\n");
            return 1;
        }
        if (tmpWord[MAX_STRING_LENGTH] != '\0') {
            fprintf(stderr, "Universum element exceeds the maximal length\n");
            return 1;
        }

        wordLength = 0;
        AddUniversumItem(universum, tmpWord);
        ClearTempWord(tmpWord);
        
    }
    return 0;
}

// Adds string into universum
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

// Handles all of the forbidden words
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

int IsCommand(char *command) {
    char *commands[] = {"empty", "card", "complement", "union",
    "intersect", "minus", "subseteq", "subset", "equals", "reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain",
    "codomain", "injective", "surjective", "bijective"};

    for (int i = 0; i < 19; i++) {
        if (strcmp(commands[i], command) == 0) {
            return 0;
        }
    }
    return 1;
}
void ClearTempWord(char *tmpWord) {
    memset(tmpWord, '\0', sizeof(char) * MAX_STRING_LENGTH);
}


//function check arguments in commands
int CheckCommandArg(int number) {
    if (number >= 100) { //number bigger than 100 * 10
        fprintf(stderr, ("Too long command number argument identifier\n"));
        return 1;
    }
    return 0;
}

// Parses command
int GetCommand(char line[], Command *command) {
    int SpaceCount = 0;
    int SpaceIdentifier = false;
    memset(command->keyword, 0, MAX_COMMAND_LENGTH + 1); //reset command
    // initializes all of the line numbers default as -1 (-1 is gonna be handled as error)
    command->A = -1;
    command->B = -1;
    command->C = -1;
    
    for (int index_1 = 0, index_2 = 0; line[index_1] != '\0'; index_1++) {
        if (line[index_1] == ' ') {
            // handle 2 spaces in a row after row identifier
            if (SpaceIdentifier) {
                fprintf(stderr, "Command contains 2 spaces in row\n");
                return 1;
            }
            SpaceCount++;
            index_2 = 0;
            SpaceIdentifier = true;
            continue;
        }
        else if (line[index_1] == '\n') {
            continue;
        }

        SpaceIdentifier = false; //if symbol is not 'space'

        // populating command->keyword
        if (SpaceCount == 0) {
            // check if command is longer than its max length, return error if so
            if (index_2 >= MAX_COMMAND_LENGTH) {
                fprintf(stderr, "Incorrect command\n");
                return 1;
            }

            command->keyword[index_2] = line[index_1];
            index_2++;
        }
        // Read first line index (command->A)
        else if (SpaceCount == 1) {

            if (CheckCommandArg(command->A) == 1) return 1;

            if (command->A == -1) {
                command->A = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->A *= index_2;
                command->A += (line[index_1] - '0');
            }
        }
        // Read second line index
        else if (SpaceCount == 2) {

            if (CheckCommandArg(command->B) == 1) return 1;

            if (command->B == -1) {
                command->B = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->B *= index_2;
                command->B += (line[index_1] - '0');
            }
        }
        // Read third line index
        else if (SpaceCount == 3) {

            if (CheckCommandArg(command->C) == 1) return 1;

            if (command->C == -1) {
                command->C = line[index_1] - '0';
                index_2 = 10;
            }
            else{
                command->C *= index_2;
                command->C += (line[index_1] - '0');
            }
        }
        else if (SpaceCount > 3) {
            fprintf(stderr, "Too many arguments in command line\n");
            return 1;
        }
    }
    // check if command->keyword is one of the command identifiers
    if (IsCommand(command->keyword) != 0) {
        fprintf(stderr, "Wrong command keyword\n");
        return 1;
    }
    return 0;
}

//Loading relations
int RelationCtor(Relation *relation) {
    if (relation == NULL) return 1;

    relation->pairs = malloc(DEFAULT_ALLOCATION_SIZE * sizeof(Pair));
    if (relation->pairs == NULL) return 1;

    relation->maxSize = DEFAULT_ALLOCATION_SIZE;
    relation->pairCount = 0;
    return 0;
}
int PopulateRelation(DataLine *source, Relation *relation, Universum *universum) {
    //Values init
    char pairFrstItem[MAX_STRING_LENGTH]= {'\0'};
    char pairSecondItem[MAX_STRING_LENGTH]= {'\0'};
    int pairIndex = 0;
    int spaceCount = 0;
    bool foundparentL = false;
    bool foundparentR = false;

    // Exception handling
    if (source->data == NULL || universum->items == NULL) {
        fprintf(stderr, "Array is not initialized.\n");
        return 1;
    }
    // Parses relation
    for (int i = 0; i < source->currentLength-1; i++) {
        char currentChar = source->data[i];

        if (i == 0 && currentChar == '\n') break; // Relation is empty
        
        if (pairIndex == MAX_STRING_LENGTH) {
            fprintf(stderr, "Relation element exceeds the maximal length\n");
            return 1;
        }

        if (currentChar == '(') {
            if (foundparentR) {
                fprintf(stderr, "Wrong relation format\n");
                return 1;
            }
            foundparentL = true;
        }
        else if (currentChar == ')') {
            if (foundparentR) {
                fprintf(stderr, "Wrong relation format\n");
                return 1;
            }
            foundparentR = true;
        }
        else if (currentChar == ' ' && spaceCount <=2) {
            spaceCount += 1;
            pairIndex = 0;
        }
        else if (foundparentL && spaceCount == 0) {
            pairFrstItem[pairIndex++] = currentChar;
        }
        else if (foundparentL && spaceCount == 1 ) {
            if (currentChar != ' ' && currentChar != '\n') {
                pairSecondItem[pairIndex++] = currentChar;
            }
        }
        else {
            fprintf(stderr, "Wrong relation format\n");
            return 1;
        }
        if ((foundparentL && foundparentR && spaceCount == 2) || currentChar == '\n') {

            int firstItemId = GetItemIndex(universum, pairFrstItem);
            int secondItemId = GetItemIndex(universum, pairSecondItem);
            if (firstItemId == -1 || secondItemId == -1) {
                fprintf(stderr, "Item is not present in universum\n");
                return 1;
            }
            int result = AddToRelation(firstItemId, secondItemId, relation);
            if (result != 0) {
                fprintf(stderr, "Cannot add pair to the relation\n");
                return 1;
            }
            
            // Clears temporary words to be reused
            ClearTempWord(pairFrstItem);
            ClearTempWord(pairSecondItem);
            foundparentL = false;
            foundparentR = false;
            spaceCount = 0;
        }
    }
    return 0;
}

// Adds each word from pair to relation
int AddToRelation(int left, int right, Relation *relation) {
    //Checking for reallocation
    if (relation->maxSize == relation->pairCount) {
        relation->maxSize *= 2;
        Pair *tmp = realloc(relation->pairs, relation->maxSize * sizeof(Pair));
        if (tmp == NULL) return 1;
        relation->pairs = tmp;
        //relation->pairs->right = tmp;
    }
    relation->pairs[relation->pairCount].left = left;
    relation->pairs[relation->pairCount].right = right;
    relation->pairCount++;
    return 0;
}

// Adds relation to relations
int AddToRelations(Relations *relColl, Relation relation) {
    //Using the function to ensure capacity
    relColl->relations = ArrAlloc(relColl->relations, sizeof(Relation), &relColl->maxRelCount, relColl->relCount);
    if (relColl == NULL) return 1;
    relColl->relations[relColl->relCount] = relation;
    relColl->relCount++;
    return 0;
}

void DisplayRelation(Universum universum, Relation relation) {
    printf("R");
    for (int i = 0; i <relation.pairCount; i++) {
        int currentLeftItemId = relation.pairs[i].left;
        int currentRightItemId = relation.pairs[i].right;
        printf(" (%s %s)",universum.items[currentLeftItemId], universum.items[currentRightItemId]);
        
    }
    printf("\n");
}

// Finds a relation index based on its line number 
int FindRelIndex(int command, Relations *relationArr) {
    for (int relindex = 0; relindex < relationArr->relCount; relindex++) {
        if (relationArr->relations[relindex].LineNumber == command) {
            return relindex;
        }
    }
    fprintf(stderr, "Not a relation\n");
    return -1;
}

int IsInSet(Set set, int elementIndex) {
    for (int i = 0; i < set.itemCount; i++) {
        if (set.items[i] == elementIndex) {
            return 1;
        }
    }
    return 0;

}

// Function for last 3 operations
int CheckFunctionValidity(Relation relation, Set setA, Set setB) {
    if (!IsFunction(relation)) return 0;
    if (setA.itemCount != relation.pairCount) return 0;
    for (int i = 0; i < relation.pairCount; i++) {
        if (!IsInSet(setA, relation.pairs[i].left)) return 0;
        }
    for (int i = 0; i < relation.pairCount; i++) {
        if (!IsInSet(setB, relation.pairs[i].right)) return 0;
    }
    return 1;
}