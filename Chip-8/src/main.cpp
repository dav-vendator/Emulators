#include "../include/chip_8.hpp"
#include "../include/platform.hpp"
#include <iostream>

const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

int main(int argc, char** argv){
   
	if (argc != 4){
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

    std::cout<<"Loading";
	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];
	Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);
	VChip8 chip8;
	chip8.loadRom(romFilename);
	int videoPitch = sizeof(chip8.video_memory[0]) * VIDEO_WIDTH;
	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	if (chip8.get_error_code() != VChip8::ALL_OKAY){
		std::cout<<"\n"<<chip8.get_error_name();
		return -1;
	}

	while (!quit){
		quit = platform.processInput(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
         
		if (dt > cycleDelay){
			lastCycleTime = currentTime;
			chip8.cycle();
			platform.update(chip8.video_memory, videoPitch);
		}

		if (chip8.get_error_code() != VChip8::ALL_OKAY){
			std::cout<<"\nAn error occurred: "<<chip8.get_error_code();
			std::cout<<"\n"<<chip8.get_error_name();
			return -1;
		}
	}

	return 0;
}
