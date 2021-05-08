# Intro-To-Computer-Systems
Assignment code for UBC's CPSC 213: Introduction to Computer Systems

This challenging course seeks to teach students what really happens in a computer when a program executes. The first half discusses memory management using features of C (and how they differ from Java), static and dynamic control flow, and procedures' use of the stack. A large focus of the course is on using Assembly to examine these concepts and write low-level programs.

The second half of the course introduces asynchronous programming, threads, and synchronization. In assignments 10 and 11, students were required to write programs using spinlocks, mutex objects and condition variables, and semaphores to achieve desired execution. 

### Course learning outcomes:
- Using a hardware based model of execution, reason about the limitations, vulnerabilities and idiosyncrasies of the behaviour of a particular program, specifically concerning performance, bugs and security vulnerabilities.
- Using a hardware based model of data, reason about how programs access data using different types of variables, including the implicit and explicit use of memory references.
- Translate a statement from a high-level programming language into assembly language; from a large block of assembly language, identify groups of instructions that correspond to high level language features and then write an equivalent high level programming language expression.
- Identify and correct memory management bugs, particularly in languages with explicit deallocation, and use best practices to write code that is less likely to incur such issues.
- Compare and contrast how Java and C are translated into a language the CPU understands; identify common features that are implemented in significantly different ways in either language (for instance, memory management, and the duality of subtype polymorphism in Java and function pointers in C). In doing so, explain the tradeoffs associated with each.
- Reason about the execution of concurrent programs, incl. real time interrupts, and use both asynchronous programming and threads to write concurrent and/or parallel programs. Explain the tradeoffs associated with each.
- Solve problems using monitors, condition variables and semaphores.
