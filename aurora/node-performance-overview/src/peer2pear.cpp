#include <cstdint>
#include <limits>
#include <mpi.h>
#include <random>
#include <sycl/sycl.hpp>
#include <vector>

void fill_randomly(sycl::queue Q, int N, std::vector<float *> ptrs) {
  std::vector<float> v(N);
  std::iota(v.begin(), v.end(), 0);

  std::minstd_rand g;
  for (auto &ptr : ptrs) {
    std::shuffle(v.begin(), v.end(), g);
    Q.memcpy(ptr, v.data(), N * sizeof(float)).wait();
  }

}

unsigned long datatransfer(int N, std::vector<std::pair<int, float *>> &sends,
                           std::vector<std::pair<int, float *>> &recvs) {

  unsigned long min_time = std::numeric_limits<unsigned long>::max();
  int num_iteration = 10;

  for (int r = 0; r < num_iteration; r++) {
    MPI_Barrier(MPI_COMM_WORLD);
    const unsigned long l_start =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::vector<MPI_Request> requests;

    for (auto &[dest, ptr] : sends) {
      MPI_Request request = MPI_REQUEST_NULL;
      MPI_Isend(ptr, N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD, &request);
      requests.push_back(request);
    }

    for (auto &[src, ptr] : recvs) {
      MPI_Request request = MPI_REQUEST_NULL;
      MPI_Irecv(ptr, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, &request);
      requests.push_back(request);
    }

    MPI_Waitall(requests.size(), requests.data(), MPI_STATUS_IGNORE);

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

int main(int argc, char *argv[]) {

  std::string mode = (argc == 1) ? "Tile2Tile" : argv[1];
  MPI_Init(NULL, NULL);
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int num_pair = world_size / 2;

  sycl::queue Q(sycl::gpu_selector_v);
  const int N = 1 << 25;
  const int N_byte = N * sizeof(float);
  auto *a_gpu = sycl::malloc_device<float>(N, Q);
  fill_randomly(Q, N, {a_gpu});

  std::vector<std::pair<int, float *>> sends;
  std::vector<std::pair<int, float *>> recvs;

  if (world_rank % 2 == 0)
    sends.push_back({world_rank + 1, a_gpu});
  else
    recvs.push_back({world_rank - 1, a_gpu});

  auto unitime = datatransfer(N, sends, recvs);
  if (world_rank == 0) {
    const double unitime_bw = (N_byte * num_pair) / unitime;
    std::cout << mode << " Unidirectional Bandwidth: " << unitime_bw << " GB/s" << std::endl;
  }
  if (world_rank % 2 == 0)
    recvs.push_back({world_rank + 1, a_gpu});
  else
    sends.push_back({world_rank - 1, a_gpu});

  auto bitime = datatransfer(N, sends, recvs);
  if (world_rank == 0) {
    const double bitime_bw = (2L * N_byte * num_pair) / bitime;
    std::cout << mode << " Bidirectional Bandwidth: " << bitime_bw << " GB/s" << std::endl;
  }
}
