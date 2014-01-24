TOP = ../..
include $(TOP)/Make.misc

TARGET=lsacct$(EXEEXT) lseligible$(EXEEXT) lshosts$(EXEEXT) lsid$(EXEEXT) lsinfo$(EXEEXT) lsloadadj$(EXEEXT) \
	lsload$(EXEEXT) lsplace$(EXEEXT) lsrcp$(EXEEXT) lsrun$(EXEEXT) lsaddhost$(EXEEXT) lsrmhost$(EXEEXT)
all: $(TARGET)

.c.o:
	$(CC) -c -o $@ $<

lsacct$(EXEEXT): lsacct.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL)  $(CRYPTO)

lseligible$(EXEEXT): lseligible.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lshosts$(EXEEXT): lshosts.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsid$(EXEEXT): lsid.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsinfo$(EXEEXT): lsinfo.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsloadadj$(EXEEXT): lsloadadj.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsload$(EXEEXT): lsload.$(OBJEXT) load.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsmon$(EXEEXT): lsmon.$(OBJEXT) load.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(NCURSES) $(LNSL) $(CRYPTO)

lsplace$(EXEEXT): lsplace.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsrcp$(EXEEXT): lsrcp.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsrun$(EXEEXT): lsrun.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsaddhost$(EXEEXT): lsaddhost.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

lsrmhost$(EXEEXT): lsrmhost.$(OBJEXT) $(LSFLIBS)
	$(LINK) $^ $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/bin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
