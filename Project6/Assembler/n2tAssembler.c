/*
Assembler for Hack Computer
/**
 * @file n2tAssembler.c
 * @brief This assembler converts assembly language code into machine code for the Hack computer.
 *
 * The assembler operates using a two-pass approach:
 * 1. **First Pass**: Scans the assembly code to build a symbol table by identifying all label declarations 
 *    and their corresponding memory addresses.
 * 2. **Second Pass**: Translates the assembly instructions into Hack machine code, using the symbol table 
 *    to resolve symbols and variables.
 * 
 * This program is an assembler for the Nand2Tetris Hack computer. It processes
 * assembly language files (.asm) and converts them into binary machine code (.hack).
 * 
 * Functionality:
 * - Opens and processes each line of the input assembly file (e.g., Add.asm).
 * - Reads each line into a temporary buffer and passes it to the "Parser" module.
 * - The "Parser" module splits the instructions into corresponding fields:
 *   - Opcode
 *   - Destination
 *   - Compare
 *   - Jump
 *   - Address field
 *   - For A-instructions: Only Opcode and Address field are valid.
 *   - For C-instructions: Address field is invalid.
 * - The parsed fields are passed to the "Command" module, which populates the
 *   corresponding bit positions based on the fields.
 * 
 * Symbol Table:
 * - Manages variables and labels used in the assembly code.
 * 
 * Handles:
 * - White spaces and comments in the input file.
 * 
 * Modules:
 * - Parser: Responsible for parsing assembly instructions into fields.
 * - Command: Converts parsed fields into binary machine code.
 * - Symbol table management: Manages symbols (variables and labels).
 * - Main function: Coordinates the overall flow of the assembler.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Macro Definitions */
#define SYSTEM_SUCCESS   (1U)
#define SYSTEM_FAILURE   ((int32_t)(-1))
#define LINEBUFFER_SIZE  (100U)
#define BITFIELD_MAX     (16U)
#define COMP_FIELD_LEN   (7U)
#define DEST_FIELD_LEN   (3U)
#define JUMP_FIELD_LEN   (3U)
#define C_INST_PAD_LEN   (3U)
#define INVALID_VALUE    (0xFFU)
#define CURR_MEMORY      (16U)
#define SYMBOLTABLE_TAIL (23U)

/* Variable Definitions */
typedef struct
{
  uint8_t *mnemonic;
  uint8_t *binary;
} Instruction_Encoding;

typedef struct
{
  uint8_t symbol[LINEBUFFER_SIZE];
  uint32_t value;
} Symbol_Table;

typedef struct
{
  uint32_t symbolTableTail;
  uint32_t currMemory;
} SymbolTableMeta;

/* ISA Fields */
typedef struct
{
  /* For C instruction */
  uint8_t destFldString[5];
  uint8_t jmpFldString[5];
  uint8_t cmpFldString[5];

  /* For A instruction */
  uint8_t addFldString[10];

  /* Bit Fields */
  uint8_t bitFields[17];
} ISA_Field;

/* LookUp Table for "Comp" Field */
Instruction_Encoding compFieldLT[] = 
{
  { "0"   , "0101010" },
  { "1"   , "0111111" },
  { "-1"  , "0111010" },
  { "D"   , "0001100" },
  { "A"   , "0110000" } ,
  { "!D"  , "0001101" },
  { "!A"  , "0110001" },
  { "-D"  , "0001111" },
  { "-A"  , "0110011" },
  { "D+1" , "0011111" },
  { "A+1" , "0110111" },
  { "D-1" , "0001110" },
  { "A-1" , "0110010" },
  { "D+A" , "0000010" },
  { "D-A" , "0010011" },
  { "A-D" , "0000111" },
  { "D&A" , "0000000" },
  { "D|A" , "0010101" },
  { "M"   , "1110000" } ,
  { "!M"  , "1110001" },
  { "-M"  , "1110011" },
  { "M+1" , "1110111" },
  { "M-1" , "1110010" },
  { "D+M" , "1000010" },
  { "D-M" , "1010011" },
  { "M-D" , "1000111" },
  { "D&M" , "1000000" },
  { "D|M" , "1010101" },
};

/* LookUp Table for "Dest" Field */
Instruction_Encoding destFieldLT[] = 
{
  { "\0"  , "000" },
  { "M"   , "001" },
  { "D"   , "010" },
  { "MD"  , "011" },
  { "A"   , "100" },
  { "AM"  , "101" },
  { "AD"  , "110" },
  { "AMD" , "111" },
};

/* LookUp Table for "Jump" Field */
Instruction_Encoding jumpFieldLT[] = 
{
  { "\0"  , "000" },
  { "JGT" , "001" },
  { "JEQ" , "010" },
  { "JGE" , "011" },
  { "JLT" , "100" },
  { "JNE" , "101" },
  { "JLE" , "110" },
  { "JMP" , "111" },
};

