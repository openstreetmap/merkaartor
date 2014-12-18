//
// C++ Implementation: TerraceDialog
//
// Description:
//
//
// Author: James Hogan <james@albanarts.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "TerraceDialog.h"

TerraceDialog::TerraceDialog(QWidget *parent)
    :QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    resize(1,1);
}

unsigned int TerraceDialog::numHouses() const
{
    return spinHouses->value();
}

unsigned int TerraceDialog::maxHouses() const
{
    return spinHouses->maximum();
}

bool TerraceDialog::hasHouseNumbers() const
{
    return radioNumbering->isChecked();
}

QStringList TerraceDialog::houseNumbers() const
{
    QStringList numbers;
    calcNumbering(comboNumberingPattern->currentIndex(), lineNumberingRanges->text(), &numbers);
    return numbers;
}

unsigned int TerraceDialog::calcNumbering(int type, const QString& ranges, QStringList* outNumbers) const
{
    unsigned int num = 0;
    unsigned int maxNum = maxHouses();
    const QStringList items = ranges.split(";");
    foreach (const QString& item, items) {
        const QStringList endsStr = item.split("-");
        if (endsStr.size() == 2) {
            bool valid[2];
            int ends[2] = { endsStr[0].toInt(&valid[0]), endsStr[1].toInt(&valid[1]) };
            if (valid[0] && valid[1]) {
                int step = ((ends[0] <= ends[1]) ? 1 : -1);
                if (type) {
                    // singular range, skip if not in type
                    if (ends[0] == ends[1] && ((ends[0] ^ type) & 1))
                        continue;
                    // move ends inwards
                    if ((ends[0] ^ type) & 1)
                        ends[0] += step;
                    if ((ends[1] ^ type) & 1)
                        ends[1] -= step;
                    step *= 2;
                }
                int count = abs(ends[0] - ends[1]);
                if (type)
                    count /= 2;
                ++count;
                num += count;
                if (outNumbers) {
                    if (num > maxNum)
                        count -= maxNum - num;
                    int cur = ends[0];
                    for (int i = 0; i < count; i++, cur += step)
                        *outNumbers << QString::fromLatin1("%1").arg(cur);
                }
                if (num >= maxNum)
                    return maxNum;
                continue;
            }
        } else if (type && endsStr.size() == 1) {
            // single number, skip if not in type
            bool valid;
            int num = endsStr[0].toInt(&valid);
            if (valid && ((num ^ type) & 1))
                continue;
        }
        ++num;
        if (outNumbers)
            *outNumbers << item;
        if (num >= maxNum)
            return num;
    }
    return num;
}

void TerraceDialog::updateNumbering(int type, const QString& ranges)
{
    spinHouses->setValue(calcNumbering(type, ranges));
}

void TerraceDialog::numberingTypeChanged(int type)
{
    updateNumbering(type, lineNumberingRanges->text());
}

void TerraceDialog::numberingRangeChanged(const QString& ranges)
{
    updateNumbering(comboNumberingPattern->currentIndex(), ranges);
}

void TerraceDialog::updateNumbering()
{
    updateNumbering(comboNumberingPattern->currentIndex(), lineNumberingRanges->text());
}

void TerraceDialog::changeEvent(QEvent * event)
{
        if (event->type() == QEvent::LanguageChange)
                retranslateUi(this);
}
