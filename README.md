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

=== MemGuard Report ===
[LEAK] 100 bytes at main.c:23
[DOUBLE-FREE] at utils.c:45
Originally allocated at utils.c:12

Total: 1 leaks, 100 bytes

=== Memory Statistics ===
Total allocated:  500 bytes
Peak memory:      450 bytes
Allocations:      5
Frees:            4

## How It Works

MemGuard wraps standard memory functions (`malloc`, `free`, etc.) to track every allocation. It maintains a linked list of allocations and detects:

- **Memory leaks**: Allocations never freed
- **Double frees**: Attempting to free already-freed memory
- **Invalid frees**: Freeing pointers not allocated by MemGuard

## Project Structure

memguard/
├── include/
│   ├── memguard.h           # Public API
│   └── memguard_internal.h  # Internal structures
├── src/
│   └── memguard.c           # Implementation
├── examples/
│   └── test_basic.c         # Example usage
└── README.md

## Requirements

- C compiler (GCC, Clang, MSVC)
- Standard C library

## License

MIT License - Free to use and modify

## Author

Nisarg Pandya 
GitHub :
LinkedIn : 
E-Mail :

## Contributing

Issues and pull requests welcome!

---

**Built with C** | Memory safety matters 🛡️
