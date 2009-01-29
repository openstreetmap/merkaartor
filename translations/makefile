LUPDATE = lupdate
TOOLKIT = .
TEMPLATES = ../Templates/*.mat
PYTHON  = PYTHONPATH=${TOOLKIT} python
ts2po   = ${PYTHON} ${TOOLKIT}/translate/convert/ts2po
po2ts   = ${PYTHON} ${TOOLKIT}/translate/convert/po2ts

update: po_changed

tar:
	tar -czf `date "+%Y-%m-%d"`.tar.gz *.pot *.po

# default - assume the po files changed or we want an update
po_changed:
	make potemplate
	make ts
	make ts_changed

# special case: the ts files have been used for updating translations
ts_changed:
	make templatepo
	make updatets
	make pot po
	make ts

pot: merkaartor.pot

po: merkaartor_cs.po merkaartor_de.po merkaartor_fr.po merkaartor_it.po merkaartor_pl.po merkaartor_ru.po
ts: merkaartor_cs merkaartor_de merkaartor_fr merkaartor_it merkaartor_pl merkaartor_ru

templatepo: ${TEMPLATES}
	perl templates2po.pl ${TEMPLATES}

potemplate:
	echo missing - po-translation to template file translation

updatets:
	pushd .. && ${LUPDATE} -noobsolete Merkaartor.pro && popd

clean:
	-rm *.po *.pot 20*.tar.gz

%.po: %.ts
	${ts2po} -i $< -o $@

%: %.po
	${po2ts} -i $< -t $@.ts -o $@
	mv $@ $@.ts

merkaartor.pot: merkaartor_de.ts
	${ts2po} -i $< -P -o $@
