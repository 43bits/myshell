🐚 MyShell — Custom Unix Shell (OS Project)
# MyShell 🐚
> A fully functional Unix shell built from scratch in C — featuring pipes, I/O redirection, background jobs, signal handling, and job control.

![Language](https://img.shields.io/badge/Language-C%20%28C11%29-blue)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL-green)
![Standard](https://img.shields.io/badge/Standard-POSIX-orange)
![License](https://img.shields.io/badge/License-MIT-lightgrey)

---

## 📸 Preview

```
╔══════════════════════════════════════════════╗
║        MyShell v1.0 — Custom Unix Shell       ║
║     Type 'help' for available commands        ║
╚══════════════════════════════════════════════╝

user@myshell:~ $ ls | grep .c | wc -l
7
user@myshell:~ $ sleep 5 &
[1] 3821
user@myshell:~ $ jobs
[1] Running   (pgid 3821)  sleep 5
user@myshell:~ $ echo $HOME
/home/user
```

---

## ✨ Features

| Feature | Example |
|---------|---------|
| Run any program | `gcc main.c -o app` |
| Multi-stage pipes | `ls -la \| grep .c \| wc -l` |
| Output redirection | `echo hello > file.txt` |
| Append redirection | `echo world >> file.txt` |
| Input redirection | `sort < data.txt` |
| Background jobs | `sleep 10 &` |
| Environment variables | `echo $HOME` |
| Built-in: cd | `cd ~/projects` or `cd -` |
| Built-in: history | `history` |
| Built-in: jobs | `jobs` |
| Built-in: export | `export NAME=value` |
| Signal handling | Ctrl+C won't kill shell |

---

## 🏗️ Architecture

```
myshell/
├── Makefile          # Build system
├── shell.h           # Shared structs, constants, prototypes
├── shell.c           # Main REPL loop, prompt, history
├── parser.c          # Tokenizer: pipes, redirects, env vars, &
├── executor.c        # fork() + execvp() + pipe chaining
├── builtins.c        # cd, exit, help, history, jobs, export
├── jobs.c            # Background job table management
└── signals.c         # SIGINT, SIGCHLD, SIGTSTP handlers
```

### Data Flow

```
User Input
    │
    ▼
parse_input()          ← parser.c
    │  builds Pipeline struct
    │  (array of Command structs)
    ▼
execute_pipeline()     ← executor.c
    │  creates N-1 pipes
    │  forks N children
    │  wires dup2() for each
    ▼
execvp()               ← OS kernel
    │  replaces child with program
    ▼
waitpid()              ← parent waits
    │  (or add_job() if background)
    ▼
Prompt again
```

---

## 🚀 Quick Start

### Requirements
- Linux or WSL (Ubuntu recommended)
- GCC with C11 support

### Build & Run

```bash
git clone https://github.com/yourusername/myshell.git
cd myshell
make
./myshell
```

### Build options

```bash
make          # Standard build
make run      # Build and run immediately
make clean    # Remove binaries
```

---

## 🔬 OS Concepts Demonstrated

### 1. Process Management
Every command spawns a real OS process via `fork()` + `execvp()`. The shell waits with `waitpid()`.

### 2. Inter-Process Communication (IPC)
Pipes are created with `pipe()` — a kernel-level byte channel. Multi-command pipelines chain N processes through N-1 pipes simultaneously.

### 3. File Descriptor Manipulation
`dup2()` rewires stdin/stdout to pipe ends or files before `execvp()`. The program never knows its I/O was redirected.

### 4. Signal Handling
`sigaction()` installs custom handlers:
- **SIGINT** (Ctrl+C): shell ignores it; children use default (die)
- **SIGCHLD**: auto-reap finished background jobs
- **SIGTSTP** (Ctrl+Z): handled gracefully

### 5. Job Control
`setpgid()` assigns all pipeline processes to one process group. Background jobs are tracked in a job table and reported when done.

---

## 📖 Built-in Commands

```
cd [dir]      Change directory (~ for home, - for previous)
pwd           Print working directory
export K=V    Set environment variable
history       Show command history
jobs          List background jobs
clear         Clear terminal
help          Show all commands
exit [code]   Exit with optional status code
```

---

## 🧠 Key System Calls Used

| Call | Purpose |
|------|---------|
| `fork()` | Create child process |
| `execvp()` | Replace child with program |
| `waitpid()` | Wait for child, prevent zombies |
| `pipe()` | Create IPC channel |
| `dup2()` | Redirect file descriptors |
| `sigaction()` | Install signal handlers |
| `setpgid()` | Process group management |
| `open()` / `close()` | File redirection |
| `getenv()` / `setenv()` | Environment variables |
| `chdir()` | Change directory (cd builtin) |

---

## 📚 References

- W. Richard Stevens — *Advanced Programming in the Unix Environment*
- Abraham Silberschatz — *Operating System Concepts*, 10th Ed.
- Linux man-pages: `fork(2)`, `execvp(3)`, `pipe(2)`, `dup2(2)`, `sigaction(2)`
- GNU Bash source — git.savannah.gnu.org/cgit/bash.git

---

## 👤 Author

Built as an Operating Systems assignment mini-project.  
Demonstrates 5 core OS concepts in ~600 lines of pure C.

---

*"Every time you open a terminal, you're using software like this. Now you know exactly how it works."*