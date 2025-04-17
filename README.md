# debt

![License](https://img.shields.io/github/license/jeebuscrossaint/debt)
![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20macOS%20%7C%20Windows-green)

> **D**on't **E**ven **B**other with **T**erminal prefixes

Never worry about accidentally copying those pesky command prompt symbols again.

## What is debt?

`debt` is a cross-platform command-line utility that strips the `$` and `>` prefixes from terminal commands you copy from the web. It solves that frustrating moment when you paste a command from Stack Overflow or documentation that includes the prompt symbol, resulting in "command not found" errors.

```bash
# Without debt, this happens:
$ $ ls -la
-bash: $: command not found

# With debt installed:
$ $ ls -la
# Works as if you typed: ls -la
```

## Installation

### Using Nix (recommended)

```bash
$ nix profile install github:jeebuscrossaint/debt
```

### From Source

#### Prerequisites
- C compiler (GCC, Clang, MSVC, etc.)
- CMake 3.10 or higher

```bash
$ git clone https://github.com/jeebuscrossaint/debt.git
$ cd debt
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo make install
```

## Usage

After installation, you can use `debt` in three ways:

### 1. Direct command execution

```bash
$ debt ls -la
```

### 2. Handling commands with $ or > prefixes

```bash
$ debt $ ls -la
$ debt > echo "hello world"
```

### 3. Transparent usage (recommended)

The installation creates symbolic links named `$` and `>` that point to the `debt` executable. This allows you to directly paste commands with prefixes:

```bash
$ $ ls -la
# Works as if you typed: ls -la

$ > npm install express
# Works as if you typed: npm install express
```

## How It Works

When you install `debt`, it:

1. Creates symbolic links (or copies on Windows) named `$` and `>` that point to the debt executable
2. When you run a command prefixed with `$` or `>`, debt:
   - Detects if it's being called via these special names
   - Strips the prefix
   - Executes the rest of the command efficiently using your system's shell

## Performance

`debt` is designed to be lightweight with minimal overhead:

- Direct execution without intermediate shells when possible
- Falls back to shell execution for complex commands with pipes, redirects, etc.
- Efficient signal handling to ensure proper behavior with Ctrl+C

## Examples

### Copy-pasting from tutorials

```
# From a tutorial:
$ npm install express
$ node server.js

# You can copy-paste directly:
$ $ npm install express
$ $ node server.js
```

### Copy-pasting Windows commands

```
> cd project
> npm start

# Copy-paste directly:
$ > cd project
$ > npm start
```

## Comparison with Similar Tools

- **undollar** ([github.com/xtyrrell/undollar](https://github.com/xtyrrell/undollar)): The original inspiration, written in JavaScript. `debt` builds on this concept with cross-platform support, performance optimizations, and support for both `$` and `>` prefixes.

- **Other shell aliases**: Many users create shell aliases like `$='command'`, but these don't work across shells and can have unexpected behavior. `debt` works regardless of your shell and provides consistent behavior.

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by [undollar](https://github.com/xtyrrell/undollar) by [xtyrrell](https://github.com/xtyrrell)
- Thanks to everyone who's ever been frustrated by copying terminal commands with prefixes 😄

---

## FAQ

**Q: Why is it called "debt"?**
A: **D**on't **E**ven **B**other with **T**erminal prefixes. Also, the dollar sign (`$`) is often associated with money and debt, and we're getting rid of it!

**Q: Will this work on all platforms?**
A: Yes! `debt` is designed to work on Linux, macOS, BSD, and Windows systems.

**Q: Is it secure to use?**
A: `debt` simply forwards commands to your system's default shell. It doesn't modify the command other than removing the leading `$` or `>`.

**Q: Why not just create a shell alias?**
A: Shell aliases only work in specific shells, need to be configured for each user, and don't handle complex edge cases. `debt` works anywhere, for any user, consistently.
