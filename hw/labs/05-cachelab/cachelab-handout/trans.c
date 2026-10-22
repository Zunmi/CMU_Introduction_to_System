/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int bi, bj, i, j;
    int a0, a1, a2, a3, a4, a5, a6, a7;

    if (M == 32 && N == 32) {
        for (bi = 0; bi < 32; bi += 8) {
            for (bj = 0; bj < 32; bj += 8) {
                for (i = bi; i < bi + 8; i++) {
                    a0 = 0;

                    for (j = bj; j < bj + 8; j++) {
                        if (i == j)
                            a0 = A[i][j];
                        else
                            B[j][i] = A[i][j];
                    }

                    if (bi == bj)
                        B[i][i] = a0;
                }
            }
        }
        return;
    }

    if (M == 64 && N == 64) {
        for (bi = 0; bi < 64; bi += 8) {
            for (bj = 0; bj < 64; bj += 8) {

            
                for (i = bi; i < bi + 4; i++) {
                    a0 = A[i][bj];
                    a1 = A[i][bj + 1];
                    a2 = A[i][bj + 2];
                    a3 = A[i][bj + 3];
                    a4 = A[i][bj + 4];
                    a5 = A[i][bj + 5];
                    a6 = A[i][bj + 6];
                    a7 = A[i][bj + 7];

                    B[bj][i] = a0;
                    B[bj + 1][i] = a1;
                    B[bj + 2][i] = a2;
                    B[bj + 3][i] = a3;

                    B[bj][i + 4] = a4;
                    B[bj + 1][i + 4] = a5;
                    B[bj + 2][i + 4] = a6;
                    B[bj + 3][i + 4] = a7;
                }

                
                for (j = bj; j < bj + 4; j++) {
                    a0 = B[j][bi + 4];
                    a1 = B[j][bi + 5];
                    a2 = B[j][bi + 6];
                    a3 = B[j][bi + 7];

                    a4 = A[bi + 4][j];
                    a5 = A[bi + 5][j];
                    a6 = A[bi + 6][j];
                    a7 = A[bi + 7][j];

                    B[j][bi + 4] = a4;
                    B[j][bi + 5] = a5;
                    B[j][bi + 6] = a6;
                    B[j][bi + 7] = a7;

                    B[j + 4][bi] = a0;
                    B[j + 4][bi + 1] = a1;
                    B[j + 4][bi + 2] = a2;
                    B[j + 4][bi + 3] = a3;
                }

                
                for (i = bi + 4; i < bi + 8; i++) {
                    a0 = A[i][bj + 4];
                    a1 = A[i][bj + 5];
                    a2 = A[i][bj + 6];
                    a3 = A[i][bj + 7];

                    B[bj + 4][i] = a0;
                    B[bj + 5][i] = a1;
                    B[bj + 6][i] = a2;
                    B[bj + 7][i] = a3;
                }
            }
        }
        return;
    }

    for (bi = 0; bi < N; bi += 16) {
        for (bj = 0; bj < M; bj += 16) {
            for (i = bi; i < bi + 16 && i < N; i++) {
                for (j = bj; j < bj + 16 && j < M; j++) {
                    B[j][i] = A[i][j];
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