/* Symbol Table */
Symbol_Table gSymbolTable[65535] = 
{
  { "R0"      , 0       },
  { "R1"      , 1       },
  { "R2"      , 2       },
  { "R3"      , 3       },
  { "R4"      , 4       },
  { "R5"      , 5       },
  { "R6"      , 6       },
  { "R7"      , 7       },
  { "R8"      , 8       },
  { "R9"      , 9       },
  { "R10"     , 10      },
  { "R11"     , 11      },
  { "R12"     , 12      },
  { "R13"     , 13      },
  { "R14"     , 14      },
  { "R15"     , 15      },
  { "SCREEN"  , 16384   },
  { "KBD"     , 24576   },
  { "SP"      , 0       },
  { "LCL"     , 1       },
  { "ARG"     , 2       },
  { "THIS"    , 3       },
  { "THAT"    , 4       },
};

ISA_Field gInsFields;
SymbolTableMeta gSymbolTableMeta =
{
  .symbolTableTail = 23,
  .currMemory = CURR_MEMORY,
};
uint8_t inputFile[LINEBUFFER_SIZE];
uint8_t outputFile[LINEBUFFER_SIZE];

/* Function Declarations */
int32_t lineParser(uint8_t *line);
int32_t lineCommand(void);
void lineWriter(FILE *filePtr);
void varInit(void *arg);
void  firstPass(FILE *filePtr);
void secondPass(FILE *ipFilePtr, FILE *opFilePtr);
void  searchSymbolEntry(uint8_t *str);

int main()
{
  int32_t status = SYSTEM_SUCCESS;
  
  /* Init the variables */
  varInit(NULL);

  printf("Enter the input file name\n");
  scanf("%s", inputFile);

  printf("Enter the output file name\n");
  scanf("%s", outputFile);

  /*Open the file*/
  FILE *ipFilePtr = fopen(inputFile,"r");
  FILE *opFilePtr = fopen(outputFile,"w");

  if(ipFilePtr == NULL || opFilePtr == NULL)
  {
    status = SYSTEM_FAILURE;
  }

  if(status == SYSTEM_SUCCESS)
  {
    /* First Pass to find lablels */
    firstPass(ipFilePtr);

    /* Second Pass to find resolve variables & instructions */
    secondPass(ipFilePtr, opFilePtr);
  }
  else
  {
    printf("Error handling files \n");
  }
}

/* First Pass of assembler to resolve labels */
void  firstPass(FILE *ipFilePtr)
{
  uint8_t lineBuff[LINEBUFFER_SIZE] = {0};
  uint32_t lineCount = 0;
  uint32_t charCount = 1;

  memset(lineBuff, 0, sizeof(lineBuff));
  static uint8_t count = 0;

  while(fgets(lineBuff, LINEBUFFER_SIZE, ipFilePtr))
  {
    count++;

    /*Check if it is a comment */
    if( (lineBuff[0] == '/') && (lineBuff[1] == '/') )
    {
      /* Skip the comment */
    }
    else if(lineBuff[0] == '\r')
    {
      /* Skip the space */
    }
    else
    {
      /* Instruction Line */
      if(lineBuff[0] == '(' )
      {
        /* Labels */
        while(lineBuff[charCount] != ')' )
        {
          gSymbolTable[gSymbolTableMeta.symbolTableTail].symbol[charCount-1] = lineBuff[charCount];
          charCount++;
        }
        gSymbolTable[gSymbolTableMeta.symbolTableTail].symbol[charCount] = '\0';
        gSymbolTable[gSymbolTableMeta.symbolTableTail].value = lineCount;

        charCount = 1U;
        gSymbolTableMeta.symbolTableTail++;
      }
      else
      {
        lineCount++;
      }
    }
    memset(lineBuff, 0, sizeof(lineBuff));
  }
}

void secondPass(FILE *ipFilePtr, FILE *opFilePtr)
{
  int32_t status = SYSTEM_SUCCESS;
  uint8_t lineBuffer[LINEBUFFER_SIZE]={0};
  memset(lineBuffer, 0, sizeof(lineBuffer));
  rewind(ipFilePtr);

  while(fgets(lineBuffer, LINEBUFFER_SIZE, ipFilePtr))
  {
    /* Pass the line to the parser */
    status = lineParser(&lineBuffer[0]);

    if(SYSTEM_SUCCESS == status)
    {
      /* Valid line and got parsed successfully */
      /* Write the binary value to file in string format */
      lineWriter(opFilePtr);
    }
    else
    {
      /* Skip the line and proceed to next */
    }
    /* Reset the variables for next instruction */
    memset(lineBuffer, 0, sizeof(lineBuffer));
    varInit(NULL);
  }
}

