# CHIP8 Emulator

![](images/tetris.png?raw=true "Tetris") | ![](images/slippery.png?raw=true "Slippery Slope")  
  
Fully functional CHIP8 emulator/interpreter using SDL2 for graphics and sound.  
Quirks for the SCHIP and XOCHIP are implemented, but not as proper extensions.  

# Build Instructions
Builds on Linux only.

Required dependencies:  
- CMake 3.26  
- SDL2  

Steps:
- Assuming current working directory is 'cchip8'.  
- Run "chmod +x bootstrap.sh" to give it execute permission.
- Run "./bootstrap.sh".  
- Run "./build/app example_rom.ch8" to run your desired ROM.
  
# References
[CHIP-8 Instruction Set](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set).  
[CHIP-8 Technical Reference](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference).  
  
