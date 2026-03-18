#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int size;
    int allocated_id; // -1 if free
    int frag;
} Block;

void run_simulation(char* algo, int blocks_raw[], int b_count, int procs_raw[], int p_count) {
    Block ram[b_count];
    for (int i = 0; i < b_count; i++) {
        ram[i].id = i;
        ram[i].size = blocks_raw[i];
        ram[i].allocated_id = -1;
        ram[i].frag = 0;
    }

    for (int i = 0; i < p_count; i++) {
        int target_idx = -1;

        if (strcmp(algo, "first") == 0) {
            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {
                    target_idx = j;
                    break;
                }
            }
        } else if (strcmp(algo, "best") == 0) {
            int best_size = 1e9;
            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {
                    if (ram[j].size < best_size) {
                        best_size = ram[j].size;
                        target_idx = j;
                    }
                }
            }
        } else if (strcmp(algo, "worst") == 0) {
            int worst_size = -1;
            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {
                    if (ram[j].size > worst_size) {
                        worst_size = ram[j].size;
                        target_idx = j;
                    }
                }
            }
        }

        if (target_idx != -1) {
            ram[target_idx].allocated_id = i + 1;
            ram[target_idx].frag = ram[target_idx].size - procs_raw[i];
        }
    }

    // Output JSON
    printf("{ \"blocks\": [");
    for (int i = 0; i < b_count; i++) {
        printf("{\"id\": %d, \"size\": %d, \"p_id\": %d, \"frag\": %d}%s", 
               ram[i].id, ram[i].size, ram[i].allocated_id, ram[i].frag, 
               (i == b_count - 1) ? "" : ",");
    }
    printf("] }");
}

int calculate_frag(char* algo, int blocks_raw[], int b_count, int procs_raw[], int p_count) {
    Block ram[b_count];
    for (int i = 0; i < b_count; i++) {
        ram[i].id = i;
        ram[i].size = blocks_raw[i];
        ram[i].allocated_id = -1;
        ram[i].frag = 0;
    }

    for (int i = 0; i < p_count; i++) {
        int target_idx = -1;

        if (strcmp(algo, "first") == 0) {
            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {
                    target_idx = j; break;
                }
            }
        } 
        else if (strcmp(algo, "best") == 0) {
            int best_size = 1000000000;
            target_idx = -1;   // 🔥 VERY IMPORTANT RESET

            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {

                    if (ram[j].size < best_size) {
                    best_size = ram[j].size;
                    target_idx = j;
                    }
                }
            }
        }       
        else if (strcmp(algo, "worst") == 0) {
            int worst = -1;
            for (int j = 0; j < b_count; j++) {
                if (ram[j].allocated_id == -1 && ram[j].size >= procs_raw[i]) {
                    if (ram[j].size > worst) {
                        worst = ram[j].size;
                        target_idx = j;
                    }
                }
            }
        }

        if (target_idx != -1) {
            ram[target_idx].allocated_id = i + 1;
            ram[target_idx].frag = ram[target_idx].size - procs_raw[i];
        }
    }

    int total_frag = 0;
    for (int i = 0; i < b_count; i++) {
        if (ram[i].allocated_id != -1)
            total_frag += ram[i].frag;
    }

    return total_frag;
}

