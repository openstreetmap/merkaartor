LUPDATE = lupdate
LRELEASE = lrealease

TEMPLATES = ../Templates/*.mat
TEMPLATES_PO = templates_*.po
TEMPLATES_LAUNCHPAD_PO = templates-*.po
MAIN = *.ts
MAIN_PO = merkaartor_*.po
MAIN_LAUNCHPAD_PO = merkaartor-*.po
CALL = perl i18n.pl

update: updatets templates main release

tar:
	tar -czf templates_`date "+%Y-%m-%d"`.tar.gz templates.pot templates_*.po
	tar -czf merkaartor_`date "+%Y-%m-%d"`.tar.gz merkaartor.pot merkaartor_*.po

templates:
	${CALL} templates ${TEMPLATES} ${TEMPLATES_LAUNCHPAD_PO} ${TEMPLATES_PO}

main:
	${CALL} merkaartor ${MAIN} ${MAIN_LAUNCHPAD_PO} ${MAIN_PO}

updatets:
	pushd .. && ${LUPDATE} -noobsolete Merkaartor.pro && popd

release:
	pushd .. && ${LRELEASE} -noobsolete Merkaartor.pro && popd

clean:
	@-${rm} *.po *.pot *.qm *.tar.gz
