#include "dsa.h"

/* Create an empty student list */
StudentList* createStudentList(void) {
    StudentList* list = (StudentList*)malloc(sizeof(StudentList));
    if (!list) {
        fprintf(stderr, "Error: Memory allocation failed for StudentList.\n");
        return NULL;
    }
    list->head = NULL;
    list->count = 0;
    return list;
}

/* Insert a student into the singly linked list (Maintains sorted order by Roll Number) */
bool insertStudent(StudentList* list, Student s) {
    if (!list) return false;

    /* Check if student with same roll number already exists */
    if (findStudentByRoll(list, s.rollNo) != NULL) {
        return false; /* Duplicate roll number */
    }

    StudentNode* newNode = (StudentNode*)malloc(sizeof(StudentNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for StudentNode.\n");
        return false;
    }
    newNode->data = s;
    newNode->next = NULL;

    /* If list is empty or new node belongs before head */
    if (list->head == NULL || list->head->data.rollNo > s.rollNo) {
        newNode->next = list->head;
        list->head = newNode;
        list->count++;
        return true;
    }

    /* Traverse to find appropriate insertion position (sorted by roll number) */
    StudentNode* current = list->head;
    while (current->next != NULL && current->next->data.rollNo < s.rollNo) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
    list->count++;
    return true;
}

/* Update an existing student record and record the old state for Undo Stack */
bool updateStudent(StudentList* list, Student s, Student* oldState) {
    if (!list) return false;

    StudentNode* current = list->head;
    while (current != NULL) {
        if (current->data.rollNo == s.rollNo) {
            if (oldState) {
                *oldState = current->data; /* Save previous state for Stack */
            }
            /* Update mutable fields */
            strncpy(current->data.name, s.name, MAX_NAME_LEN - 1);
            current->data.name[MAX_NAME_LEN - 1] = '\0';

            strncpy(current->data.department, s.department, MAX_DEPT_LEN - 1);
            current->data.department[MAX_DEPT_LEN - 1] = '\0';

            current->data.cgpa = s.cgpa;

            strncpy(current->data.email, s.email, MAX_EMAIL_LEN - 1);
            current->data.email[MAX_EMAIL_LEN - 1] = '\0';

            strncpy(current->data.phone, s.phone, MAX_PHONE_LEN - 1);
            current->data.phone[MAX_PHONE_LEN - 1] = '\0';

            return true;
        }
        current = current->next;
    }
    return false;
}

/* Delete a student record by roll number */
bool deleteStudent(StudentList* list, int rollNo, Student* deletedData) {
    if (!list || list->head == NULL) return false;

    StudentNode* temp = list->head;
    StudentNode* prev = NULL;

    /* If head node itself holds the roll number */
    if (temp != NULL && temp->data.rollNo == rollNo) {
        list->head = temp->next;
        if (deletedData) *deletedData = temp->data;
        free(temp);
        list->count--;
        return true;
    }

    /* Search for the key to be deleted */
    while (temp != NULL && temp->data.rollNo != rollNo) {
        prev = temp;
        temp = temp->next;
    }

    /* Key was not present in linked list */
    if (temp == NULL) return false;

    /* Unlink node from linked list */
    prev->next = temp->next;
    if (deletedData) *deletedData = temp->data;
    free(temp);
    list->count--;
    return true;
}

/* Find a student by roll number in the linked list */
Student* findStudentByRoll(StudentList* list, int rollNo) {
    if (!list) return NULL;
    StudentNode* current = list->head;
    while (current != NULL) {
        if (current->data.rollNo == rollNo) {
            return &(current->data);
        }
        current = current->next;
    }
    return NULL;
}

/* Free all nodes in the linked list to prevent memory leaks */
void freeStudentList(StudentList* list) {
    if (!list) return;
    StudentNode* current = list->head;
    while (current != NULL) {
        StudentNode* next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
    list->count = 0;
    free(list);
}

int getStudentCount(StudentList* list) {
    return list ? list->count : 0;
}
