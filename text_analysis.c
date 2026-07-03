/* =====================================================================
   DYNAMIC TEXT ANALYSIS TOOL
   -----------------------------------------------------------------
   Developer Student ID : 54321
   Analyzed excerpt      : Opening line of "Pride and Prejudice" by
                            Jane Austen (1813) - public domain text.
   Custom Analysis Func  : Lexical Diversity Score (Type-Token Ratio)
                            -> measures how varied the vocabulary is
                               (unique words / total words * 100),
                               a rule not among the assignment's own
                               examples (vowel density, avg word
                               length, palindrome detection).
   -----------------------------------------------------------------
   Demonstrates:
     - dynamic memory allocation for text, word array, and stats array
     - pointer traversal to split raw text into words
     - struct wordStat { char name[40]; int count; }
     - function pointers for analysis dispatch
     - strlen / strcmp / strcpy usage
     - printing memory addresses of key dynamic structures
   ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUDENT_ID       54321
#define MAX_WORD_LEN     40
#define INITIAL_CAPACITY 16

/* Required structure (exact fields as specified) ------------------- */
struct wordStat {
    char name[40];
    int  count;
};

/* Context bundling every dynamically-allocated structure we work with */
typedef struct {
    char   *text;         /* dynamically allocated raw text buffer     */
    char  **words;        /* dynamically allocated array of word ptrs  */
    int     wordCount;
    struct wordStat *stats; /* dynamically allocated aggregated stats  */
    int     statCount;
} TextContext;

/* ---------------------------------------------------------------- */
/* Prototypes                                                         */
/* ---------------------------------------------------------------- */
static void print_program_header(void);
static void load_default_text(TextContext *ctx);
static void load_custom_text(TextContext *ctx);
static void set_text(TextContext *ctx, const char *source);
static void split_into_words(TextContext *ctx);
static void build_word_stats(TextContext *ctx);
static void free_context(TextContext *ctx);
static void flush_stdin(void);
static void print_tokens(const TextContext *ctx);
static void print_stats_table(const TextContext *ctx);
static void print_memory_addresses(const TextContext *ctx);

/* Analysis functions (all share one signature -> function pointers) */
static void count_words(TextContext *ctx);
static void longest_word(TextContext *ctx);
static void most_frequent(TextContext *ctx);
static void lexical_diversity(TextContext *ctx);   /* custom rule    */

typedef void (*AnalysisFunc)(TextContext *);

