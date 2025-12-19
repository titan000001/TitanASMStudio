# TitanASM Studio 🚀

**An Advanced 8086 Assembler & Simulator for Students.**

```
  _______  _   _                 _    __  __ 
 |__   __|(_) | |               / \  |  \/  |
    | |    _  | |_   __ _  _   / _ \ | .  . |
    | |   | | | __| / _` || | / ___ \| |\/| |
    | |   | | | |_ | (_| || |/ /   \ \ |  | |
    |_|   |_|  \__| \__,_||_/_/     \_\_|  |_|
```

## Overview
TitanASM is a robust learning tool designed for **Compiler Design** courses. It provides a complete development environment for writing, assembling, and debugging 8086 assembly code.

**Key Features:**
*   **Hybrid Architecture**: High-performance C++ Core (Assembler+VM) with a modern C# Windows Interface.
*   **Interactive Debugger**: Step-by-step execution with live Register Views (`AX`, `BX`, `SP`, `IP`).
*   **Stack Support**: Fully implemented System Stack (`PUSH`, `POP`, `CALL`, `RET`).
*   **Syntax Highlighting**: Color-coded editor for easy reading.
*   **emu8086 Compatible**: Supports `printn`, `mov ah, 4ch`, and standard interrupts.

## Getting Started

### Prerequisites
*   Windows OS (7/10/11)
*   .NET Framework 4.5+

### Running the Studio
1.  Navigate to the **`bin/`** folder.
2.  Launch **`TitanASMStudio.exe`**.
3.  Write your code or load an example from the `examples/` folder.
4.  Click **Assemble** -> **Run** (or **Debug** for step-by-step mode).

## Example Code

### Hello World
```asm
org 100h
main proc
    printn "Hello Titan!"
    mov ah, 4Ch
    int 21h
main endp
```

### Recursive Function (Stack Demo)
```asm
org 100h
    mov ax, 0
    call my_func
    add ax, 1     ; Result AX=6
    mov ah, 4Ch
    int 21h

my_func:
    mov ax, 5
    ret
```

## Project Structure
*   `src/backend/`: C++ Source Code ( Assembler, Simulator, MacroProcessor).
*   `src/frontend/`: C# Source Code (Windows Forms GUI).
*   `bin/`: Compiled Executables.
*   `examples/`: Sample Assembly Programs.

## Build from Source
**Backend**:
```bash
g++ src/backend/*.cpp -o bin/TitanASM.exe
```
**Frontend**:
```bash
csc /target:winexe /out:bin/TitanASMStudio.exe src/frontend/AssemblerGUI.cs
```

## License
Created for Compiler Design Course. Open Source.
