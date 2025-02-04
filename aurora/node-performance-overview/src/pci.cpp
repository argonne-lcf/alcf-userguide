#include <cstdint>
#include <limits>
#include <mpi.h>
#include <numeric>
#include <random>
#include <sycl/sycl.hpp>
#include <vector>

void fill_randomly(sycl::queue Q, int N, std::vector<int *> ptrs) {
  std::vector<int> v(N);
  std::iota(v.begin(), v.end(), 0);

  std::minstd_rand g;
  for (auto &ptr : ptrs) {
    std::shuffle(v.begin(), v.end(), g);
    Q.memcpy(ptr, v.data(), N * sizeof(int)).wait();
  }
}

unsigned long datatransfer(sycl::queue Q, int N_byte, std::vector<std::pair<int *, int *>> ptrs) {

  unsigned long min_time = std::numeric_limits<unsigned long>::max();
  int num_iteration = 10;

  for (int r = 0; r < num_iteration; r++) {
    MPI_Barrier(MPI_COMM_WORLD);
    const unsigned long l_start =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    for (auto [dest, src] : ptrs)
      Q.memcpy(dest, src, N_byte);
    Q.wait();
    const unsigned long l_end =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    unsigned long start, end;
    MPI_Reduce(&l_start, &start, 1, MPI_UNSIGNED_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&l_end, &end, 1, MPI_UNSIGNED_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

    const unsigned long time = end - start;
    min_time = std::min(time, min_time);
  }
  return min_time;
}

int main() {
  MPI_Init(NULL, NULL);
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  sycl::queue Q;
  const int N = 1 << 28;
  const int N_byte = N * sizeof(int);

  int *a_cpu = sycl::malloc_host<int>(N, Q);
  int *b_cpu = sycl::malloc_host<int>(N, Q);
  int *a_gpu = sycl::malloc_device<int>(N, Q);
  int *b_gpu = sycl::malloc_device<int>(N, Q);
  fill_randomly(Q, N, {a_cpu, b_cpu, a_gpu, b_gpu});

  auto H2D_time = datatransfer(Q, N_byte, {{a_gpu, a_cpu}});
  if (world_rank == 0) {
    const double H2D_bw = (N_byte * world_size) / H2D_time;
    std::cout << "PCIe Unidirectional Bandwidth (H2D): " << H2D_bw << " GB/s" << std::endl;
  }
  auto D2H_time = datatransfer(Q, N_byte, {{a_cpu, a_gpu}});
  if (world_rank == 0) {
    const double D2H_bw = (N_byte * world_size) / D2H_time;
    std::cout << "PCIe Unidirectional Bandwidth (D2H): " << D2H_bw << " GB/s" << std::endl;
  }
  auto H2D_D2H_time = datatransfer(Q, N_byte, {{a_gpu, a_cpu}, {b_cpu, b_gpu}});
  if (world_rank == 0) {
    const double H2D_D2H_bw = (2L * N_byte * world_size) / H2D_D2H_time;
    std::cout << "PCIe Bidirectional Bandwidth: " << H2D_D2H_bw << " GB/s" << std::endl;
  }
}
