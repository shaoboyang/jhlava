TOP = ..
include $(TOP)/Make.misc

DEFAULT_METHOD= echkpnt.default erestart.default
TARGET= echkpnt$(EXEEXT) erestart$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

echkpnt$(EXEEXT): echkpnt.$(OBJEXT) echkpnt.env.$(OBJEXT) echkpnt.lib.$(OBJEXT) ../lsf/lib/liblsf.a
	$(LINK) $^ $(LNSL) $(CRYPTO)

erestart$(EXEEXT):  erestart.$(OBJEXT) echkpnt.env.$(OBJEXT) echkpnt.lib.$(OBJEXT) ../lsf/lib/liblsf.a
	$(LINK) $^ $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(DEFAULT_METHOD) $(DISTDIR)/sbin/$(BINARY_TYPE)
	
clean:
	rm -f *.$(OBJEXT) $(TARGET)
