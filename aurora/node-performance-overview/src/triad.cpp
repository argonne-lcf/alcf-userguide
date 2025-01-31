#include <algorithm>
#include <assert.h>
#include <iostream>
#include <limits>
#include <mpi.h>
#include <omp.h>
#include <vector>

bool almost_equal(double x, double gold, double rel_tol = 1e-09, double abs_tol = 0.0) {
  return std::abs(x - gold) <= std::max(rel_tol * std::max(std::abs(x), std::abs(gold)), abs_tol);
}

int main() {
  MPI_Init(NULL, NULL);
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  const int globalWI{65536000};
  const int num_iteration{100};

  std::vector<double> A(globalWI), B(globalWI), C(globalWI);
  std::generate(B.begin(), B.end(), std::rand);
  std::generate(C.begin(), C.end(), std::rand);
  double *Aptr{A.data()};
  double *Bptr{B.data()};
  double *Cptr{C.data()};
#pragma omp target enter data map(alloc : Aptr[ : globalWI])                                       \
    map(to : Bptr[ : globalWI], Cptr[ : globalWI])
  double min_time = std::numeric_limits<double>::max();
  for (int r = 0; r < num_iteration; r++) {
    MPI_Barrier(MPI_COMM_WORLD);
    const double l_start = omp_get_wtime();
#pragma omp target teams distribute parallel for
    for (int i = 0; i < globalWI; i++)
      Aptr[i] = 2.0 * Bptr[i] + Cptr[i];
    const double l_end = omp_get_wtime();
    double start, end;
    MPI_Reduce(&l_start, &start, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&l_end, &end, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    const double time = end - start;
    min_time = std::min(time, min_time);
  }
#pragma omp target exit data map(from : Aptr[ : globalWI])
  const double bw = (3 * sizeof(double) * globalWI * world_size * 1E-9) / min_time;
  if (world_rank == 0)
    std::cout << "Memory Bandwidth (triad): " << bw << " GB/s"<< std::endl;

  for (int i = 0; i < globalWI; i++) {
    assert(almost_equal(Aptr[i], 2.0 * Bptr[i] + Cptr[i], 0.01));
  }
  MPI_Finalize();
}
