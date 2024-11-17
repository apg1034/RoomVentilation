#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

void print_with_timestamp(const char *format, ...) {
    // Zeitstempel holen
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    // Zeitstempel formatieren
    char timestamp[20]; // Platz für "YYYY-MM-DD HH:MM:SS\0"
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    // Zeitstempel ausgeben
    printf("[%s] ", timestamp);

    // Variadic Arguments für printf verarbeiten
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}