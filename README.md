# 🚀 TitanASM: The Ultimate 8086 Development Environment

![Build Status](https://img.shields.io/badge/Build-Success-brightgreen)
![Tech Stack](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20C%23-blue)
![Architecture](https://img.shields.io/badge/Architecture-x86%20Simulation-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)

```text
  _______  _  _                  _      ____   __  __ 
 |__   __|(_)| |                / \    / ___| |  \/  |
    | |    _ | |_   __ _  _ __ / _ \   \___ \ | |\/| |
    | |   | || __| / _` || '_ / ___ \   ___) || |  | |
    |_|   |_| \__| \__,_||_| /_/   \_\ |____/ |_|  |_|
```

TitanASM is a high-performance, hybrid development environment designed specifically for **Compiler Design** and **Computer Architecture** students. It combines the raw power of a C++ backend with the intuitive interface of a C# Studio.

---

## 🔥 Key Features

| Feature | Description |
| :--- | :--- |
| **Hybrid Core** | Blazing fast C++ Assembler & Virtual Machine wrapped in a modern C# GUI. |
| **Full Debugger** | Step-by-step execution with a **Live Register & Stack Viewer**. |
| **Macro Processor** | Support for `PROCLIB`, `INCLUDE`, and complex macro expansions. |
| **emu8086 Ready** | Native support for `print`, `printn`, and standard BIOS interrupts. |
| **Universal HEX** | Synchronized 2-digit HEX protocol for perfect Machine Code accuracy. |

---

## 📸 Screenshots

### 🖥️ The Studio
![Studio Interface](docs/screenshots/main_ui.png)
*Write, Assemble, and Debug all in one place.*

### 🌍 Hello World
![Hello World](docs/screenshots/hello_world.png)
*Native string handling for quick prototyping.*

---

## 🛠️ Instruction Set Support

TitanASM supports a comprehensive subset of the 8086 instruction set:

*   **Move & Math**: `MOV`, `ADD`, `SUB`, `CMP`
*   **Flow Control**: `JMP`, `JZ`, `JNZ`, `CALL`, `RET`
*   **Stack Logic**: `PUSH`, `POP`
*   **Interrupts**: `INT 21h` (AH=1: Input, AH=2: Output, AH=4Ch: Exit)
*   **Directives**: `.data`, `.code`, `.model`, `org`, `db`, `include`

---

## 🚀 Getting Started

### Prerequisites
- **Windows OS**
- **MinGW (g++)** for backend compilation.
- **.NET Framework** for the Studio interface.

### Installation & Run
1. Clone the repository.
2. Build the system (see Build section).
3. Open `bin/TitanASMStudio.exe`.
4. Click **Assemble** then **Run**!

---

## 🏗️ Build from Source

### Backend (Assembler & VM)
```powershell
g++ src/backend/*.cpp -I src/backend -o bin/TitanASM.exe
```

### Frontend (Visual Studio UI)
```powershell
csc /target:winexe /out:bin/TitanASMStudio.exe src/frontend/AssemblerGUI.cs
```

---

## 📜 Example: Interactive Calculator
```asm
org 100h
.code
main proc
    ; Input Number 1
    mov ah, 1
    int 21h
    sub al, 48
    mov bl, al

    ; Input Number 2
    mov ah, 1
    int 21h
    sub al, 48
    
    ; Add and Print
    add bl, al
    add bl, 48
    mov dl, bl
    mov ah, 2
    int 21h

    mov ah, 4Ch
    int 21h
main endp
end main
```

---

## ⚖️ License
This project is open-source and intended for educational purposes. 

**Developed with ❤️ for Advanced Agentic Coding.**