/* ================================================================== */
int main(void)
{
    TextContext ctx = {0};

    AnalysisFunc analyses[4] = { count_words, longest_word,
                                  most_frequent, lexical_diversity };
    const char *labels[4] = {
        "Count Words (total & unique)",
        "Longest Word",
        "Most Frequent Word",
        "Custom Analysis: Lexical Diversity Score"
    };

    print_program_header();
    load_default_text(&ctx);
    split_into_words(&ctx);
    build_word_stats(&ctx);

    int choice = 0;
    do {
        printf("\n-------------------------- MENU --------------------------\n");
        printf("  1. Load default excerpt (Pride and Prejudice)\n");
        printf("  2. Enter your own text\n");
        printf("  3. Show split word tokens\n");
        printf("  4. Show word statistics table\n");
        printf("  5. %s\n", labels[0]);
        printf("  6. %s\n", labels[1]);
        printf("  7. %s\n", labels[2]);
        printf("  8. %s\n", labels[3]);
        printf("  9. Print memory addresses of key structures\n");
        printf(" 10. Exit\n");
        printf("-------------------------------------------------------------\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            flush_stdin();
            choice = -1;
            printf("Invalid input. Enter a number 1-10.\n");
            continue;
        }
        flush_stdin();

        switch (choice) {
            case 1:
                load_default_text(&ctx);
                split_into_words(&ctx);
                build_word_stats(&ctx);
                printf("Default excerpt loaded and re-analyzed.\n");
                break;
            case 2:
                load_custom_text(&ctx);
                split_into_words(&ctx);
                build_word_stats(&ctx);
                printf("Custom text loaded and re-analyzed.\n");
                break;
            case 3: print_tokens(&ctx); break;
            case 4: print_stats_table(&ctx); break;
            case 5: analyses[0](&ctx); break;   /* function-pointer dispatch */
            case 6: analyses[1](&ctx); break;
            case 7: analyses[2](&ctx); break;
            case 8: analyses[3](&ctx); break;
            case 9: print_memory_addresses(&ctx); break;
            case 10: printf("Exiting. Freeing all dynamically allocated memory...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 10);

    free_context(&ctx);
    return 0;
}

/* ================================================================== */
/* Setup / header                                                      */
/* ================================================================== */
static void print_program_header(void)
{
    printf("========================================================\n");
    printf(" DYNAMIC TEXT ANALYSIS TOOL\n");
    printf(" Student ID: %d\n", STUDENT_ID);
    printf(" Custom Analysis Function: Lexical Diversity Score\n");
    printf("========================================================\n");
}

static void flush_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

/* Frees any previously-held text/word/stat memory before loading new */
static void reset_context(TextContext *ctx)
{
    if (ctx->words) {
        for (int i = 0; i < ctx->wordCount; i++) free(ctx->words[i]);
        free(ctx->words);
        ctx->words = NULL;
    }
    if (ctx->stats) {
        free(ctx->stats);
        ctx->stats = NULL;
    }
    if (ctx->text) {
        free(ctx->text);
        ctx->text = NULL;
    }
    ctx->wordCount = 0;
    ctx->statCount = 0;
}

/* Dynamically allocates ctx->text and copies "source" into it        */
static void set_text(TextContext *ctx, const char *source)
{
    reset_context(ctx);
    size_t len = strlen(source);              /* strlen usage */
    ctx->text = (char *)malloc(len + 1);
    if (!ctx->text) {
        fprintf(stderr, "Fatal: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(ctx->text, source);                /* strcpy usage */
}

static void load_default_text(TextContext *ctx)
{
    const char *excerpt =
        "It is a truth universally acknowledged, that a single man in "
        "possession of a good fortune, must be in want of a wife. However "
        "little known the feelings or views of such a man may be on his "
        "first entering a neighbourhood, this truth is so well fixed in "
        "the minds of the surrounding families, that he is considered as "
        "the rightful property of some one or other of their daughters.";
    set_text(ctx, excerpt);
}

static void load_custom_text(TextContext *ctx)
{
    char buffer[2000];
    printf("\nEnter a paragraph to analyze (single line, max 1999 chars):\n> ");
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        printf("Input error. Keeping previous text.\n");
        return;
    }
    /* strip trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

    if (strlen(buffer) == 0) {
        printf("Empty input. Keeping previous text.\n");
        return;
    }
    set_text(ctx, buffer);
}

/* ================================================================== */
/* Splitting text into words via pointer traversal                     */
/* ================================================================== */
static void split_into_words(TextContext *ctx)
{
    if (ctx->words) {
        for (int i = 0; i < ctx->wordCount; i++) free(ctx->words[i]);
        free(ctx->words);
        ctx->words = NULL;
    }
    ctx->wordCount = 0;

    int capacity = INITIAL_CAPACITY;
    ctx->words = (char **)malloc(sizeof(char *) * capacity);
    if (!ctx->words) { fprintf(stderr, "Allocation failed.\n"); exit(EXIT_FAILURE); }

    char *p = ctx->text;               /* pointer traversal starts here */
    while (*p != '\0') {
        /* skip any non-alphabetic characters (spaces, punctuation) */
        while (*p != '\0' && !isalpha((unsigned char)*p)) p++;
        if (*p == '\0') break;

        char *start = p;               /* mark start of a word via pointer */
        while (*p != '\0' && isalpha((unsigned char)*p)) p++;  /* advance pointer to end of word */

        long wlen = p - start;         /* pointer subtraction gives length */
        if (wlen <= 0) continue;
        if (wlen >= MAX_WORD_LEN) wlen = MAX_WORD_LEN - 1;  /* guard overflow */

        if (ctx->wordCount == capacity) {
            capacity *= 2;
            char **tmp = (char **)realloc(ctx->words, sizeof(char *) * capacity);
            if (!tmp) { fprintf(stderr, "Reallocation failed.\n"); exit(EXIT_FAILURE); }
            ctx->words = tmp;
        }

        char *word = (char *)malloc(wlen + 1);
        if (!word) { fprintf(stderr, "Allocation failed.\n"); exit(EXIT_FAILURE); }
        strncpy(word, start, wlen);
        word[wlen] = '\0';

        ctx->words[ctx->wordCount++] = word;
    }
}

/* ================================================================== */
/* Building aggregated word statistics (dynamic array of wordStat)     */
/* ================================================================== */
static void build_word_stats(TextContext *ctx)
{
    if (ctx->stats) { free(ctx->stats); ctx->stats = NULL; }
    ctx->statCount = 0;

    int capacity = INITIAL_CAPACITY;
    ctx->stats = (struct wordStat *)malloc(sizeof(struct wordStat) * capacity);
    if (!ctx->stats) { fprintf(stderr, "Allocation failed.\n"); exit(EXIT_FAILURE); }

    for (int i = 0; i < ctx->wordCount; i++) {
        /* build a lowercase copy so "The" and "the" count as one word */
        char lower[MAX_WORD_LEN];
        size_t len = strlen(ctx->words[i]);           /* strlen usage */
        if (len >= MAX_WORD_LEN) len = MAX_WORD_LEN - 1;
        for (size_t j = 0; j < len; j++)
            lower[j] = (char)tolower((unsigned char)ctx->words[i][j]);
        lower[len] = '\0';

        int found = -1;
        for (int k = 0; k < ctx->statCount; k++) {
            if (strcmp(ctx->stats[k].name, lower) == 0) {  /* strcmp usage */
                found = k;
                break;
            }
        }

        if (found >= 0) {
            ctx->stats[found].count++;
        } else {
            if (ctx->statCount == capacity) {
                capacity *= 2;
                struct wordStat *tmp = (struct wordStat *)realloc(
                    ctx->stats, sizeof(struct wordStat) * capacity);
                if (!tmp) { fprintf(stderr, "Reallocation failed.\n"); exit(EXIT_FAILURE); }
                ctx->stats = tmp;
            }
            strcpy(ctx->stats[ctx->statCount].name, lower);   /* strcpy usage */
            ctx->stats[ctx->statCount].count = 1;
            ctx->statCount++;
        }
    }
}

static void free_context(TextContext *ctx)
{
    reset_context(ctx);
}

/* ================================================================== */
/* Display helpers                                                     */
/* ================================================================== */
static void print_tokens(const TextContext *ctx)
{
    printf("\n-- Word Tokens (split via pointer traversal) --\n");
    for (int i = 0; i < ctx->wordCount; i++) {
        printf("%3d: %s\n", i + 1, ctx->words[i]);
    }
    printf("Total tokens: %d\n", ctx->wordCount);
}

static void print_stats_table(const TextContext *ctx)
{
    printf("\n-- Word Statistics (struct wordStat array) --\n");
    printf("%-20s %-10s\n", "Word", "Count");
    printf("------------------------------\n");
    for (int i = 0; i < ctx->statCount; i++) {
        printf("%-20s %-10d\n", ctx->stats[i].name, ctx->stats[i].count);
    }
    printf("------------------------------\n");
    printf("Unique words: %d\n", ctx->statCount);
}

static void print_memory_addresses(const TextContext *ctx)
{
    printf("\n-- Memory Addresses of Key Data Structures --\n");
    printf("Text buffer stored at memory address: %p\n", (void *)ctx->text);
    printf("Word array stored at memory address: %p\n", (void *)ctx->words);
    printf("Word stats array stored at memory address: %p\n", (void *)ctx->stats);

    int sample = ctx->wordCount < 3 ? ctx->wordCount : 3;
    for (int i = 0; i < sample; i++) {
        printf("  words[%d] (\"%s\") stored at: %p\n",
               i, ctx->words[i], (void *)ctx->words[i]);
    }
}

/* ================================================================== */
/* Required analysis functions                                         */
/* ================================================================== */
static void count_words(TextContext *ctx)
{
    printf("\n-- Count Words --\n");
    printf("Total word occurrences: %d\n", ctx->wordCount);
    printf("Unique words: %d\n", ctx->statCount);
}

static void longest_word(TextContext *ctx)
{
    if (ctx->wordCount == 0) { printf("No words to analyze.\n"); return; }

    char **p = ctx->words;             /* pointer traversal over word array */
    char *longest = *p;
    size_t longest_len = strlen(*p);

    for (int i = 0; i < ctx->wordCount; i++, p++) {
        size_t len = strlen(*p);       /* strlen usage */
        if (len > longest_len) {
            longest_len = len;
            longest = *p;
        }
    }

    printf("\n-- Longest Word --\n");
    printf("\"%s\" (%zu letters)\n", longest, longest_len);
}

static void most_frequent(TextContext *ctx)
{
    if (ctx->statCount == 0) { printf("No words to analyze.\n"); return; }

    struct wordStat *p = ctx->stats;   /* pointer traversal over stats array */
    struct wordStat *top = p;

    for (int i = 0; i < ctx->statCount; i++, p++) {
        if (p->count > top->count) top = p;
    }

    printf("\n-- Most Frequent Word --\n");
    printf("\"%s\" appears %d time(s)\n", top->name, top->count);
}

/* ================================================================== */
/* CUSTOM ANALYSIS: LEXICAL DIVERSITY SCORE (Type-Token Ratio)        */
/*                                                                     */
/* Rationale: raw word/frequency counts don't say much about how      */
/* *varied* a passage's vocabulary is. This computes the ratio of     */
/* unique words to total words (as a percentage). A ratio near 100%   */
/* means almost every word is different (highly diverse); a low       */
/* ratio means heavy repetition of the same words.                    */
/* ================================================================== */
static void lexical_diversity(TextContext *ctx)
{
    if (ctx->wordCount == 0) { printf("No words to analyze.\n"); return; }

    double ratio = ((double)ctx->statCount / (double)ctx->wordCount) * 100.0;

    const char *label;
    if (ratio >= 80.0)      label = "Highly Diverse";
    else if (ratio >= 50.0) label = "Moderately Diverse";
    else                     label = "Repetitive";

    printf("\n-- Custom Analysis: Lexical Diversity Score --\n");
    printf("Total words : %d\n", ctx->wordCount);
    printf("Unique words: %d\n", ctx->statCount);
    printf("Diversity   : %.2f%% (%s)\n", ratio, label);
    printf("Note: Score = (unique words / total words) * 100.\n");
}