.PHONY: all clean mrproper re

CC		:=	cc
CC_FLAGS	:=	-Wall -Wextra -Werror -Iinc  -Itools/inc -g
CC_MACRO	:=	-DOPTIMIZE
LD_FLAGS	:=
OBJ		:=	main.o dfa.o tokendef.o nfa.o regex.o utils.o dfagen.o
OBJ		:=	$(addprefix src/,$(OBJ))
TARGET		:=	fgfl

all: $(TARGET);

$(TARGET): $(OBJ)
	$(MAKE) -C tools/ 
	$(CC) $^ -o $@ -ltools -Ltools/ $(LD_FLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CC_FLAGS) $(CC_MACRO)

clean:
	@rm -rf $(OBJ)
	$(MAKE) -C tools/ clean

mrproper: clean
	@rm -rf $(TARGET)
	$(MAKE) -C tools/ mrproper

re: mrproper all;
