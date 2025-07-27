// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.


@R2
D=0
M=D

@i
D=0
M=D

// Iter over loop for R1 Times 
(LOOP_BEG)
@i
D=M
@R1
D=M-D       // n-i
@END
D;JEQ       //n-i -> n=0
@R0
D=M         //D=R0
@R2
M=D+M       //sum=sum+R0
@i
D=1
M=D+M       //i=i+1
@LOOP_BEG
0;JMP

(END)
@END
0;JMP


