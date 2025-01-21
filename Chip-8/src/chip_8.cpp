#include "../include/chip_8.hpp"
#include <iostream>
#include <iomanip>

VChip8::VChip8(){
    //initialization
    this->randGen = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    this->randByte = std::uniform_int_distribution<uint8_t>(0, 255u); 

    this->program_counter =  this->ROM_MEM;

	this->error_code = ALL_OKAY;
    loadFontSet();

    //setting up function tables
    this->table[0x0] = (&VChip8::Table0);
	this->table[0x1] = (&VChip8::OP_1nnn);
	this->table[0x2] = (&VChip8::OP_2nnn);
	this->table[0x3] = (&VChip8::OP_3xkk);
	this->table[0x4] = (&VChip8::OP_4xkk);
	this->table[0x5] = (&VChip8::OP_5xy0);
	this->table[0x6] = (&VChip8::OP_6xkk);
	this->table[0x7] = (&VChip8::OP_7xkk);
	this->table[0x8] = (&VChip8::Table8);
	this->table[0x9] = (&VChip8::OP_9xy0);
	this->table[0xA] = (&VChip8::OP_Annn);
	this->table[0xB] = (&VChip8::OP_Bnnn);
	this->table[0xC] = (&VChip8::OP_Cxkk);
	this->table[0xD] = (&VChip8::OP_Dxyn);
	this->table[0xE] = (&VChip8::TableE);
	this->table[0xF] = (&VChip8::TableF);

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &VChip8::OP_NULL;
		table8[i] = &VChip8::OP_NULL;
		tableE[i] = &VChip8::OP_NULL;
	}
	table0[0x0] = &VChip8::OP_00E0;
	table0[0xE] = &VChip8::OP_00EE;
	table8[0x0] = &VChip8::OP_8xy0;
	table8[0x1] = &VChip8::OP_8xy1;
	table8[0x2] = &VChip8::OP_8xy2;
	table8[0x3] = &VChip8::OP_8xy3;
	table8[0x4] = &VChip8::OP_8xy4;
	table8[0x5] = &VChip8::OP_8xy5;
	table8[0x6] = &VChip8::OP_8xy6;
	table8[0x7] = &VChip8::OP_8xy7;
	table8[0xE] = &VChip8::OP_8xyE;
	tableE[0x1] = &VChip8::OP_ExA1;
	tableE[0xE] = &VChip8::OP_Ex9E;
	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &VChip8::OP_NULL;
	}
	tableF[0x07] = &VChip8::OP_Fx07;
	tableF[0x0A] = &VChip8::OP_Fx0A;
	tableF[0x15] = &VChip8::OP_Fx15;
	tableF[0x18] = &VChip8::OP_Fx18;
	tableF[0x1E] = &VChip8::OP_Fx1E;
	tableF[0x29] = &VChip8::OP_Fx29;
	tableF[0x33] = &VChip8::OP_Fx33;
	tableF[0x55] = &VChip8::OP_Fx55;
	tableF[0x65] = &VChip8::OP_Fx65;
	
}

void VChip8::loadFontSet(){
    for (unsigned int i = 0; i < this->FONT_SET_SIZE; i++){
        this->memory[this->FONT_MEM + i] = this->font_set[i];
    }
}

void VChip8::loadRom(const char* file_path){
    //binary and at the end
    std::fstream file(file_path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()){
        //add error handling/reporting here
		this->error_code = FILE_NOT_FOUND;
        return ;
    }
    std::streampos size = file.tellg();
    std::cerr<<size<<"\n";
    char* buffer = new char[size];
    file.seekg(0, std::ios::beg); //reverse to the start
    file.read(buffer, size);
    file.close();
	
	if (size > (0xFFFu - this->ROM_MEM)){
		this->error_code = ROM_OVERFLOW;
		return;
	}
    for (long i = 0; i < size; i++){
        this->memory[this->ROM_MEM +  i] =  buffer[i];
    }
    delete [] buffer;
}


