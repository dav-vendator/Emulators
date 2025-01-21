/*
A simple implementation of Chip-8 emulator.
Componenets:
    1. Register: 16 8-bit registers, Labeled from V0 till VF.
    2. Special Register: VF (flag register)
    3. Memory(4K Bytes), address space -> 0x000 till 0xFFF
    4. RESERVED_MEM: 0x000 -  0x1FF (interpreter)
    5. FONT_MEM: 0x050 - 0x0A0 for storing characters
    6. ROM_MEM: 0x200 - 0xFFF for ROM and program usage
    7. 16-bit Index register [indexing memory]
    8. 16-bit Program Counter [Count of next instruction to be fetched]
    9. 16-Level Stack for function calls
    10. 8-bit Stack Pointer that always points to the top of the stack
    11. 8-bit Delay Timer (decrement the value placed at 60Hz)
    12. 8-bit Sound Timer (decrement the value placed at 60Hz while producing the sound)
    13. 16 Input Keys, 0 through F mapped as follows:
             Keypad      -->         Keyboard
            +-+-+-+-+               +-+-+-+-+    
            |1|2|3|C|               |1|2|3|4|
            +-+-+-+-+               +-+-+-+-+
            |4|5|6|D|               |Q|W|E|R|
            +-+-+-+-+               +-+-+-+-+
            |7|8|9|E|               |A|S|D|F|
            +-+-+-+-+               +-+-+-+-+
            |A|0|B|F|               |Z|X|C|V|
            +-+-+-+-+               +-+-+-+-+  
    14. 64x32 Monochrome Display Memory: Each pixel/memory location is either 0 or 1, total space is 2048 or 2K bits
            PixelOnSprite XOR PixelInMemory = Pixel in Memory on or off
            to move something already drawn, first we again issue the command to draw at that same location (this removes the drawn object)
            and then issue another draw command at the new location.               
*/

#ifndef __V_CHIP_8__
#define __V_CHIP_8__

#include <cstdint>
#include <fstream>
#include <chrono>
#include <string>
#include <random>



#define OP_MEMORY(opcode) (opcode & 0x0FFFu)
#define OP_REGISTER(opcode) (opcode & 0x0F00u)
#define OP_REGISTER_2(opcode) (opcode & 0x00F0u)
#define OP_LAST_BYTE(opcode) (opcode & 0x00FFu)


class  VChip8{
    public:
         //error codes:
        enum ErrorCodes:char{
                ALL_OKAY = 0,
                FILE_NOT_FOUND,
                UNDEFINED_INSTR,
                ROM_OVERFLOW
                 //rest of the code if any
        } ;

    private:
    
        const int ROM_MEM = 0x200;
        const int FONT_MEM = 0x050;
        const unsigned int FONT_SET_SIZE  = 80;
        const unsigned int VIDEO_WIDTH = 64;
        const unsigned int VIDEO_HEIGHT = 32;

        ErrorCodes error_code;
        
        void loadFontSet();


    public:
        uint8_t  registers[16]{};
        uint8_t  memory[4096]{}; //each memory location is 8 bit --> 1 Byte * 4096
        uint8_t  keypad[16]{}; //for storing which key was pressed 
        uint16_t stack[16]{}; //16 level stack with each entry of 16 bits to store memory address
        uint32_t video_memory[2048]{}; //for compatibility with SDL using uint32_t


        using Chip8Func = void (VChip8::*) (); //function pointer
        
        Chip8Func table[0xF + 1];
        Chip8Func table0[0xE + 1];
        Chip8Func table8[0xE + 1];
        Chip8Func tableE[0xE + 1];
        Chip8Func tableF[0x65 + 1];

        uint8_t font_set[80] = {
            //every 1 is a pixel active and 0 is pixel off
	            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 
	            0x20, 0x60, 0x20, 0x20, 0x70, // 1
	            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        }; //font set for storing 16 characters each of 5 * 8 bits

        uint16_t index_register{};
        uint16_t program_counter{};
        uint8_t stack_pointer{};
        
        uint8_t delay_timer{};
        uint8_t sound_timer{};
        uint8_t keypad_mapping[16]{};
        uint16_t opcode;

        std::default_random_engine randGen;
        std::uniform_int_distribution<uint8_t> randByte;

        VChip8();
        //functions
        void loadRom(const char*);

        //see: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
        void OP_00E0(); // - CLS
        void OP_00EE(); // - RET
//      void OP_0nnn(); // - SYS addr, not implemented
        void OP_1nnn(); // - JP addr
        void OP_2nnn(); // - CALL addr
        void OP_3xkk(); // - SE Vx, byte
        void OP_4xkk(); // - SNE Vx, byte
        void OP_5xy0(); // - SE Vx, Vy
        void OP_6xkk(); // - LD Vx, byte
        void OP_7xkk(); // - ADD Vx, byte
        void OP_8xy0(); // - LD Vx, Vy
        void OP_8xy1(); // - OR Vx, Vy
        void OP_8xy2(); // - AND Vx, Vy
        void OP_8xy3(); // - XOR Vx, Vy
        void OP_8xy4(); // - ADD Vx, Vy
        void OP_8xy5(); // - SUB Vx, Vy
        void OP_8xy6(); // - SHR Vx {, Vy}
        void OP_8xy7(); // - SUBN Vx, Vy
        void OP_8xyE(); // - SHL Vx {, Vy}
        void OP_9xy0(); // - SNE Vx, Vy
        void OP_Annn(); // - LD I, addr
        void OP_Bnnn(); // - JP V0, addr
        void OP_Cxkk(); // - RND Vx, byte
        void OP_Dxyn(); // - DRW Vx, Vy, nibble
        void OP_Ex9E(); // - SKP Vx
        void OP_ExA1(); // - SKNP Vx
        void OP_Fx07(); // - LD Vx, DT
        void OP_Fx0A(); // - LD Vx, K
        void OP_Fx15(); // - LD DT, Vx
        void OP_Fx18(); // - LD ST, Vx
        void OP_Fx1E(); // - ADD I, Vx
        void OP_Fx29(); // - LD F, Vx
        void OP_Fx33(); // - LD B, Vx
        void OP_Fx55(); // - LD [I], Vx
        void OP_Fx65(); // - LD Vx, [I]

        void cycle(); //fetch-decode-execute

        int get_error_code();

        std::string get_error_name();
        
        //tables 
        // Set up function pointer table
		
	void Table0()
	{
		((*this).*(table0[opcode & 0x000Fu]))();
	}

	void Table8()
	{
		((*this).*(table8[opcode & 0x000Fu]))();
	}

	void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}

	void OP_NULL()
	{}

        
        //TODO: Add super chip-8 instructions


        
};

#endif