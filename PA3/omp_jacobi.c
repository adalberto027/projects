#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main() {
    double *b, *x, *xnew;
    double d, r, t;
    int i, it, m = 10000, n = 50000;
    double start, end;

    // Memory allocation
    b = (double *)malloc(n * sizeof(double));
    x = (double *)malloc(n * sizeof(double));
    xnew = (double *)malloc(n * sizeof(double));

    printf("\nJACOBI_OPENMP_OPTIMIZED:\nC/OpenMP optimized version\n");
    printf("Jacobi iteration to solve A*x=b.\n\n");
    printf("Number of variables  N = %d\n", n);
    printf("Number of iterations M = %d\n\n");

    // Data initialization
    for (i = 0; i < n; i++) {
        b[i] = 0.0;
        x[i] = 0.0;
    }
    b[n - 1] = (double)(n + 1);

    start = omp_get_wtime();
    for (it = 0; it < m; it++) {

        // Jacobi update with dynamic scheduling
        #pragma omp parallel for schedule(dynamic, 500) private(i)
        for (i = 0; i < n; i++) {
            xnew[i] = b[i];
            if (i > 0) xnew[i] += x[i - 1];
            if (i < n - 1) xnew[i] += x[i + 1];
            xnew[i] /= 2.0;
        }

        // Compute difference with reduction
        d = 0.0;
        #pragma omp parallel for reduction(+:d) private(i)
        for (i = 0; i < n; i++) {
            d += pow(x[i] - xnew[i], 2);
        }

        // Update the solution
        #pragma omp parallel for schedule(dynamic, 500) private(i)
        for (i = 0; i < n; i++) {
            x[i] = xnew[i];
        }

        // Compute residual with reduction
        r = 0.0;
        #pragma omp parallel for reduction(+:r) private(i, t)
        for (i = 0; i < n; i++) {
            t = b[i] - 2.0 * x[i];
            if (i > 0) t += x[i - 1];
            if (i < n - 1) t += x[i + 1];
            r += t * t;
        }
    }
    end = omp_get_wtime();
    printf("Time for jacobi iteration: %f seconds\n", end - start);

    // Print part of the final solution
    printf("\nPart of final solution estimate:\n");
    for (i = 0; i < 10; i++) {
        printf("  %8d  %14.6g\n", i, x[i]);
    }
    printf("...\n");
    for (i = n - 11; i < n; i++) {
        printf("  %8d  %14.6g\n", i, x[i]);
    }

    // Free memory
    free(b);
    free(x);
    free(xnew);

    printf("\nJACOBI_OPENMP_OPTIMIZED:\nExecution finished successfully.\n");

    return 0;
}
