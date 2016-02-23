#Sursa: http://ubuntuforums.org/showthread.php?t=1181546
#Rugati-va sa nu se strice, ca eu nu inteleg nimic 
#din ce scrie aici :))

#Compiler
CC = g++
OPTS = -std=c++11 -c -Wall -g

#Project name
PROJECT = passive_front_end

#Directories
OBJDIR = obj
SRCDIR = src

#Files and folders
SRCS = $(shell find $(SRCDIR) -name '*.cpp')
#for every file .cpp will create a .o file in objdir
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

#Targets
$(PROJECT): buildrepo $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm $(PROJECT) $(OBJDIR) -Rf

buildrepo:
	@$(call make-repo)

#Create obj directory structure
define make-repo
	mkdir -p $(OBJDIR)
endef
