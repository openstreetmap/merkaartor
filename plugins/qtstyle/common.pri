DESTDIR = $$OUTPUT_DIR/$$(QMAKESPEC)/bin/plugins/styles
macx {
	target.path = $${PREFIX}/merkaartor.app/Contents/plugins/styles
}
unix:!macx {
	target.path = $${LIBDIR}/merkaartor/plugins/styles
}
