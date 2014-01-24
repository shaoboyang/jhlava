TOP = ../..
include $(TOP)/Make.misc

TARGET=lim$(EXEEXT) melim$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

lim_OBJECTS = lim.cluster.$(OBJEXT) lim.control.$(OBJEXT) \
	lim.internal.$(OBJEXT) lim.main.$(OBJEXT) lim.policy.$(OBJEXT) \
	lim.xdr.$(OBJEXT) lim.conf.$(OBJEXT) lim.info.$(OBJEXT) \
	lim.load.$(OBJEXT) lim.misc.$(OBJEXT) lim.rload.$(OBJEXT)

lim$(EXEEXT) : $(lim_OBJECTS) $(LSFLIBS)
	$(LINK) $^ $(TCL) $(LNSL) $(LICENSING) $(ELF) $(ODM) -lm
#	$(LINK) $^ $(TCL) $(DETECTVM) $(LNSL) $(CRYPT) $(LICENSING) $(CRYPTO) $(ELF) $(ODM) -lm
melim(EXEEXT):
	$(CC) melim.c -o melim$(EXEEXT)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
