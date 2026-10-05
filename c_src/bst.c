#include "dsa.h"

/* Create a new BST node representing a department */
BSTNode* createBSTNode(const char* dept) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for BSTNode.\n");
        return NULL;
    }
    strncpy(node->department, dept, MAX_DEPT_LEN - 1);
    node->department[MAX_DEPT_LEN - 1] = '\0';
    node->studentsHead = NULL;
    node->studentCount = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Insert student into the department's student sub-list */
static void addStudentToDeptList(BSTNode* node, Student s) {
    BSTStudentNode* newNode = (BSTStudentNode*)malloc(sizeof(BSTStudentNode));
    if (!newNode) return;
    newNode->data = s;
    newNode->next = node->studentsHead;
    node->studentsHead = newNode;
    node->studentCount++;
}

/* Insert a student into the BST keyed by department name */
BSTNode* insertBST(BSTNode* root, Student s) {
    if (root == NULL) {
        BSTNode* newNode = createBSTNode(s.department);
        addStudentToDeptList(newNode, s);
        return newNode;
    }

    int cmp = strcmp(s.department, root->department);
    if (cmp == 0) {
        /* Department node exists: attach student to this department's list */
        addStudentToDeptList(root, s);
    } else if (cmp < 0) {
        root->left = insertBST(root->left, s);
    } else {
        root->right = insertBST(root->right, s);
    }
    return root;
}

/* Free student sub-list within a department node */
static void freeDeptStudents(BSTStudentNode* head) {
    while (head != NULL) {
        BSTStudentNode* next = head->next;
        free(head);
        head = next;
    }
}

/* Free the entire BST post-order */
void freeBST(BSTNode* root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    freeDeptStudents(root->studentsHead);
    free(root);
}

/* Build a department-wise BST from the primary student linked list */
BSTNode* buildDepartmentBST(StudentList* list) {
    if (!list) return NULL;
    BSTNode* root = NULL;
    StudentNode* curr = list->head;
    while (curr != NULL) {
        root = insertBST(root, curr->data);
        curr = curr->next;
    }
    return root;
}
