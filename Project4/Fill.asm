// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

(LOOP_BEG)
@KBD
D=M
@SCREEN_BLACK
D;JGT

// If key not pressed -> Make screen white and go back to beginning
@SCREEN_WHITE
0;JMP

// If key is pressed 
//  for(row=0;row<256;row+=32)
//  {
//    ADDR=SCREEN_BASEAADR+row
//    for(colomn=0;coloumn<32;column+=1)
//    {
//        addr = ADDR + column
//    }
//  }

(SCREEN_BLACK)
@8192
D=A
@R2
M=D

@32
D=A
@R3
M=D

//Set Row
D=0
@row
M=D

//Set coloumn
D=0
@col
M=D

// Row loop 
(LOOP_ROW)
D=0
@col
M=D
@row
D=M
@R2
D=M-D   //Check row value is less than 256
@LOOP_BEG
D;JEQ

// Set addr pointer to row base address 
@SCREEN
D=A     //D=SCREEN_BASEADDR
@addr   //ADDR = SCRREN_BASE_ADDR + row
M=D     //addr = screen_base_Address
@row
D=M     //D=row
@addr
M=D+M   //addr=addr+row

@row
D=M
@32
D=D+A
@row
M=D //Row = row+1

// Coloumn Loop 
(LOOP_COL)
@col
D=M
@R3
D=M-D   //Check col value is less than 32
@LOOP_ROW
D;JEQ

@addr
D=M
A=D
M=-1
@addr
D=M
D=D+1
M=D

@col
D=M
D=D+1
M=D
@LOOP_COL
0;JMP

// Done with the job, go back and check again key 
@LOOP_BEG
0;JMP

(SCREEN_WHITE)
@8192
D=A
@R2
M=D

@32
D=A
@R3
M=D

//Set Row
D=0
@row
M=D

//Set coloumn
D=0
@col
M=D

// Row loop 
(LOOP_ROW_WHITE)
D=0
@col
M=D
@row
D=M
@R2
D=M-D //Check row value is less than 256
@LOOP_BEG
D;JEQ

// Set addr pointer to row base address 
@SCREEN
D=A     //D=SCREEN_BASEADDR
@addr   //ADDR = SCRREN_BASE_ADDR + row
M=D     //addr = screen_base_Address
@row
D=M     //D=row
@addr
M=D+M   //addr=addr+row

@row
D=M
@32
D=D+A
@row
M=D //Row = row+1

// Coloumn Loop 
(LOOP_COL_WHITE)
@col
D=M
@R3
D=M-D   //Check col value is less than 32
@LOOP_ROW_WHITE
D;JEQ

@addr
D=M
A=D
D=0
M=D
@addr
D=M
D=D+1
M=D

@col
D=M
D=D+1
M=D
@LOOP_COL_WHITE
0;JMP

// Done with the job, go back and check again key 
@LOOP_BEG
0;JMP

