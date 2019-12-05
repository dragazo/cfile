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

	std::cerr << "write benchmark " << (integral ? "(integral)\n" : "(floating point)\n");

	{
		auto start = high_resolution_clock::now();
		{
			std::ofstream f(file);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "    ofstream: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::ofstream f(file, std::ios::binary);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin ofstream: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::out);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "     fstream: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::out | std::ios::binary);
			for (std::size_t i = 0; i < vals; ++i) f << r() << '\n';
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin  fstream: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "w");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "       FILE*: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::FILE *f = std::fopen(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
			fclose(f);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin    FILE*: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "w");
			for (std::size_t i = 0; i < vals; ++i) f.printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "       cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) f.printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin    cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "w");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "    () cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "wb");
			for (std::size_t i = 0; i < vals; ++i) std::fprintf(f, fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin () cfile: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		auto start = high_resolution_clock::now();
		{
			std::freopen(file, "w", stdout);
			for (std::size_t i = 0; i < vals; ++i) std::printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "      stdout: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		auto start = high_resolution_clock::now();
		{
			std::freopen(file, "wb", stdout);
			for (std::size_t i = 0; i < vals; ++i) std::printf(fmt, r());
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin   stdout: " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	std::cerr << '\n';
}

template<bool integral>
void read_benchmark(const char *file)
{
	using namespace std::chrono;

	typedef std::conditional_t<integral, std::size_t, double> type;
	const char *fmt = integral ? "%zu\n" : "%lf\n";

	std::cerr << "read benchmark " << (integral ? "(integral)\n" : "(floating point)\n");

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::ifstream f(file);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "    ifstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::ifstream f(file, std::ios::binary);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin ifstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::in);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "     fstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::fstream f(file, std::ios::in | std::ios::binary);
			for (type temp; f >> temp; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin  fstream: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
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
		std::cerr << "       FILE*: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
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
		std::cerr << "bin    FILE*: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "r");
			for (type temp; f.scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "       cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "rb");
			for (type temp; f.scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin    cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "r");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "    () cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			cfile f(file, "rb");
			for (type temp; std::fscanf(f, fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin () cfile: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::freopen(file, "r", stdin);
			for (type temp; std::scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "       stdin: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}
	{
		type sum = 0;
		auto start = high_resolution_clock::now();
		{
			std::freopen(file, "rb", stdin);
			for (type temp; std::scanf(fmt, &temp) == 1; sum += temp);
		}
		auto stop = high_resolution_clock::now();
		std::cerr << "bin    stdin: " << sum << " - " << duration_cast<milliseconds>(stop - start).count() << " ms\n";
	}

	std::cerr << '\n';
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