int VChip8::get_error_code(){
	return this->error_code;
}

void VChip8::OP_00E0(){ // - CLS
 //clear the chip's video memory
  memset(this->video_memory, 0, sizeof(this->video_memory));
} 

void VChip8::OP_00EE(){
    //returns to the location stored in PC
    --this->stack_pointer;
    this->program_counter = this->stack[this->stack_pointer];
} // - RET

// void VChip8::OP_0nnn(){
//     //
// } // - SYS addr

void VChip8::OP_1nnn(){
    //JP addr
    uint16_t address = OP_MEMORY(this->opcode);
    this->program_counter= address;
} // - JP addr

void VChip8::OP_2nnn(){
    //CALL addr
    uint16_t address = OP_MEMORY(this->opcode);
    this->stack[this->stack_pointer] = this->program_counter;
    this->program_counter =  address;
    this->stack_pointer++;
} // - CALL addr

void VChip8::OP_3xkk(){
    //skips the next register if Vx register equals kk bytes
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;
    uint8_t byte = OP_LAST_BYTE(this->opcode);

    if (this->registers[register_idx] == byte)
        this->program_counter += 2; 
} // - SE Vx, byte

void VChip8::OP_4xkk(){
    //skips the next instruction if Vx register not equal to kk bytes
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;
    uint8_t byte = OP_LAST_BYTE(this->opcode);
    
    if (this->registers[register_idx] != byte)
        this->program_counter += 2;
} // - SNE Vx, byte

void VChip8::OP_5xy0(){
    //skip the next instruction if Vx = Vy
    //shift right to map layouts
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_2_idx = OP_REGISTER_2(this->opcode) >> 4u; 

    if (this->registers[register_idx] == this->registers[register_2_idx])
        this->program_counter += 2;
} // - SE Vx, Vy

void VChip8::OP_6xkk(){
    //Load byte into Vx register
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;
    uint8_t byte = OP_LAST_BYTE(this->opcode);

    this->registers[register_idx] = byte;
} // - LD Vx, byte

void VChip8::OP_7xkk(){
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;
    uint8_t byte = OP_LAST_BYTE(this->opcode);

    this->registers[register_idx] += byte;

} // - ADD Vx, byte

void VChip8::OP_8xy0(){
    //Load Vy into Vx
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    this->registers[register_idx_x] = this->registers[register_idx_y];
} // - LD Vx, Vy

void VChip8::OP_8xy1(){
    //bitwise OR between register Vx and Vy, Vx = Vx 
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    this->registers[register_idx_x] |= this->registers[register_idx_y];
} // - OR Vx, Vy

void VChip8::OP_8xy2(){
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    this->registers[register_idx_x] &= this->registers[register_idx_y];
} // - AND Vx, Vy

void VChip8::OP_8xy3(){
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    this->registers[register_idx_x] ^= this->registers[register_idx_y];
} // - XOR Vx, Vy

void VChip8::OP_8xy4(){
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

	uint16_t sum = this->registers[register_idx_x] + 
                        this->registers[register_idx_y];

    //setting the carry flag
	if (sum > 255U)
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	this->registers[register_idx_x] = sum & 0xFFu;
} // - ADD Vx, Vy

void VChip8::OP_8xy5(){
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    if (this->registers[register_idx_x] > this->registers[register_idx_y])
		this->registers[0xF] = 1;
	else
		this->registers[0xF] = 0;

	this->registers[register_idx_x] -= this->registers[register_idx_y];
} // - SUB Vx, Vy

void VChip8::OP_8xy6(){
    //shift right Vx by 1 and save the least significant bit to the V_F
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
	// Save LSB in VF
	this->registers[0xF] = (this->registers[register_idx_x] & 0x1u);

	this->registers[register_idx_x] >>= 1;
} // - SHR Vx by 1 

