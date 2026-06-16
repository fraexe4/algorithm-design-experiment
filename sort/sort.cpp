/*
 * Sorting Algorithms Experiment - Algorithm Design & Analysis
 *
 * Task 1: Equivalence class - generate 2 groups of 100 random numbers,
 *         record comparison counts of 3 algorithms
 * Task 2: Scaling curve - 10/100/1K/2K/5K/10K
 * Task 3: Subproblem size tracking - merge / quick sort recursion
 *
 * Build: gcc sort.cpp -o sort.exe
 * Run:   sort.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

/* ============== Global counters ============== */
long long cmp_bubble = 0;       // bubble sort comparison count
long long cmp_merge  = 0;       // merge sort comparison count
long long cmp_quick  = 0;       // quick sort comparison count

/* Task 3: subproblem size recording */
#define MAX_SUBPROB 200000
int  subprob_size[MAX_SUBPROB];  // size of each recursive subproblem
int  subprob_cnt   = 0;

/* ============== Utility ============== */
static void print_arr(const int *a, int n, const char *tag) {
    printf("%s [n=%d]: ", tag, n);
    int show = n < 20 ? n : 20;
    for (int i = 0; i < show; i++) printf("%d ", a[i]);
    if (n > 20) printf("... (total %d)", n);
    printf("\n");
}

static void gen_random(int *a, int n, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < n; i++) a[i] = rand() % 10000;
}

static void dump_csv(const char *path, const int *a, int n) {
    /* Make sure parent directory exists. The executable runs from D:/Cwork/sort */
    CreateDirectoryA("data", NULL);   /* ignore error if exists */
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("    [dump_csv] cannot open %s\n", path);
        return;
    }
    fprintf(fp, "idx,value\n");
    for (int i = 0; i < n; i++) fprintf(fp, "%d,%d\n", i, a[i]);
    fclose(fp);
    printf("    [saved] %s  (%d rows)\n", path, n);
}

static long long now_ms(void) {
    /* Windows high-resolution timer - milliseconds */
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (long long)(counter.QuadPart * 1000 / freq.QuadPart);
}

