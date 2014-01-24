TOP = ../..
include $(TOP)/Make.misc

TARGET=res$(EXEEXT) nios$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

res_OBJECTS = res.$(OBJEXT) res.handler.$(OBJEXT) \
	res.misc.$(OBJEXT) res.rf.$(OBJEXT) res.getproc.$(OBJEXT) \
	res.init.$(OBJEXT) res.pty.$(OBJEXT) res.tasklog.$(OBJEXT)

nios_OBJECTS = nios.$(OBJEXT) nios.handler.$(OBJEXT)

res$(EXEEXT) : $(res_OBJECTS) $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO) $(TCL)



nios$(EXEEXT) : $(nios_OBJECTS) ../../lsbatch/lib/liblsbatch.a $(LSFLIBS) 
	$(LINK) $^ -lm $(TCL) $(LNSL) $(CRYPTO)


package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
