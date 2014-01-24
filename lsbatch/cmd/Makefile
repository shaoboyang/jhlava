TOP = ../..
include $(TOP)/Make.misc

TARGET= badmin$(EXEEXT) bkill$(EXEEXT) bparams$(EXEEXT) brestart$(EXEEXT) btop$(EXEEXT) bbot$(EXEEXT) bmgroup$(EXEEXT) \
	bpeek$(EXEEXT) brun$(EXEEXT) busers$(EXEEXT) bhosts$(EXEEXT) bmig$(EXEEXT) bqueues$(EXEEXT) bsub$(EXEEXT) bjobs$(EXEEXT) bmod$(EXEEXT) \
	brequeue$(EXEEXT) bswitch$(EXEEXT)

all: $(TARGET)

INCLUDES = -I$(TOP)/lsf/lib -I$(TOP)/lsbatch -I$(TOP)/lsbatch/lib


.c.o:
	$(CC) -c -o $@ $<


badmin_OBJECTS = badmin.$(OBJEXT) cmd.bqc.$(OBJEXT) \
	cmd.hist.$(OBJEXT) cmd.bhc.$(OBJEXT) cmd.misc.$(OBJEXT) \
	cmd.job.$(OBJEXT) cmd.prt.$(OBJEXT)

badmin$(EXEEXT) : $(badmin_OBJECTS) ../../lsf/lsadm/startup.o ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bbot_OBJECTS = bbot.$(OBJEXT) cmd.move.$(OBJEXT) \
	cmd.jobid.$(OBJEXT) cmd.misc.$(OBJEXT) cmd.prt.$(OBJEXT) \
	cmd.err.$(OBJEXT)

bbot$(EXEEXT) : $(bbot_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bhosts_OBJECTS = bhosts.$(OBJEXT) cmd.prt.$(OBJEXT) \
	cmd.misc.$(OBJEXT)

bhosts$(EXEEXT) : $(bhosts_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bjobs_OBJECTS = bjobs.$(OBJEXT) cmd.prt.$(OBJEXT) cmd.err.$(OBJEXT) \
	cmd.job.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.misc.$(OBJEXT)

bjobs$(EXEEXT) : $(bjobs_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bkill_OBJECTS = bkill.$(OBJEXT) cmd.sig.$(OBJEXT) \
	cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

bkill$(EXEEXT) : $(bkill_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bmgroup_OBJECTS = bmgroup.$(OBJEXT) cmd.misc.$(OBJEXT)

bmgroup$(EXEEXT) : $(bmgroup_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bmig_OBJECTS = bmig.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

bmig$(EXEEXT) : $(bmig_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bmod_OBJECTS = bmod.$(OBJEXT) cmd.sub.$(OBJEXT) cmd.jobid.$(OBJEXT) \
	cmd.err.$(OBJEXT)

bmod$(EXEEXT) : $(bmod_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bparams_OBJECTS = bparams.$(OBJEXT)

bparams$(EXEEXT) : $(bparams_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bpeek_OBJECTS = bpeek.$(OBJEXT) cmd.err.$(OBJEXT) \
	cmd.jobid.$(OBJEXT) cmd.misc.$(OBJEXT) cmd.prt.$(OBJEXT)

bpeek$(EXEEXT) : $(bpeek_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bqueues_OBJECTS = bqueues.$(OBJEXT) cmd.prt.$(OBJEXT) cmd.misc.$(OBJEXT)

bqueues$(EXEEXT) : $(bqueues_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)





brequeue_OBJECTS = brequeue.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

brequeue$(EXEEXT) : $(brequeue_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)





brestart_OBJECTS = brestart.$(OBJEXT) cmd.sub.$(OBJEXT) \
	cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

brestart$(EXEEXT) : $(brestart_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)





brun_OBJECTS = brun.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

brun$(EXEEXT) : $(brun_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)





bsub_OBJECTS = bsub.$(OBJEXT) cmd.sub.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

bsub$(EXEEXT) : $(bsub_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




bswitch_OBJECTS = bswitch.$(OBJEXT) cmd.jobid.$(OBJEXT) cmd.err.$(OBJEXT)

bswitch$(EXEEXT) : $(bswitch_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




btop_OBJECTS = btop.$(OBJEXT) cmd.move.$(OBJEXT) \
	cmd.jobid.$(OBJEXT) cmd.misc.$(OBJEXT) cmd.prt.$(OBJEXT) \
	cmd.err.$(OBJEXT)
btop$(EXEEXT) : $(btop_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)




busers_OBJECTS = busers.$(OBJEXT) cmd.misc.$(OBJEXT)

busers$(EXEEXT) : $(busers_OBJECTS) ../lib/liblsbatch.a $(LSFLIBS)
	$(LINK) $^ -lm $(LNSL) $(CRYPTO)


package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin/$(BINARY_TYPE)
	$(INSTALL) -m 755 $(TARGET) $(DISTDIR)/bin/$(BINARY_TYPE)

clean:
	rm -f *.$(OBJEXT) $(TARGET)