void VChip8::OP_8xy7(){
    //Set Vx = Vy - Vx, set VF = NOT borrow, basically reverse of SUB
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    if (this->registers[register_idx_x] > this->registers[register_idx_y])
		this->registers[0xF] = 1;
	else
		this->registers[0xF] = 0;

	this->registers[register_idx_x] = this->registers[register_idx_y] - this->registers[register_idx_x];

} // - SUBN Vx, Vy

void VChip8::OP_8xyE(){
    //shift left Vx by 1 and save the least significant bit to the V_F
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	// Save MSB in VF
	this->registers[0xF] = (this->registers[register_idx_x] & 0x80) >> 7u;
    
	this->registers[register_idx_x] = this->registers[register_idx_x] << 1;

} // - SHL Vx {, Vy}

void VChip8::OP_9xy0(){
    //Skip next instruction if Vx != Vy.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;

    if (this->registers[register_idx_x] != this->registers[register_idx_y])
        this->program_counter += 2; //if matches shift the program counter
} // - SNE Vx, Vy

void VChip8::OP_Annn(){
    //Load index register with address 
    uint16_t address = OP_MEMORY(this->opcode);
    this->index_register = address;
} // - LD I, addr

void VChip8::OP_Bnnn(){
    //Jump to location nnn + V0
    uint8_t address = OP_MEMORY(this->opcode);
    this->program_counter = this->registers[0] + address;
} // - JP V0, addr

void VChip8::OP_Cxkk(){
    //Set Vx = random byte AND kk.
    uint8_t register_idx = OP_REGISTER(this->opcode) >> 8u;

    uint8_t byte =  OP_LAST_BYTE(this->opcode);

	this->registers[register_idx] = randByte(randGen) & byte;
} // - RND Vx, byte

