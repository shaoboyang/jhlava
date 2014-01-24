TOP = ../..
include $(TOP)/Make.misc

TARGET=liblsfint.$(LEXT)
all: $(TARGET)

liblsfint_OBJECTS = admin.$(OBJEXT) cmdtime.$(OBJEXT) jidx.$(OBJEXT) lsftcl.$(OBJEXT) \
	resreq.$(OBJEXT) bitset.$(OBJEXT) conf.$(OBJEXT) list.$(OBJEXT) misc.$(OBJEXT) \
	userok.$(OBJEXT) window.$(OBJEXT) callex.$(OBJEXT) daemon.$(OBJEXT) listset.$(OBJEXT) \
	resourcecmd.$(OBJEXT) testbitset.$(OBJEXT) list2.$(OBJEXT) link.$(OBJEXT) 

.c.o:
	$(CC) -c -o $@ $<

$(TARGET): $(liblsfint_OBJECTS)
	rm -rf $(TARGET)
	$(AR) $@ $(liblsfint_OBJECTS)
	$(RANLIB) $(TARGET)

clean:
	rm -f *.$(OBJEXT) *.d $(TARGET)