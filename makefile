BASEFLAGS := -Wall -pedantic -pipe -std=c++14 -fopenmp
DEBUGFLAGS := -g -pg
RELEASEFLAGS := -O3 -march=native -flto -DNDEBUG
CLIBS := -pthread
CXXFLAGS := $(BASEFLAGS)
APPNAME := 4981_ass3
ODIR := bin
SRC := src

OBJS := $(ODIR)/%.o
SRCOBJS := %.cpp
#Find all .cpp files recursively, optionally excluding directories if FINDARGS is used
SRCWILD := $(shell find . -name *\.cpp | tr '\n' ' ')
HEADWILD := $(shell find . -name *\.h | tr '\n' ' ')

#Convert all source files to bin.o equivalent
CONVERT := $(patsubst $(SRCOBJS), $(OBJS), $(shell basename -a $(SRCWILD)))

EXEC := $(ODIR)/$(APPNAME)
DEPS := $(EXEC).d

release: all
debug: all

all: $(CONVERT)
# Command takes all bin .o files and creates an executable called chess in the bin folder
	$(CXX) $(CFLAGS) $(CXXFLAGS) $^ $(CLIBS) -o $(EXEC)

$(ODIR):
	@mkdir -p $(ODIR)

# Create dependency file for make and manually adjust it silently to work with other directories
$(DEPS): $(SRCWILD) $(HEADWILD) | $(ODIR) 
# Compile the non-system dependencies and store it in outputdir/execname.d
	@$(CXX) -MM $(CFLAGS) $(CXXFLAGS) $(SRCWILD) > $(DEPS)
# Take the temp file contents, do a regex text replace to change all .o strings into
# outputdir/.o strings, and store the result in outputdir/execname.d
	@sed -i.bak -e 's/\w\+\.o/$(ODIR)\/&/g' $(DEPS)
# Delete the temp file
	@$(RM) $(DEPS).bak

# Add the dependencies into make and don't throw an error if it doesn't exist
# Also don't generate dependency file during a clean
ifeq (,$(filter clean, $(MAKECMDGOALS)))
-include $(DEPS)
endif

#Check if in debug mode and set the appropriate compile flags
ifeq (,$(filter debug dserver, $(MAKECMDGOALS)))
$(eval CXXFLAGS := $(BASEFLAGS) $(RELEASEFLAGS))
else
$(eval CXXFLAGS := $(BASEFLAGS) $(DEBUGFLAGS))
endif

#Target needed for use of automatic variable used below
.SECONDEXPANSION:

# Target is any bin .o file, prereq is the equivalent src .cpp file
$(OBJS): $(filter .+$$@, $(SRCWILD))
# Command compiles the src .cpp file with the listed flags and turns it into a bin .o file
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $< -o $@

# Prevent clean from trying to do anything with a file called clean
.PHONY: clean

# Deletes the executable and all .o and .d files in the bin folder
clean: | $(ODIR)
	$(RM) $(EXEC) $(wildcard $(ODIR)/server*) $(wildcard $(EXEC).*) $(wildcard $(ODIR)/*.d*) $(wildcard $(ODIR)/*.o)

