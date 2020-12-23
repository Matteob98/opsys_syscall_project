C_FILES := $(wildcard ./*.c)
EXEC_FILES := $(notdir $(C_FILES:.c=))

all: $(EXEC_FILES)

clean:
	rm -f $(EXEC_FILES)

%: %.c
	gcc -o $@ $< -lm -ggdb
