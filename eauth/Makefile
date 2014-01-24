TOP = ..
include $(TOP)/Make.misc

TARGET = eauth$(EXEEXT)
all: $(TARGET)

INCLUDES = -I$(top_srcdir)/lsf/lib \
           -I$(top_srcdir)/lsbatch  -I$(top_srcdir)/lsbatch/lib
.c.o:
	$(CC) -c -o $@ $<

eauth$(EXEEXT) : eauth.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)