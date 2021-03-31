#include <iostream>

using BYTE = unsigned char;
using WORD = unsigned short;
using u32 = unsigned int;

struct MEM
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    BYTE data[MAX_MEM];

    void Initialize()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            data[i] = 0;
        }
    }

    BYTE operator[](u32 address) const
    {
        return data[address];
    }

    BYTE &operator[](u32 address)
    {
        return data[address];
    }

    void WriteWord(u32 address, u32 &cycles, WORD value)
    {
        data[address] = value & 0xff;
        data[address + 1] = (value >> 8);
        cycles -= 2;
    }
};

struct CPU
{

    WORD PC; //program counter
    WORD SP; //stack pointer

    BYTE A, X, Y; //registers

    BYTE Z : 1; // zero flag
    BYTE C : 1; // carry flag
    BYTE I : 1; // interuubp disable flag
    BYTE D : 1; // decimal flag
    BYTE B : 1; // B flag
    BYTE V : 1; // overflow flag
    BYTE N : 1; // negative flag

    void Reset(MEM &memory)
    {
        PC = 0xfffc;
        SP = 0x0100;
        A = X = Y = 0;
        Z = C = I = D = B = V = N = 0;

        memory.Initialize();
    }

    void LDA_Status()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    BYTE FetchByte(u32 &cycles, MEM &memory)
    {
        BYTE data = memory[PC++];
        cycles--;
        return data;
    }

    WORD FetchWord(u32 &cycles, MEM &memory)
    {
        WORD data = memory[PC++];
        data |= (memory[PC++] << 8);
        cycles -= 2;
        return data;
    }

    BYTE ReadByte(u32 address, u32 &cycles, MEM &memory)
    {
        BYTE data = memory[address];
        cycles--;
        return data;
    }

    static constexpr BYTE
        INS_LDA_IM = 0xa9,
        INS_LDA_ZP = 0xa5,
        INS_LDA_ZPX = 0xb5,
        INS_JSR = 0x20;

    void Execute(u32 cycles, MEM &memory)
    {
        while (cycles)
        {
            BYTE INS = FetchByte(cycles, memory);
            switch (INS)
            {
            case INS_LDA_IM:
            {
                BYTE value = FetchByte(cycles, memory);
                A = value;
                std::cout << (int)A;
                LDA_Status();
                break;
            }
            case INS_LDA_ZP:
            {
                BYTE zeroPageAdress = FetchByte(cycles, memory);
                A = ReadByte(zeroPageAdress, cycles, memory);
                LDA_Status();
                break;
            }

            case INS_LDA_ZPX:
            {
                BYTE zeroPageAdress = FetchByte(cycles, memory);

                zeroPageAdress += X;
                cycles--;

                A = ReadByte(zeroPageAdress, cycles, memory);
                LDA_Status();
                break;
            }

            case INS_JSR:
            {
                WORD SubRoutineAddr = FetchWord(cycles, memory);
                memory.WriteWord(SP, cycles, PC - 1);
                SP += 2;
                PC = SubRoutineAddr;
                cycles--;

                break;
            }

            default:
            {
                printf("Incorrect instruction!\n");
            }
            break;
            }
        }
    }
};

int main()
{

    MEM mem;
    CPU cpu;
    cpu.Reset(mem);
    mem[0xfffc] = CPU::INS_JSR;
    mem[0xfffd] = 0x12;
    mem[0xfffe] = 0x34;
    mem[0x3412] = CPU::INS_LDA_IM;
    mem[0x3413] = 0xc;

    cpu.Execute(8, mem);
}