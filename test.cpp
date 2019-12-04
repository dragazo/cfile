#include <fstream>
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>

#include "cfile.h"

template<bool integral>
void write_benchmark(const char *file, std::size_t vals)
{
	using namespace std::chrono;
	std::mt19937 _r{ std::random_device{}() };
	std::conditional_t<integral, std::uniform_int_distribution<>, std::uniform_real_distribution<>> _d;
	auto r = [=] () mutable { return _d(_r); };

	const char *const fmt = integral ? "%u\n" : "%f\n";

	std::cout << "write benchmark " << (integral ? "(integral)" : "(floating point)") << std::endl;

	{
		auto start = high_resolution_clock::now();
		{
			std::ofstream f(file);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cout << "    ofstream: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::ofstream f(file, std::ios::binary);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin ofstream: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::out);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cout << "     fstream: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::out | std::ios::binary);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin  fstream: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "w");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "       FILE*: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin    FILE*: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "w");
			for (std::size_t i = 0; i < vals; ++i) f.printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cout << "       cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) f.printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin    cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "w");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cout << "    () cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin () cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	std::cout << '\n';
}

template<bool integral>
void read_benchmark(const char *file)
{
	using namespace std::chrono;

	typedef std::conditional_t<integral, std::size_t, double> type;
	const char *fmt = integral ? "%zu\n" : "%lf\n";

	std::cout << "read benchmark " << (integral ? "(integral)" : "(floating point)") << std::endl;

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::ifstream f(file);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "    ifstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::ifstream f(file, std::ios::binary);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin ifstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::in);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "     fstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::in | std::ios::binary);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin  fstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "r");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "       FILE*: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "rb");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin    FILE*: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "r");
			for (type temp; f.scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "       cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "rb");
			for (type temp; f.scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin    cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "r");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "    () cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "rb");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cout << "bin () cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms" << std::endl;
	}

	std::cout << '\n';
}

int main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " [test size]\n";
		return 1;
	}
	
	auto count = atoll(argv[1]);

	write_benchmark<true>("data.dat", count);
	read_benchmark<true>("data.dat");

	write_benchmark<false>("data-f.dat", count);
	read_benchmark<false>("data-f.dat");

	return 0;
}