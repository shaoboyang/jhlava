TOP = ../..
include $(TOP)/Make.misc

TARGET = bhist$(EXEEXT)
all: $(TARGET)

INCLUDES = -I$(top_srcdir)/lsf/lib -I$(top_srcdir)/lsbatch  -I$(top_srcdir)/lsbatch/lib

.c.o:
	$(CC) -c -o $@ $<

bhist$(EXEEXT) : bhist.$(OBJEXT) read.event.$(OBJEXT) ../cmd/cmd.job.o ../cmd/cmd.misc.o ../cmd/cmd.jobid.o ../cmd/cmd.prt.o ../cmd/cmd.err.o ../cmd/cmd.hist.o ../lib/liblsbatch.a $(LSFLIBS) 
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/bin/$(BINARY_TYPE) 

clean:
	rm -f *.$(OBJEXT) $(TARGET)
