# Nand2Tetris Part 1: Course Projects

This repository contains all the projects from Part 1 of the [Nand2Tetris](https://www.nand2tetris.org/) course. The course takes you through the process of building a computer system from the ground up, starting with basic logic gates and culminating in a working assembler and computer architecture.

---

## Table of Contents
1. [Overview](#overview)
2. [Included Projects](#included-projects)
3. [Features](#features)
4. [How to Use](#how-to-use)
   - [HDL Projects](#hdl-projects)
   - [Assembler](#assembler)

---

## Overview

The Nand2Tetris course is a hands-on journey into computer science fundamentals. This repository contains the implementation of all Part 1 projects, including:
- Designing logic gates and circuits using HDL.
- Writing assembly programs for the Hack computer.
- Building an assembler to translate Hack assembly into binary machine code.

---

## Included Projects

### 1. Boolean Logic
- **Description**: Implementation of basic logic gates (e.g., AND, OR, NOT) and combinational circuits like multiplexers and adders.
- **Key Components**: 
  - AND, OR, NOT gates
  - Multiplexers

### 2. Boolean Arithmetic
- **Description**: Construction of arithmetic circuits, including a full adder and the ALU (Arithmetic Logic Unit).
- **Key Components**:
  - Half Adder and Full Adder
  - Incrementer
  - ALU (Arithmetic Logic Unit)

### 3. Sequential Logic
- **Description**: Design of sequential circuits like flip-flops, registers, and RAM.
- **Key Components**:
  - Program Counter 
  - D Flip-Flop
  - 16-bit Register
  - RAM (Random Access Memory)

### 4. Machine Language
- **Description**: Writing simple programs in Hack assembly language to run on the Hack computer.
- **Key Programs**:
  - Add.asm
  - Max.asm
  - Rect.asm
  - Fill.asm
  - Mult.asm

### 5. Computer Architecture
- **Description**: Building the Hack computer by integrating the ALU, registers, and memory.
- **Key Components**:
  - CPU
  - Memory
  - Hack Computer

### 6. Assembler
- **Description**: Implementation of an assembler to translate Hack assembly (`.asm`) into binary machine code (`.hack`).
- **Key Features**:
  - Two-pass assembler
  - Symbol table for labels and variables
  - A-instruction and C-instruction parsing

---

## Features

- **Hardware Description Language (HDL)**: Used to design and simulate hardware components.
- **Hack Assembly Language**: Programs written in assembly for the Hack computer.
- **Assembler**: Converts assembly code into binary machine code.
- **Symbol Table**: Supports predefined symbols, labels, and variables in assembly programs.
- **Two-Pass Assembler**:
  - **First Pass**: Resolves labels and populates the symbol table.
  - **Second Pass**: Translates instructions into binary machine code.

---

## How to Use

### HDL Projects
1. Open the Hardware Simulator provided by the Nand2Tetris course.
2. Load the `.hdl` files for the respective project.
3. Run the test scripts (`.tst`) to verify the implementation.

### Assembler
1. Compile the assembler:
   ```bash
   gcc -o assembler n2tAssembler.c -lm
