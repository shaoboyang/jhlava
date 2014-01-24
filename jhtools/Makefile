TOP = ..
include $(TOP)/Make.misc

TARGET=jhpasswd$(EXEEXT)

all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

jhpasswd$(EXEEXT) : jhpasswd.$(OBJEXT) encrypt.$(OBJEXT) pwdfile.$(OBJEXT)
	$(LINK) $^ -lcrypto -lglib-2.0

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin/$(BINARY_TYPE)
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 jhpasswd$(EXEEXT) $(DISTDIR)/bin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
