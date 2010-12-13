#include "TagSelector.h"

#include "IFeature.h"

void skipWhite(const QString& Expression, int& idx)
{
    while (idx < Expression.length())
        if (Expression[idx] == ' ')
            ++idx;
        else
            return;
}

bool canParseSymbol(const QString& Expression, int& idx, char Symbol)
{
    skipWhite(Expression, idx);
    if ((idx < Expression.length()) && (Expression[idx] == Symbol))
    {
        ++idx;
        return true;
    }
    return false;
}

bool canParseValue(const QString& Expression, int& idx, QString& Key)
{
    Key = "";
    skipWhite(Expression,idx);
    unsigned short opened =0;
    while (idx < Expression.length())
    {
        if ( ((Expression[idx] == '_') || (Expression[idx].isLetterOrNumber()) /*|| (Expression[idx].isPunct())*/ || (Expression[idx] == '*') || (Expression[idx] == ':') || (Expression[idx] == '?'))
                &&  ( (Expression[idx] != '[') && (Expression[idx] != ']') && (Expression[idx] != ',') && (Expression[idx] != '(')&& (Expression[idx] != ')')) )
            Key += Expression[idx++];
        else if ( Expression[idx] == '[' )
        {
            opened++;
            Key += Expression[idx++];
        }
        else if ( Expression[idx] == ']' )
        {
            if( opened == 0) break;
            opened--;
            Key += Expression[idx++];
        }
        else
            break;
    }
    return Key.length() > 0;
}

bool canParseKey(const QString& Expression, int& idx, QString& Key)
{
    if (!canParseSymbol(Expression,idx,'['))
        return false;
    if (!canParseValue(Expression,idx,Key))
        return false;
    canParseSymbol(Expression,idx,']');
    return true;
}

bool canParseLiteral(const QString& Expression, int& idx, const QString& Literal)
{
    skipWhite(Expression,idx);
    if (Expression.indexOf(Literal, idx, Qt::CaseInsensitive) == idx) {
        idx += Literal.length();
        return true;
    }
    return false;
}

TagSelectorOperator* parseTagSelectorOperator(const QString& Expression, int& idx)
{
    QString Key, Oper, Value;
    if ((!canParseKey(Expression, idx, Key)) && (!canParseValue(Expression, idx, Key)))
        return 0;

    if (canParseLiteral(Expression, idx, "is"))
        Oper = "=";
    if (canParseLiteral(Expression, idx, "!="))
        Oper = "!=";
    if (canParseSymbol(Expression, idx, '<'))
        Oper = "<";
    if (canParseSymbol(Expression, idx, '>'))
        Oper = ">";
    if (canParseLiteral(Expression, idx, "<="))
        Oper = "<=";
    if (canParseLiteral(Expression, idx, ">="))
        Oper = ">=";
    if (canParseSymbol(Expression, idx, '='))
        Oper = "=";
    if (Oper.isNull())
        return 0;

    if (!canParseValue(Expression, idx, Value))
        return 0;
    return new TagSelectorOperator(Key, Oper, Value);
}

TagSelectorTypeIs* parseTagSelectorTypeIs(const QString& Expression, int& idx)
{
    if (canParseLiteral(Expression, idx, "node"))
        return new TagSelectorTypeIs("node");
    if (canParseLiteral(Expression, idx, "way"))
        return new TagSelectorTypeIs("way");
    if (canParseLiteral(Expression, idx, "relation"))
        return new TagSelectorTypeIs("relation");

    QString Type;
    if (!canParseLiteral(Expression, idx, "Type"))
        return 0;
    if (!canParseLiteral(Expression, idx, "is"))
        return 0;
    if (!canParseValue(Expression, idx, Type))
        return 0;
    return new TagSelectorTypeIs(Type);
}

TagSelectorHasTags* parseTagSelectorHasTags(const QString& Expression, int& idx)
{
    if (!canParseLiteral(Expression, idx, "HasTags"))
        return 0;
    return new TagSelectorHasTags();
}

