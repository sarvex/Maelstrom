
ACLOCAL_AMFLAGS = -I acinclude

bin_PROGRAMS = Maelstrom
noinst_PROGRAMS = MaelstromLobby

Maelstrom_SOURCES = mainstub.cpp

Maelstrom_LDADD = 			\
	game/libgame.a		\
	screenlib/libSDLscreen.a	\
	maclib/libSDLmac.a		\
	utils/libutils.a		\
	@MATHLIB@ @INETLIB@ @ICONOBJ@

MaelstromLobby_SOURCES = MaelstromLobby.cpp

MaelstromLobby_LDADD = @INETLIB@

SUBDIRS = game screenlib maclib utils

EXTRA_DIST =			\
	Docs			\
	COPYING			\
	COPYING.GPL		\
	CREDITS			\
	Changelog		\
	Maelstrom.spec		\
	Maelstrom_Data.zip	\
	README.joystick		\
	README.network		\
	README.options		\
	README.simple		\
	VisualC			\
	Xcode			\
	autogen.sh		\
	ico_o			\
	icon.xpm

DIST_SUBDIRS = $(SUBDIRS)

install-exec-hook:
	make append-data BINARY="$(DESTDIR)$(bindir)/Maelstrom$(EXEEXT)"

append-data:
	cat Maelstrom_Data.zip >>$(if $(BINARY),$(BINARY),Maelstrom$(EXEEXT))

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz
