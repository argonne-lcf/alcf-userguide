#include "float.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <random>
#include <vector>

#include "mkl.h"
#include "oneapi/mkl/blas.hpp"
#include <sycl/sycl.hpp>

#include <mpi.h>

template <typename fp_ab, typename fp_c, typename fp_scalar>
void run_gemm_example(
    sycl::queue Q, int size, std::string name,
    oneapi::mkl::blas::compute_mode mode = oneapi::mkl::blas::compute_mode::standard) {

  auto transA = oneapi::mkl::transpose::nontrans;
  auto transB = oneapi::mkl::transpose::nontrans;

  fp_scalar alpha = fp_scalar(1.0);
  fp_scalar beta = fp_scalar(0.0);

  auto A = sycl::malloc_shared<fp_ab>(size * size, Q);
  auto B = sycl::malloc_shared<fp_ab>(size * size, Q);
  auto C = sycl::malloc_shared<fp_c>(size * size, Q);

  if (!A || !B || !C)
    throw std::runtime_error("Failed to allocate USM memory.");

  fp_ab max_ab = std::numeric_limits<fp_ab>::max();

  // Workaround for some type
  if (max_ab == 0)
    max_ab = 100;

  fp_c max_c_array_value = std::sqrt(std::numeric_limits<fp_c>::max() / size);
  // assumes fp_c is bigger
  fp_ab max_array_value = std::min((fp_c)max_c_array_value, (fp_c)max_ab / 2);

  // A(size, size)
  for (size_t i = 0; i < (size * size); i++) {
    A[i] = fp_ab(max_array_value) * double((std::rand() / (double)RAND_MAX));
  }

  // B(size,size)
  for (size_t i = 0; i < (size * size); i++) {
    B[i] = fp_ab(max_array_value) * double((std::rand() / (double)RAND_MAX));
  }

  unsigned long min_time = std::numeric_limits<unsigned long>::max();

  int niter = 100;
  for (int i = 0; i < niter; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    const unsigned long l_start = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                      std::chrono::high_resolution_clock::now().time_since_epoch())
                                      .count();

    oneapi::mkl::blas::column_major::gemm(Q, transA, transB, size, size, size, alpha, A, size, B,
                                          size, beta, C, size, mode)
        .wait();
    const unsigned long l_end = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::high_resolution_clock::now().time_since_epoch())
                                    .count();
    unsigned long start, end;
    MPI_Reduce(&l_start, &start, 1, MPI_UNSIGNED_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&l_end, &end, 1, MPI_UNSIGNED_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

    const unsigned long time = end - start;
    min_time = std::min(time, min_time);
  }

  free(A, Q);
  free(B, Q);
  free(C, Q);

  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  const double flops = (2. * size * size * size * world_size) / min_time;
  if (world_rank == 0) {
    std::cout << name << ": " << flops << " GFlop/s" << std::endl;
  }
}

int main(int argc, char **argv) {

  MPI_Init(NULL, NULL);
  int size = 8192;
  sycl::queue Q;
  run_gemm_example<double, double, double>(Q, size, "DGEMM");
  run_gemm_example<float, float, float>(Q, size, "SGEMM");
  run_gemm_example<sycl::half, sycl::half, sycl::half>(Q, size, "HGEMM");

  run_gemm_example<oneapi::mkl::bfloat16, float, float>(Q, size, "BF16GEMM");
  run_gemm_example<float, float, float>(Q, size, "TF32GEMM",
                                        oneapi::mkl::blas::compute_mode::float_to_tf32);
  run_gemm_example<std::int8_t, float, float>(Q, size, "I8GEMM");
  MPI_Finalize();
}
