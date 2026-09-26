/* host_api.c - a C "engine" embedding the Strata compiler through strata.h. */
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <psapi.h>
#include "strata.h"

static void show(const char* what, bool ok) { printf("%s: %s\n", what, ok ? "ok" : "FAILED"); }

static size_t memory_mb(void) {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc);
    return pmc.WorkingSetSize / (1024 * 1024);
}

int main(void) {
    show("version", strlen(strata_version()) > 0);
    show("check good source", strata_check_source("prog/main.strata", "print(1 + 2)\n"));
    show("check bad source fails", !strata_check_source("prog/main.strata", "var x = 1 + true\nprint(nope)\n"));
    printf("%s", strata_diagnostics());
    show("check a file with a module", strata_check("prog/main.strata"));
    show("no diagnostics after success", strata_diagnostics()[0] == '\0');

    const char* c = strata_emit("prog/main.strata");
    show("emit", strstr(c, "int main(") != NULL && strstr(c, "shout") != NULL);
    show("emit of a bad program is empty", strata_emit_source("prog/main.strata", "print(nope)\n")[0] == '\0');

    show("build a file (lib/ found next to the dll)", strata_build("prog/main.strata", false, false));
    FILE* f = fopen("prog/main.exe", "rb");
    show("the exe exists", f != NULL);
    if (f) fclose(f);
    show("build a dll project", strata_build("../projects/lib", false, false));
    show("output path", strstr(strata_output_path("../projects/lib"), "mathlib.dll") != NULL);
    show("a bad target reports why", !strata_build("nowhere", false, false) && strstr(strata_diagnostics(), "no strata.toml in nowhere") != NULL);

    /* an engine recompiling on every save: memory must stay flat */
    for (int i = 0; i < 50; i++) { strata_check("prog/main.strata"); strata_emit("prog/main.strata"); strata_reset(); }
    size_t before = memory_mb();
    for (int i = 0; i < 300; i++) { strata_check("prog/main.strata"); strata_emit("prog/main.strata"); strata_reset(); }
    size_t after = memory_mb();
    show("300 more check+emit+reset cycles don't grow memory", after <= before + 8);
    show("still works after resets", strata_check("prog/main.strata"));
    return 0;
}
