#ifndef DSA_H
#define DSA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LEN 64
#define MAX_DEPT_LEN 32
#define MAX_EMAIL_LEN 64
#define MAX_PHONE_LEN 16
#define MAX_CERT_LEN 32
#define HASH_TABLE_SIZE 17
#define MAX_COURSES 8
#define DATA_FILE "data/students.dat"
#define QUEUE_FILE "data/requests.dat"
#define STACK_FILE "data/undo_stack.dat"

/* ========================================================================= */
/* 1. STUDENT ENTITY & LIST ADT / SINGLY LINKED LIST                         */
/* ========================================================================= */

typedef struct Student {
    int rollNo;
    char name[MAX_NAME_LEN];
    char department[MAX_DEPT_LEN];
    float cgpa;
    char email[MAX_EMAIL_LEN];
    char phone[MAX_PHONE_LEN];
} Student;

/* Singly Linked List Node */
typedef struct StudentNode {
    Student data;
    struct StudentNode* next;
} StudentNode;

/* Singly Linked List ADT */
typedef struct {
    StudentNode* head;
    int count;
} StudentList;

/* ========================================================================= */
/* 2. STACK ADT (For Undo Last Record Update / Action)                       */
/* ========================================================================= */

typedef enum {
    ACTION_UPDATE,
    ACTION_ADD,
    ACTION_DELETE
} ActionType;

typedef struct UndoAction {
    ActionType type;
    Student previousState;
    Student currentState;
} UndoAction;

typedef struct StackNode {
    UndoAction action;
    struct StackNode* next;
} StackNode;

typedef struct {
    StackNode* top;
    int size;
} UndoStack;

/* ========================================================================= */
/* 3. QUEUE ADT (For Certificate / ID-Card Request Processing)               */
/* ========================================================================= */

typedef struct CertRequest {
    int reqId;
    int rollNo;
    char studentName[MAX_NAME_LEN];
    char certType[MAX_CERT_LEN];  // e.g., "Bonafide", "ID Card", "Transcript"
    char status[16];             // "Pending", "Processed"
} CertRequest;

typedef struct QueueNode {
    CertRequest req;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int count;
} RequestQueue;

/* ========================================================================= */
/* 4. HASH TABLE (Separate Chaining for O(1) Roll Number Lookup)             */
/* ========================================================================= */

typedef struct HashNode {
    Student data;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode* buckets[HASH_TABLE_SIZE];
    int count;
} HashTable;

/* ========================================================================= */
/* 5. BINARY SEARCH TREE (BST for Department-Wise Student Organization)      */
/* ========================================================================= */

typedef struct BSTStudentNode {
    Student data;
    struct BSTStudentNode* next;
} BSTStudentNode;

typedef struct BSTNode {
    char department[MAX_DEPT_LEN];
    BSTStudentNode* studentsHead;
    int studentCount;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

/* ========================================================================= */
/* 6. GRAPH (Course Prerequisites DAG & Topological Sort)                    */
/* ========================================================================= */

typedef struct {
    int id;
    char code[16];
    char title[64];
    int credits;
    int semester;
} Course;

typedef struct AdjNode {
    int destCourseId;
    struct AdjNode* next;
} AdjNode;

typedef struct {
    int numVertices;
    Course courses[MAX_COURSES];
    AdjNode* adjLists[MAX_COURSES];
    int inDegree[MAX_COURSES];
} CourseGraph;

/* ========================================================================= */
/* FUNCTION DECLARATIONS                                                     */
/* ========================================================================= */

/* --- Linked List Operations --- */
StudentList* createStudentList(void);
bool insertStudent(StudentList* list, Student s);
bool updateStudent(StudentList* list, Student s, Student* oldState);
bool deleteStudent(StudentList* list, int rollNo, Student* deletedData);
Student* findStudentByRoll(StudentList* list, int rollNo);
void freeStudentList(StudentList* list);
int getStudentCount(StudentList* list);

/* --- Stack Operations (Undo) --- */
UndoStack* createUndoStack(void);
void pushUndo(UndoStack* stack, UndoAction action);
bool popUndo(UndoStack* stack, UndoAction* action);
bool peekUndo(UndoStack* stack, UndoAction* action);
bool isUndoStackEmpty(UndoStack* stack);
void freeUndoStack(UndoStack* stack);
void saveUndoStackToFile(UndoStack* stack);
void loadUndoStackFromFile(UndoStack* stack);

/* --- Queue Operations (Certificates) --- */
RequestQueue* createRequestQueue(void);
void enqueueRequest(RequestQueue* q, CertRequest req);
bool dequeueRequest(RequestQueue* q, CertRequest* req);
bool isQueueEmpty(RequestQueue* q);
void freeRequestQueue(RequestQueue* q);
void saveQueueToFile(RequestQueue* q);
void loadQueueFromFile(RequestQueue* q);

/* --- Hash Table Operations --- */
HashTable* createHashTable(void);
int hashFunction(int rollNo);
void insertHash(HashTable* ht, Student s);
Student* searchHash(HashTable* ht, int rollNo);
bool deleteHash(HashTable* ht, int rollNo);
void freeHashTable(HashTable* ht);
void rebuildHashTable(HashTable* ht, StudentList* list);

/* --- BST Operations --- */
BSTNode* createBSTNode(const char* dept);
BSTNode* insertBST(BSTNode* root, Student s);
void freeBST(BSTNode* root);
BSTNode* buildDepartmentBST(StudentList* list);

/* --- Graph Operations --- */
CourseGraph* createCourseGraph(void);
void addPrerequisite(CourseGraph* g, int srcCourseId, int destCourseId);
void getTopologicalSort(CourseGraph* g, int* order, int* orderSize, bool* hasCycle);
void freeCourseGraph(CourseGraph* g);

/* --- Sorting & Searching --- */
void sortStudentsByCGPA(Student* arr, int n, bool ascending);
void sortStudentsByName(Student* arr, int n, bool ascending);
int binarySearchByRoll(Student* arr, int n, int rollNo);
int linearSearchByName(Student* arr, int n, const char* query, Student* results, int maxResults);

/* --- File Handling --- */
bool saveStudentsToFile(StudentList* list);
bool loadStudentsFromFile(StudentList* list);

/* --- JSON & Output Utilities --- */
void printStudentJson(const Student* s);
void escapeJsonString(const char* input, char* output, size_t maxLen);

#endif /* DSA_H */
