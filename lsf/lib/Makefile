TOP = ../..
include $(TOP)/Make.misc

TARGET=liblsf.$(LEXT)
all: $(TARGET)

liblsf_OBJECTS = lib.channel.$(OBJEXT) lib.err.$(OBJEXT) \
	lib.limits.$(OBJEXT) lib.rcp.$(OBJEXT) lib.so.$(OBJEXT) \
	lib.xdr.$(OBJEXT) lib.comm.$(OBJEXT) lib.errno.$(OBJEXT) \
	lib.load.$(OBJEXT) lib.rdwr.$(OBJEXT) lib.sock.$(OBJEXT) \
	lib.xdrlim.$(OBJEXT) lib.conf.$(OBJEXT) lib.esub.$(OBJEXT) \
	lib.lsf.$(OBJEXT) lib.res.$(OBJEXT) lib.syntax.$(OBJEXT) \
	lib.xdrmisc.$(OBJEXT) lib.confmisc.$(OBJEXT) \
	lib.misc.$(OBJEXT) lib.resctrl.$(OBJEXT) \
	lib.syslog.$(OBJEXT) lib.xdrnio.$(OBJEXT) lib.conn.$(OBJEXT) \
	lib.host.$(OBJEXT) lib.nioback.$(OBJEXT) lib.reslog.$(OBJEXT) \
	lib.table.$(OBJEXT) lib.xdrres.$(OBJEXT) lib.control.$(OBJEXT) \
	lib.i18n.$(OBJEXT) lib.nios.$(OBJEXT) lib.rex.$(OBJEXT) \
	lib.term.$(OBJEXT) lib.xdrrf.$(OBJEXT) lib.cwd.$(OBJEXT) \
	lib.id.$(OBJEXT) lib.osal.$(OBJEXT) lib.rf.$(OBJEXT) \
	lib.tid.$(OBJEXT) usleep.$(OBJEXT) lib.debug.$(OBJEXT) \
	lib.info.$(OBJEXT) lib.pim.$(OBJEXT) \
	lib.tty.$(OBJEXT) whathost.$(OBJEXT) lib.dir.$(OBJEXT) \
	lib.init.$(OBJEXT) lib.place.$(OBJEXT) lib.rtask.$(OBJEXT) \
	lib.utmp.$(OBJEXT) lib.eauth.$(OBJEXT) lib.initenv.$(OBJEXT) \
	lib.priority.$(OBJEXT) lib.rwait.$(OBJEXT) lib.wconf.$(OBJEXT) \
	lib.eligible.$(OBJEXT) lib.lim.$(OBJEXT) lib.queue.$(OBJEXT) \
	lib.sig.$(OBJEXT) lib.words.$(OBJEXT) \
	$(TOP)/jhtools/encrypt.$(OBJEXT) $(TOP)/jhtools/pwdfile.$(OBJEXT)

.c.o:
	$(CC) -c -o $@ $<

liblsf.$(LEXT): $(liblsf_OBJECTS)
	rm -rf liblsf.$(LEXT)
	$(AR) $@ $(liblsf_OBJECTS)
	$(RANLIB) liblsf.$(LEXT)

clean:
	rm -f *.$(OBJEXT) *.d $(TARGET)
