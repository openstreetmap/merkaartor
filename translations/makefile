LUPDATE = lupdate
LRELEASE = lrelease

TEMPLATES = ../Templates/*.mat
TEMPLATES_PO = templates_*.po
TEMPLATES_LAUNCHPAD_PO = templates-*.po templates/*.po
MAIN = *.ts
MAIN_PO = merkaartor_*.po
MAIN_LAUNCHPAD_PO = merkaartor-*.po merkaartor/*.po
CALL = perl i18n.pl
TARCALL = perl maketar.pl

update: updatets template main release

tar:
	${TARCALL} *_*.po *.pot

template:
	${CALL} templates ${TEMPLATES} ${TEMPLATES_LAUNCHPAD_PO} ${TEMPLATES_PO}

main:
	${CALL} merkaartor ${MAIN} ${MAIN_LAUNCHPAD_PO} ${MAIN_PO}

updatets:
	${CD} .. && ${LUPDATE} -noobsolete Merkaartor.pro

release:
	${CD} .. && ${LRELEASE} Merkaartor.pro

clean:
	@-${RM} *.po *.pot *.qm *.tar.gz