void VChip8::OP_Dxyn(){

    //draw a sprite(nibble at x and y location contained in Vx and Vy registers)
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    uint8_t register_idx_y = OP_REGISTER_2(this->opcode) >> 4u;
    uint8_t height = this->opcode & 0x000Fu;

    //wrap around
    uint8_t xPos = this->registers[register_idx_x] % this->VIDEO_WIDTH;
    uint8_t yPos = this->registers[register_idx_y] % this->VIDEO_HEIGHT;

    this->registers[0xFu] = 0; //set the 16th register to 0

    for (unsigned int row = 0; row < height; ++row) {

        //fetching the sprite byte
        uint8_t spriteByte = this->memory[this->index_register + row];
        for (unsigned int col = 0; col < 8; ++col){
            //filling the video memory
            uint8_t spritePixel = (spriteByte & (0x80u >> col));
            uint32_t* screenPixel = &this->video_memory[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            //checking for the collisions 
            if (spritePixel){
                //whether the screen pixel is high as well?
                if (*screenPixel == 0xFFFFFFFF) 
                    this->registers[0xFu] = 1;
                *screenPixel ^= 0xFFFFFFFF;
            }

        }
    }
} // - DRW Vx, Vy, nibble

void VChip8::OP_Ex9E(){
    //Skip next instruction if key with the value of Vx is pressed.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
	uint8_t key = this->registers[register_idx_x];
    //if the key was pressed?
	if (this->keypad[key])
		this->program_counter += 2;
}// - SKP Vx

void VChip8::OP_ExA1(){
    //Skip next instruction if key with the value of Vx is not pressed.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
	uint8_t key = this->registers[register_idx_x];
    //if the key was pressed?
	if (!this->keypad[key])
		this->program_counter += 2;
} // - SKNP Vx

void VChip8::OP_Fx07(){
    //Set Vx = delay timer value.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
    this->registers[register_idx_x] = this->delay_timer;
}// - LD Vx, DT

void VChip8::OP_Fx0A(){
    //Wait for a key press, store the value of the key in Vx.
  uint8_t Vx = OP_REGISTER(this->opcode) >> 8u;

	if (this->keypad[0])
	{
		this->registers[Vx] = 0;
	}
	else if (this->keypad[1])
	{
		this->registers[Vx] = 1;
	}
	else if (this->keypad[2])
	{
		this->registers[Vx] = 2;
	}
	else if (this->keypad[3])
	{
		this->registers[Vx] = 3;
	}
	else if (this->keypad[4])
	{
		this->registers[Vx] = 4;
	}
	else if (this->keypad[5])
	{
		this->registers[Vx] = 5;
	}
	else if (this->keypad[6])
	{
		this->registers[Vx] = 6;
	}
	else if (this->keypad[7])
	{
		this->registers[Vx] = 7;
	}
	else if (this->keypad[8])
	{
		this->registers[Vx] = 8;
	}
	else if (this->keypad[9])
	{
		this->registers[Vx] = 9;
	}
	else if (this->keypad[10])
	{
		this->registers[Vx] = 10;
	}
	else if (this->keypad[11])
	{
		this->registers[Vx] = 11;
	}
	else if (this->keypad[12])
	{
		this->registers[Vx] = 12;
	}
	else if (this->keypad[13])
	{
		this->registers[Vx] = 13;
	}
	else if (this->keypad[14])
	{
		this->registers[Vx] = 14;
	}
	else if (this->keypad[15])
	{
		this->registers[Vx] = 15;
	}
	else
	{
		this->program_counter -= 2;
	}
}// - LD Vx, K          

void VChip8::OP_Fx15(){
    // Set delay timer = Vx.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	this->delay_timer = this->registers[register_idx_x];
}// - LD DT, Vx

void VChip8::OP_Fx18(){
     // Set sound timer = Vx.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	this->sound_timer = this->registers[register_idx_x];
}// - LD ST, Vx

void VChip8::OP_Fx1E(){
    //Set I = I + Vx.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	this->index_register += this->registers[register_idx_x];
}// - ADD I, Vx

void VChip8::OP_Fx29(){
    //Set I = location of sprite for digit Vx.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	uint8_t digit = this->registers[register_idx_x];

	this->index_register = FONT_MEM + (5 * digit);
}// - LD F, Vx

void VChip8::OP_Fx33(){
    //Store BCD representation of Vx in memory locations I, I+1, and I+2.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
	uint8_t value = this->registers[register_idx_x];

	// Ones-place
	this->memory[this->index_register + 2] = value % 10;
	value /= 10;

	// Tens-place
	this->memory[this->index_register + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	this->memory[this->index_register] = value % 10;
}// - LD B, Vx

void VChip8::OP_Fx55(){
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;
	for (uint8_t i = 0; i <= register_idx_x; ++i)
		this->memory[this->index_register + i] = registers[i];
}// - LD [I], Vx

void VChip8::OP_Fx65(){
    // Read registers V0 through Vx from memory starting at location I.
    uint8_t register_idx_x = OP_REGISTER(this->opcode) >> 8u;

	for (uint8_t i = 0; i <= register_idx_x; ++i)
		registers[i] = this->memory[this->index_register + i];
}// - LD Vx, [I]


void VChip8::cycle(){
	//Fetch
	this->opcode = (this->memory[this->program_counter] << 8u) | this->memory[this->program_counter + 1];;

	// Increment the PC before we execute anything
	this->program_counter += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (this->delay_timer > 0)
		--this->delay_timer;

	// Decrement the sound timer if it's been set
	if (this->sound_timer > 0)
	{
		--this->sound_timer;
	}
}

std::string VChip8::get_error_name(){
	switch (this->error_code)
	{
	case ALL_OKAY:
		return "ALL OKAY";
	case ROM_OVERFLOW:
		return "Error, size of ROM is larger than the memory.";
	case FILE_NOT_FOUND:
		return "Error, couldn't load the ROM file";
	default:
		return "Uknown, error occurred";
	}
	return ""; //for the sake of return
}