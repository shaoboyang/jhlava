TOP = ../..
include $(TOP)/Make.misc

TARGET=lsadmin$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

lsadmin_OBJECTS = lsadmin.$(OBJEXT) startup.$(OBJEXT) \
	cmd.limcontrol.$(OBJEXT) cmd.rescontrol.$(OBJEXT)

$(TARGET) : $(lsadmin_OBJECTS) $(LSFLIBS)
	$(LINK) $^ $(TCL) $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/bin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
