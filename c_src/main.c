#include "dsa.h"

/* Helper to convert linked list to an array for sorting and searching */
static Student* listToArray(StudentList* list, int* count) {
    if (!list) {
        *count = 0;
        return NULL;
    }
    *count = list->count;
    if (*count <= 0) return NULL;

    Student* arr = (Student*)malloc(sizeof(Student) * (*count));
    if (!arr) return NULL;

    StudentNode* curr = list->head;
    int idx = 0;
    while (curr != NULL && idx < *count) {
        arr[idx++] = curr->data;
        curr = curr->next;
    }
    return arr;
}

/* ========================================================================= */
/* JSON API MODE HANDLERS                                                    */
/* ========================================================================= */

static void apiListStudents(StudentList* list) {
    printf("{\"status\":\"success\",\"count\":%d,\"students\":[", list->count);
    StudentNode* curr = list->head;
    bool first = true;
    while (curr != NULL) {
        if (!first) printf(",");
        printStudentJson(&curr->data);
        first = false;
        curr = curr->next;
    }
    printf("]}\n");
}

static void apiAddStudent(StudentList* list, int roll, const char* name, const char* dept, float cgpa, const char* email, const char* phone) {
    Student s;
    s.rollNo = roll;
    strncpy(s.name, name, MAX_NAME_LEN - 1); s.name[MAX_NAME_LEN - 1] = '\0';
    strncpy(s.department, dept, MAX_DEPT_LEN - 1); s.department[MAX_DEPT_LEN - 1] = '\0';
    s.cgpa = cgpa;
    strncpy(s.email, email, MAX_EMAIL_LEN - 1); s.email[MAX_EMAIL_LEN - 1] = '\0';
    strncpy(s.phone, phone, MAX_PHONE_LEN - 1); s.phone[MAX_PHONE_LEN - 1] = '\0';

    if (insertStudent(list, s)) {
        saveStudentsToFile(list);
        printf("{\"status\":\"success\",\"message\":\"Student added successfully\",\"student\":");
        printStudentJson(&s);
        printf("}\n");
    } else {
        printf("{\"status\":\"error\",\"message\":\"Student with Roll No %d already exists\"}\n", roll);
    }
}

static void apiUpdateStudent(StudentList* list, UndoStack* stack, int roll, const char* name, const char* dept, float cgpa, const char* email, const char* phone) {
    Student s;
    s.rollNo = roll;
    strncpy(s.name, name, MAX_NAME_LEN - 1); s.name[MAX_NAME_LEN - 1] = '\0';
    strncpy(s.department, dept, MAX_DEPT_LEN - 1); s.department[MAX_DEPT_LEN - 1] = '\0';
    s.cgpa = cgpa;
    strncpy(s.email, email, MAX_EMAIL_LEN - 1); s.email[MAX_EMAIL_LEN - 1] = '\0';
    strncpy(s.phone, phone, MAX_PHONE_LEN - 1); s.phone[MAX_PHONE_LEN - 1] = '\0';

    Student oldState;
    if (updateStudent(list, s, &oldState)) {
        /* Record update action on the Undo Stack */
        UndoAction action;
        action.type = ACTION_UPDATE;
        action.previousState = oldState;
        action.currentState = s;
        pushUndo(stack, action);
        saveUndoStackToFile(stack);
        saveStudentsToFile(list);

        printf("{\"status\":\"success\",\"message\":\"Record updated and state saved to Undo Stack\",\"student\":");
        printStudentJson(&s);
        printf("}\n");
    } else {
        printf("{\"status\":\"error\",\"message\":\"Student with Roll No %d not found\"}\n", roll);
    }
}

