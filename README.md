```text
  _______  _  _                  _      ____   __  __ 
 |__   __|(_)| |                / \    / ___| |  \/  |
    | |    _ | |_   __ _  _ __ / _ \   \___ \ | |\/| |
    | |   | || __| / _` || '_ / ___ \   ___) || |  | |
    |_|   |_| \__| \__,_||_| /_/   \_\ |____/ |_|  |_|
```

## Overview
TitanASM is a robust learning tool designed for **Compiler Design** courses. It provides a complete development environment for writing, assembling, and debugging 8086 assembly code.

**Key Features:**
*   **Hybrid Architecture**: High-performance C++ Core (Assembler+VM) with a modern C# Windows Interface.
*   **Interactive Debugger**: Step-by-step execution with live Register Views (`AX`, `BX`, `SP`, `IP`).
*   **Advanced Instruction Set**: 
    *   **Stack**: `PUSH`, `POP`, `CALL`, `RET`.
    *   **Arithmetic**: `ADD`, `SUB`.
    *   **Variables**: `.data`, `DB`, `DW` with memory access (`MOV Reg, [Var]`).
    *   **I/O**: Keyboard Input (`AH=1`) and Display Output (`AH=2`).
*   **Syntax Highlighting**: Color-coded editor for easy reading.
*   **emu8086 Compatible**: Supports `printn`, `.model`, `.stack`, and standard interrupts.

## Getting Started

### Prerequisites
*   Windows OS (7/10/11)
*   .NET Framework 4.5+

### Running the Studio
1.  **Open the Project Folder**: Navigate to the **`bin/`** directory.
2.  **Launch the App**: Double-click **`TitanASMStudio.exe`**.
    *   *Note: Ensure `TitanASM.exe` is in the same folder.*
3.  **Start Coding**: Write your code or load an example from the `examples/` folder.
4.  **Execute**:
    *   Click **Assemble** to compile.
    *   Click **Run** for standard execution.
    *   Click **Debug** for interactive step-by-step mode (Great for visualizing logic!).

## Example Code

### Variables & I/O
```asm
.model small
.data
    val1 db 10
.code
main proc
    mov al, val1    ; Load Variable
    add al, 5       ; Arithmetic
    
    mov ah, 1       ; Input Char
    int 21h
    
    mov ah, 4ch     ; Exit
    int 21h
main endp
```

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
g++ src/backend/*.cpp -I src/backend -o bin/TitanASM.exe
```
**Frontend**:
```bash
csc /target:winexe /out:bin/TitanASMStudio.exe src/frontend/AssemblerGUI.cs
```

## License
Created for Compiler Design Course. Open Source.
