TOP = ../..
include $(TOP)/Make.misc

TARGET=pim$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

pim_OBJECTS = pim.main.$(OBJEXT) 

$(TARGET) : $(pim_OBJECTS) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)