static void apiDeleteStudent(StudentList* list, UndoStack* stack, int roll) {
    Student deletedData;
    if (deleteStudent(list, roll, &deletedData)) {
        /* Record deletion on Undo Stack */
        UndoAction action;
        action.type = ACTION_DELETE;
        action.previousState = deletedData;
        memset(&action.currentState, 0, sizeof(Student));
        pushUndo(stack, action);
        saveUndoStackToFile(stack);
        saveStudentsToFile(list);

        printf("{\"status\":\"success\",\"message\":\"Student deleted successfully\",\"deleted\":");
        printStudentJson(&deletedData);
        printf("}\n");
    } else {
        printf("{\"status\":\"error\",\"message\":\"Student with Roll No %d not found\"}\n", roll);
    }
}

static void apiSearchRoll(StudentList* list, HashTable* ht, int roll) {
    /* Fast O(1) Hash Table Lookup */
    Student* hashResult = searchHash(ht, roll);

    /* Also verify using Binary Search on sorted array */
    int count = 0;
    Student* arr = listToArray(list, &count);
    int bSearchIndex = -1;
    if (arr) {
        bSearchIndex = binarySearchByRoll(arr, count, roll);
        free(arr);
    }

    if (hashResult) {
        printf("{\"status\":\"success\",\"method\":\"HashTable (O(1)) & BinarySearch\",\"found\":true,\"bSearchIndex\":%d,\"student\":", bSearchIndex);
        printStudentJson(hashResult);
        printf("}\n");
    } else {
        printf("{\"status\":\"success\",\"found\":false,\"message\":\"Student with Roll No %d not found\"}\n", roll);
    }
}

static void apiSearchName(StudentList* list, const char* query) {
    int count = 0;
    Student* arr = listToArray(list, &count);
    if (!arr || count == 0) {
        printf("{\"status\":\"success\",\"count\":0,\"results\":[]}\n");
        return;
    }

    Student* results = (Student*)malloc(sizeof(Student) * count);
    int matchCount = linearSearchByName(arr, count, query, results, count);

    printf("{\"status\":\"success\",\"count\":%d,\"results\":[", matchCount);
    for (int i = 0; i < matchCount; i++) {
        if (i > 0) printf(",");
        printStudentJson(&results[i]);
    }
    printf("]}\n");

    free(results);
    free(arr);
}

static void apiSortCGPA(StudentList* list, bool ascending) {
    int count = 0;
    Student* arr = listToArray(list, &count);
    if (!arr || count == 0) {
        printf("{\"status\":\"success\",\"count\":0,\"students\":[]}\n");
        return;
    }

    sortStudentsByCGPA(arr, count, ascending);

    printf("{\"status\":\"success\",\"sortedBy\":\"cgpa\",\"order\":\"%s\",\"count\":%d,\"students\":[",
           ascending ? "asc" : "desc", count);
    for (int i = 0; i < count; i++) {
        if (i > 0) printf(",");
        printStudentJson(&arr[i]);
    }
    printf("]}\n");

    free(arr);
}

static void apiSortName(StudentList* list, bool ascending) {
    int count = 0;
    Student* arr = listToArray(list, &count);
    if (!arr || count == 0) {
        printf("{\"status\":\"success\",\"count\":0,\"students\":[]}\n");
        return;
    }

    sortStudentsByName(arr, count, ascending);

    printf("{\"status\":\"success\",\"sortedBy\":\"name\",\"order\":\"%s\",\"count\":%d,\"students\":[",
           ascending ? "asc" : "desc", count);
    for (int i = 0; i < count; i++) {
        if (i > 0) printf(",");
        printStudentJson(&arr[i]);
    }
    printf("]}\n");

    free(arr);
}