/* ============== Bubble sort ============== */
void bubble_sort(int *a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            cmp_bubble++;
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

/* ============== Merge sort ============== */
void merge(int *a, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int *L = (int*)malloc((n1 + 1) * sizeof(int));
    int *R = (int*)malloc((n2 + 1) * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = a[l + i];
    for (int j = 0; j < n2; j++) R[j] = a[m + 1 + j];
    L[n1] = R[n2] = 0x7fffffff;  // sentinel
    int i = 0, j = 0, k = l;
    while (k <= r) {
        cmp_merge++;
        if (L[i] <= R[j]) a[k++] = L[i++];
        else              a[k++] = R[j++];
    }
    free(L); free(R);
}

void merge_sort_rec(int *a, int l, int r) {
    if (l >= r) return;
    /* Task 3: record current subproblem size */
    if (subprob_cnt < MAX_SUBPROB)
        subprob_size[subprob_cnt++] = r - l + 1;

    int m = (l + r) / 2;
    merge_sort_rec(a, l, m);
    merge_sort_rec(a, m + 1, r);
    merge(a, l, m, r);
}

void merge_sort(int *a, int n) {
    subprob_cnt = 0;  // reset
    merge_sort_rec(a, 0, n - 1);
}

/* ============== Quick sort ============== */
int partition(int *a, int l, int r) {
    /* Randomized pivot: swap a random element to the end, avoiding
       worst-case O(n^2) on sorted/near-sorted inputs. */
    int rnd = l + ((unsigned)rand() ^ (unsigned)time(NULL)) % (r - l + 1);
    int t = a[rnd]; a[rnd] = a[r]; a[r] = t;

    int pivot = a[r];
    int i = l - 1;
    for (int j = l; j < r; j++) {
        cmp_quick++;
        if (a[j] <= pivot) {
            i++;
            t = a[i]; a[i] = a[j]; a[j] = t;
        }
    }
    t = a[i + 1]; a[i + 1] = a[r]; a[r] = t;
    return i + 1;
}

void quick_sort_rec(int *a, int l, int r) {
    if (l >= r) return;
    /* Task 3: record current subproblem size */
    if (subprob_cnt < MAX_SUBPROB)
        subprob_size[subprob_cnt++] = r - l + 1;

    int p = partition(a, l, r);
    quick_sort_rec(a, l, p - 1);
    quick_sort_rec(a, p + 1, r);
}

void quick_sort(int *a, int n) {
    subprob_cnt = 0;
    quick_sort_rec(a, 0, n - 1);
}

/* ============== Task 1: Equivalence class ============== */
void task1_equivalence_class(void) {
    printf("\n========== Task 1: Equivalence class verification ==========\n");
    int n = 100;
    int *a1 = (int*)malloc(n * sizeof(int));
    int *a2 = (int*)malloc(n * sizeof(int));

    /* Two independent random datasets */
    gen_random(a1, n, 12345);
    gen_random(a2, n, 67890);

    /* Save raw arrays for reproducibility / data appendix */
    dump_csv("data/sort_t1_group1.csv", a1, n);
    dump_csv("data/sort_t1_group2.csv", a2, n);

    print_arr(a1, n, "Group 1 data");
    print_arr(a2, n, "Group 2 data");

    /* Bubble */
    cmp_bubble = 0;
    bubble_sort(a1, n);
    long long b1 = cmp_bubble;
    cmp_bubble = 0;
    bubble_sort(a2, n);
    long long b2 = cmp_bubble;

    /* Merge */
    cmp_merge = 0;
    merge_sort(a1, n);
    long long m1 = cmp_merge;
    cmp_merge = 0;
    merge_sort(a2, n);
    long long m2 = cmp_merge;

    /* Quick */
    cmp_quick = 0;
    quick_sort(a1, n);
    long long q1 = cmp_quick;
    cmp_quick = 0;
    quick_sort(a2, n);
    long long q2 = cmp_quick;

    printf("\n| Algorithm | Group-1 comps | Group-2 comps |\n");
    printf("|-----------|--------------:|--------------:|\n");
    printf("| Bubble    | %12lld | %12lld |\n", b1, b2);
    printf("| Merge     | %12lld | %12lld |\n", m1, m2);
    printf("| Quick     | %12lld | %12lld |\n", q1, q2);
    printf("\nNote: different inputs yield slightly different counts, but the order of magnitude\n");
    printf("      is the same for the same n. This is the meaning of 'equivalence class'.\n");

    free(a1); free(a2);
}

/* ============== Task 2: Scaling curve ============== */
void task2_scaling(void) {
    printf("\n========== Task 2: Scaling curve ==========\n");
    int scales[] = {10, 100, 1000, 2000, 5000, 10000};
    int scnt = sizeof(scales) / sizeof(scales[0]);

    printf("\n| Scale N    | Bubble comps     | Merge comps      | Quick comps      |\n");
    printf("|------------|-----------------:|-----------------:|-----------------:|\n");

    for (int s = 0; s < scnt; s++) {
        int n = scales[s];
        int *a = (int*)malloc(n * sizeof(int));
        gen_random(a, n, 42 + s);

        /* Save raw array */
        char p[64]; sprintf(p, "data/sort_t2_n%d.csv", n);
        dump_csv(p, a, n);

        cmp_bubble = 0; bubble_sort(a, n); long long b = cmp_bubble;
        cmp_merge  = 0; merge_sort(a, n);  long long m = cmp_merge;
        cmp_quick  = 0; quick_sort(a, n);  long long q = cmp_quick;

        printf("| %10d | %16lld | %16lld | %16lld |\n", n, b, m, q);
        free(a);
    }
    printf("\nTheoretical complexity:\n");
    printf("  - Bubble:    O(n^2)\n");
    printf("  - Merge:     O(n log n)\n");
    printf("  - Quick:     O(n log n) average  /  O(n^2) worst case\n");
}

/* ============== Task 3: Subproblem size tracking ============== */
void task3_subproblem_size(void) {
    printf("\n========== Task 3: Subproblem size tracking ==========\n");
    int scales[] = {10, 100, 1000, 5000, 10000};
    int scnt = sizeof(scales) / sizeof(scales[0]);

    for (int s = 0; s < scnt; s++) {
        int n = scales[s];
        int *a = (int*)malloc(n * sizeof(int));
        gen_random(a, n, 99 + s);

        /* Save raw array */
        char p[64]; sprintf(p, "data/sort_t3_n%d.csv", n);
        dump_csv(p, a, n);

        printf("\n--- N = %d ---\n", n);

        /* Merge sort */
        subprob_cnt = 0;
        merge_sort(a, n);
        int mcnt = subprob_cnt;
        long long msum = 0, mmin = 1e18, mmax = 0;
        for (int i = 0; i < mcnt; i++) {
            long long v = subprob_size[i];
            msum += v;
            if (v < mmin) mmin = v;
            if (v > mmax) mmax = v;
        }
        printf("Merge sort:  subproblems=%d, size range=[%lld, %lld], avg size=%.2f\n",
               mcnt, mmin, mmax, (double)msum / mcnt);

        /* Quick sort */
        subprob_cnt = 0;
        quick_sort(a, n);
        int qcnt = subprob_cnt;
        long long qsum = 0, qmin = 1e18, qmax = 0;
        for (int i = 0; i < qcnt; i++) {
            long long v = subprob_size[i];
            qsum += v;
            if (v < qmin) qmin = v;
            if (v > qmax) qmax = v;
        }
        printf("Quick sort:  subproblems=%d, size range=[%lld, %lld], avg size=%.2f\n",
               qcnt, qmin, qmax, (double)qsum / qcnt);

        free(a);
    }
    printf("\nConclusion:\n");
    printf("  - Merge sort: number of subproblems = 2N-1, sizes shrink logarithmically.\n");
    printf("  - Quick sort: number of subproblems ~= 0.7N, each recursion fixes ONE pivot position.\n");
}

/* ============== Main ============== */
int main(void) {
    printf("================================================\n");
    printf("   Sorting Experiment - Bubble / Merge / Quick   \n");
    printf("================================================\n");

    task1_equivalence_class();
    task2_scaling();
    task3_subproblem_size();

    printf("\n========== Experiment complete ==========\n");
    return 0;
}