/* Instruction line parser */
int32_t lineParser(uint8_t *line)
{
  int32_t status = SYSTEM_SUCCESS;
  uint8_t *ptr = line;
  uint8_t str[LINEBUFFER_SIZE] = {0};
  uint8_t index = 0U;

  /*Check if it is a comment */
  if( (*ptr == '/') && (*(ptr+1) == '/') )
  {
    /* Skip the comment */
    status = SYSTEM_FAILURE;
  }
  else if(*ptr == '\r')
  {
    /* Skip the space */
    status = SYSTEM_FAILURE;
  }
  else
  {
    uint8_t count = 0U;

    /* Parse the fields */

    if(*ptr == '@')
    {
      /* A Instruction */
      uint8_t index = 0U;
      /* A Instruction ( @2 or @sum ) */
      ptr++;

      /* Check if it is a symbol */
      if(*ptr > '9' )
      {
        /* Symbol - Check the entry in symbol table */
        uint8_t str[LINEBUFFER_SIZE];
        strcpy(str, ptr);
        searchSymbolEntry(str);
      }
      else
      {
        /* Address*/
        while(*ptr != '\r')
        {
          gInsFields.addFldString[index++] = *(ptr);
          ptr++;
        }
        gInsFields.addFldString[index]='\0';
      }
      index = 0U;

      /* Pass the line parsed to lineCommand() to get bitfields */
      status = lineCommand();
    }
    else if(*ptr == '(' )
    {
      /* Labels */
      ptr++;
      while(*ptr != ')' )
      {
        str[index] = *ptr;
        index++;
        ptr++;
      }
      str[index++] = '\r';
      str[index++] = '\n';
      str[index++] = '\0';
      searchSymbolEntry(str);

      status = SYSTEM_FAILURE;
    }
    else
    {
      /* C Instruction */
      gInsFields.addFldString[0] = INVALID_VALUE;
      uint8_t index = 0U;

      while(*ptr != '=')
      {
        /* Destination Field */
        if(*ptr == ';')
        {
          status = SYSTEM_FAILURE;
          break;
        }
        else
        {
          gInsFields.destFldString[index++] = *ptr;
          ptr++;
        }
      }

      if(status == SYSTEM_SUCCESS)
      {
        /* Destination field is present and valid */
        gInsFields.destFldString[index]='\0';
        ptr++;
        index = 0U;
      }
      else
      {
        /* Destination field is not present and valid */
        ptr = line;
        index = 0U;
        memset(gInsFields.destFldString,  0, sizeof(gInsFields.destFldString) );
        gInsFields.destFldString[0] = '\0';
        status = SYSTEM_SUCCESS;
      }

      while( (*ptr != ';') && (*ptr != '\r') )
      {
        /* Compare Field */
        gInsFields.cmpFldString[index++] = *ptr;
        ptr++;
      }
      gInsFields.cmpFldString[index]='\0';
      index = 0U;

      if(*ptr == ';')
      {
        ptr++;
        /* Jump Field */
        while( (*ptr != '\r') && (*ptr != ' ') )
        {
          /* Jump Field */
          gInsFields.jmpFldString[index] = *ptr;
          index++;
          ptr++;
        }
        gInsFields.jmpFldString[index] = '\0';
      }
      else
      {
        /* No Jump Field */
        gInsFields.jmpFldString[index++] = '\0';
      }
      index = 0U;

      while(gInsFields.jmpFldString[index] != '\0')
      {
        index++;
      }
      /* Pass the line parsed to lineCommand() to get bitfields */
      status = lineCommand();
    }
  }

  return status;
}

void  searchSymbolEntry(uint8_t *line)
{
  uint8_t foundEntry = 0U;
  uint32_t symbolTableTail = gSymbolTableMeta.symbolTableTail;
  uint32_t currMemory = gSymbolTableMeta.currMemory;
  uint32_t value = 0;
  uint8_t str[LINEBUFFER_SIZE];
  uint8_t strLen = 0U;
  uint8_t index = 0U;
  uint8_t tempIndex = 0U;

  memset(str, 0, sizeof(str));
  strcpy(str, line);
  strLen = strlen(line);

  str[strLen - 2] = '\0';

  for(uint32_t i = 0;  i< symbolTableTail; i++)
  {
    if(!strcmp(str, gSymbolTable[i].symbol))
    {
      foundEntry = 1;
      value =  gSymbolTable[i].value;
      break;
    }
  }

  if(!foundEntry)
  {
    /* Add the new entry */
    strcpy(gSymbolTable[symbolTableTail].symbol, str);
    gSymbolTable[symbolTableTail].value = currMemory;
    gSymbolTableMeta.symbolTableTail++;
    gSymbolTableMeta.currMemory++;
    value = gSymbolTable[symbolTableTail].value;
  }
  else
  {
  }

  /* Copy the value to address field string */
  memset(str, 0, sizeof(str));

  while(value != 0)
  {
    str[index] = (value % 10) + '0';
    value = value/10;
    index++;
  }

  while(index)
  {
    gInsFields.addFldString[tempIndex] = str[index-1];
    index--;
    tempIndex++;
  }
  gInsFields.addFldString[tempIndex] = '\0';
}

