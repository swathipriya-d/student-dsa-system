#include "dsa.h"

/* Escape string characters for safe JSON output */
void escapeJsonString(const char* input, char* output, size_t maxLen) {
    if (!input || !output || maxLen == 0) return;
    size_t o = 0;
    for (size_t i = 0; input[i] && o + 2 < maxLen; i++) {
        unsigned char c = (unsigned char)input[i];
        if (c == '"') {
            output[o++] = '\\';
            output[o++] = '"';
        } else if (c == '\\') {
            output[o++] = '\\';
            output[o++] = '\\';
        } else if (c == '\n') {
            output[o++] = '\\';
            output[o++] = 'n';
        } else if (c == '\r') {
            output[o++] = '\\';
            output[o++] = 'r';
        } else if (c == '\t') {
            output[o++] = '\\';
            output[o++] = 't';
        } else if (c >= 32 && c < 127) {
            output[o++] = c;
        }
    }
    output[o] = '\0';
}

/* Print a single student record as a JSON object */
void printStudentJson(const Student* s) {
    if (!s) {
        printf("null");
        return;
    }
    char safeName[MAX_NAME_LEN * 2];
    char safeDept[MAX_DEPT_LEN * 2];
    char safeEmail[MAX_EMAIL_LEN * 2];
    char safePhone[MAX_PHONE_LEN * 2];

    escapeJsonString(s->name, safeName, sizeof(safeName));
    escapeJsonString(s->department, safeDept, sizeof(safeDept));
    escapeJsonString(s->email, safeEmail, sizeof(safeEmail));
    escapeJsonString(s->phone, safePhone, sizeof(safePhone));

    printf("{\"roll\":%d,\"name\":\"%s\",\"dept\":\"%s\",\"cgpa\":%.2f,\"email\":\"%s\",\"phone\":\"%s\"}",
           s->rollNo, safeName, safeDept, s->cgpa, safeEmail, safePhone);
}

/* Save student linked list to binary file */
bool saveStudentsToFile(StudentList* list) {
    if (!list) return false;

    FILE* fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        /* If directory doesn't exist or permission denied */
        return false;
    }

    /* Write total record count first */
    fwrite(&list->count, sizeof(int), 1, fp);

    /* Write each student record consecutively */
    StudentNode* curr = list->head;
    while (curr != NULL) {
        fwrite(&curr->data, sizeof(Student), 1, fp);
        curr = curr->next;
    }

    fclose(fp);
    return true;
}

/* Load student linked list from binary file */
bool loadStudentsFromFile(StudentList* list) {
    if (!list) return false;

    FILE* fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        /* File does not exist yet; populate initial seed records for college demo */
        Student sampleStudents[] = {
            {101, "Aarav Sharma", "AI & DS", 9.42f, "aarav.sharma@college.edu", "+91 9876543210"},
            {102, "Diya Patel", "AI & DS", 8.85f, "diya.patel@college.edu", "+91 9876543211"},
            {103, "Rohan Verma", "CSE", 7.95f, "rohan.verma@college.edu", "+91 9876543212"},
            {104, "Ananya Iyer", "AI & DS", 9.78f, "ananya.iyer@college.edu", "+91 9876543213"},
            {105, "Vikram Singh", "ECE", 8.20f, "vikram.singh@college.edu", "+91 9876543214"},
            {106, "Pooja Hegde", "MECH", 7.45f, "pooja.hegde@college.edu", "+91 9876543215"},
            {107, "Karthik Nair", "CSE", 8.60f, "karthik.nair@college.edu", "+91 9876543216"}
        };
        int sampleCount = sizeof(sampleStudents) / sizeof(sampleStudents[0]);
        for (int i = 0; i < sampleCount; i++) {
            insertStudent(list, sampleStudents[i]);
        }
        saveStudentsToFile(list);
        return true;
    }

    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count <= 0) {
        fclose(fp);
        return true;
    }

    Student s;
    for (int i = 0; i < count; i++) {
        if (fread(&s, sizeof(Student), 1, fp) == 1) {
            insertStudent(list, s);
        }
    }

    fclose(fp);
    return true;
}
