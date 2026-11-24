/* ow-simd – Simulator für Sensor- und Systemdaten. Schreibt jede Sekunde OW_STATE (Standard /tmp/openwatch/state.conf).
 * Auf der echten Uhr ersetzt später ein Dienst, der dieselbe Datei aus Akkutreiber/Sensoren füllt – Shell und Apps bleiben gleich.
 *   ow-simd [--once] [--battery N] [--charging] [--no-notif] [--steps N] [--hr N] */
#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
static const char *NOTIF[] = {
    "Nachrichten|Mia|Wann bist du heute zuhause? Ich koche etwas.|10:42",
    "Homelab|pve2 Backup fertig|5 Gäste gesichert, jüngstes Backup 2 Minuten alt.|09:15",
    "Kalender|Kernel-Build prüfen|Um 15:30 – openwatch-dev, VM 220.|08:50",
    "System|Akku bei 20 %|Lade die Uhr bald auf, um Always-on weiter zu nutzen.|08:10",
};
int main(int argc, char **argv) {
    const char *path = getenv("OW_STATE") ? getenv("OW_STATE") : "/tmp/openwatch/state.conf";
    int battery = 82, charging = 0, steps = 4312, hr = 72, once = 0, notif = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--once")) once = 1; else if (!strcmp(argv[i], "--charging")) charging = 1;
        else if (!strcmp(argv[i], "--no-notif")) notif = 0;
        else if (!strcmp(argv[i], "--battery") && i + 1 < argc) battery = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--steps") && i + 1 < argc) steps = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--hr") && i + 1 < argc) hr = atoi(argv[++i]);
    }
    char dir[512]; snprintf(dir, sizeof dir, "%s", path); char *sl = strrchr(dir, '/'); if (sl) { *sl = 0; mkdir(dir, 0755); }
    srand((unsigned)time(NULL)); int tick = 0; double temp = 31.5;
    do {
        char tmp[600]; snprintf(tmp, sizeof tmp, "%s.tmp", path); FILE *f = fopen(tmp, "w");
        if (!f) { perror("ow-simd"); return 1; }
        fprintf(f, "# Simulierte Werte (ow-simd)\nbattery=%d\ncharging=%d\nsteps=%d\nheart_rate=%d\ntemp_c=%.1f\n", battery, charging, steps, hr, temp);
        if (notif) for (unsigned i = 0; i < sizeof NOTIF / sizeof *NOTIF; i++) fprintf(f, "notif%u=%s\n", i, NOTIF[i]);
        fclose(f); rename(tmp, path);
        if (once) break;
        sleep(1); tick++;
        if (tick % 20 == 0) { if (charging) { if (battery < 100) battery++; } else if (battery > 1) battery--; }
        if (tick % 3 == 0) steps += rand() % 4;
        hr += (rand() % 5) - 2; if (hr < 58) hr = 58; if (hr > 118) hr = 118;
        temp += ((rand() % 5) - 2) * 0.05; if (temp < 30) temp = 30; if (temp > 36) temp = 36;
    } while (1);
    return 0;
}
