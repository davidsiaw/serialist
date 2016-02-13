V = 0
AT_0 := @
AT_1 :=
REDIR_0 := > /dev/null
REDIR_1 :=
AT = $(AT_$(V))
REDIR = $(REDIR_$(V))

CXXFLAGS=-g -Wall -std=gnu++11
	
LDFLAGS=
TARGET=../exe/serialist-parse
SRCDIR=.
SRC=$(shell find $(SRCDIR) -name "*.cpp") 
OUT=obj
OBJS=$(SRC:%.cpp=$(OUT)/%.o)
DEPS=$(SRC:%.cpp=$(OUT)/%.d)
NODEPS=clean

.PHONY = all clean

.SECONDEXPANSION:

all: $(TARGET)
	$(AT)echo "Build complete"

clean:
	$(AT)-rm -rf $(TARGET) $(OUT)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(DEPS)
endif

$(TARGET): $(OBJS) $(LUA)
	$(AT)echo [LINK] $@
	$(AT)$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ 

%/.f:
	$(AT)echo [MKDIR] $(dir $@)
	$(AT)mkdir -p $(dir $@) 
	$(AT)touch $@

$(OUT)/%.d:%.cpp $$(@D)/.f 
	$(AT)echo [DEP] $<
	$(AT)$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.d,%.o,$@)' $< -MF $@

$(OUT)/%.o:%.cpp $(OUT)/%.d
	$(AT)echo [C++] $<
	$(AT)$(CXX) $< $(CXXFLAGS) -c -o $@

.PRECIOUS: %/.f

