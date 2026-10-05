#include "dsa.h"
#include <ctype.h>

/* Helper for case-insensitive string comparison */
static bool containsIgnoreCase(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;
    if (strlen(needle) == 0) return true;

    char hLower[MAX_NAME_LEN];
    char nLower[MAX_NAME_LEN];

    size_t i = 0;
    for (; haystack[i] && i < sizeof(hLower) - 1; i++) {
        hLower[i] = (char)tolower((unsigned char)haystack[i]);
    }
    hLower[i] = '\0';

    size_t j = 0;
    for (; needle[j] && j < sizeof(nLower) - 1; j++) {
        nLower[j] = (char)tolower((unsigned char)needle[j]);
    }
    nLower[j] = '\0';

    return strstr(hLower, nLower) != NULL;
}

/* ========================================================================= */
/* MERGE SORT FOR CGPA                                                       */
/* ========================================================================= */

static void mergeCGPA(Student* arr, int left, int mid, int right, bool ascending) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Student* L = (Student*)malloc(sizeof(Student) * n1);
    Student* R = (Student*)malloc(sizeof(Student) * n2);
    if (!L || !R) {
        if (L) free(L);
        if (R) free(R);
        return;
    }

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        bool condition = ascending ? (L[i].cgpa <= R[j].cgpa) : (L[i].cgpa >= R[j].cgpa);
        if (condition) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

static void mergeSortCGPAHelper(Student* arr, int left, int right, bool ascending) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortCGPAHelper(arr, left, mid, ascending);
        mergeSortCGPAHelper(arr, mid + 1, right, ascending);
        mergeCGPA(arr, left, mid, right, ascending);
    }
}

void sortStudentsByCGPA(Student* arr, int n, bool ascending) {
    if (!arr || n <= 1) return;
    mergeSortCGPAHelper(arr, 0, n - 1, ascending);
}

/* ========================================================================= */
/* MERGE SORT FOR NAME (Alphabetical)                                        */
/* ========================================================================= */

static void mergeName(Student* arr, int left, int mid, int right, bool ascending) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Student* L = (Student*)malloc(sizeof(Student) * n1);
    Student* R = (Student*)malloc(sizeof(Student) * n2);
    if (!L || !R) {
        if (L) free(L);
        if (R) free(R);
        return;
    }

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        int cmp = strcmp(L[i].name, R[j].name);
        bool condition = ascending ? (cmp <= 0) : (cmp >= 0);
        if (condition) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

static void mergeSortNameHelper(Student* arr, int left, int right, bool ascending) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortNameHelper(arr, left, mid, ascending);
        mergeSortNameHelper(arr, mid + 1, right, ascending);
        mergeName(arr, left, mid, right, ascending);
    }
}

void sortStudentsByName(Student* arr, int n, bool ascending) {
    if (!arr || n <= 1) return;
    mergeSortNameHelper(arr, 0, n - 1, ascending);
}

/* ========================================================================= */
/* BINARY SEARCH BY ROLL NUMBER (Requires array sorted by Roll No)           */
/* ========================================================================= */

int binarySearchByRoll(Student* arr, int n, int rollNo) {
    if (!arr || n <= 0) return -1;
    int low = 0, high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid].rollNo == rollNo) {
            return mid;
        }
        if (arr[mid].rollNo < rollNo) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

/* ========================================================================= */
/* LINEAR SEARCH BY NAME (Substring Match)                                   */
/* ========================================================================= */

int linearSearchByName(Student* arr, int n, const char* query, Student* results, int maxResults) {
    if (!arr || !query || !results || n <= 0 || maxResults <= 0) return 0;
    int count = 0;

    for (int i = 0; i < n && count < maxResults; i++) {
        if (containsIgnoreCase(arr[i].name, query)) {
            results[count++] = arr[i];
        }
    }
    return count;
}
