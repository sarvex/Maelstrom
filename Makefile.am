

bin_PROGRAMS = Maelstrom
noinst_PROGRAMS = MaelstromLobby

Maelstrom_SOURCES = main.cpp

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
	MPWmake.sea.bin		\
	Maelstrom.spec		\
	Maelstrom_Data.zip	\
	PBProjects.tar.gz	\
	README.joystick		\
	README.network		\
	README.options		\
	README.simple		\
	VisualC.zip		\
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

# Rule to build a Mac OS X application
Maelstrom.app: Maelstrom$(EXEEXT)
	unzip $@.zip
	cp $< $@/Contents/MacOS/$<
	strip $@/Contents/MacOS/$<
	install_name_tool -change `otool -L $< | fgrep libSDL-1.3.0.dylib | awk '{print $$1}'` @executable_path/../Frameworks/libSDL-1.3.0.dylib $@/Contents/MacOS/$<
	install_name_tool -change `otool -L $< | fgrep libSDL_net-1.2.0.dylib | awk '{print $$1}'` @executable_path/../Frameworks/libSDL_net-1.2.0.dylib $@/Contents/MacOS/$<
	install_name_tool -change `otool -L $< | fgrep libphysfs.1.dylib | awk '{print $$1}'` @executable_path/../Frameworks/libphysfs.1.dylib $@/Contents/MacOS/$<
	make append-data BINARY="$@/Contents/MacOS/$<"

Maelstrom.dmg: Maelstrom.app
	-mkdir tmp
	mv $< tmp/
	cp README.simple tmp/README.txt
	cp COPYING tmp/COPYING.txt
	cp COPYING.GPL tmp/
	cp -r Docs tmp/
	hdiutil create -fs HFS+ -srcfolder tmp -volname "Maelstrom" $@
	$(RM) -r tmp
