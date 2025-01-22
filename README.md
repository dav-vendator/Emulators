# Emulators

A collection of emulators developed for educational and exploration purposes, currently including:

1. **Chip-8 Interpreter**: A simple implementation to emulate the Chip-8 virtual machine.
2. **MOS 6502 Emulator**: (Upcoming) An emulator for the MOS 6502 microprocessor.

## Features
- **Chip-8 Interpreter**:
  - Configurable display scale.
  - Adjustable instruction delay.
  - Load and run Chip-8 ROMs.

## Requirements
- A C++ compiler supporting C++17 or later.
- [CMake](https://cmake.org/) for building the project.

## Usage

### Building the Project
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/Emulators.git
   cd Emulators
   ```
2. Create a build directory and compile:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

### Running the Chip-8 Emulator
After building, use the following command to run the Chip-8 emulator:
```bash
./chip8 <Display Scale> <Instruction Delay> <ROM>
```

#### Arguments:
- **Display Scale**: The scale factor for the display (e.g., `10` for 10x scaling).
- **Instruction Delay**: Delay between instructions in milliseconds.
- **ROM**: Path to the Chip-8 ROM file to load and execute.

#### Example:
```bash
./chip8 10 5 path/to/rom.ch8
```

## Roadmap
- [x] Implement Chip-8 emulator.
- [ ] Make Chip-8 Class Dynamic and add Super Chip-8 functionality
- [ ] Develop MOS 6502 emulator.
- [ ] Add comprehensive test cases for both emulators.
- [ ] Enhance documentation with diagrams and examples.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributions
Contributions are welcome! Feel free to open issues or submit pull requests to improve the project.

---
Developed with curiosity and passion for retro computing and low-level programming.