int32_t lineCommand(void)
{
  uint8_t strlength = 0;
  memset(gInsFields.bitFields, 0, sizeof(gInsFields.bitFields) );
  int32_t status = SYSTEM_SUCCESS; 

  if(gInsFields.addFldString[0] != INVALID_VALUE)
  {
    /* A Instruction */
    uint16_t addr = 0U;
    uint8_t i = 0U;
    uint8_t powTen = 0;
    strlength = strlen(&gInsFields.addFldString[0]);
    powTen = strlength - 1;

    while(i < strlength)
    {
      addr += ( gInsFields.addFldString[i] - '0' ) * pow(10, powTen);
      i++;
      powTen--;
    }

    /* Get the binary value to file in string format */

    for(int8_t bitPos = BITFIELD_MAX - 1; bitPos >= 0; bitPos--)
    {
      gInsFields.bitFields[BITFIELD_MAX - bitPos - 1] = (( (addr & (1 << bitPos) ) ) >> bitPos) + '0';
      uint8_t value = ( (addr & (1 << bitPos) ) ) >> bitPos ;
    }
    gInsFields.bitFields[BITFIELD_MAX] = '\0';
  }
  else 
  {
    uint8_t bitPos = 0U;
    uint8_t fldValid = 0U;
    status = SYSTEM_FAILURE;

    /* C Instruction */
    for(bitPos = 0U; bitPos < C_INST_PAD_LEN; bitPos++)
    {
      gInsFields.bitFields[bitPos] = '1';
    }

    /* Populate the Compare field */

    for(uint8_t tableIdx = 0U; tableIdx < (sizeof(compFieldLT)/sizeof(Instruction_Encoding)); tableIdx++)
    {
      if(!strcmp(compFieldLT[tableIdx].mnemonic, gInsFields.cmpFldString))
      {
        strcpy(&gInsFields.bitFields[bitPos], compFieldLT[tableIdx].binary);
        status = SYSTEM_SUCCESS;
        bitPos = bitPos + COMP_FIELD_LEN;
        break;
      }
    }

    if(status == SYSTEM_SUCCESS)
    {
      /* Populate the Destination field */
      status = SYSTEM_FAILURE;

      for(uint8_t tableIdx = 0U; tableIdx < (sizeof(destFieldLT)/sizeof(Instruction_Encoding)); tableIdx++)
      {
        if(!strcmp(destFieldLT[tableIdx].mnemonic, gInsFields.destFldString))
        {
          strcpy(&gInsFields.bitFields[bitPos], destFieldLT[tableIdx].binary);
          fldValid = 1U;
          status = SYSTEM_SUCCESS;
          break;
        }
      }

      if(SYSTEM_SUCCESS == status)
      {
        status = SYSTEM_FAILURE;
        bitPos = bitPos + DEST_FIELD_LEN;
  
        /* Populate the Jump field */
        for(uint8_t tableIdx = 0U; tableIdx < (sizeof(jumpFieldLT)/sizeof(Instruction_Encoding)); tableIdx++)
        {
          if(!strcmp(jumpFieldLT[tableIdx].mnemonic, gInsFields.jmpFldString))
          {
            strcpy(&gInsFields.bitFields[bitPos], jumpFieldLT[tableIdx].binary);
            status = SYSTEM_SUCCESS;
            break;
          }
        }
      }
      else
      {
        printf("Invalid Dest Instruction\n");
      }

      if(SYSTEM_SUCCESS == status)
      {
        bitPos = bitPos + JUMP_FIELD_LEN;
        gInsFields.bitFields[bitPos] = '\0';
      }
      else
      {
        printf("Invalid Jump Instruction\n");
      }
    }
    else
    {
      printf("Invalid Comp Instruction\n");
    }
  }
  return status;
}

void lineWriter(FILE *filePtr)
{
  fputs(&gInsFields.bitFields[0], filePtr);
  fputs("\n", filePtr);
}

void varInit(void *arg)
{
  memset(gInsFields.destFldString,  0, sizeof(gInsFields.destFldString) );
  memset(gInsFields.jmpFldString,   0, sizeof(gInsFields.jmpFldString) );
  memset(gInsFields.cmpFldString,   0, sizeof(gInsFields.cmpFldString) );
  memset(gInsFields.addFldString,   INVALID_VALUE, sizeof(gInsFields.addFldString) );
  memset(gInsFields.bitFields,      0, sizeof(gInsFields.bitFields) );
}