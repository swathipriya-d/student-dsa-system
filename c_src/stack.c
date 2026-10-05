#include "dsa.h"

/* Create an empty Undo Stack */
UndoStack* createUndoStack(void) {
    UndoStack* stack = (UndoStack*)malloc(sizeof(UndoStack));
    if (!stack) {
        fprintf(stderr, "Error: Memory allocation failed for UndoStack.\n");
        return NULL;
    }
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

/* Push an undo action onto the Stack (LIFO) */
void pushUndo(UndoStack* stack, UndoAction action) {
    if (!stack) return;

    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for StackNode.\n");
        return;
    }
    newNode->action = action;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
}

/* Pop the top undo action from the Stack */
bool popUndo(UndoStack* stack, UndoAction* action) {
    if (!stack || stack->top == NULL) {
        return false;
    }

    StackNode* temp = stack->top;
    if (action) {
        *action = temp->action;
    }
    stack->top = temp->next;
    free(temp);
    stack->size--;
    return true;
}

/* Inspect the top undo action without removing it */
bool peekUndo(UndoStack* stack, UndoAction* action) {
    if (!stack || stack->top == NULL) {
        return false;
    }
    if (action) {
        *action = stack->top->action;
    }
    return true;
}

/* Check if the stack is empty */
bool isUndoStackEmpty(UndoStack* stack) {
    return (!stack || stack->top == NULL);
}

/* Free all nodes in the stack */
void freeUndoStack(UndoStack* stack) {
    if (!stack) return;
    StackNode* current = stack->top;
    while (current != NULL) {
        StackNode* next = current->next;
        free(current);
        current = next;
    }
    stack->top = NULL;
    stack->size = 0;
    free(stack);
}

/* Save undo stack to binary file */
void saveUndoStackToFile(UndoStack* stack) {
    if (!stack) return;
    FILE* fp = fopen(STACK_FILE, "wb");
    if (!fp) return;

    /* Write size */
    fwrite(&stack->size, sizeof(int), 1, fp);

    /* Write all actions from top to bottom */
    StackNode* curr = stack->top;
    while (curr != NULL) {
        fwrite(&curr->action, sizeof(UndoAction), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

/* Load undo stack from binary file */
void loadUndoStackFromFile(UndoStack* stack) {
    if (!stack) return;
    FILE* fp = fopen(STACK_FILE, "rb");
    if (!fp) return;

    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count <= 0) {
        fclose(fp);
        return;
    }

    /* Read into temporary buffer to preserve correct LIFO ordering */
    UndoAction* buffer = (UndoAction*)malloc(sizeof(UndoAction) * count);
    if (!buffer) {
        fclose(fp);
        return;
    }

    int readCount = fread(buffer, sizeof(UndoAction), count, fp);
    fclose(fp);

    /* Push in reverse order so original top remains on top */
    for (int i = readCount - 1; i >= 0; i--) {
        pushUndo(stack, buffer[i]);
    }
    free(buffer);
}