static void apiUndo(StudentList* list, UndoStack* stack) {
    if (isUndoStackEmpty(stack)) {
        printf("{\"status\":\"error\",\"message\":\"Undo Stack is empty. No previous actions to undo.\"}\n");
        return;
    }

    UndoAction action;
    if (popUndo(stack, &action)) {
        if (action.type == ACTION_UPDATE) {
            /* Restore previous state */
            Student dummy;
            updateStudent(list, action.previousState, &dummy);
            saveStudentsToFile(list);
            saveUndoStackToFile(stack);

            printf("{\"status\":\"success\",\"action\":\"undo_update\",\"restored\":");
            printStudentJson(&action.previousState);
            printf(",\"stackRemaining\":%d}\n", stack->size);
        } else if (action.type == ACTION_DELETE) {
            /* Re-insert deleted student */
            insertStudent(list, action.previousState);
            saveStudentsToFile(list);
            saveUndoStackToFile(stack);

            printf("{\"status\":\"success\",\"action\":\"undo_delete\",\"restored\":");
            printStudentJson(&action.previousState);
            printf(",\"stackRemaining\":%d}\n", stack->size);
        } else {
            printf("{\"status\":\"error\",\"message\":\"Unknown undo action type\"}\n");
        }
    }
}

static void apiEnqueueRequest(StudentList* list, RequestQueue* q, int roll, const char* certType) {
    Student* s = findStudentByRoll(list, roll);
    if (!s) {
        printf("{\"status\":\"error\",\"message\":\"Student with Roll No %d does not exist\"}\n", roll);
        return;
    }

    static int nextReqId = 1001;
    CertRequest req;
    req.reqId = nextReqId++;
    req.rollNo = s->rollNo;
    strncpy(req.studentName, s->name, MAX_NAME_LEN - 1);
    req.studentName[MAX_NAME_LEN - 1] = '\0';
    strncpy(req.certType, certType, MAX_CERT_LEN - 1);
    req.certType[MAX_CERT_LEN - 1] = '\0';
    strncpy(req.status, "Pending", sizeof(req.status) - 1);

    enqueueRequest(q, req);
    saveQueueToFile(q);

    printf("{\"status\":\"success\",\"message\":\"Request added to FIFO Queue\",\"queueSize\":%d,\"request\":{\"reqId\":%d,\"roll\":%d,\"name\":\"%s\",\"type\":\"%s\",\"status\":\"Pending\"}}\n",
           q->count, req.reqId, req.rollNo, req.studentName, req.certType);
}

static void apiDequeueRequest(RequestQueue* q) {
    if (isQueueEmpty(q)) {
        printf("{\"status\":\"error\",\"message\":\"Certificate Request Queue is empty\"}\n");
        return;
    }

    CertRequest req;
    if (dequeueRequest(q, &req)) {
        saveQueueToFile(q);
        printf("{\"status\":\"success\",\"message\":\"Request processed from front of FIFO Queue\",\"remaining\":%d,\"processed\":{\"reqId\":%d,\"roll\":%d,\"name\":\"%s\",\"type\":\"%s\"}}\n",
               q->count, req.reqId, req.rollNo, req.studentName, req.certType);
    }
}

static void apiListRequests(RequestQueue* q) {
    printf("{\"status\":\"success\",\"count\":%d,\"requests\":[", q->count);
    QueueNode* curr = q->front;
    bool first = true;
    while (curr != NULL) {
        if (!first) printf(",");
        printf("{\"reqId\":%d,\"roll\":%d,\"name\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}",
               curr->req.reqId, curr->req.rollNo, curr->req.studentName, curr->req.certType, curr->req.status);
        first = false;
        curr = curr->next;
    }
    printf("]}\n");
}

/* Recursive helper to print BST in JSON */
static void printBSTNodeJson(BSTNode* root, bool* first) {
    if (root == NULL) return;

    /* In-order traversal: Left, Root, Right */
    printBSTNodeJson(root->left, first);

    if (!(*first)) printf(",");
    printf("{\"dept\":\"%s\",\"studentCount\":%d,\"students\":[", root->department, root->studentCount);

    BSTStudentNode* sNode = root->studentsHead;
    bool firstStudent = true;
    while (sNode != NULL) {
        if (!firstStudent) printf(",");
        printStudentJson(&sNode->data);
        firstStudent = false;
        sNode = sNode->next;
    }
    printf("]}");
    *first = false;

    printBSTNodeJson(root->right, first);
}

