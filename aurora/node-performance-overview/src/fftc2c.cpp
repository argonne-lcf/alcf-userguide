#include <complex>
#include <mpi.h>
#include <oneapi/mkl.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sycl/sycl.hpp>
#include <sys/time.h>

typedef oneapi::mkl::dft::descriptor<oneapi::mkl::dft::precision::SINGLE,
                                     oneapi::mkl::dft::domain::COMPLEX>
    descriptor_t;

// using batch version of oneMKL FFT
void fft_c2c_batch_onemkl(descriptor_t *desc, int size, int howmany, std::string input_string) {

  sycl::queue Q;

  size_t n_elem = size * howmany;
  auto *gpu_dpcpp = sycl::malloc_device<std::complex<float>>(n_elem, Q);
  auto *cpu_dpcpp = (std::complex<float> *)malloc(n_elem * sizeof(std::complex<float>));

  desc->commit(Q);

  // fill r-space array with random values between 0.5 and -0.5 for real and imaginary.
  for (size_t i = 0; i < n_elem; i++) {
    cpu_dpcpp[i] = std::complex<float>(float(std::rand() / (double)RAND_MAX) - 0.5,
                                       float(std::rand() / (double)RAND_MAX) - 0.5);
  }

  unsigned long min_time = std::numeric_limits<unsigned long>::max();

  int repetitions = 100;

  for (size_t rep = 0; rep < repetitions; rep++) {

    Q.copy(cpu_dpcpp, gpu_dpcpp, n_elem).wait();
    MPI_Barrier(MPI_COMM_WORLD);

    const unsigned long l_start = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                      std::chrono::high_resolution_clock::now().time_since_epoch())
                                      .count();
    compute_forward(*desc, gpu_dpcpp).wait();
    const unsigned long l_end = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::high_resolution_clock::now().time_since_epoch())
                                    .count();

    unsigned long start, end;

    MPI_Reduce(&l_start, &start, 1, MPI_UNSIGNED_LONG, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&l_end, &end, 1, MPI_UNSIGNED_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

    const unsigned long time = end - start;
    min_time = std::min(time, min_time);
  }

  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    // min_time is in nanseconds. Flop value is from http://www.fftw.org/speed/method.html
    const double flops = (5. * size * log2(size) * howmany * world_size) / min_time;
    std::cout << input_string << ": " << flops << " GFlop/s" << std::endl;
  }

  free(gpu_dpcpp, Q);
  free(cpu_dpcpp);
  return;
}

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);

  int size = 4096;
  int howmany_1d = 20000;
  int howmany_2d = 10;

  descriptor_t desc_1d(size);
  desc_1d.set_value(oneapi::mkl::dft::config_param::NUMBER_OF_TRANSFORMS, howmany_1d);
  desc_1d.set_value(oneapi::mkl::dft::config_param::FWD_DISTANCE, size);
  desc_1d.set_value(oneapi::mkl::dft::config_param::BWD_DISTANCE, size);

  fft_c2c_batch_onemkl(&desc_1d, size, howmany_1d, "Single-precision FFT C2C 1D");

  descriptor_t desc_2d({size, size});
  std::int64_t rstrides[3] = {0, size, 1};
  std::int64_t cstrides[3] = {0, size, 1};

  desc_2d.set_value(oneapi::mkl::dft::config_param::NUMBER_OF_TRANSFORMS, howmany_2d);
  desc_2d.set_value(oneapi::mkl::dft::config_param::FWD_DISTANCE, size * size);
  desc_2d.set_value(oneapi::mkl::dft::config_param::BWD_DISTANCE, size * size);
  desc_2d.set_value(oneapi::mkl::dft::config_param::FWD_STRIDES, rstrides);
  desc_2d.set_value(oneapi::mkl::dft::config_param::BWD_STRIDES, cstrides);

  fft_c2c_batch_onemkl(&desc_2d, size * size, howmany_2d, "Single-precision FFT C2C 2D");

  MPI_Finalize();
  return 0;
}
