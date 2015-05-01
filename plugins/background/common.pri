OUTPUT_DIR = $$OUT_PWD/../../../binaries
DESTDIR = $$OUTPUT_DIR/bin/plugins/background

macx {
	target.path = $${PREFIX}/merkaartor.app/Contents/plugins/background
}
unix:!macx {
	target.path = $${LIBDIR}/merkaartor/plugins/background
}