TagSelectorIsOneOf* parseTagSelectorIsOneOf(const QString& Expression, int& idx)
{
    QString Key;
    if (!canParseKey(Expression, idx, Key))
        return 0;
    if (!canParseLiteral(Expression, idx, "isoneof"))
        return 0;
    if (!canParseSymbol(Expression, idx, '('))
        return 0;
    QList<QString> Values;
    while (true)
    {
        QString Value;
        if (!canParseValue(Expression, idx, Value))
            break;
        Values.push_back(Value);
        if (!canParseSymbol(Expression, idx, ','))
            break;
    }
    canParseSymbol(Expression, idx, ')');
    if (Values.size())
        return new TagSelectorIsOneOf(Key,Values);
    return 0;
}

TagSelectorFalse* parseTagSelectorFalse(const QString& Expression, int& idx)
{
    if (!canParseLiteral(Expression, idx, "false"))
        return 0;
    return new TagSelectorFalse();
}

TagSelectorTrue* parseTagSelectorTrue(const QString& Expression, int& idx)
{
    if (!canParseLiteral(Expression, idx, "true"))
        return 0;
    return new TagSelectorTrue();
}

TagSelector* parseTagSelector(const QString& Expression, int& idx);

TagSelector* parseFactor(const QString& Expression, int& idx)
{
    TagSelector* Current = 0;
    if (canParseLiteral(Expression,idx,"[Default]")) {
        TagSelector* defFactor = parseTagSelector(Expression, idx);
        Current = new TagSelectorDefault(defFactor);
    }
    int Saved = idx;
    if (!Current) {
        if (canParseSymbol(Expression, idx, '('))
        {
            Current = parseTagSelector(Expression, idx);
            canParseSymbol(Expression, idx, ')');
        }
    }

    if (!Current)
    {
        idx = Saved;
        Current = parseTagSelectorTypeIs(Expression, idx);
    }
    if (!Current)
    {
        idx = Saved;
        Current = parseTagSelectorIsOneOf(Expression, idx);
    }
    if (!Current) {
        idx = Saved;
        Current = parseTagSelectorOperator(Expression, idx);
    }

    if (!Current)
    {
        idx = Saved;
        Current = parseTagSelectorFalse(Expression, idx);
    }
    if (!Current)
    {
        idx = Saved;
        Current = parseTagSelectorTrue(Expression, idx);
    }
    if (!Current)
    {
        Current = parseTagSelectorHasTags(Expression, idx);
    }
    if (!Current)
    {
        idx = Saved;
        if ((canParseLiteral(Expression,idx,"not")) || canParseSymbol(Expression,idx,'!')) {
            TagSelector* notFactor = parseFactor(Expression, idx);
            Current = new TagSelectorNot(notFactor);
        }
    }
    if (!Current)
    {
        idx = Saved;
        if (canParseLiteral(Expression,idx,"parent")) {
            TagSelector* parentFactor = parseFactor(Expression, idx);
            Current = new TagSelectorParent(parentFactor);
        }
    }
    if (!Current) {
        idx = Saved;
        if (canParseSymbol(Expression, idx, '['))
        {
            Current = parseFactor(Expression, idx);
            canParseSymbol(Expression, idx, ']');
        }
    }
    if (!Current)
    {
        idx = Saved;
        QString Key;
        if (canParseValue(Expression,idx,Key)) {
            int TmpIdx = 0;
            Current = parseFactor("not(" + Key + " is _NULL_)", TmpIdx);
        }
    }
    return Current;
}

TagSelector* parseTerm(const QString& Expression, int& idx)
{
    QList<TagSelector*> Factors;
    while (idx < Expression.length())
    {
        TagSelector* Current = parseFactor(Expression, idx);
        if (!Current)
            break;
        Factors.push_back(Current);
        if (canParseLiteral(Expression,idx,"and"))
            continue;
        int TempIdx = idx;
        if (canParseSymbol(Expression, TempIdx, '['))
            continue;
        break;
    }
    if (Factors.size() == 1)
        return Factors[0];
    else if (Factors.size() > 1)
        return new TagSelectorAnd(Factors);
    return 0;
}

TagSelector* parseTagSelector(const QString& Expression, int& idx)
{
    QList<TagSelector*> Terms;
    while (idx < Expression.length())
    {
        TagSelector* Current = parseTerm(Expression, idx);
        if (!Current)
            break;
        Terms.push_back(Current);
        if ((!canParseLiteral(Expression,idx,"or")) && (!canParseLiteral(Expression,idx,",")))
            break;
    }
    if (Terms.size() == 1)
        return Terms[0];
    else if (Terms.size() > 1)
        return new TagSelectorOr(Terms);
    return 0;
}

