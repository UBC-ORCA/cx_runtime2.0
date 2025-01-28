#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/ci.h"
#include "../zoo/mulacc/mulacc.h"
#include "../zoo/max/max.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define SIZE_0 32
#define SIZE_1 32
#define NTHREAD_0 2
#define NTHREAD_1 2

typedef struct thread_args {
  int **A;
  int **B;
  int **C;
  int m;
  int n;
  int nThreads;
  int tid;
  cx_select_t cx_sel;
  cx_select_t cx_sel_max;
} thread_args;

static inline int scalar_max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int** alloc_mat(int m, int n) {
  int **matrix = (int **)calloc(n, sizeof(int*));
  for(int i = 0; i < n; i++) {
      matrix[i] = (int *)calloc(m, sizeof(int));
  }
  return matrix;
}

int **gen_mat(int m, int n) 
{
    int **matrix = alloc_mat(m, n);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++){
            matrix[i][j] = (rand() % 10) - 5;
        }
    }
    return matrix;
}

thread_args** init_thread_args(int m, int n, int nThreads, cx_select_t cx_sel_max) {
  int **A = gen_mat(m, n);
  int **B = gen_mat(m, n);
  int **C = alloc_mat(m, n);
  thread_args **mat_args = malloc(sizeof(thread_args *) * nThreads);
  for (int i = 0; i < nThreads; i++) {
    mat_args[i] = malloc(sizeof(thread_args));
    mat_args[i]->A = A;
    mat_args[i]->B = B;
    mat_args[i]->C = C;
    mat_args[i]->m = m;
    mat_args[i]->n = n;
    mat_args[i]->nThreads = nThreads;
    mat_args[i]->tid = i;
    mat_args[i]->cx_sel_max = cx_sel_max;
  }
  return mat_args;
}

void scalar_nn_layer(int** A, int** B, int** C, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < m; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
      C[i][j] = scalar_max(0, C[i][j]);
    }
  }
}

void print_mat(int **A, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d ", A[i][j]);
    }
    printf("\n");
  }
}

void* nn_layer_worker(void *targs) {
  thread_args *args = (thread_args *)targs;
  int **A = args->A;
  int **B = args->B;
  int **C = args->C;
  int m = args->m;
  int n = args->n;
  int nThreads = args->nThreads;
  int tid = args->tid;
  int temp = 0;
  cx_sel(args->cx_sel);
  for (int i = tid; i < m; i += nThreads) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < m; k++) {
        mac(A[i][k], B[k][j]);
      }
      temp = CX_READ_STATE(0);
      cx_sel(args->cx_sel_max);
      C[i][j] = max(0, temp);
      cx_sel(args->cx_sel);
      reset(0, 0);
    }
  }
  return NULL;
}

void mat_check_equal(int **A, int **B, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      if (A[i][j] != B[i][j]) {
        printf("i: %d, j: %d, A: %d, B: %d\n", i, j, A[i][j], B[i][j]);
      }
      assert(A[i][j] == B[i][j]);
    }
  }
}

void free_mat(int **A, int n) {
  for (int i = 0; i < n; i++) {
    free(A[i]);
  }
  free(A);
}

void free_thread_args(thread_args **targs, int nThreads) {
  free_mat(targs[0]->A, targs[0]->n);
  free_mat(targs[0]->B, targs[0]->n);
  free_mat(targs[0]->C, targs[0]->n);
  for (int i = 0; i < nThreads; i++) {
    free(targs[i]);
  }
  free(targs);
}

void threaded_matmul(int m, int n, int nThreads, cx_select_t cx_sel_A0, cx_select_t cx_sel_max, cx_virt_t virt) {
    pthread_t tid[ nThreads ];
    void *ret;

    thread_args **mat_args = init_thread_args(m, n, nThreads, cx_sel_max);
    printf("Thread args set\n");

    for (int i = 0; i < nThreads; i++) {
      cx_select_t cx_sel_thread = cx_open(CX_GUID_MULACC, virt, cx_sel_A0);
      assert(cx_sel_thread > 0);
      mat_args[i]->cx_sel = cx_sel_thread;
      assert( pthread_create(&tid[i], NULL, nn_layer_worker, (void *)mat_args[i]) == 0 );
    }

    for (int i = 0; i < nThreads; i++) {
      assert( pthread_join(tid[i], &ret) == 0 );
    }

    int **C_ref = alloc_mat(m, n);

    // cleanup
    cx_close(cx_sel_A0);
    for (int i = 0; i < nThreads; i++) {
      cx_close(mat_args[i]->cx_sel);
    }
    cx_close(cx_sel_max);

    scalar_nn_layer(mat_args[0]->A, mat_args[0]->B, C_ref, m, n);
    mat_check_equal(mat_args[0]->C, C_ref, m, n);
    
    free_thread_args(mat_args, nThreads);
    free_mat(C_ref, n);
    free(ret);
}

void matmul_multiP_multiT_full() {
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_select_t cx_index, cx_index_0;

  cx_select_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_A0 > 0);

  cx_select_t cx_sel_max = cx_open(CX_GUID_MAX, CX_NO_VIRT, -1);
  assert(cx_sel_max > 0);

  cx_sel(cx_sel_A0);

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) { 
  // child process
    threaded_matmul(SIZE_0, SIZE_0, NTHREAD_0, cx_sel_A0, cx_sel_max, CX_FULL_VIRT);
    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    threaded_matmul(SIZE_1, SIZE_1, NTHREAD_1, cx_sel_A0, cx_sel_max, CX_FULL_VIRT);
    int status;
    // Wait for child
    waitpid(pid, &status, 0);
    assert(status == 0);
  }
  cx_sel(CX_LEGACY);
}


void matmul_multiP_multiT_intra() {
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_select_t cx_index, cx_index_0;

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) { 
  // child process
    cx_select_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t cx_sel_max = cx_open(CX_GUID_MAX, CX_NO_VIRT, -1);
    assert(cx_sel_max > 0);
    assert(cx_sel_A0 > 0);

    printf("forked in child\n");
    cx_sel(cx_sel_A0);
    threaded_matmul(SIZE_0, SIZE_0, NTHREAD_0, cx_sel_A0, cx_sel_max, CX_INTRA_VIRT);
    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    cx_select_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_INTRA_VIRT, -1);
    cx_select_t cx_sel_max = cx_open(CX_GUID_MAX, CX_NO_VIRT, -1);
    assert(cx_sel_max > 0);
    assert(cx_sel_A0 > 0);

    cx_sel(cx_sel_A0);
    printf("forked in parent\n");

    threaded_matmul(SIZE_1, SIZE_1, NTHREAD_1, cx_sel_A0, cx_sel_max, CX_INTRA_VIRT);
    int status;
    // Wait for child
    waitpid(pid, &status, 0);
    assert(status == 0);
  }
  cx_sel(CX_LEGACY);
}

int main() {
    cx_sel(CX_LEGACY);
    // test_matmul();
    matmul_multiP_multiT_intra();
    // matmul_multiP_multiT_full();
    printf("Finished Multi-thread Multi-process test\n");
    return 0;
}
