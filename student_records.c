#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define MY_STUDENT_ID   100234
#define ANALYSIS_NAME   "Grade Stability Score"
#define NAME_LEN        50
#define NUM_TESTS       5      
#define INITIAL_CAPACITY 4


typedef struct {
    int   id;
    char  name[NAME_LEN];
    float grades[NUM_TESTS];
} Student;


float calculate_average(const Student *s) {
    float sum = 0.0f;
    const float *g = s->grades;              
    for (int i = 0; i < NUM_TESTS; i++, g++)
        sum += *g;
    return sum / NUM_TESTS;
}

float grade_stability_score(const Student *s) {
    float avg = calculate_average(s);
    float sum_sq_diff = 0.0f;

    const float *g = s->grades;               
    for (int i = 0; i < NUM_TESTS; i++, g++) {
        float diff = *g - avg;
        sum_sq_diff += diff * diff;
    }
    float variance = sum_sq_diff / NUM_TESTS;
    float stddev = sqrtf(variance);

    float score = 100.0f - stddev;
    if (score < 0.0f)   score = 0.0f;
    if (score > 100.0f) score = 100.0f;
    return score;
}


const char* stability_label(float score) {
    if (score >= 90.0f) return "Very Stable";
    if (score >= 75.0f) return "Stable";
    if (score >= 55.0f) return "Moderate";
    return "Volatile";
}


void add_student(Student **arr, int *count, int *capacity,
                  int id, const char *name, const float grades[NUM_TESTS]) {
    if (*count >= *capacity) {
        int new_capacity = (*capacity == 0) ? INITIAL_CAPACITY : (*capacity * 2);
        Student *resized = (Student *)realloc(*arr, new_capacity * sizeof(Student));
        if (resized == NULL) {
            printf("Memory allocation failed! Could not add student.\n");
            return;
        }
        *arr = resized;
        *capacity = new_capacity;
    }

    Student *slot = &(*arr)[*count];          
    slot->id = id;
    strncpy(slot->name, name, NAME_LEN - 1);
    slot->name[NAME_LEN - 1] = '\0';
    for (int i = 0; i < NUM_TESTS; i++)
        slot->grades[i] = grades[i];

    (*count)++;
}


void print_header(void) {
    printf("\n============================================================\n");
    printf(" Student ID: %d   Analysis Function: %s\n", MY_STUDENT_ID, ANALYSIS_NAME);
    printf("============================================================\n");
}

void display_students(const Student *arr, int count) {
    print_header();
    printf("%-8s %-20s %-10s %-14s\n", "ID", "Name", "Average", "Test Grades");
    printf("------------------------------------------------------------\n");

    const Student *p = arr;                    /* pointer traversal */
    for (int i = 0; i < count; i++, p++) {
        printf("%-8d %-20s %-10.2f [", p->id, p->name, calculate_average(p));
        for (int j = 0; j < NUM_TESTS; j++) {
            printf("%.1f", p->grades[j]);
            if (j < NUM_TESTS - 1) printf(", ");
        }
        printf("]\n");
    }
    printf("------------------------------------------------------------\n");
    printf("Total students: %d\n", count);
}

float calculate_class_average(const Student *arr, int count) {
    if (count == 0) return 0.0f;
    float total = 0.0f;
    const Student *p = arr;
    for (int i = 0; i < count; i++, p++)
        total += calculate_average(p);
    return total / count;
}


const Student* find_top_student(const Student *arr, int count) {
    if (count == 0) return NULL;
    const Student *best = arr;
    const Student *p = arr;
    for (int i = 0; i < count; i++, p++) {
        if (calculate_average(p) > calculate_average(best))
            best = p;
    }
    return best;
}

const Student* search_by_name(const Student *arr, int count, const char *query) {
    const Student *p = arr;
    for (int i = 0; i < count; i++, p++) {
        if (strcmp(p->name, query) == 0)       /* string comparison */
            return p;
    }
    return NULL;
}

int cmp_by_average_desc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    float avgA = calculate_average(sa);
    float avgB = calculate_average(sb);
    if (avgA < avgB) return 1;
    if (avgA > avgB) return -1;
    return 0;
}

int cmp_by_name_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return strcmp(sa->name, sb->name);          
}

int cmp_by_id_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return sa->id - sb->id;
}

void sort_students(Student *arr, int count, int (*cmp)(const void *, const void *)) {
    qsort(arr, count, sizeof(Student), cmp);
}

void run_custom_analysis(Student *arr, int count) {
    print_header();
    printf("%-8s %-20s %-16s %-12s\n", "ID", "Name", "Stability Score", "Category");
    printf("------------------------------------------------------------\n");

    Student *temp = (Student *)malloc(count * sizeof(Student));
    if (!temp) { printf("Memory allocation failed.\n"); return; }
    memcpy(temp, arr, count * sizeof(Student));

    for (int i = 1; i < count; i++) {
        Student key = temp[i];
        float key_score = grade_stability_score(&key);
        int j = i - 1;
        while (j >= 0 && grade_stability_score(&temp[j]) < key_score) {
            temp[j + 1] = temp[j];
            j--;
        }
        temp[j + 1] = key;
    }

    Student *p = temp;
    for (int i = 0; i < count; i++, p++) {
        float score = grade_stability_score(p);
        printf("%-8d %-20s %-16.2f %-12s\n", p->id, p->name, score, stability_label(score));
    }
    printf("------------------------------------------------------------\n");
    free(temp);
}