TagSelector* TagSelector::parse(const QString& Expression)
{
    int idx = 0;
    return parseTagSelector(Expression,idx);
}

TagSelector* TagSelector::parse(const QString& Expression, int& idx)
{
    return parseTagSelector(Expression,idx);
}

TagSelector::~TagSelector()
{
}


/* TAGSELECTOROPERATOR */

TagSelectorOperator::TagSelectorOperator(const QString& key, const QString& oper, const QString& value)
    : Key(key), Oper(oper), Value(value), UseRegExp(false)
    , specialKey(TagSelectKey_None)
    , specialValue(TagSelectValue_None)
{
    if (key.toLower() == ":id")
        specialKey = TagSelectKey_Id;
    else if (key.toLower() == ":user")
        specialKey = TagSelectKey_User;
    else if (key.toLower() == ":time") {
        specialKey = TagSelectKey_Time;
        dtValue = QDateTime::fromString(value, Qt::ISODate);
    } else if (key.toLower() == ":version") {
        specialKey = TagSelectKey_Version;
        bool ok;
        numValue = value.toDouble(&ok);
        if (!ok)
            numValue = -1;
    } else if (key.toLower() == ":zoomlevel")
        specialKey = TagSelectKey_ZoomLevel;
    else if (key.toLower() == ":pixelperm")
        specialKey = TagSelectKey_PixelPerM;
    else if (key.toLower() == ":dirty")
        specialKey = TagSelectKey_Dirty;
    else if (key.toLower() == ":uploaded")
        specialKey = TagSelectKey_Uploaded;

    if (value.toUpper() == "_NULL_") {
        specialValue = TagSelectValue_Empty;
    } else if (value.contains(QRegExp("[][*?]"))) {
        UseRegExp = true;
        rx = QRegExp(value, Qt::CaseInsensitive);
        rx.setPatternSyntax(QRegExp::Wildcard);
    }
    // Else exact match against ->Value only

    if (Oper == ">")
        theOp = GT;
    else if (Oper == "<")
        theOp = LT;
    else if (Oper == ">=")
        theOp = GE;
    else if (Oper == "<=")
        theOp = LE;
    else if (Oper == "!=")
        theOp = NE;
    else
        theOp = EQ;
}

TagSelector* TagSelectorOperator::copy() const
{
    return new TagSelectorOperator(Key,Oper,Value);
}

static const QString emptyString("__EMPTY__");

