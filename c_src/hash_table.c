#include "dsa.h"

/* Create a Hash Table initialized with NULL buckets */
HashTable* createHashTable(void) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Error: Memory allocation failed for HashTable.\n");
        return NULL;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    ht->count = 0;
    return ht;
}

/* Division-based Hash Function (using prime bucket size 17) */
int hashFunction(int rollNo) {
    int hash = rollNo % HASH_TABLE_SIZE;
    if (hash < 0) hash += HASH_TABLE_SIZE;
    return hash;
}

/* Insert a student into Hash Table using Separate Chaining for collision resolution */
void insertHash(HashTable* ht, Student s) {
    if (!ht) return;

    int bucketIndex = hashFunction(s.rollNo);

    /* Check if student already exists in bucket chain; if so, update */
    HashNode* curr = ht->buckets[bucketIndex];
    while (curr != NULL) {
        if (curr->data.rollNo == s.rollNo) {
            curr->data = s;
            return;
        }
        curr = curr->next;
    }

    /* Allocate new node and insert at head of the bucket chain */
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for HashNode.\n");
        return;
    }
    newNode->data = s;
    newNode->next = ht->buckets[bucketIndex];
    ht->buckets[bucketIndex] = newNode;
    ht->count++;
}

/* Search for a student by Roll Number in O(1) average time */
Student* searchHash(HashTable* ht, int rollNo) {
    if (!ht) return NULL;

    int bucketIndex = hashFunction(rollNo);
    HashNode* curr = ht->buckets[bucketIndex];

    while (curr != NULL) {
        if (curr->data.rollNo == rollNo) {
            return &(curr->data);
        }
        curr = curr->next;
    }
    return NULL;
}

/* Delete a student from Hash Table by Roll Number */
bool deleteHash(HashTable* ht, int rollNo) {
    if (!ht) return false;

    int bucketIndex = hashFunction(rollNo);
    HashNode* curr = ht->buckets[bucketIndex];
    HashNode* prev = NULL;

    while (curr != NULL && curr->data.rollNo != rollNo) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return false;

    if (prev == NULL) {
        ht->buckets[bucketIndex] = curr->next;
    } else {
        prev->next = curr->next;
    }

    free(curr);
    ht->count--;
    return true;
}

/* Free all nodes in all buckets */
void freeHashTable(HashTable* ht) {
    if (!ht) return;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* curr = ht->buckets[i];
        while (curr != NULL) {
            HashNode* next = curr->next;
            free(curr);
            curr = next;
        }
        ht->buckets[i] = NULL;
    }
    ht->count = 0;
    free(ht);
}

/* Rebuild entire hash table from the primary StudentList */
void rebuildHashTable(HashTable* ht, StudentList* list) {
    if (!ht || !list) return;

    /* Clear existing buckets */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* curr = ht->buckets[i];
        while (curr != NULL) {
            HashNode* next = curr->next;
            free(curr);
            curr = next;
        }
        ht->buckets[i] = NULL;
    }
    ht->count = 0;

    /* Populate from list */
    StudentNode* curr = list->head;
    while (curr != NULL) {
        insertHash(ht, curr->data);
        curr = curr->next;
    }
}
