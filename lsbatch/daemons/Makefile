TOP = ../..
include $(TOP)/Make.misc

TARGET=mbatchd$(EXEEXT) sbatchd$(EXEEXT)

all: $(TARGET)


.c.o:
	$(CC) -c -o $@ $<

mbatchd_OBJECTS = mbd.comm.$(OBJEXT) mbd.host.$(OBJEXT) \
	mbd.jgrp.$(OBJEXT) mbd.main.$(OBJEXT) mbd.proxy.$(OBJEXT) \
	mbd.resource.$(OBJEXT) mbd.dep.$(OBJEXT) mbd.init.$(OBJEXT) \
	mbd.job.$(OBJEXT) mbd.misc.$(OBJEXT) mbd.queue.$(OBJEXT) \
	mbd.serv.$(OBJEXT) mbd.grp.$(OBJEXT) mbd.jarray.$(OBJEXT) \
	mbd.log.$(OBJEXT) mbd.policy.$(OBJEXT) mbd.requeue.$(OBJEXT) \
	mbd.window.$(OBJEXT) elock.$(OBJEXT) misc.$(OBJEXT) \
	mail.$(OBJEXT) daemons.$(OBJEXT) daemons.xdr.$(OBJEXT) \
	mbd.preempt.$(OBJEXT) \
	mbd.fairshare.o ugtree.o jhReadConf.o

mbatchd$(EXEEXT) : $(mbatchd_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -ltcl -lm -lglib-2.0 $(LNSL) $(CRYPTO)

sbatchd_OBJECTS = sbd.comm.$(OBJEXT) sbd.file.$(OBJEXT) \
	sbd.job.$(OBJEXT) sbd.main.$(OBJEXT) sbd.misc.$(OBJEXT) \
	sbd.policy.$(OBJEXT) sbd.serv.$(OBJEXT) sbd.sig.$(OBJEXT) \
	sbd.xdr.$(OBJEXT) elock.$(OBJEXT) mail.$(OBJEXT) \
	misc.$(OBJEXT) daemons.$(OBJEXT) daemons.xdr.$(OBJEXT)

sbatchd$(EXEEXT) : $(sbatchd_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ $(TCL) -lm $(LNSL) $(CRYPTO)

fairshare$(EXEEXT) : mbd.fairshare.o ugtree.o jhReadConf.o $(LSFLIBS)
	$(LINK) $^ -lglib-2.0 $(CRYPTO)

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/sbin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/sbin/$(BINARY_TYPE) 

clean:
	rm -f *.$(OBJEXT) $(TARGET)
