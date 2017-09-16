#include "sys_common.h"
#include "system.h"


//r0 address = m(x)
//r1 address = c(x)
//r2 = m(x)*x^k / new m(x) / CRC
//r3 = dummy
//r4 = counter
//r5 = dummy
//r6 = holds decimal #1
//r7 = c(x) MSB variant
//r8 = (m(x)*x^k) MSB
//r9 = shifted c(x) variant
//r10 = (if any) Error statement
//r11 = k value
//r12 = dummy

void main(void)
{



asm("START    LDR R2, ER2");          //Initialize address for R2
asm("         LDR R7, ER7");          //Initialize address for R7
asm("         LDR R8, ER8");          //Initialize address for R8
asm("         LDR R9, ER9");          //Initialize address for R9

asm("         ADR R1, CX");           //Puts predefined CX FLASH address into R1
asm("         LDR R3, [R1]");         //Load CX into R3 dummy
asm("         LDR R1, ER1");          //Initialize RAM address for R1 (R1=0x8000020)
asm("         STR R3, [R1]");         //Stores C(x) into R1 RAM address

asm("         ADR R0, MSG");          //Puts predefined m(x) FLASH address into R1
asm("         LDR R3, [R0]");         //Load CX into R3 dummy
asm("         LDR R0, ER0");          //Initialize RAM address for R0 (R0=0x8000010)
asm("         STR R3, [R0]");         //Stores M(x) into R0 RAM address


asm("         MOV R4, #0");           //initialize k value
asm("         MOV R6, #1");           //initialize #1 register
asm("         LDR R3, [R1]");         //load c(x) into register 3
asm("         BL MXKCMSB");           //branch to c(x) MSB / m(x)*x^k function
asm("LOOP     BL MXKMSB");            //branch to m(x)*x^k MSB variant function
asm("         B DIVCM");              //branch to new m(x) function

//Put c(x) MSB variant into register 7 and m(x)*x^k into register 2
//Get c(x) MSB into R7
asm("MXKCMSB  CMP R3, #1");           //compare c(x) with 1
asm("         MOVNE R3, R3, LSR #1"); //shift c(x) right one
asm("         ADDNE R4, R4, #1");     //add one to k
asm("         BNE MXKCMSB");          //Loop if z is not set
asm("         MOV R3, R6, LSL R4");   //shift #1 register left by k and and move into r3
asm("         STR R3, [R7]");         //store c(x) MSB into address at R7 (c(x) MSB register)
asm("         MOV R11, R4");          //Assign k value to R11
//Shift m(x) to get m(x)*x^k and store in register 2
asm("         LDR R3, [R0]");         //Load m(x) into R3
asm("         MOV R12, #32");         //Move #32 into R12
asm("         SUB R4, R12, R11");     //32 - k to get shift right value
asm("         MOV R12, R3, LSR R4");  //Shift right by 32 - k and store in R12 (upper 32 bits)
asm("         MOV R5, R3, LSL R11");  //Shift left by k and store in R5        (lower 32 bits)
asm("         STMIA R2, {R5, R12}");  //Store m(x)*x^k into R2 address
asm("         MOV PC, LR");



//Put m(x)*x^k MSB variant into register 8
asm("MXKMSB");
asm("         LDMIA R2, {R3, R5}");   //load m(x)*x^k into r3 (lower) and r5 (upper)
asm("         MOV R4, #0");           //reset counter to 0
asm("         CMP R5, #0");           //Prelim check to see if m(x)*x^k upper 32 bits is already 0
asm("         BEQ  LOWER");           //Branch to lower 32 bit m(x)*x^k loop
asm("GENMSB   CMP R5, #0");           //compare m(x)*x^k upper 32 bits to 1
asm("         MOVNE R5, R5, LSR #1"); //shift m(x)*x^k upper 32 bits right 1
asm("         ADDNE R4, R4, #1");     //add one to counter
asm("         BNE GENMSB");           //loop if z is not set
asm("         B UPSHIFT");            //Branch to m(x)*x^k upper 32 bits loops
//If m(x)*x^k upper 32 bits is already 0
asm("LOWER");
asm("GENMMSB  CMP R3, #1");           //compare m(x)*x^k lower 32 bits to #1
asm("         MOVNE R3, R3, LSR #1"); //Shift m(x)*x^k right if not = to 1
asm("         ADDNE R4, R4, #1");     //Add 1 to counter
asm("         BNE GENMMSB ");         //Loop if not equal to 1
asm("         MOV R3, R6, LSL R4");   //put m(x)*x^k MSB lower 32 bits into  r3
asm("         STR R3, [R8]");         //store m(x)*x^k MSB lower 32 bits into r8 (m(x)*x^k MSB register)
asm("         B LOSHIFT");


//Calculates c(x) shift after c(x) MSB -
//If m(x)*x^k > 32 bits, c(x) shift
asm("UPSHIFT");
asm("         LDR R3, [R7]");         //load c(x) MSB into r3
asm("         MOV R6, R6, LSL #31");  //Shift #1 register left 31 times to get MSB #32
asm("USHIFT   CMP R6, R3");           //Compare upper m(x)*x^k MSB with c(x)
asm("         MOVNE R6, R6, LSR #1"); //shift r6 right by 1
asm("         ADDNE R4, R4, #1");     //add one to counter
asm("         BNE USHIFT");           //loop if z is not set
asm("         LDR R5, [R1]");         //load c(x) into r3
asm("         MOV R12, #32");         //Move #32 into R12
asm("         SUB R12, R12, R4");     //32 - counter to get shift right value and store in r12
asm("         MOV R12, R5, LSR R12"); //Shift right by 32 - counter and store in R12 (upper 32 bits)
asm("         MOV R5, R5, LSL R4");   //Shift left by counter and store in R5        (lower 32 bits)
asm("         STMIA R9, {R5, R12}");  //Store c(x) shifted into R9 address
asm("         MOV R6, #1");           //Reset #1 register
asm("         MOV PC, LR");
//If m(x)*x^k <= 32 bits, c(x) shift
asm("LOSHIFT");
asm("         MOV R4, #0");           //reset counter to 0
asm("         LDR R3, [R7]");         //load c(x) MSB into r3
asm("         LDR R5, [R8]");         //load (m(x)*x^k) MSB into r5
asm("         CMP R5, R3");           //prelim compare c(x) MSB with (m(x)*x^k) MSB
asm("         BMI TESTERR ");         //Branch to end if c(x) MSB > (m(x)*x^k) MSB (to see if error or not)
asm("SHIFT    CMP R5, R3");           //compare (m(x)*x^k) MSB with c(x) MSB
asm("         MOVNE R3, R3, LSL #1"); //shift r3 left by 1
asm("         ADDNE R4, R4, #1");     //add one to counter
asm("         BNE SHIFT");            //loop if z is not set
asm("         LDR R3, [R1]");         //load c(x) into r3
asm("         MOV R3, R3, LSL R4");   //shift c(x) left by k and store in r3
asm("         STR R3, [R9]");         //store shifted c(x) into r9 (shifted c(x) register)
asm("         MOV R5, #0");           //store shifted c(x) into r9 (shifted c(x) register)
asm("         STM R9, {R3, R5}");     //store shifted c(x) into r9 (shifted c(x) register)
asm("         STR R10, [R4]");           //K BACKUP
asm("         MOV R4, #0");           //reset k register
asm("         MOV PC, LR");

//Creates new m(x)*x^k and store in register 2
asm("DIVCM");
asm("         LDM R2, {R3, R5}");     //Load m(x)*x^k into R3 (lower) and R5(upper)
asm("         LDM R9, {R10, R12}");   //Load shifted c(x) into R10 (lower) and R12 (upper)
asm("         EOR R5, R5, R12");      //XOR m(x)*x^k and shifted c(x) upper and store in R3
asm("         EOR R3, R3, R10");      //XOR m(x)*x^k and shifted c(x) lower and store in R3
asm("         STM R2, {R3, R5}");     //Store new m(x)*x^k into R2 address

asm("         B LOOP");               //Branch back to MXKMSB to find MSB of new m(x)*x^k) MSB and repeat process

//Appends CRC to m(x) in memory
asm("TESTERR  LDR R3, [R2]");       //Loads CRC into r3
asm("         CMP R11, #0");        //RESET CSPR
asm("         CMP R11, #3");
asm("         MOVEQ R5, #28");       //if k=3 need to shift 28 bits
asm("         CMP R11, #8");
asm("         MOVEQ R5, #24");       //if k=8 need to shift 24 bits
asm("         CMP R11, #10");
asm("         MOVEQ R5, #20");       //if k=10 need to shift 20 bits
asm("         CMP R11, #16");
asm("         MOVEQ R5, #16");      //if k=16 need to shift 16 bits
asm("         LSL R3, R5");         //shift left to align with BIT31 (ex: put into form 0xB36A0000)
asm("         MOV R12, R0");        //Load R0 address value into R12
asm("         SUB R12, R12, #4");   //Subtract 4 from address value to align R12 to store CRC
asm("         STR R3, [R12]");      //Store r3 contents into r12 address

asm("         LDR R3, [R2]");     //Loads CRC into r3
asm("         LDR R6, [R12],#4"); //Loads R6 with 0x800001C address in memory
asm("         LDR R5, [R12]");    //Loads R5 with 0x8000020 address in memory    R5+R6 represent entire MSG+CRC


asm("         B END");                //Branch to end

asm("END      B END");

asm("ER0     .int     0x8000010");    //RAM Addresses we want for registers 0, 1, 2, 7, 8, and 9
asm("ER1     .int     0x8000020");
asm("ER2     .int     0x8000030");
asm("ER7     .int     0x8000040");
asm("ER8     .int     0x8000050");
asm("ER9     .int     0x8000060");
asm("CX      .int     0x9");           //C(x) assigned to FLASH memory
asm("MSG     .uword   0x9ABE24FA");        //Message assigned to FLASH memory
}
