#include "dsa.h"

/* Initialize the AI & DS curriculum course graph */
CourseGraph* createCourseGraph(void) {
    CourseGraph* g = (CourseGraph*)malloc(sizeof(CourseGraph));
    if (!g) {
        fprintf(stderr, "Error: Memory allocation failed for CourseGraph.\n");
        return NULL;
    }
    g->numVertices = MAX_COURSES;

    /* Define real B.Tech AI & DS courses */
    Course curriculum[MAX_COURSES] = {
        {0, "CS101", "Programming in C", 4, 1},
        {1, "MA102", "Discrete Mathematics", 4, 1},
        {2, "CS201", "Data Structures", 4, 2},
        {3, "CS202", "Object Oriented Programming", 3, 2},
        {4, "AI201", "Design & Analysis of Algorithms", 4, 3},
        {5, "AI202", "Database Management Systems", 3, 3},
        {6, "AI301", "Machine Learning Foundations", 4, 4},
        {7, "AI401", "Deep Learning & Capstone Project", 4, 5}
    };

    for (int i = 0; i < MAX_COURSES; i++) {
        g->courses[i] = curriculum[i];
        g->adjLists[i] = NULL;
        g->inDegree[i] = 0;
    }

    /* Add prerequisite directed edges:
       src -> dest means "src is a prerequisite for dest" */
    addPrerequisite(g, 0, 2); // C Programming -> Data Structures
    addPrerequisite(g, 0, 3); // C Programming -> OOP
    addPrerequisite(g, 1, 4); // Discrete Math -> Algorithms
    addPrerequisite(g, 2, 4); // Data Structures -> Algorithms
    addPrerequisite(g, 2, 5); // Data Structures -> DBMS
    addPrerequisite(g, 4, 6); // Algorithms -> Machine Learning
    addPrerequisite(g, 5, 6); // DBMS -> Machine Learning
    addPrerequisite(g, 6, 7); // Machine Learning -> Deep Learning & Capstone

    return g;
}

/* Add a directed prerequisite edge: src is required before dest */
void addPrerequisite(CourseGraph* g, int srcCourseId, int destCourseId) {
    if (!g || srcCourseId < 0 || srcCourseId >= g->numVertices ||
        destCourseId < 0 || destCourseId >= g->numVertices) {
        return;
    }

    AdjNode* newNode = (AdjNode*)malloc(sizeof(AdjNode));
    if (!newNode) return;
    newNode->destCourseId = destCourseId;
    newNode->next = g->adjLists[srcCourseId];
    g->adjLists[srcCourseId] = newNode;

    /* Increment in-degree of destination node */
    g->inDegree[destCourseId]++;
}

/* Topological Sort using Kahn's Algorithm (BFS with In-degree Queue) */
void getTopologicalSort(CourseGraph* g, int* order, int* orderSize, bool* hasCycle) {
    if (!g || !order || !orderSize || !hasCycle) return;

    int inDegreeCopy[MAX_COURSES];
    for (int i = 0; i < g->numVertices; i++) {
        inDegreeCopy[i] = g->inDegree[i];
    }

    /* Queue to store vertices with in-degree 0 */
    int queue[MAX_COURSES];
    int front = 0, rear = 0;

    for (int i = 0; i < g->numVertices; i++) {
        if (inDegreeCopy[i] == 0) {
            queue[rear++] = i;
        }
    }

    int count = 0;
    while (front < rear) {
        int u = queue[front++];
        order[count++] = u;

        /* Reduce in-degree of all adjacent vertices */
        AdjNode* curr = g->adjLists[u];
        while (curr != NULL) {
            int v = curr->destCourseId;
            inDegreeCopy[v]--;
            if (inDegreeCopy[v] == 0) {
                queue[rear++] = v;
            }
            curr = curr->next;
        }
    }

    *orderSize = count;
    *hasCycle = (count != g->numVertices);
}

/* Free graph nodes */
void freeCourseGraph(CourseGraph* g) {
    if (!g) return;
    for (int i = 0; i < g->numVertices; i++) {
        AdjNode* curr = g->adjLists[i];
        while (curr != NULL) {
            AdjNode* next = curr->next;
            free(curr);
            curr = next;
        }
        g->adjLists[i] = NULL;
    }
    free(g);
}