static void apiDepartmentBST(StudentList* list) {
    BSTNode* root = buildDepartmentBST(list);
    printf("{\"status\":\"success\",\"departments\":[");
    bool first = true;
    printBSTNodeJson(root, &first);
    printf("]}\n");
    freeBST(root);
}

static void apiPrerequisitesGraph(void) {
    CourseGraph* g = createCourseGraph();
    int order[MAX_COURSES];
    int orderSize = 0;
    bool hasCycle = false;

    getTopologicalSort(g, order, &orderSize, &hasCycle);

    printf("{\"status\":\"success\",\"hasCycle\":%s,\"nodes\":[", hasCycle ? "true" : "false");
    for (int i = 0; i < g->numVertices; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"code\":\"%s\",\"title\":\"%s\",\"credits\":%d,\"semester\":%d}",
               g->courses[i].id, g->courses[i].code, g->courses[i].title, g->courses[i].credits, g->courses[i].semester);
    }
    printf("],\"edges\":[");

    bool firstEdge = true;
    for (int u = 0; u < g->numVertices; u++) {
        AdjNode* curr = g->adjLists[u];
        while (curr != NULL) {
            if (!firstEdge) printf(",");
            printf("{\"source\":%d,\"target\":%d}", u, curr->destCourseId);
            firstEdge = false;
            curr = curr->next;
        }
    }
    printf("],\"topologicalOrder\":[");
    for (int i = 0; i < orderSize; i++) {
        if (i > 0) printf(",");
        printf("%d", order[i]);
    }
    printf("]}\n");

    freeCourseGraph(g);
}

/* ========================================================================= */
/* MENU-DRIVEN VIVA / CLI INTERFACE (For College Faculty Demonstration)       */
/* ========================================================================= */

