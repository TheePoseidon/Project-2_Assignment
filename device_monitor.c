#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME_LEN          30
#define INITIAL_CAPACITY  8

/* Device type identifiers */
#define TYPE_TEMPERATURE 0
#define TYPE_PRESSURE    1
#define TYPE_VOLTAGE     2


struct Device {
    char name[30];
    int  type;
    union {
        float temperature;
        int   pressure;
        float voltage;
    } reading;
};


typedef void (*CallbackFunc)(struct Device *);


void process_device(struct Device *dev, void (*callback)(struct Device *));


static void print_program_header(void);
static void load_default_devices(struct Device **arr, int *count, int *capacity);
static int  ensure_capacity(struct Device **arr, int *capacity, int count);
static void add_device(struct Device **arr, int *count, int *capacity);
static void display_devices(struct Device *arr, int count);
static const char *type_name(int type);
static CallbackFunc select_callback(int type);
static void run_simulation(struct Device *arr, int count);
static void flush_stdin(void);


static void temperature_monitor(struct Device *dev);
static void pressure_monitor(struct Device *dev);
static void battery_monitor(struct Device *dev);


static void reading_logger(struct Device *dev);
static void print_log(void);
static void free_log(void);


int main(void)
{
    struct Device *devices = NULL;
    int count = 0;
    int capacity = 0;

    srand((unsigned int)time(NULL));

    print_program_header();
    load_default_devices(&devices, &count, &capacity);

    int choice = 0;
    do {
        printf("\n-------------------------- MENU --------------------------\n");
        printf("  1. Add Device\n");
        printf("  2. Display Devices\n");
        printf("  3. Run Simulation (10 random readings)\n");
        printf("  4. Show Custom Analysis Log (reading_logger callback)\n");
        printf("  5. Exit\n");
        printf("-------------------------------------------------------------\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            flush_stdin();
            choice = -1;
            printf("Invalid input. Enter a number 1-5.\n");
            continue;
        }
        flush_stdin();

        switch (choice) {
            case 1: add_device(&devices, &count, &capacity); break;
            case 2: display_devices(devices, count); break;
            case 3: run_simulation(devices, count); break;
            case 4: print_log(); break;
            case 5: printf("Exiting. Freeing dynamically allocated memory...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 5);

    free(devices);
    devices = NULL;
    free_log();
    return 0;
}

/* ================================================================== */
/* Setup / header                                                      */
/* ================================================================== */
static void print_program_header(void)
{
    printf("========================================================\n");
    printf(" DEVICE MONITORING SIMULATOR\n");
    printf(" Device Types: Temperature, Pressure, Voltage/Battery\n");
    printf(" Custom Callback: reading_logger()\n");
    printf("========================================================\n");
}

static void flush_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

static const char *type_name(int type)
{
    switch (type) {
        case TYPE_TEMPERATURE: return "Temperature";
        case TYPE_PRESSURE:    return "Pressure";
        case TYPE_VOLTAGE:     return "Voltage/Battery";
        default:               return "Unknown";
    }
}

/* Doubles capacity via realloc whenever the array is full            */
static int ensure_capacity(struct Device **arr, int *capacity, int count)
{
    if (count < *capacity) return 1;
    int new_capacity = (*capacity == 0) ? INITIAL_CAPACITY : (*capacity) * 2;
    struct Device *tmp = (struct Device *)realloc(*arr, sizeof(struct Device) * new_capacity);
    if (tmp == NULL) {
        fprintf(stderr, "Memory reallocation failed.\n");
        return 0;
    }
    *arr = tmp;
    *capacity = new_capacity;
    return 1;
}

/* Loads a starter fleet of 6 devices (2 of each required type) into
   a dynamically allocated array.                                    */
static void load_default_devices(struct Device **arr, int *count, int *capacity)
{
    *capacity = INITIAL_CAPACITY;
    *arr = (struct Device *)malloc(sizeof(struct Device) * (*capacity));
    if (*arr == NULL) {
        fprintf(stderr, "Fatal: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    *count = 0;

    struct { const char *name; int type; } seed[] = {
        {"TempSensor-A",   TYPE_TEMPERATURE},
        {"TempSensor-B",   TYPE_TEMPERATURE},
        {"PressureUnit-A", TYPE_PRESSURE},
        {"PressureUnit-B", TYPE_PRESSURE},
        {"BatteryPack-A",  TYPE_VOLTAGE},
        {"BatteryPack-B",  TYPE_VOLTAGE},
    };
    int n = (int)(sizeof(seed) / sizeof(seed[0]));

    for (int i = 0; i < n; i++) {
        ensure_capacity(arr, capacity, *count);
        struct Device *slot = *arr + (*count);   /* pointer arithmetic */
        strncpy(slot->name, seed[i].name, NAME_LEN - 1);
        slot->name[NAME_LEN - 1] = '\0';
        slot->type = seed[i].type;

        /* initialize the union member matching this device's type */
        switch (slot->type) {
            case TYPE_TEMPERATURE: slot->reading.temperature = 20.0f; break;
            case TYPE_PRESSURE:    slot->reading.pressure    = 1013;  break;
            case TYPE_VOLTAGE:     slot->reading.voltage     = 5.0f;  break;
        }
        (*count)++;
    }
}

static void add_device(struct Device **arr, int *count, int *capacity)
{
    if (!ensure_capacity(arr, capacity, *count)) return;

    char name[NAME_LEN];
    int type;

    printf("\n-- Add Device --\n");
    printf("Enter device name (no spaces): ");
    if (scanf("%29s", name) != 1) { flush_stdin(); printf("Invalid name.\n"); return; }
    flush_stdin();

    printf("Enter device type (0=Temperature, 1=Pressure, 2=Voltage/Battery): ");
    if (scanf("%d", &type) != 1 || type < 0 || type > 2) {
        flush_stdin();
        printf("Invalid type.\n");
        return;
    }
    flush_stdin();

    struct Device *slot = *arr + (*count);
    strncpy(slot->name, name, NAME_LEN - 1);
    slot->name[NAME_LEN - 1] = '\0';
    slot->type = type;

    switch (type) {
        case TYPE_TEMPERATURE: slot->reading.temperature = 20.0f; break;
        case TYPE_PRESSURE:    slot->reading.pressure    = 1013;  break;
        case TYPE_VOLTAGE:     slot->reading.voltage     = 5.0f;  break;
    }

    (*count)++;
    printf("Device '%s' (%s) added.\n", name, type_name(type));
}

static void display_devices(struct Device *arr, int count)
{
    printf("%-16s %-18s %-15s\n", "Name", "Type", "Reading");
    printf("---------------------------------------------------\n");

    struct Device *p = arr;            /* pointer traversal */
    for (int i = 0; i < count; i++, p++) {
        printf("%-16s %-18s ", p->name, type_name(p->type));
        /* print reading using the correct union member for this type */
        switch (p->type) {
            case TYPE_TEMPERATURE: printf("%.2f C\n", p->reading.temperature); break;
            case TYPE_PRESSURE:    printf("%d hPa\n", p->reading.pressure);    break;
            case TYPE_VOLTAGE:     printf("%.2f V\n", p->reading.voltage);     break;
        }
    }
    printf("---------------------------------------------------\n");
    printf("Total devices: %d\n", count);
}

/* ================================================================== */
/* Callback dispatcher (required exact signature)                     */
/* ================================================================== */
void process_device(struct Device *dev, void (*callback)(struct Device *))
{
    if (dev == NULL || callback == NULL) return;
    callback(dev);   /* invoke through the function pointer */
}

/* Picks the right monitor callback for a device's type -- returns a
   function pointer, demonstrating dynamic callback selection.       */
static CallbackFunc select_callback(int type)
{
    switch (type) {
        case TYPE_TEMPERATURE: return temperature_monitor;
        case TYPE_PRESSURE:    return pressure_monitor;
        case TYPE_VOLTAGE:     return battery_monitor;
        default:                return NULL;
    }
}

/* ================================================================== */
/* Required-example callback functions                                */
/* ================================================================== */
static void temperature_monitor(struct Device *dev)
{
    float t = dev->reading.temperature;
    const char *status = "Normal";
    if (t >= 75.0f)      status = "ALERT: Overheating";
    else if (t <= -10.0f) status = "ALERT: Freezing";

    printf("[Temperature] %-16s %.2f C -> %s\n", dev->name, t, status);
}

static void pressure_monitor(struct Device *dev)
{
    int p = dev->reading.pressure;
    const char *status = "Normal";
    if (p > 1050)      status = "ALERT: Overpressure";
    else if (p < 950)  status = "ALERT: Underpressure";

    printf("[Pressure]    %-16s %d hPa -> %s\n", dev->name, p, status);
}

static void battery_monitor(struct Device *dev)
{
    float v = dev->reading.voltage;
    const char *status = "Normal";
    if (v < 3.3f)      status = "ALERT: Low Battery";
    else if (v > 4.8f) status = "Fully Charged";

    printf("[Battery]     %-16s %.2f V -> %s\n", dev->name, v, status);
}

/* ================================================================== */
/* CUSTOM CALLBACK: reading_logger()                                   */
/*                                                                     */
/* Rationale: the three example callbacks each react to a single       */
/* reading in isolation (threshold checks). This custom callback       */
/* instead builds a persistent, dynamically allocated history of       */
/* every reading that has been processed across the whole simulation,  */
/* independent of device type, so the fleet's activity can be          */
/* reviewed afterwards. It demonstrates dynamic memory management      */
/* driven entirely from inside a callback.                             */
/* ================================================================== */
typedef struct {
    char  device_name[NAME_LEN];
    int   type;
    float value;      /* pressure is stored widened to float for a uniform log */
} LogEntry;

static LogEntry *g_log = NULL;
static int g_log_count = 0;
static int g_log_capacity = 0;

static void reading_logger(struct Device *dev)
{
    if (g_log_count == g_log_capacity) {
        int new_capacity = (g_log_capacity == 0) ? INITIAL_CAPACITY : g_log_capacity * 2;
        LogEntry *tmp = (LogEntry *)realloc(g_log, sizeof(LogEntry) * new_capacity);
        if (!tmp) { fprintf(stderr, "Log reallocation failed.\n"); return; }
        g_log = tmp;
        g_log_capacity = new_capacity;
    }

    LogEntry *entry = g_log + g_log_count;   /* pointer arithmetic */
    strncpy(entry->device_name, dev->name, NAME_LEN - 1);
    entry->device_name[NAME_LEN - 1] = '\0';
    entry->type = dev->type;

    switch (dev->type) {
        case TYPE_TEMPERATURE: entry->value = dev->reading.temperature; break;
        case TYPE_PRESSURE:    entry->value = (float)dev->reading.pressure; break;
        case TYPE_VOLTAGE:     entry->value = dev->reading.voltage; break;
        default:               entry->value = 0.0f;
    }

    g_log_count++;
}

static void print_log(void)
{
    printf("\n-- Custom Analysis: Reading Log (reading_logger callback) --\n");
    if (g_log_count == 0) {
        printf("Log is empty. Run the simulation first.\n");
        return;
    }
    printf("%-5s %-16s %-18s %-10s\n", "#", "Device", "Type", "Value");
    printf("-----------------------------------------------------------\n");

    LogEntry *p = g_log;                /* pointer traversal */
    for (int i = 0; i < g_log_count; i++, p++) {
        printf("%-5d %-16s %-18s %-10.2f\n",
               i + 1, p->device_name, type_name(p->type), p->value);
    }
    printf("-----------------------------------------------------------\n");
    printf("Total logged readings: %d\n", g_log_count);
}

static void free_log(void)
{
    free(g_log);
    g_log = NULL;
    g_log_count = 0;
    g_log_capacity = 0;
}

/* ================================================================== */
/* Simulation: 10 random readings distributed across the fleet         */
/* ================================================================== */
static void run_simulation(struct Device *arr, int count)
{
    if (count == 0) { printf("No devices available. Add a device first.\n"); return; }

    printf("\n-- Simulating 10 Random Device Readings --\n");

    for (int i = 0; i < 10; i++) {
        int idx = rand() % count;
        struct Device *dev = arr + idx;   /* pointer traversal to chosen device */

        /* generate a random reading appropriate to the device's type,
           written into the correct union member                     */
        switch (dev->type) {
            case TYPE_TEMPERATURE:
                dev->reading.temperature = -20.0f + ((float)rand() / RAND_MAX) * 120.0f; /* -20..100 C */
                break;
            case TYPE_PRESSURE:
                dev->reading.pressure = 900 + (rand() % 201); /* 900..1100 hPa */
                break;
            case TYPE_VOLTAGE:
                dev->reading.voltage = 2.5f + ((float)rand() / RAND_MAX) * 2.5f; /* 2.5..5.0 V */
                break;
        }

        printf("\nReading #%d -> %s\n", i + 1, dev->name);

        /* dispatch to the type-specific monitor callback through the
           required process_device() function-pointer interface       */
        CallbackFunc monitor = select_callback(dev->type);
        process_device(dev, monitor);

        /* also dispatch to the custom logging callback                */
        process_device(dev, reading_logger);
    }

    printf("\nSimulation complete. Use menu option 4 to review the log.\n");
}