void run_fifo(char *ref_str, int frames) {
    int ref[100], n = 0;

    char *token = strtok(ref_str, ",");
    while (token) {
        ref[n++] = atoi(token);
        token = strtok(NULL, ",");
    }

    int frame[10];
    for (int i = 0; i < frames; i++) frame[i] = -1;

    int pointer = 0;

    printf("{\"steps\":[");

    for (int i = 0; i < frames; i++) {
        printf("[");
        for (int j = 0; j < n; j++) {
            printf("\"-\"%s", (j == n-1) ? "" : ",");
        }
        printf("]%s", (i == frames-1) ? "" : ",");
    }
    printf("]}");
}int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    char *mode = argv[1];

    // =========================
    // 🔥 PAGING MODE (SEPARATE)
    // =========================
    if (strcmp(mode, "paging") == 0) {

    char *algo = argv[2];
    char *ref_str = argv[3];
    int frames = atoi(argv[4]);

    int ref[100], n = 0;
    char *token = strtok(ref_str, ",");

    while (token) {
        ref[n++] = atoi(token);
        token = strtok(NULL, ",");
    }

    int frame[50];
    for (int i = 0; i < frames; i++) frame[i] = -1;

    int pointer = 0;

    printf("{\"steps\":[");

    for (int i = 0; i < n; i++) {

        int found = 0;

        // check hit
        for (int j = 0; j < frames; j++) {
            if (frame[j] == ref[i]) {
                found = 1;
                break;
            }
        }

        if (!found) {

            // ================= FIFO =================
            if (strcmp(algo, "fifo") == 0) {
                frame[pointer] = ref[i];
                pointer = (pointer + 1) % frames;
            }

            // ================= LRU =================
            else if (strcmp(algo, "lru") == 0) {

                int lru_idx = -1;
                int min_use = 1e9;

                for (int j = 0; j < frames; j++) {

                    int last_used = -1;

                    for (int k = i - 1; k >= 0; k--) {
                        if (frame[j] == ref[k]) {
                            last_used = k;
                            break;
                        }
                    }

                    if (last_used < min_use) {
                        min_use = last_used;
                        lru_idx = j;
                    }
                }

                // empty frame priority
                for (int j = 0; j < frames; j++) {
                    if (frame[j] == -1) {
                        lru_idx = j;
                        break;
                    }
                }

                frame[lru_idx] = ref[i];
            }

            // ================= OPTIMAL =================
            else if (strcmp(algo, "optimal") == 0) {

                int opt_idx = -1;
                int farthest = -1;

                for (int j = 0; j < frames; j++) {

                    int next_use = 1e9;

                    for (int k = i + 1; k < n; k++) {
                        if (frame[j] == ref[k]) {
                            next_use = k;
                            break;
                        }
                    }

                    if (next_use > farthest) {
                        farthest = next_use;
                        opt_idx = j;
                    }
                }

                // empty frame priority
                for (int j = 0; j < frames; j++) {
                    if (frame[j] == -1) {
                        opt_idx = j;
                        break;
                    }
                }

                frame[opt_idx] = ref[i];
            }
        }

        // print frame state
        printf("[");
        for (int j = 0; j < frames; j++) {
            if (frame[j] == -1)
                printf("\"-\"");
            else
                printf("\"%d\"", frame[j]);

            if (j != frames - 1) printf(",");
        }
        printf("]");

        if (i != n - 1) printf(",");
    }

    printf("]}");
    return 0;
}

    // =========================
    // 🔥 MEMORY MODES (NORMAL)
    // =========================

    if (argc < 4) return 1;

    int blocks[50], procs[50], b_c = 0, p_c = 0;

    char *token = strtok(argv[2], ",");
    while (token) { blocks[b_c++] = atoi(token); token = strtok(NULL, ","); }

    token = strtok(argv[3], ",");
    while (token) { procs[p_c++] = atoi(token); token = strtok(NULL, ","); }

    // compare mode
    if (strcmp(mode, "compare") == 0) {

        int f1 = calculate_frag("first", blocks, b_c, procs, p_c);
        int f2 = calculate_frag("best", blocks, b_c, procs, p_c);
        int f3 = calculate_frag("worst", blocks, b_c, procs, p_c);

        printf("{\"results\":[");
        printf("{\"name\":\"First Fit\",\"frag\":%d},", f1);
        printf("{\"name\":\"Best Fit\",\"frag\":%d},", f2);
        printf("{\"name\":\"Worst Fit\",\"frag\":%d}", f3);
        printf("]}");

        return 0;
    }

    // normal simulation
    run_simulation(mode, blocks, b_c, procs, p_c);
    return 0;
}