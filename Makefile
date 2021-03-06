.PHONY: all clean mrproper re

CC			:=	cc
#C_STD      :=  -std=c99
INCLUDE		:=	fgfl/inc fgfp/inc tools/inc bootstrap/
#CC_OPTI    :=  -O3

CC_FLAGS	:=	-Wall -Wextra $(addprefix -I,$(INCLUDE))\
                $(C_STD)  $(CC_OPTI) -g # -Werror

CC_MACRO	:=  -DPRINT_DEBUG
LD_FLAGS	:=	# -pg

include tools/Makefile
TOOLS_OBJ	:=	$(addprefix tools/,$(TOOLS_OBJ))

include fgfl/Makefile
FGFL_OBJ	:=	$(addprefix fgfl/,$(FGFL_OBJ))

include fgfp/Makefile
FGFP_OBJ	:=	$(addprefix fgfp/,$(FGFP_OBJ))

ALL_OBJ		:=	$(TOOLS_OBJ) $(FGFL_OBJ) $(FGFP_OBJ)

EFGFL		:=	efgfl
EFGFP		:=	efgfp

all: $(EFGFL) $(EFGFP);

$(EFGFL): $(TOOLS_OBJ) $(FGFL_OBJ)
	$(CC) $^ -o $@ $(LD_FLAGS)

$(EFGFP): $(TOOLS_OBJ) $(FGFP_OBJ)
	$(CC) $^ -o $@ $(LD_FLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CC_FLAGS) $(CC_MACRO)

clean:
	@rm -rf $(ALL_OBJ)

mrproper: clean
	@rm -rf $(EFGFL)
	@rm -rf $(EFGFP)

re: mrproper all;
