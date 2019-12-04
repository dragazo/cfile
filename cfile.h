#ifndef DRAGAZO_CFILE_H
#define DRAGAZO_CFILE_H

#include <cstdio>
#include <utility>
#include <memory>
#include <cstdarg>
#include <type_traits>

// represents an owning wrapper for a C-style FILE*.
// includes wrapper functions for convenience.
class cfile
{
private: // -- data -- //

	struct deleter { void operator()(std::FILE *f) { std::fclose(f); } };

	std::unique_ptr<std::FILE, deleter> f; // the raw file handle

public: // -- ctor / dtor / asgn -- //

	// creates an unlinked file handle.
	constexpr cfile() = default;

	// constructs a new file handle from the given file.
	// if file is non-null, this instance of cfile now owns file, otherwise unlinked state.
	// WARNING: this is an owning relationship - DO NOT call this with the result of .get() from another cfile.
	explicit cfile(std::FILE *file) : f(file) {}

	// opens a file and links it to this file handle.
	// equivalent to calling open(filename, mode).
	cfile(const char *filename, const char *mode) : f(std::fopen(filename, mode)) {}

	// constructs a file handle by stealing the file handle of other.
	// other is left in the unlinked state after this operation.
	cfile(cfile &&other) noexcept : f(std::move(other.f)) {}
	// steals the file handle from other.
	// if this file handle is already linked to a file, it is first closed.
	cfile &operator=(cfile &&other) noexcept { f = std::move(other.f); return *this; }

	cfile(const cfile&) = delete;
	cfile operator=(const cfile&) = delete;

public: // -- accessors -- //

	// returns the raw FILE* of this file handle if linked, otherwise null.
	std::FILE *get() const& noexcept { return f.get(); }
	std::FILE *get() && noexcept = delete; // to help avoid dangling file pointers

	// cfile wrappers are implicitly convertible to FILE* for convenience.
	operator std::FILE*() const& noexcept { return get(); }
	operator std::FILE*() && noexcept = delete; // to help avoid dangling file pointers

	// returns the raw FILE* of this file handle if linked, otherwise null.
	// after this operation, this file handle is set to unlinked state without closing the file.
	std::FILE *release() noexcept { return f.release(); }

	// returns true if this file handle is currently linked (to an open file).
	explicit operator bool() const noexcept { return get() != nullptr; }
	// returns true if this file handle is currently unlinked.
	bool operator!() const noexcept { return get() == nullptr; }

public: // -- file state -- //

	// opens a file and associates it with this file handle.
	// if this handle is currently associated with a file, that file is first closed.
	// if file fails to open, this handle is left in unlinked state.
	cfile &open(const char *filename, const char *mode) &
	{
		f.reset(std::fopen(filename, mode));
		return *this;
	}
	cfile &open(const char *filename, const char *mode) && = delete;

	// attempts to reuse the file to open a new file or to change the mode of an already open file.
	// identical to calling freopen() with the stored file pointer.
	// returns true if the operation succeeds.
	bool reopen(const char *filename, const char *mode) & { return std::freopen(filename, mode, get()) != nullptr; }
	bool reopen(const char *filename, const char *mode) && = delete;
	// convenience function - passes null as filename param to reopen() to facilitate changing file mode for an opened file.
	// returns tru if the operation succeeds.
	bool chmode(const char *mode) & { return std::freopen(nullptr, mode, get()) != nullptr; }
	bool chmode(const char *mode) && = delete;

	// closes (and flushes) the stream (if any) and enters the unlinked state.
	void close() { f.reset(); }

	// flushes the stream.
	void flush() { std::fflush(get()); }

	// sets the buffer used by this stream (assumed to be at least length BUFSIZ).
	// equivalent to calling setbuf() with the stored file pointer.
	template<int _ = 0>
	void setbuf(char *buffer) { std::setbuf(get(), buffer); }
	template<int len, typename std::enable_if_t<(len < BUFSIZ), int> = 0>
	void setbuf(char(&buffer)[len]) = delete; // to help avoid using buffer that is too small

	// sets the buffer used by this stream.
	// equivalent to calling setvbuf() with the stored file pointer.
	int setvbuf(char *buffer, int mode, std::size_t size) { return std::setvbuf(get(), buffer, mode, size); }

