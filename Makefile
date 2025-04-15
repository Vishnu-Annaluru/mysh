CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined


SRC_DIR = src
OBJDIR = obj

SRCS = mysh.c arraylist.c pwd.c cd.c which.c die.c tokenizer.c wildcard.c executor.c
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mysh

$(OBJDIR)/%.o: $(SRC_DIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) mysh