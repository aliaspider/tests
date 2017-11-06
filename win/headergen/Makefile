
TARGETS :=
TARGETS += d3d9_common.h

all: $(TARGETS)


CPPFLAGS += -P -C
#CPPFLAGS += -I../../
ASTYLEFLAGS :=
ASTYLEFLAGS += --style=allman
ASTYLEFLAGS += --indent=spaces=3
#ASTYLEFLAGS += --max-code-length=100
ASTYLEFLAGS += --pad-oper
ASTYLEFLAGS += --unpad-paren
ASTYLEFLAGS += --pad-header
ASTYLEFLAGS += --fill-empty-lines
ASTYLEFLAGS += --align-pointer=name
ASTYLEFLAGS += --align-reference=name
ASTYLEFLAGS += --remove-brackets
ASTYLEFLAGS += --convert-tabs
ASTYLEFLAGS += --break-blocks


%.h: %.c Makefile
	$(CPP) $< $(CPPFLAGS) -MT $@ -MMD -MP -MF $*.depend -c -o $@
	astyle $(ASTYLEFLAGS) $@


clean:
	rm -f $(TARGETS) $(TARGETS:.h=.depend) $(TARGETS:.h=.h.orig)

-include $(TARGETS:.h=.depend)