TagSelectorMatchResult TagSelectorOperator::matches(const IFeature* F, double PixelPerM) const
{
    bool boolVal = false, valB;
    if (Value.toLower() == "true") {
        boolVal = true;
        valB = true;
    } else if (Value.toLower() == "false") {
        boolVal = true;
        valB = false;
    }

    if (specialKey != TagSelectKey_None) {
        switch (specialKey) {
        case TagSelectKey_Id:
            switch (theOp) {
            case EQ:
                return (F->xmlId() == Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case NE:
                return (F->xmlId() != Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GT:
                return (F->xmlId() > Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LT:
                return (F->xmlId() < Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GE:
                return (F->xmlId() >= Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LE:
                return (F->xmlId() <= Value ? TagSelect_Match : TagSelect_NoMatch);
                break;
            }
            break;

        case TagSelectKey_User:
            switch (theOp) {
            case EQ:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) == 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case NE:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) != 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GT:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) > 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LT:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) < 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GE:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) >= 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LE:
                return (QString::compare(F->user(), Value, Qt::CaseInsensitive) <= 0 ? TagSelect_Match : TagSelect_NoMatch);
                break;
            }
            break;

        case TagSelectKey_Time: {
                if (!dtValue.isValid())
                    return TagSelect_NoMatch;
                if (dtValue.time() == QTime(0, 0, 0))
                    switch (theOp) {
                    case EQ:
                        return (F->time().date() == dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case NE:
                        return (F->time().date() != dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case GT:
                        return (F->time().date() > dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case LT:
                        return (F->time().date() < dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case GE:
                        return (F->time().date() >= dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case LE:
                        return (F->time().date() <= dtValue.date() ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    }
                else
                    switch (theOp) {
                    case EQ:
                        return (F->time() == dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case NE:
                        return (F->time() != dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case GT:
                        return (F->time() > dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case LT:
                        return (F->time() < dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case GE:
                        return (F->time() >= dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    case LE:
                        return (F->time() <= dtValue ? TagSelect_Match : TagSelect_NoMatch);
                        break;
                    }

                break;
        }

        case TagSelectKey_Version:
            switch (theOp) {
            case EQ:
                return (F->versionNumber() == numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case NE:
                return (F->versionNumber() != numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GT:
                return (F->versionNumber() > numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LT:
                return (F->versionNumber() < numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GE:
                return (F->versionNumber() >= numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LE:
                return (F->versionNumber() <= numValue ? TagSelect_Match : TagSelect_NoMatch);
                break;
            }

            break;

        case TagSelectKey_PixelPerM: {
            if (!PixelPerM)
                return TagSelect_Match;
            bool okval;
            double valN = Value.toDouble(&okval);
            if (!okval)
                return TagSelect_NoMatch;
            switch (theOp) {
            case EQ:
                return (PixelPerM == valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case NE:
                return (PixelPerM != valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GT:
                return (PixelPerM > valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LT:
                return (PixelPerM < valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case GE:
                return (PixelPerM >= valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            case LE:
                return (PixelPerM <= valN ? TagSelect_Match : TagSelect_NoMatch);
                break;
            }
            break;
        }

        case TagSelectKey_Dirty: {
            if (!boolVal)
                return TagSelect_NoMatch;

            switch (theOp) {
            case EQ:
                if (valB) {
                    if (F->isDirty())
                        return TagSelect_Match;
                    else
                        return TagSelect_NoMatch;
                } else {
                    if (!F->isDirty())
                        return TagSelect_Match;
                    else
                        return TagSelect_NoMatch;
                }
                break;

            case NE:
                if (valB) {
                    if (F->isDirty())
                        return TagSelect_NoMatch;
                    else
                        return TagSelect_Match;
                } else {
                    if (!F->isDirty())
                        return TagSelect_NoMatch;
                    else
                        return TagSelect_Match;
                }
                break;

            default:
                return TagSelect_NoMatch;
            }
            break;
        }

        case TagSelectKey_Uploaded: {
            if (!boolVal)
                return TagSelect_NoMatch;

            switch (theOp) {
            case EQ:
                if (valB) {
                    if (F->isUploaded())
                        return TagSelect_Match;
                    else
                        return TagSelect_NoMatch;
                } else {
                    if (!F->isUploaded())
                        return TagSelect_Match;
                    else
                        return TagSelect_NoMatch;
                }
                break;

            case NE:
                if (valB) {
                    if (F->isUploaded())
                        return TagSelect_NoMatch;
                    else
                        return TagSelect_Match;
                } else {
                    if (!F->isUploaded())
                        return TagSelect_NoMatch;
                    else
                        return TagSelect_Match;
                }
                break;

            default:
                return TagSelect_NoMatch;
            }
            break;
        }

        default:
            return TagSelect_NoMatch;
            break;
        }
    } else {
        QList<QString> valueList;
        if (Key != "*")
            valueList << F->tagValue(Key, emptyString);
        else {
            for (int i=0; i<F->tagSize(); ++i)
                valueList << F->tagValue(i);
        }
        foreach (QString val, valueList) {
            if (val == emptyString && specialValue != TagSelectValue_Empty)
                return TagSelect_NoMatch;
            if (specialValue == TagSelectValue_Empty) {
                if (theOp == EQ) {
                    if (val.toUpper() == emptyString) return TagSelect_Match;
                } else {
                    if (val.toUpper() != emptyString) return TagSelect_Match;
                }
            } else if (UseRegExp) {
                if (rx.exactMatch(val)) return TagSelect_Match;
            } else {
                bool okkey, okval;
                double keyN = val.toDouble(&okkey);
                double valN = Value.toDouble(&okval);
                if (boolVal)
                    switch (theOp) {
                    case EQ:
                    if (valB) {
                        if (val.toLower() == "true" || val.toLower() == "yes" || val == "1")
                            return TagSelect_Match;
                    } else {
                        if (val.toLower() == "false" || val.toLower() == "no" || val == "0")
                            return TagSelect_Match;
                    }
                    break;

                case NE:
                    if (valB) {
                        if (val.toLower() == "false" || val.toLower() == "no" || val == "0")
                            return TagSelect_Match;
                    } else {
                        if (val.toLower() == "true" || val.toLower() == "yes" || val == "1")
                            return TagSelect_Match;
                    }
                    break;

                default:
                    break;
                }
                else if (okkey && okval)
                    switch (theOp) {
                    case EQ:
                        if (keyN == valN) return TagSelect_Match;
                        break;
                    case NE:
                        if (keyN != valN) return TagSelect_Match;
                        break;
                    case GT:
                        if (keyN > valN) return TagSelect_Match;
                        break;
                    case LT:
                        if (keyN < valN) return TagSelect_Match;
                        break;
                    case GE:
                        if (keyN >= valN) return TagSelect_Match;
                        break;
                    case LE:
                        if (keyN <= valN) return TagSelect_Match;
                        break;
                    }
                else
                    switch (theOp) {
                    case EQ:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) == 0) return TagSelect_Match;
                        break;
                    case NE:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) != 0 ) return TagSelect_Match;
                        break;
                    case GT:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) > 0) return TagSelect_Match;
                        break;
                    case LT:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) < 0) return TagSelect_Match;
                        break;
                    case GE:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) >= 0) return TagSelect_Match;
                        break;
                     case LE:
                        if ((QString::compare(val, Value, Qt::CaseInsensitive)) <= 0) return TagSelect_Match;
                        break;
                    }
            }
        }
    }
    return TagSelect_NoMatch;
}

QString TagSelectorOperator::asExpression(bool) const
{
    QString R;
    R += "[";
    R += Key;
    R += "]";
    R += Oper;
    R += Value;
    return R;
}

/* TAGSELECTORISONEOF */

TagSelectorIsOneOf::TagSelectorIsOneOf(const QString& key, const QList<QString>& values)
    : Key(key), Values(values)
    , specialKey(TagSelectKey_None)
    , specialValue(TagSelectValue_None)
{
    if (key.toUpper() == ":ID")
        specialKey = TagSelectKey_Id;
    else if (key.toUpper() == ":USER")
        specialKey = TagSelectKey_User;
    else if (key.toUpper() == ":TIME")
        specialKey = TagSelectKey_Time;
    else if (key.toUpper() == ":VERSION")
        specialKey = TagSelectKey_Version;

    for (int i=0; i<values.size(); ++i)
    {
        if (values[i].toUpper() == "_NULL_") {
            specialValue = TagSelectValue_Empty;
        } else if (values[i].contains(QRegExp("[][*?]"))) {
            QRegExp rx(values[i], Qt::CaseInsensitive);
            rx.setPatternSyntax(QRegExp::Wildcard);
            rxv.append(rx);
        } else {
            exactMatchv.append(values[i]);
        }
    }
}

TagSelector* TagSelectorIsOneOf::copy() const
{
    return new TagSelectorIsOneOf(Key,Values);
}

TagSelectorMatchResult TagSelectorIsOneOf::matches(const IFeature* F, double /*PixelPerM*/) const
{
    if (specialKey != TagSelectKey_None) {
        foreach (QString Value, exactMatchv) {
            switch (specialKey) {
            case TagSelectKey_Id:
                if (F->xmlId() == Value)
                    return TagSelect_Match;
                break;

            case TagSelectKey_User:
                if (QString::compare(F->user(), Value, Qt::CaseInsensitive) == 0)
                    return TagSelect_Match;
                break;

            case TagSelectKey_Time: {
                QDateTime dtValue = QDateTime::fromString(Value, Qt::ISODate);
                if (!dtValue.isValid())
                    break;
                if (dtValue.time() == QTime(0, 0, 0)) {
                    if (F->time().date() == dtValue.date())
                        return TagSelect_Match;
                } else {
                    if (F->time() == dtValue)
                        return TagSelect_Match;
                }

                break;
            }

            case TagSelectKey_Version:
                if (F->versionNumber() == Value.toInt())
                    return TagSelect_Match;
                break;

            default:
                break;
            }
        }
    } else {
        QString V = F->tagValue(Key, emptyString);
        if (specialValue == TagSelectValue_Empty && V.isEmpty()) {
            return TagSelect_Match;
        }
        foreach (QString pattern, exactMatchv) {
            if (QString::compare(V, pattern) == 0) return TagSelect_Match;
        }
        foreach (QRegExp pattern, rxv) {
            if (pattern.exactMatch(V)) return TagSelect_Match;
        }
    }
    return TagSelect_NoMatch;
}

QString TagSelectorIsOneOf::asExpression(bool) const
{
    QString R;
    R += "[";
    R += Key;
    R += "] isoneof (";
    for (int i=0; i<Values.size(); ++i)
    {
        if (i)
            R += " , ";
        R += Values[i];
    }
    R += ")";
    return R;
}

/* TAGSELECTORTYPEIS */

TagSelectorTypeIs::TagSelectorTypeIs(const QString& type)
: Type(type)
{
}

TagSelector* TagSelectorTypeIs::copy() const
{
    return new TagSelectorTypeIs(Type);
}

TagSelectorMatchResult TagSelectorTypeIs::matches(const IFeature* F, double /*PixelPerM*/) const
{
    QString t = Type.toLower();
    if (t == "node")
        return (F->getType() & IFeature::Point) ? TagSelect_Match : TagSelect_NoMatch;
    else if (t == "way")
        return (F->getType() & IFeature::LineString && !(F->getType() & IFeature::Polygon)) ? TagSelect_Match : TagSelect_NoMatch;
    else if (t == "area")
        return (F->getType() & IFeature::Polygon) ? TagSelect_Match : TagSelect_NoMatch;
    else if (t == "relation")
        return (F->getType() & IFeature::OsmRelation) ? TagSelect_Match : TagSelect_NoMatch;
    else if (t == "tracksegment")
        return (F->getType() & IFeature::GpxSegment) ? TagSelect_Match : TagSelect_NoMatch;

    return TagSelect_NoMatch;
}

QString TagSelectorTypeIs::asExpression(bool) const
{
    QString R;
    R += "Type is ";
    R += Type;
    return R;
}

/* TAGSELECTORHASTAGS */

TagSelectorHasTags::TagSelectorHasTags()
{
    TechnicalTags = QString(TECHNICAL_TAGS).split("#");
}

TagSelector* TagSelectorHasTags::copy() const
{
    return new TagSelectorHasTags();
}

TagSelectorMatchResult TagSelectorHasTags::matches(const IFeature* F, double /*PixelPerM*/) const
{
    for (int i=0; i<F->tagSize(); ++i) {
        if (!TechnicalTags.contains(F->tagKey(i))) {
            return TagSelect_Match;
        }
    }
    return TagSelect_NoMatch;
}

QString TagSelectorHasTags::asExpression(bool) const
{
    QString R;
    R += "HasTags";
    return R;
}

/* TAGSELECTOROR */

TagSelectorOr::TagSelectorOr(const QList<TagSelector*> terms)
: Terms(terms)
{
}

TagSelectorOr::~TagSelectorOr()
{
    for (int i=0; i<Terms.size(); ++i)
        delete Terms[i];
}

TagSelector* TagSelectorOr::copy() const
{
    QList<TagSelector*> Copied;
    for (int i=0; i<Terms.size(); ++i)
        Copied.push_back(Terms[i]->copy());
    return new TagSelectorOr(Copied);
}

TagSelectorMatchResult TagSelectorOr::matches(const IFeature* F, double PixelPerM) const
{
    for (int i=0; i<Terms.size(); ++i)
        if (Terms[i]->matches(F,PixelPerM) == TagSelect_Match)
            return TagSelect_Match;
    return TagSelect_NoMatch;
}

QString TagSelectorOr::asExpression(bool Precedence) const
{
    QString R;
    if (Precedence)
        R += "(";
    for (int i=0; i<Terms.size(); ++i)
    {
        if (i)
            R += " or ";
        R += Terms[i]->asExpression(false);
    }
    if (Precedence)
        R += ")";
    return R;
}


/* TAGSELECTORAND */

TagSelectorAnd::TagSelectorAnd(const QList<TagSelector*> terms)
: Terms(terms)
{
}

TagSelectorAnd::~TagSelectorAnd()
{
    for (int i=0; i<Terms.size(); ++i)
        delete Terms[i];
}

TagSelector* TagSelectorAnd::copy() const
{
    QList<TagSelector*> Copied;
    for (int i=0; i<Terms.size(); ++i)
        Copied.push_back(Terms[i]->copy());
    return new TagSelectorAnd(Copied);
}

TagSelectorMatchResult TagSelectorAnd::matches(const IFeature* F, double PixelPerM) const
{
    for (int i=0; i<Terms.size(); ++i)
        if (Terms[i]->matches(F,PixelPerM) == TagSelect_NoMatch)
            return TagSelect_NoMatch;
    return TagSelect_Match;
}

QString TagSelectorAnd::asExpression(bool /* Precedence */) const
{
    QString R;
    for (int i=0; i<Terms.size(); ++i)
    {
        if (i)
            R += " and ";
        R += Terms[i]->asExpression(true);
    }
    return R;
}

/* TAGSELECTORNOT */

TagSelectorNot::TagSelectorNot(TagSelector* term)
: Term(term)
{
}

TagSelectorNot::~TagSelectorNot()
{
    delete Term;
}

TagSelector* TagSelectorNot::copy() const
{
    if (!Term)
        return NULL;
    return new TagSelectorNot(Term->copy());
}

TagSelectorMatchResult TagSelectorNot::matches(const IFeature* F, double PixelPerM) const
{
    if (!Term)
        return TagSelect_NoMatch;
    return (Term->matches(F,PixelPerM) == TagSelect_Match) ? TagSelect_NoMatch : TagSelect_Match;
}

QString TagSelectorNot::asExpression(bool /* Precedence */) const
{
    if (!Term)
        return "";
    QString R;
    R += "not(";
    R += Term->asExpression(true);
    R += ")";
    return R;
}

/* TAGSELECTORPARENT */

TagSelectorParent::TagSelectorParent(TagSelector* term)
: Term(term)
{
}

TagSelectorParent::~TagSelectorParent()
{
    delete Term;
}

TagSelector* TagSelectorParent::copy() const
{
    if (!Term)
        return NULL;
    return new TagSelectorParent(Term->copy());
}

TagSelectorMatchResult TagSelectorParent::matches(const IFeature* F, double PixelPerM) const
{
    if (!Term)
        return TagSelect_NoMatch;

    TagSelectorMatchResult ret = TagSelect_NoMatch;
    for (int i=0; i<F->sizeParents(); ++i) {
        if (Term->matches(F->getParent(i),PixelPerM) == TagSelect_Match) {
            ret = TagSelect_Match;
            break;
        }
    }
    return ret;
}

QString TagSelectorParent::asExpression(bool /* Precedence */) const
{
    if (!Term)
        return "";
    QString R;
    R += " parent(";
    R += Term->asExpression(true);
    R += ")";
    return R;
}

/* TAGSELECTORFALSE */

TagSelectorFalse::TagSelectorFalse()
{
}

TagSelector* TagSelectorFalse::copy() const
{
    return new TagSelectorFalse();
}

TagSelectorMatchResult TagSelectorFalse::matches(const IFeature* /* F */, double /*PixelPerM*/) const
{
    return TagSelect_NoMatch;
}

QString TagSelectorFalse::asExpression(bool /* Precedence */) const
{
    QString R;
    R += " false ";
    return R;
}

/* TAGSELECTORTRUE */

TagSelectorTrue::TagSelectorTrue()
{
}

TagSelector* TagSelectorTrue::copy() const
{
    return new TagSelectorFalse();
}

TagSelectorMatchResult TagSelectorTrue::matches(const IFeature* /* F */, double /*PixelPerM*/) const
{
    return TagSelect_Match;
}

QString TagSelectorTrue::asExpression(bool /* Precedence */) const
{
    QString R;
    R += " true ";
    return R;
}

/* TAGSELECTORDEFAULT */

TagSelectorDefault::TagSelectorDefault(TagSelector* term)
: Term(term)
{
}

TagSelectorDefault::~TagSelectorDefault()
{
    delete Term;
}

TagSelector* TagSelectorDefault::copy() const
{
    return new TagSelectorDefault(Term->copy());
}

TagSelectorMatchResult TagSelectorDefault::matches(const IFeature* F, double PixelPerM) const
{
    //return (Term->matches(F) == TagSelect_Match) ? TagSelect_DefaultMatch : TagSelect_NoMatch;
    if (Term->matches(F,PixelPerM) == TagSelect_Match)
        return TagSelect_DefaultMatch;
    else
        return TagSelect_NoMatch;
}

QString TagSelectorDefault::asExpression(bool /* Precedence */) const
{
    QString R;
    R += " [Default] ";
    R += Term->asExpression(true);
    return R;
}