	// gets the current position in the stream.
	// equivalent to calling fgetpos() with the stored file pointer.
	int getpos(std::fpos_t *pos) const { return std::fgetpos(get(), pos); }
	// sets the current position in the stream.
	// equivalent to calling fsetpos() with the stored file pointer.
	int setpos(const std::fpos_t *pos) { return std::fsetpos(get(), pos); }

	// gets the current position in the file.
	// equivalent to calling ftell() with the stored file pointer.
	long int tell() const { return std::ftell(get()); }
	// sets the current position in the file.
	// equivalent to calling fseek() with the stored file pointer.
	int seek(long int offset, int origin = SEEK_SET) { return std::fseek(get(), offset, origin); }

	// repositions the stream to the beginning.
	// equivalent to calling rewind() with the stored file pointer.
	void rewind() { std::rewind(get()); }

	// clears the error state of the linked file.
	// equivalent to calling clearerr() with the stored file pointer.
	void clearerr() { std::clearerr(get()); }

	// checks if the file has reached eof.
	// equivalent to calling feof() with the stored file pointer.
	int eof() const { return std::feof(get()); }
	// checks if there was an error.
	// equivalent to calling ferror() with the stored file pointer.
	int error() const { return std::ferror(get()); }

public: // -- input -- //

	// gets a character from the file.
	// functions identically to calling fgetc() with the stored file pointer.
	int getc() { return std::fgetc(get()); }

	// puts a character that was just read back into the stream.
	// functions identically to calling ungetc() with the stored file pointer.
	int ungetc(int ch) { return std::ungetc(ch, get()); }
	// convenience function - equivalent to getting a character, then putting it back, and returning it.
	int peek() { return std::ungetc(std::fgetc(get()), get()); }

	// reads at most num-1 chars into the specified buffer.
	// functions identically to calling fgets() with the stored file pointer.
	char *gets(char *str, int num) { return std::fgets(str, num, get()); }
	// convenience function - given a buffer of known size calls gets() with correct size arg.
	template<int len>
	char *gets(char(&str)[len]) { return std::fgets(str, len, get()); }

	// reads (count) elements of size (size) from the file (binary data).
	// functions identically to calling fread() with the stored file pointer.
	std::size_t read(void *ptr, std::size_t size, std::size_t count) { return std::fread(ptr, size, count, get()); }
	// convenience function - passes correct size parameter to read() based on T.
	// additionally, guarantees that T is a valid type to be read in this manner.
	template<typename T, std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
	std::size_t read(T *ptr, std::size_t count) { return std::fread(ptr, sizeof(T), count, get()); }
	// convenience function - passes correct size and count parameters to read() based on T and length of array.
	// additionally, guarantees that T is a valid type to be read in this manner.
	template<typename T, int len, std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
	std::size_t read(T(&ptr)[len]) { return std::fread(ptr, sizeof(T), len, get()); }

	// prints a formatted string to the file.
	// equivalent to calling fprintf() with the stored file pointer.
	int printf(const char *fmt, ...)
	{
		va_list v;
		va_start(v, fmt);
		int r = std::vfprintf(get(), fmt, v);
		va_end(v);
		return r;
	}

public: // -- output -- //

	// writes a char to the file.
	// functions identically to calling fputc() with the stored file pointer.
	int putc(int ch) { return std::fputc(ch, get()); }

	// writes a string to the file.
	// functions identically to calling fputs() with the stored file pointer.
	int puts(const char *str) { return std::fputs(str, get()); }

	// writes (count) elements of size (size) to the file (binary data).
	// functions identically to calling fwrite() with the stored file pointer.
	std::size_t write(void *ptr, std::size_t size, std::size_t count) { return std::fwrite(ptr, size, count, get()); }
	// convenience function - passes correct size parameter to write() based on T.
	// additionally, guarantees that T is a valid type to be written in this manner.
	template<typename T, std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
	std::size_t write(T *ptr, std::size_t count) { return std::fwrite(ptr, sizeof(T), count, get()); }
	// convenience function - passes correct size and count parameters to write() based on T and length of array.
	// additionally, guarantees that T is a valid type to be written in this manner.
	template<typename T, int len, std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
	std::size_t write(T(&ptr)[len]) { return std::fwrite(ptr, sizeof(T), len, get()); }

	// reads a formatted string from the file.
	// equivalent to calling fscanf() with the stored file pointer.
	int scanf(const char *fmt, ...)
	{
		va_list v;
		va_start(v, fmt);
		int r = std::vfscanf(get(), fmt, v);
		va_end(v);
		return r;
	}
};

#endif
