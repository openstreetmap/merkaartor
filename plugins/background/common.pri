DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins/background
macx {
	target.path = $${PREFIX}/merkaartor.app/Contents/plugins/background
}
unix:!macx {
	target.path = $${LIBDIR}/merkaartor/plugins/background
}
