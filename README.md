<div align="center">

  <h1>💻 Operating Systems II (SO2)</h1>
  
  <p>
    <strong>Bachelors in Computer Science | [Your University Name]</strong>
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
├── 📁 zeos/               # The Educational OS Source Code
│   ├── entry.S            # Assembly routines for system calls/interrupts
│   ├── sys.c              # C implementation of system calls
│   ├── sched.c            # Scheduler logic and process switching
│   ├── user.c             # User-space program to test the kernel
│   └── interrupt.c        # IDT and interrupt handling
├── 📁 linux_concurrency/  # Standard Linux Synchronization
│   ├── philosophers/      # Solution to Dining Philosophers
│   └── producer_consumer/ # Shared buffer implementations
├── 📁 docs/               # Subject Theory & Diagrams
└── README.md