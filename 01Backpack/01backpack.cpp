/*
 * 0-1 Knapsack Problem Experiment - Algorithm Design & Analysis
 *
 * 4 algorithms: Brute force (2^n), Dynamic programming (O(nC)),
 *               Greedy (approximation), Backtracking (with pruning)
 *
 * Build: gcc 01backpack.cpp -o 01backpack.exe
 * Run:   01backpack.exe
 *   - Brute force only for n <= 3000 (auto-truncated at 2^20)
 *   - Full sweep: 1K ~ 320K items, 3 capacities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <math.h>

/* ============== Global parameters ============== */
#define MAX_N   320000
#define MAX_C   1000000
#define MAX_VAL 1e18

/* Item data */
static int    g_n;                        // number of items
static int    g_C;                        // knapsack capacity
static double g_w[MAX_N + 5];             // weight
static double g_v[MAX_N + 5];             // value
static double g_ratio[MAX_N + 5];         // value/weight ratio (for greedy)

/* Result */
static double g_best_value;

/* Timing - milliseconds */
static long long now_ms(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (long long)(counter.QuadPart * 1000 / freq.QuadPart);
}

/* ============== Utility ============== */
static void gen_items(int n, unsigned int seed) {
    srand(seed);
    g_n = n;
    for (int i = 0; i < n; i++) {
        g_w[i] = 1.0 + rand() % 100;                      // 1~100
        g_v[i] = 100.0 + (rand() % 90100) / 100.0;       // 100~1000
        g_ratio[i] = g_v[i] / g_w[i];
    }
}

static void dump_items_csv(const char *path, int n, int full) {
    CreateDirectoryA("data", NULL);   /* ignore if exists */
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("    [dump] cannot open %s\n", path);
        return;
    }
    fprintf(fp, "idx,weight,value,ratio\n");

    if (full) {
        for (int i = 0; i < n; i++)
            fprintf(fp, "%d,%.0f,%.2f,%.4f\n", i, g_w[i], g_v[i], g_ratio[i]);
    } else {
        /* Summary: head 20 + 30 mid-samples + tail 20 */
        int head = 20, tail = 20, mid = 30;
        for (int i = 0; i < head; i++)
            fprintf(fp, "%d,%.0f,%.2f,%.4f\n", i, g_w[i], g_v[i], g_ratio[i]);
        int span = n - head - tail;
        if (span > 0) {
            long step = span / (mid + 1);
            for (int k = 1; k <= mid; k++) {
                int i = head + (int)(k * step);
                if (i >= n - tail) break;
                fprintf(fp, "%d,%.0f,%.2f,%.4f\n", i, g_w[i], g_v[i], g_ratio[i]);
            }
        }
        for (int i = n - tail; i < n; i++)
            fprintf(fp, "%d,%.0f,%.2f,%.4f\n", i, g_w[i], g_v[i], g_ratio[i]);
    }
    fclose(fp);
    printf("    [saved] %s  (n=%d, full=%d)\n", path, n, full);
}

static int cmp_desc(const void *a, const void *b) {
    double ra = *(const double*)a, rb = *(const double*)b;
    return rb > ra ? 1 : (rb < ra ? -1 : 0);
}

static void print_result(const char *alg, double elapsed_ms) {
    printf("| %-10s | %5d | %7d | %16.2f | %14lld |\n",
           alg, g_n, g_C, g_best_value, (long long)elapsed_ms);
}

/* ============== Brute force O(2^n) ============== */
void brute_force(int n, int C) {
    int total = 1 << n;   /* caller guarantees n <= 24 */
    g_best_value = 0;
    for (int mask = 0; mask < total; mask++) {
        double sw = 0, sv = 0;
        for (int i = 0; i < n; i++) {
            if (mask & (1 << i)) {
                sw += g_w[i];
                if (sw > C) { sv = 0; break; }
                sv += g_v[i];
            }
        }
        if (sw <= C && sv > g_best_value) g_best_value = sv;
    }
}

/* ============== Dynamic programming O(nC) ==============
 * dp[j] = max value under capacity j (rolling array)
 */
void dynamic_programming(int n, int C) {
    double *dp = (double*)calloc(C + 1, sizeof(double));
    if (!dp) {
        printf("    [DP] memory allocation failed.\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        int wi = (int)g_w[i];
        double vi = g_v[i];
        for (int j = C; j >= wi; j--) {
            double cand = dp[j - wi] + vi;
            if (cand > dp[j]) dp[j] = cand;
        }
    }
    g_best_value = dp[C];
    free(dp);
}

/* ============== Greedy (by value/weight) - approximation ============== */
void greedy(int n, int C) {
    /* Indices sorted by ratio descending */
    int *idx = (int*)malloc(n * sizeof(int));
    double *key = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) { idx[i] = i; key[i] = g_ratio[i]; }
    /* Simple selection sort (n may be large; only swaps index/key, not data) */
    for (int i = 0; i < n; i++) {
        int mx = i;
        for (int j = i + 1; j < n; j++)
            if (key[j] > key[mx]) mx = j;
        if (mx != i) {
            double tk = key[i]; key[i] = key[mx]; key[mx] = tk;
            int ti = idx[i]; idx[i] = idx[mx]; idx[mx] = ti;
        }
    }
    double rem = C, val = 0;
    for (int i = 0; i < n; i++) {
        int k = idx[i];
        if (g_w[k] <= rem) { val += g_v[k]; rem -= g_w[k]; }
    }
    g_best_value = val;
    free(idx); free(key);
}

/* ============== Backtracking + upper bound pruning ==============
 * Sort items by ratio, branch on include/exclude, prune by upper bound.
 */