static void runCliMenu(StudentList* list, UndoStack* stack, RequestQueue* queue, HashTable* ht) {
    int choice = 0;
    while (1) {
        printf("\n");
        printf("====================================================================\n");
        printf("         STUDENT RECORD MANAGEMENT SYSTEM (DSA CAPSTONE IN C)       \n");
        printf("====================================================================\n");
        printf(" [CORE CRUD & LINKED LIST]       [DSA STACK & QUEUE]                \n");
        printf("  1. Add Student Record           9. Undo Last Update (Stack)       \n");
        printf("  2. Update Student Record       10. Enqueue Certificate Request    \n");
        printf("  3. Delete Student Record       11. Process Request (Queue FIFO)   \n");
        printf("  4. Display All Records         12. View Pending Requests Queue    \n");
        printf("                                                                    \n");
        printf(" [SEARCHING & SORTING]           [ADVANCED TREES & GRAPHS]          \n");
        printf("  5. Search by Roll (Hash & BS)  13. Department View (BST In-Order) \n");
        printf("  6. Search by Name (Linear)     14. Course Prerequisites (Graph)   \n");
        printf("  7. Sort by CGPA (Merge Sort)                                      \n");
        printf("  8. Sort by Name (Merge Sort)   15. Save & Exit Program            \n");
        printf("====================================================================\n");
        printf(" Enter choice (1-15): ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Exiting.\n");
            break;
        }

        if (choice == 1) {
            Student s;
            printf("\n--- Add Student ---\n");
            printf("Roll Number: "); scanf("%d", &s.rollNo);
            printf("Name: "); scanf(" %[^\n]", s.name);
            printf("Department: "); scanf(" %[^\n]", s.department);
            printf("CGPA: "); scanf("%f", &s.cgpa);
            printf("Email: "); scanf(" %[^\n]", s.email);
            printf("Phone: "); scanf(" %[^\n]", s.phone);

            if (insertStudent(list, s)) {
                insertHash(ht, s);
                saveStudentsToFile(list);
                printf(">> Success: Student [%d] %s added to Linked List & Hash Table!\n", s.rollNo, s.name);
            } else {
                printf(">> Error: Student with Roll No %d already exists!\n", s.rollNo);
            }
        } else if (choice == 2) {
            int roll;
            printf("\nEnter Roll Number to update: "); scanf("%d", &roll);
            Student* existing = findStudentByRoll(list, roll);
            if (!existing) {
                printf(">> Student not found!\n");
            } else {
                Student updated = *existing;
                printf("New Name (current: %s): ", existing->name); scanf(" %[^\n]", updated.name);
                printf("New Department (current: %s): ", existing->department); scanf(" %[^\n]", updated.department);
                printf("New CGPA (current: %.2f): ", existing->cgpa); scanf("%f", &updated.cgpa);
                printf("New Email (current: %s): ", existing->email); scanf(" %[^\n]", updated.email);
                printf("New Phone (current: %s): ", existing->phone); scanf(" %[^\n]", updated.phone);

                Student oldState;
                if (updateStudent(list, updated, &oldState)) {
                    UndoAction act = { ACTION_UPDATE, oldState, updated };
                    pushUndo(stack, act);
                    saveUndoStackToFile(stack);
                    saveStudentsToFile(list);
                    rebuildHashTable(ht, list);
                    printf(">> Success: Record updated! Previous state pushed to Undo Stack (Depth: %d).\n", stack->size);
                }
            }
        } else if (choice == 3) {
            int roll;
            printf("\nEnter Roll Number to delete: "); scanf("%d", &roll);
            Student deleted;
            if (deleteStudent(list, roll, &deleted)) {
                UndoAction act = { ACTION_DELETE, deleted, deleted };
                pushUndo(stack, act);
                saveUndoStackToFile(stack);
                deleteHash(ht, roll);
                saveStudentsToFile(list);
                printf(">> Success: Student deleted and pushed to Undo Stack!\n");
            } else {
                printf(">> Student with Roll No %d not found!\n", roll);
            }
        } else if (choice == 4) {
            printf("\n--- All Student Records (Singly Linked List) [Count: %d] ---\n", list->count);
            StudentNode* curr = list->head;
            printf("%-8s %-22s %-10s %-6s %-28s %-15s\n", "ROLL", "NAME", "DEPT", "CGPA", "EMAIL", "PHONE");
            printf("----------------------------------------------------------------------------------------------\n");
            while (curr != NULL) {
                printf("%-8d %-22s %-10s %-6.2f %-28s %-15s\n",
                       curr->data.rollNo, curr->data.name, curr->data.department,
                       curr->data.cgpa, curr->data.email, curr->data.phone);
                curr = curr->next;
            }
        } else if (choice == 5) {
            int roll;
            printf("\nEnter Roll Number to search: "); scanf("%d", &roll);
            Student* hRes = searchHash(ht, roll);
            if (hRes) {
                printf(">> [HASH TABLE O(1) HIT]: Found %s (Dept: %s, CGPA: %.2f)\n", hRes->name, hRes->department, hRes->cgpa);
            } else {
                printf(">> Not found in Hash Table.\n");
            }
        } else if (choice == 6) {
            char query[MAX_NAME_LEN];
            printf("\nEnter name/substring to search: "); scanf(" %[^\n]", query);
            int count = 0;
            Student* arr = listToArray(list, &count);
            if (arr) {
                Student results[100];
                int hits = linearSearchByName(arr, count, query, results, 100);
                printf(">> Linear Search found %d matches:\n", hits);
                for (int i = 0; i < hits; i++) {
                    printf("   [%d] %s (%s, CGPA: %.2f)\n", results[i].rollNo, results[i].name, results[i].department, results[i].cgpa);
                }
                free(arr);
            }
        } else if (choice == 7) {
            int count = 0;
            Student* arr = listToArray(list, &count);
            if (arr) {
                sortStudentsByCGPA(arr, count, false); // Descending (Merit List)
                printf("\n--- Students Sorted by CGPA (Merge Sort - Highest to Lowest) ---\n");
                for (int i = 0; i < count; i++) {
                    printf("Rank %d: [%.2f CGPA] Roll %d - %s (%s)\n", i + 1, arr[i].cgpa, arr[i].rollNo, arr[i].name, arr[i].department);
                }
                free(arr);
            }
        } else if (choice == 8) {
            int count = 0;
            Student* arr = listToArray(list, &count);
            if (arr) {
                sortStudentsByName(arr, count, true);
                printf("\n--- Students Sorted by Name Alphabetically (Merge Sort) ---\n");
                for (int i = 0; i < count; i++) {
                    printf("%d. %s (Roll: %d, Dept: %s, CGPA: %.2f)\n", i + 1, arr[i].name, arr[i].rollNo, arr[i].department, arr[i].cgpa);
                }
                free(arr);
            }
        } else if (choice == 9) {
            if (isUndoStackEmpty(stack)) {
                printf(">> Undo Stack is empty!\n");
            } else {
                UndoAction act;
                popUndo(stack, &act);
                if (act.type == ACTION_UPDATE) {
                    Student dummy;
                    updateStudent(list, act.previousState, &dummy);
                    saveStudentsToFile(list);
                    saveUndoStackToFile(stack);
                    rebuildHashTable(ht, list);
                    printf(">> Restored previous state for student Roll No %d via Stack POP!\n", act.previousState.rollNo);
                } else if (act.type == ACTION_DELETE) {
                    insertStudent(list, act.previousState);
                    saveStudentsToFile(list);
                    saveUndoStackToFile(stack);
                    rebuildHashTable(ht, list);
                    printf(">> Restored deleted student Roll No %d via Stack POP!\n", act.previousState.rollNo);
                }
            }
        } else if (choice == 10) {
            int roll;
            char type[MAX_CERT_LEN];
            printf("Enter Student Roll No: "); scanf("%d", &roll);
            printf("Enter Request Type (Bonafide/ID Card/Transcript): "); scanf(" %[^\n]", type);
            Student* s = findStudentByRoll(list, roll);
            if (!s) {
                printf(">> Student not found!\n");
            } else {
                CertRequest req = { rand() % 9000 + 1000, roll, "", "", "Pending" };
                strncpy(req.studentName, s->name, sizeof(req.studentName) - 1);
                strncpy(req.certType, type, sizeof(req.certType) - 1);
                enqueueRequest(queue, req);
                saveQueueToFile(queue);
                printf(">> Request #%d enqueued to Queue (Total pending: %d)\n", req.reqId, queue->count);
            }
        } else if (choice == 11) {
            if (isQueueEmpty(queue)) {
                printf(">> Queue is empty!\n");
            } else {
                CertRequest req;
                dequeueRequest(queue, &req);
                saveQueueToFile(queue);
                printf(">> Processed Request #%d: %s for %s (Roll %d) [FIFO Front Dequeued]\n",
                       req.reqId, req.certType, req.studentName, req.rollNo);
            }
        } else if (choice == 12) {
            printf("\n--- Pending Certificate Requests (FIFO Queue) [Count: %d] ---\n", queue->count);
            QueueNode* qCurr = queue->front;
            while (qCurr != NULL) {
                printf("Ticket #%d: %s | Student: %s (Roll %d)\n",
                       qCurr->req.reqId, qCurr->req.certType, qCurr->req.studentName, qCurr->req.rollNo);
                qCurr = qCurr->next;
            }
        } else if (choice == 13) {
            BSTNode* root = buildDepartmentBST(list);
            printf("\n--- Department Hierarchy (Binary Search Tree In-Order) ---\n");
            bool first = true;
            printBSTNodeJson(root, &first);
            printf("\n");
            freeBST(root);
        } else if (choice == 14) {
            printf("\n--- Course Prerequisites Graph (Topological Sort) ---\n");
            apiPrerequisitesGraph();
        } else if (choice == 15) {
            saveStudentsToFile(list);
            printf(">> Records saved. Goodbye!\n");
            break;
        } else {
            printf(">> Invalid choice. Please select 1-15.\n");
        }
    }
}

