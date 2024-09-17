#include<stdio.h>
#include<stdlib.h>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

struct Mem{
    static constexpr u32 MAX_MEM = 1024*64;
    Byte data[MAX_MEM];

    void Initialise(){
        for(u32 i = 0; i < MAX_MEM; i++){
            data[i] = 0;
        }
    }


    //read byte
    Byte operator[](u32 address) const{
        // if < max_mem
        return data[address];
    }

    //write byte
    Byte& operator[](u32 address){
        // if < max_mem
        return data[address];
    }

    void writeWord(u32& cycles,Word value, u32 address){
        data[address] = value & 0xFF;
        data[address + 1] = value >> 8;
        cycles -= 2;
    }
    
};

struct CPU{


    Word PC;        // program ctr
    Word SP;        // stack ptr

    Byte A, X, Y;   // general usage registers

    Byte C : 1; //carry
    Byte Z : 1; //zero
    Byte I : 1; //interrupt
    Byte D : 1; //decimal flag
    Byte B : 1; //break
    Byte O : 1; //overflow
    Byte N : 1; //negative 

    void Reset(Mem& memory){
        PC = 0xFFFC;
        SP = 0x0100;
        D = 0;

        C = Z = I = D = B = O = N = 0;

        A = X = Y = 0;
        memory.Initialise();
    }

    Byte FetchByte(u32& cycles, Mem& memory){
        Byte data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    Byte ReadByte(u32& cycles,Byte address, Mem& memory){
        Byte data = memory[address];
        cycles--;
        return data;
    }

    Word FetchWord(u32& cycles, Mem& memory){
        Word data = memory[PC];
        PC++;
        cycles--;

        data = data | (memory[PC] << 8);
        PC++;
        cycles--;

        return data;
    }

    
    //opcodes
    static constexpr Byte INS_LDA_IM = 0xA9; 
    static constexpr Byte INS_LDA_ZP = 0xA5; 
    static constexpr Byte INS_LDA_ZPX = 0xB5;
    static constexpr Byte INS_JSR = 0x20;


    void LDASetStatus(){
        Z = (A == 0);
        N = (A & (0b10000000)) > 0;
    }

    void Execute(u32 cycles, Mem& memory){
        while(cycles > 0){
            Byte ins = FetchByte(cycles,memory);
            switch (ins)
            {
            case INS_LDA_IM:{
                Byte value = FetchByte(cycles,memory);
                A = value;
                LDASetStatus();
            }
            break;
            case INS_LDA_ZP:{
                Byte zeroPageAddress = FetchByte(cycles,memory);
                A = ReadByte(cycles, zeroPageAddress, memory);
                LDASetStatus();
            }
                break;
            case INS_LDA_ZPX:{
                Byte zeroPageAddress = FetchByte(cycles,memory);
                zeroPageAddress += X;
                cycles--;
                A = ReadByte(cycles, zeroPageAddress, memory);
                LDASetStatus();
            }
                break;
            case INS_JSR:{
                Word subAddr = FetchWord(cycles,memory);
                memory.writeWord(cycles,PC-1, SP);

                //SP++;
                PC = subAddr;
                cycles--;

            }
            break;
            default:{
                printf("Instruction is not handled %d", ins);
            }    
                break;
            }

        }
    }
};


int main(){
    Mem mem; 
    CPU cpu;

    cpu.Reset(mem);

    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IM;
    mem[0x4243] = 0x84;
    
    
    
    cpu.Execute(9,mem);
    printf("A Register: %02X\n", cpu.A);


    return 0;
}