static int   *bs_idx;
static double *bs_psum;  // suffix value sum (in ratio order)

static void backtrack(int pos, double cur_v, double rem_w) {
    if (pos >= g_n) {
        if (cur_v > g_best_value) g_best_value = cur_v;
        return;
    }
    /* Prune: current value + remaining suffix sum <= best */
    if (cur_v + bs_psum[pos] <= g_best_value) return;
    int k = bs_idx[pos];
    if (g_w[k] <= rem_w) {
        backtrack(pos + 1, cur_v + g_v[k], rem_w - g_w[k]);
    }
    backtrack(pos + 1, cur_v, rem_w);
}

void backtrack_search(int n, int C) {
    bs_idx = (int*)malloc(n * sizeof(int));
    double *key = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) { bs_idx[i] = i; key[i] = g_ratio[i]; }
    /* Sort indices by ratio descending (selection sort over index array) */
    for (int i = 0; i < n; i++) {
        int mx = i;
        for (int j = i + 1; j < n; j++)
            if (g_ratio[bs_idx[j]] > g_ratio[bs_idx[mx]]) mx = j;
        if (mx != i) {
            int ti = bs_idx[i]; bs_idx[i] = bs_idx[mx]; bs_idx[mx] = ti;
        }
    }
    /* Suffix value sum (in ratio order) */
    bs_psum = (double*)malloc((n + 1) * sizeof(double));
    bs_psum[n] = 0;
    for (int i = n - 1; i >= 0; i--) {
        int k = bs_idx[i];
        bs_psum[i] = bs_psum[i + 1] + g_v[k];
    }
    /* Seed upper-bound pruning with a fast greedy lower bound,
     * so we can prune branches before finding the first exact solution. */
    g_best_value = 0;
    {
        double rem = C, val = 0;
        for (int i = 0; i < n; i++) {
            int k = bs_idx[i];
            if (g_w[k] <= rem) { val += g_v[k]; rem -= g_w[k]; }
        }
        g_best_value = val;
    }
    backtrack(0, 0, C);
    free(bs_idx); free(key); free(bs_psum);
}

/* ============== Main test flow ============== */
void run_one(int n, int C) {
    gen_items(n, (unsigned)(n * 31 + C));
    g_C = C;   /* FIX: was never set, so prints were always 0 */

    /* Save raw items for data appendix (small: full; large: summary) */
    char path[128];
    sprintf(path, "data/ks_n%d_C%d.csv", n, C);
    dump_items_csv(path, n, n <= 100);

    long long t0, t1;
    printf("\n[N=%d, C=%d]\n", n, C);
    printf("| Algorithm   | Items  | Capacity | Max value          | Time (ms)         |\n");
    printf("|-------------|-------:|---------:|-------------------:|------------------:|\n");
    fflush(stdout);  /* ensure header appears before slow algorithms */

    /* DP */
    t0 = now_ms();
    dynamic_programming(n, C);
    t1 = now_ms();
    print_result("DP", t1 - t0);

    /* Greedy */
    t0 = now_ms();
    greedy(n, C);
    t1 = now_ms();
    print_result("Greedy", t1 - t0);

    /* Backtracking - only for small n; large n explodes exponentially */
    t0 = now_ms();
    if (n <= 60) {
        printf("| %-10s  | %5d | %7d | (running)         | %16s |\n",
               "Backtrack", n, C, "running");
        fflush(stdout);
        backtrack_search(n, C);
        t1 = now_ms();
        print_result("Backtrack", t1 - t0);
    } else {
        t1 = now_ms();
        printf("| %-10s  | %5d | %7d | (skip:n>60)       | %16lld |\n",
               "Backtrack", n, C, t1 - t0);
    }

    /* Brute force - small scale only */
    t0 = now_ms();
    if (n <= 24) {
        printf("| %-10s  | %5d | %7d | (running)         | %16s |\n",
               "Brute", n, C, "running");
        fflush(stdout);
        brute_force(n, C);
        t1 = now_ms();
        print_result("Brute", t1 - t0);
    } else {
        t1 = now_ms();
        printf("| %-10s  | %5d | %7d | (skip:n>24)       | %16lld |\n",
               "Brute", n, C, t1 - t0);
    }
}

int main(void) {
    printf("==========================================================\n");
    printf("   0-1 Knapsack Experiment                                \n");
    printf("   Brute / Dynamic Programming / Greedy / Backtracking    \n");
    printf("==========================================================\n");

    int scales[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000};
    int scnt = sizeof(scales) / sizeof(scales[0]);
    int caps[] = {10000, 100000, 1000000};
    int ccnt = sizeof(caps) / sizeof(caps[0]);

    /* Small-scale runs that include all 4 algorithms */
    int small_scales[] = {10, 15, 20, 24};
    int small_cnt = sizeof(small_scales) / sizeof(small_scales[0]);

    /* Step 1: small-scale run, all 4 algorithms (incl. brute / backtrack) */
    printf("\n########## Small-scale runs: all 4 algorithms ##########\n");
    for (int c = 0; c < ccnt; c++) {
        printf("\n#################### Capacity C = %d ####################\n", caps[c]);
        for (int s = 0; s < small_cnt; s++) {
            run_one(small_scales[s], caps[c]);
        }
    }

    /* Step 2: large-scale run, DP and Greedy only (brute/backtrack infeasible) */
    printf("\n########## Large-scale runs: DP + Greedy only ##########\n");
    for (int c = 0; c < ccnt; c++) {
        printf("\n#################### Capacity C = %d ####################\n", caps[c]);
        for (int s = 0; s < scnt; s++) {
            run_one(scales[s], caps[c]);
        }
    }

    printf("\n========== Experiment complete ==========\n");
    return 0;
}
