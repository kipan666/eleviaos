CPP_FILES=$(shell find . -type f -name '*.cpp')
S_FILES=$(shell find . -type f -name '*.asm')
DFILES=$(CPP_FILES:.cpp=.d)
FONT_OBJ=../resources/fonts/default.o 
OBJ=$(patsubst ./%.cpp,../build/kernel/%.o,$(CPP_FILES)) $(patsubst %.asm,../build/kernel/%_asm.o,$(S_FILES))
KERNEL=../build/kernel.elf

CFLAGS= -g -O2 -pipe -Wall -Wextra -c\
	-std=c++03            \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-stack-check     \
	-fno-lto             \
	-fno-pie             \
	-fno-pic             \
	-m64                 \
	-march=x86-64        \
	-mabi=sysv           \
	-mno-80387           \
	-mno-mmx             \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone        \
	-mcmodel=kernel      \
	-MMD                 \
	-nostdlib           \
	-nodefaultlibs    \
	-O2                 \
	-I .

# Internal linker flags that should not be changed by the user.
LDFLAGS = -nostdlib               \
	-static                 \
	-m elf_x86_64           \
	-z max-page-size=0x1000 \
	-T linker.ld

.PHONY: all
all: $(OBJ)	
	x86_64-elf-ld $(LDFLAGS) $(OBJ) $(FONT_OBJ) -o $(KERNEL)

-include $(DFILES)
	
../build/kernel/%.o:%.cpp	
	mkdir -p $(dir $@)
	x86_64-elf-g++ $(CFLAGS) $< -o $@	

../build/kernel/%_asm.o:%.asm
	nasm -f elf64 $< -o $@

.PHONY: clean
clean:
	rm -f $(KERNEL) $(OBJ) $(DFILES)	