/* ========================================================================= */
/* MAIN ENTRY POINT                                                          */
/* ========================================================================= */

int main(int argc, char* argv[]) {
    /* Initialize Core Data Structures */
    StudentList* list = createStudentList();
    UndoStack* stack = createUndoStack();
    RequestQueue* queue = createRequestQueue();
    HashTable* ht = createHashTable();

    /* Load persistent data from storage files */
    loadStudentsFromFile(list);
    loadUndoStackFromFile(stack);
    loadQueueFromFile(queue);
    rebuildHashTable(ht, list);

    /* Check if executed in JSON API mode: `--api <command> [args...]` */
    if (argc >= 2 && strcmp(argv[1], "--api") == 0) {
        if (argc < 3) {
            printf("{\"status\":\"error\",\"message\":\"Missing API command\"}\n");
        } else {
            const char* cmd = argv[2];
            if (strcmp(cmd, "list") == 0) {
                apiListStudents(list);
            } else if (strcmp(cmd, "add") == 0 && argc >= 9) {
                apiAddStudent(list, atoi(argv[3]), argv[4], argv[5], (float)atof(argv[6]), argv[7], argv[8]);
            } else if (strcmp(cmd, "update") == 0 && argc >= 9) {
                apiUpdateStudent(list, stack, atoi(argv[3]), argv[4], argv[5], (float)atof(argv[6]), argv[7], argv[8]);
            } else if (strcmp(cmd, "delete") == 0 && argc >= 4) {
                apiDeleteStudent(list, stack, atoi(argv[3]));
            } else if (strcmp(cmd, "search_roll") == 0 && argc >= 4) {
                apiSearchRoll(list, ht, atoi(argv[3]));
            } else if (strcmp(cmd, "search_name") == 0 && argc >= 4) {
                apiSearchName(list, argv[3]);
            } else if (strcmp(cmd, "sort_cgpa") == 0) {
                bool asc = (argc >= 4 && strcmp(argv[3], "asc") == 0);
                apiSortCGPA(list, asc);
            } else if (strcmp(cmd, "sort_name") == 0) {
                bool asc = (argc >= 4 && strcmp(argv[3], "asc") == 0);
                apiSortName(list, asc);
            } else if (strcmp(cmd, "undo") == 0) {
                apiUndo(list, stack);
            } else if (strcmp(cmd, "req_enqueue") == 0 && argc >= 5) {
                apiEnqueueRequest(list, queue, atoi(argv[3]), argv[4]);
            } else if (strcmp(cmd, "req_dequeue") == 0) {
                apiDequeueRequest(queue);
            } else if (strcmp(cmd, "req_list") == 0) {
                apiListRequests(queue);
            } else if (strcmp(cmd, "departments_bst") == 0) {
                apiDepartmentBST(list);
            } else if (strcmp(cmd, "prerequisites_graph") == 0) {
                apiPrerequisitesGraph();
            } else {
                printf("{\"status\":\"error\",\"message\":\"Unknown or invalid arguments for command: %s\"}\n", cmd);
            }
        }
    } else {
        /* Run Interactive Console Menu Mode */
        runCliMenu(list, stack, queue, ht);
    }

    /* Clean up all dynamically allocated memory */
    freeStudentList(list);
    freeUndoStack(stack);
    freeRequestQueue(queue);
    freeHashTable(ht);

    return 0;
}
