# MemGuard - Memory Debugger for C

A lightweight memory debugging tool that detects memory leaks, double-frees, and other memory issues in C programs.

## Features

✅ Memory leak detection  
✅ Double-free detection  
✅ Memory usage statistics  
✅ Detailed error reporting with file and line numbers  
✅ Zero configuration required  

## Quick Start

### Installation

1. Clone the repository:
```bash
git clone https://github.com/Nisarg-Pandyaa/memguard.git
cd memguard
```

2. Include in your C project:
```c
#define MEMGUARD_ENABLED
#include "memguard.h"

int main() {
    mg_init();
    
    // Your code here
    int* data = malloc(100 * sizeof(int));
    free(data);
    
    mg_cleanup();  // Shows leak report
    return 0;
}
```

3. Compile with MemGuard:
```bash
gcc -I./include -o myprogram examples/your_file.c src/memguard.c
```

## Example Output

=== MemGuard Report === </br>
[LEAK] 100 bytes at main.c:23 </br>
[DOUBLE-FREE] at utils.c:45 </br>
Originally allocated at utils.c:12 </br> </br>

Total: 1 leaks, 100 bytes </br> </br>

=== Memory Statistics === </br>
Total allocated:  500 bytes </br>
Peak memory:      450 bytes </br>
Allocations:      5 </br>
Frees:            4 </br> </br>

## How It Works

MemGuard wraps standard memory functions (`malloc`, `free`, etc.) to track every allocation. It maintains a linked list of allocations and detects:

- **Memory leaks**: Allocations never freed
- **Double frees**: Attempting to free already-freed memory
- **Invalid frees**: Freeing pointers not allocated by MemGuard

## Known Limitations

### 1. Extreme Buffer Overflows
MemGuard detects buffer overflows using canary values placed 
around allocated memory. However, extreme overflows (writing 
far past the buffer end) may corrupt MemGuard's own internal 
tracking structures.

For production debugging of extreme overflows, use:
- **AddressSanitizer** (built into GCC/Clang)
- **Valgrind** (Linux)

### 2. Heap Memory Only
MemGuard only tracks heap memory (malloc/free).
These are NOT monitored:
- Stack variables: `int x = 5;`
- Global variables: `int global = 0;`
- String literals: `char* s = "hello";`

### 3. Performance Overhead
MemGuard adds overhead to every malloc/free call.
Use only during development/debugging, not in production.

## Future Improvements (Version 2.0)
- [ ] Separate tracking memory using mmap()
- [ ] Stack variable monitoring
- [ ] GUI report viewer
- [ ] CI/CD integration
- [ ] Thread safety

## Project Structure

memguard/ </br>
├── include/ </br>
│   ├── memguard.h           # Public API </br>
│   └── memguard_internal.h  # Internal structures </br>
├── src/ </br>
│   └── memguard.c           # Implementation </br>
├── examples/ </br>
│   └── test_basic.c         # Example usage </br>
└── README.md </br> </br>

## Requirements

- C compiler (GCC, Clang, MSVC)
- Standard C library

## License

MIT License - Free to use and modify

## Author

Nisarg Pandya </br> </br>
GitHub : https://github.com/Nisarg-Pandyaa</br>
LinkedIn : www.linkedin.com/in/nisarg-s-pandya</br>
E-Mail : nisargspandyaa@gmail.com</br>

## Contributing

Issues and pull requests welcome!

---

**Built with C** | Memory safety matters 🛡️
