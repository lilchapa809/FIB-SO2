<div align="center">

  <h1>💻 Operating Systems II (SO2)</h1>
  
  <p>
    <strong>Bachelors in Computer Science | Polytechnic University of Catalonia - FIB</strong>
  </p>

  <p>
    <a href="https://github.com/yourusername/repo/graphs/contributors">
      <img src="https://img.shields.io/badge/Contributors-1-blue?style=for-the-badge" />
    </a>
    <a href="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c">
      <img src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c" />
    </a>
    <a href="https://img.shields.io/badge/Language-x86_Assembly-red?style=for-the-badge">
      <img src="https://img.shields.io/badge/Language-x86_Assembly-red?style=for-the-badge" />
    </a>
     <a href="https://img.shields.io/badge/OS-Zeos_&_Linux-black?style=for-the-badge&logo=linux">
      <img src="https://img.shields.io/badge/OS-Zeos_&_Linux-black?style=for-the-badge&logo=linux" />
    </a>
  </p>
  
  <h3> 
    From User Space Concurrency to Kernel Space Implementation.
  </h3>
</div>

---

## 📖 About The Course

**Operating Systems II (SO2)** bridges the gap between application-level programming and kernel-level architecture. This subject is distinct because it is split into two critical environments:

1.  **Linux (User Space):** Advanced concurrency, synchronization, and process management using POSIX standards.
2.  **Zeos (Kernel Space):** Development of a small, educational x86 operating system. We implemented the kernel "guts" from a basic skeleton.



### 🧠 Key Learning Modules

| Module | Environment | Focus Concepts |
| :--- | :--- | :--- |
| **Kernel Entry** | **Zeos** | `entry.S`, Interrupts, Traps, and Context Switching (SAVE_ALL/RESTORE_ALL). |
| **System Calls** | **Zeos** | Implementing `fork`, `exit`, `write`, `getpid` inside the kernel. |
| **Scheduling** | **Zeos** | Implementing a Round Robin scheduler, `task_struct` management, and Runqueues. |
| **Concurrency** | **Linux** | Thread safety, Mutexes, Semaphores, and Monitors (Pthreads). |

---

## 📂 Repository Structure

This repository is organized to separate the educational OS development from the Linux exercises:

```text
.
├── 📂 E1_E2/                           # Phase 1: Core Kernel Development
│   └── 📂 zeos/
│       ├── 📂 include/                 # Kernel Header files (.h)
│       │   ├── entry.h                 # Defines for saving/restoring context
│       │   ├── sched.h                 # Task struct & runqueue definitions
│       │   └── ...
│       ├── 📜 entry.S                  # (CRITICAL) ASM Interrupt Handlers & Context Switch
│       ├── 📜 sched.c                  # (CRITICAL) Round Robin Scheduler & Task Management
│       ├── 📜 sys.c                    # (CRITICAL) System Calls (fork, exit, write, etc.)
│       ├── 📜 user.c                   # User-space code to test the kernel features
│       ├── 📜 interrupt.c              # IDT (Interrupt Descriptor Table) setup
│       ├── 📜 mm.c                     # Memory Management (Paging/Frames)
│       ├── 📜 system.lds               # Linker script (defines kernel memory map)
│       └── ...
│
├── 📂 EXAM/                            # 25-26Q1 Exam (Time-constrained coding)
│   ├── 📂 Ex1/                         # Excercise 1: Process management & Scheduling
│   │   ├── 📂 zeos/
│   │   │   ├── 📜 p_stats.c            # Custom statistics (CPU usage, blocking time)
│   │   │   ├── 📜 user-utils.S         # Assembly wrappers for custom syscalls
│   │   │   └── ...
│   │   └── 📜 ejercicio1.tar.gz        # Compressed submission
│   │
│   └── 📂 Ex2/                         # Excercise 2: Advanced Synchronization
│       └── 📂 zeos/                    # (Similar kernel structure with extended features)
│
├── 📂 GAME/                            # Final Project: "The Game"
│   └── 📂 zeos/                        # A fully functional OS running a game
│       ├── 📜 bootsect.S               # Bootloader assembly code
│       ├── 📜 user.c                   # The Game Logic (Drawing to screen via syscalls)
│       ├── 📜 devices.c                # Console I/O & Device abstraction
│       ├── 📜 sys_call_table.S         # Mapping syscall numbers to C functions
│       ├── ...
│       └── 📄 Project.pdf              # Documentation of the game mechanics
│
└── 📄 README.md                        # Documentation