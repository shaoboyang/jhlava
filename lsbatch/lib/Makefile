TOP = ../..
include $(TOP)/Make.misc

TARGET=liblsbatch.$(LEXT)
all: $(TARGET)

INCLUDES = -I$(top_srcdir)/lsf/lib -I$(top_srcdir)/lsbatch

liblsbatch_OBJECTS = lsb.comm.$(OBJEXT) lsb.groups.$(OBJEXT) \
	lsb.jobs.$(OBJEXT) lsb.modify.$(OBJEXT) lsb.peek.$(OBJEXT) \
	lsb.reconfig.$(OBJEXT) lsb.signals.$(OBJEXT) \
	lsb.users.$(OBJEXT) lsb.conf.$(OBJEXT) lsb.hc.$(OBJEXT) \
	lsb.log.$(OBJEXT) lsb.move.$(OBJEXT) lsb.qc.$(OBJEXT) \
	lsb.resource.$(OBJEXT) lsb.spool.$(OBJEXT) lsb.xdr.$(OBJEXT) \
	lsb.debug.$(OBJEXT) lsb.hosts.$(OBJEXT) lsb.mig.$(OBJEXT) \
	lsb.msg.$(OBJEXT) lsb.queues.$(OBJEXT) lsb.rexecv.$(OBJEXT) \
	lsb.sub.$(OBJEXT) lsb.err.$(OBJEXT) lsb.init.$(OBJEXT) \
	lsb.misc.$(OBJEXT) lsb.params.$(OBJEXT) lsb.reason.$(OBJEXT) \
	lsb.sig.$(OBJEXT) lsb.switch.$(OBJEXT)

.c.o:
	$(CC) -c -o $@ $<

$(TARGET): $(liblsbatch_OBJECTS)
	rm -rf $(TARGET)
	$(AR) $@ $(liblsbatch_OBJECTS)
	$(RANLIB) $(TARGET)

clean:
	rm -f *.$(OBJEXT) *.d $(TARGET)