typedef void (*MenuHandler)(Student **, int *, int *);

void handle_add(Student **arr, int *count, int *capacity) {
    int id;
    char name[NAME_LEN];
    float grades[NUM_TESTS];

    printf("Enter student ID: ");
    scanf("%d", &id);
    printf("Enter student name: ");
    scanf(" %49[^\n]", name);
    printf("Enter %d test grades (space-separated): ", NUM_TESTS);
    for (int i = 0; i < NUM_TESTS; i++)
        scanf("%f", &grades[i]);

    add_student(arr, count, capacity, id, name, grades);
    printf("Student added successfully.\n");
}

void handle_display(Student **arr, int *count, int *capacity) {
    (void)capacity;
    display_students(*arr, *count);
}

void handle_sort(Student **arr, int *count, int *capacity) {
    (void)capacity;
    printf("Sort by: 1) Average Grade (desc)  2) Name (A-Z)  3) ID (asc)\n> ");
    int choice;
    scanf("%d", &choice);

    switch (choice) {
        case 1: sort_students(*arr, *count, cmp_by_average_desc); break;
        case 2: sort_students(*arr, *count, cmp_by_name_asc);     break;
        case 3: sort_students(*arr, *count, cmp_by_id_asc);       break;
        default: printf("Invalid choice.\n"); return;
    }
    printf("Students sorted.\n");
    display_students(*arr, *count);
}

void handle_average(Student **arr, int *count, int *capacity) {
    (void)capacity;
    print_header();
    printf("Class average grade: %.2f\n", calculate_class_average(*arr, *count));
}

void handle_top(Student **arr, int *count, int *capacity) {
    (void)capacity;
    print_header();
    const Student *top = find_top_student(*arr, *count);
    if (top)
        printf("Top student: %s (ID: %d) with average %.2f\n",
               top->name, top->id, calculate_average(top));
    else
        printf("No students available.\n");
}

void handle_custom_analysis(Student **arr, int *count, int *capacity) {
    (void)capacity;
    run_custom_analysis(*arr, *count);
}

void handle_search(Student **arr, int *count, int *capacity) {
    (void)capacity;
    char query[NAME_LEN];
    printf("Enter name to search: ");
    scanf(" %49[^\n]", query);

    const Student *found = search_by_name(*arr, *count, query);
    if (found)
        printf("Found -> ID: %d, Name: %s, Average: %.2f\n",
               found->id, found->name, calculate_average(found));
    else
        printf("Student '%s' not found.\n", query);
}

int main(void) {
    Student *students = NULL;
    int count = 0;
    int capacity = 0;

    struct { int id; const char *name; float grades[NUM_TESTS]; } seed[] = {
        {101, "Alice Uwase",     {88, 92, 85, 90, 91}},
        {102, "Brian Mugisha",   {70, 65, 78, 60, 72}},
        {103, "Claudine Ingabire",{95, 96, 94, 97, 93}},
        {104, "David Niyonzima", {55, 80, 45, 90, 60}},
        {105, "Esther Umutoni",  {77, 79, 76, 78, 80}},
        {106, "Felix Habimana",  {60, 62, 58, 61, 59}},
        {107, "Grace Mukamana",  {89, 84, 91, 87, 86}},
        {108, "Henry Bizimana",  {40, 85, 30, 92, 50}},
        {109, "Irene Cyuzuzo",   {73, 74, 72, 75, 71}},
        {110, "Jack Ndayisenga", {99, 98, 97, 99, 100}},
        {111, "Kevin Iradukunda",{65, 70, 68, 72, 66}},
        {112, "Liliane Keza",    {82, 81, 83, 80, 84}}
    };
    int seed_count = sizeof(seed) / sizeof(seed[0]);
    for (int i = 0; i < seed_count; i++)
        add_student(&students, &count, &capacity, seed[i].id, seed[i].name, seed[i].grades);

    MenuHandler menu[] = {
        handle_add,             
        handle_display,         
        handle_sort,            
        handle_average,         
        handle_top,             
        handle_custom_analysis, 
        handle_search           
    };
    const char *menu_labels[] = {
        "Add Student",
        "Display Students",
        "Sort Students",
        "Calculate Class Average",
        "Find Top Student",
        "Run Custom Analysis (Grade Stability Score)",
        "Search Student by Name"
    };
    int menu_size = sizeof(menu) / sizeof(menu[0]);

    int choice;
    do {
        print_header();
        for (int i = 0; i < menu_size; i++)
            printf(" %d. %s\n", i + 1, menu_labels[i]);
        printf(" %d. Exit\n", menu_size + 1);
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice >= 1 && choice <= menu_size) {
            menu[choice - 1](&students, &count, &capacity);
        } else if (choice == menu_size + 1) {
            printf("Exiting. Freeing allocated memory...\n");
        } else {
            printf("Invalid choice, try again.\n");
        }
    } while (choice != menu_size + 1);

    free(students);   
    students = NULL;

    return 0;

}