#include "dsa.h"

/* Create an empty FIFO Queue for certificate / ID-card requests */
RequestQueue* createRequestQueue(void) {
    RequestQueue* q = (RequestQueue*)malloc(sizeof(RequestQueue));
    if (!q) {
        fprintf(stderr, "Error: Memory allocation failed for RequestQueue.\n");
        return NULL;
    }
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    return q;
}

/* Enqueue a new request at the rear of the Queue (FIFO) */
void enqueueRequest(RequestQueue* q, CertRequest req) {
    if (!q) return;

    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for QueueNode.\n");
        return;
    }
    newNode->req = req;
    newNode->next = NULL;

    /* If queue is empty, both front and rear point to new node */
    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->count++;
}

/* Dequeue the request at the front of the Queue (First-Come, First-Served) */
bool dequeueRequest(RequestQueue* q, CertRequest* req) {
    if (!q || q->front == NULL) {
        return false;
    }

    QueueNode* temp = q->front;
    if (req) {
        *req = temp->req;
    }

    q->front = q->front->next;

    /* If front becomes NULL, rear must also become NULL */
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    q->count--;
    return true;
}

/* Check if Queue is empty */
bool isQueueEmpty(RequestQueue* q) {
    return (!q || q->front == NULL);
}

/* Free all nodes in the Queue */
void freeRequestQueue(RequestQueue* q) {
    if (!q) return;
    QueueNode* current = q->front;
    while (current != NULL) {
        QueueNode* next = current->next;
        free(current);
        current = next;
    }
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    free(q);
}

/* Save queue to binary file */
void saveQueueToFile(RequestQueue* q) {
    if (!q) return;
    FILE* fp = fopen(QUEUE_FILE, "wb");
    if (!fp) return;

    fwrite(&q->count, sizeof(int), 1, fp);
    QueueNode* curr = q->front;
    while (curr != NULL) {
        fwrite(&curr->req, sizeof(CertRequest), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

/* Load queue from binary file */
void loadQueueFromFile(RequestQueue* q) {
    if (!q) return;
    FILE* fp = fopen(QUEUE_FILE, "rb");
    if (!fp) return;

    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count <= 0) {
        fclose(fp);
        return;
    }

    CertRequest req;
    for (int i = 0; i < count; i++) {
        if (fread(&req, sizeof(CertRequest), 1, fp) == 1) {
            enqueueRequest(q, req);
        }
    }
    fclose(fp);
}
