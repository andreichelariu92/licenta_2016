#Sursa: http://ubuntuforums.org/showthread.php?t=1181546
#Rugati-va sa nu se strice, ca eu nu inteleg nimic 
#din ce scrie aici :))

#Compiler
CC = g++
OPTS = -std=c++11 -c -Wall -g -lpthread

#Project name
PROJECT = liblog.a

#Directories
OBJDIR = ../obj
SRCDIR = .

#Files and folders
SRCS = $(shell find $(SRCDIR) -name '*.cpp')
#for every file .cpp will create a .o file in objdir
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

#Targets
$(PROJECT): buildrepo $(OBJS)
	ar rcs $(PROJECT) $(OBJS)
	mv $(PROJECT) ../

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(OPTS) -c $< -o $@
	ctags -f ./tags -R ./*

clean:
	rm $(PROJECT) $(OBJDIR) -Rf

buildrepo:
	@$(call make-repo)

#Create obj directory structure
define make-repo
	mkdir -p $(OBJDIR)
endef
