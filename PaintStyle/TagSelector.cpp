#include "TagSelector.h"

#include "Map/MapFeature.h"
#include "Map/Road.h"

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
		if ( ((Expression[idx] == '_') || (Expression[idx].isLetterOrNumber()) || (Expression[idx].isPunct()) || (Expression[idx] == '*') || (Expression[idx] == '?')) 
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
	QString Result;
	int TempIdx = idx;
	if (canParseValue(Expression,TempIdx,Result))
	{
		if (Result == Literal)
		{
			idx = TempIdx;
			return true;
		}
	}
	return false;
}

TagSelectorIs* parseTagSelectorIs(const QString& Expression, int& idx)
{
	QString Key, Value;
	if (!canParseKey(Expression, idx, Key))
		return 0;
	if (!canParseLiteral(Expression, idx, "is"))
		return 0;
	if (!canParseValue(Expression, idx, Value))
		return 0;
	return new TagSelectorIs(Key, Value);
}

TagSelectorTypeIs* parseTagSelectorTypeIs(const QString& Expression, int& idx)
{
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
	std::vector<QString> Values;
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

	if (!Current) {
		idx = Saved;
		Current = parseTagSelectorIs(Expression, idx);
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
		idx = Saved;
		Current = parseTagSelectorTypeIs(Expression, idx);
	}
	if (!Current)
	{
		idx = Saved;
		Current = parseTagSelectorHasTags(Expression, idx);
	}
	if (!Current)
	{
		idx = Saved;
		Current = parseTagSelectorIsOneOf(Expression, idx);
	}
	if (!Current)
	{
		if (canParseLiteral(Expression,idx,"not")) {
			TagSelector* notFactor = parseFactor(Expression, idx);
			Current = new TagSelectorNot(notFactor);
		}
	}
	return Current;
}

TagSelector* parseTerm(const QString& Expression, int& idx)
{
	std::vector<TagSelector*> Factors;
	while (idx < Expression.length())
	{
		TagSelector* Current = parseFactor(Expression, idx);
		if (!Current)
			break;
		Factors.push_back(Current);
		if (!canParseLiteral(Expression,idx,"and"))
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
	std::vector<TagSelector*> Terms;
	while (idx < Expression.length())
	{
		TagSelector* Current = parseTerm(Expression, idx);
		if (!Current)
			break;
		Terms.push_back(Current);
		if (!canParseLiteral(Expression,idx,"or"))
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

TagSelector::~TagSelector()
{
}


/* TAGSELECTORIS */

TagSelectorIs::TagSelectorIs(const QString& key, const QString& value)
: Key(key), Value(value)
{
	if (value != "_NULL_") {
		rx = QRegExp(value);
		rx.setPatternSyntax(QRegExp::Wildcard);
	}
}

TagSelector* TagSelectorIs::copy() const
{
	return new TagSelectorIs(Key,Value);
}

TagSelectorMatchResult TagSelectorIs::matches(const MapFeature* F) const
{
	if (Value == "_NULL_") {
		QString val = F->tagValue(Key, "");
		return val.isEmpty() ? TagSelect_Match : TagSelect_NoMatch;
	}
	return rx.exactMatch(F->tagValue(Key, "")) ? TagSelect_Match : TagSelect_NoMatch;
}

QString TagSelectorIs::asExpression(bool) const
{
	QString R;
	R += "[";
	R += Key;
	R += "] is ";
	R += Value;
	return R;
}

/* TAGSELECTORISONEOF */

TagSelectorIsOneOf::TagSelectorIsOneOf(const QString& key, const std::vector<QString>& values)
: Key(key), Values(values)
{
	for (unsigned int i=0; i<values.size(); ++i)
	{
		if (values[i] != "_NULL_") {
			QRegExp rx(values[i]);
			rx.setPatternSyntax(QRegExp::Wildcard);
			rxv.append(rx);
		}
	}
}

TagSelector* TagSelectorIsOneOf::copy() const
{
	return new TagSelectorIsOneOf(Key,Values);
}

TagSelectorMatchResult TagSelectorIsOneOf::matches(const MapFeature* F) const
{
	QString V = F->tagValue(Key, "");
	for (unsigned int i=0; i<Values.size(); ++i)
	{
		if (Values[i] == "_NULL_") {
			if (V.isEmpty())
				return TagSelect_Match;
		}
	}
	for (int i=0; i<rxv.size(); ++i)
	{
		if(rxv[i].exactMatch(V)) return TagSelect_Match;
	}
	return TagSelect_NoMatch;
}

QString TagSelectorIsOneOf::asExpression(bool) const
{
	QString R;
	R += "[";
	R += Key;
	R += "] isoneof (";
	for (unsigned int i=0; i<Values.size(); ++i)
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

TagSelectorMatchResult TagSelectorTypeIs::matches(const MapFeature* F) const
{
	if (F->getClass() == Type) 
		return TagSelect_Match;
	else
		if (Type.toLower() == "area")
			if (Road* R = dynamic_cast<Road*>((MapFeature*)F))
				if (R->isClosed())
					return TagSelect_Match;

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
}

TagSelector* TagSelectorHasTags::copy() const
{
	return new TagSelectorHasTags();
}

TagSelectorMatchResult TagSelectorHasTags::matches(const MapFeature* F) const
{
	return (F->tagSize()==0 || (F->tagSize()==1 && F->tagKey(0)=="created_by" )) ? TagSelect_NoMatch : TagSelect_Match;
}

QString TagSelectorHasTags::asExpression(bool) const
{
	QString R;
	R += "HasTags";
	return R;
}

/* TAGSELECTOROR */

TagSelectorOr::TagSelectorOr(const std::vector<TagSelector*> terms)
: Terms(terms)
{
}

TagSelectorOr::~TagSelectorOr()
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		delete Terms[i];
}

TagSelector* TagSelectorOr::copy() const
{
	std::vector<TagSelector*> Copied;
	for (unsigned int i=0; i<Terms.size(); ++i)
		Copied.push_back(Terms[i]->copy());
	return new TagSelectorOr(Copied);
}

TagSelectorMatchResult TagSelectorOr::matches(const MapFeature* F) const
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		if (Terms[i]->matches(F) == TagSelect_Match)
			return TagSelect_Match;
	return TagSelect_NoMatch;
}

QString TagSelectorOr::asExpression(bool Precedence) const
{
	QString R;
	if (Precedence)
		R += "(";
	for (unsigned int i=0; i<Terms.size(); ++i)
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

TagSelectorAnd::TagSelectorAnd(const std::vector<TagSelector*> terms)
: Terms(terms)
{
}

TagSelectorAnd::~TagSelectorAnd()
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		delete Terms[i];
}

TagSelector* TagSelectorAnd::copy() const
{
	std::vector<TagSelector*> Copied;
	for (unsigned int i=0; i<Terms.size(); ++i)
		Copied.push_back(Terms[i]->copy());
	return new TagSelectorAnd(Copied);
}

TagSelectorMatchResult TagSelectorAnd::matches(const MapFeature* F) const
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		if (Terms[i]->matches(F) == TagSelect_NoMatch)
			return TagSelect_NoMatch;
	return TagSelect_Match;
}

QString TagSelectorAnd::asExpression(bool /* Precedence */) const
{
	QString R;
	for (unsigned int i=0; i<Terms.size(); ++i)
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
	return new TagSelectorNot(Term->copy());
}

TagSelectorMatchResult TagSelectorNot::matches(const MapFeature* F) const
{
	if (!Term)
		return TagSelect_NoMatch;
	return (Term->matches(F) == TagSelect_Match) ? TagSelect_NoMatch : TagSelect_Match;
}

QString TagSelectorNot::asExpression(bool /* Precedence */) const
{
	QString R;
	R += " not ";
	R += Term->asExpression(true);
	return R;
}

/* TAGSELECTORFALSE */

TagSelectorFalse::TagSelectorFalse()
{
}

TagSelectorFalse::~TagSelectorFalse()
{
}

TagSelector* TagSelectorFalse::copy() const
{
	return new TagSelectorFalse();
}

TagSelectorMatchResult TagSelectorFalse::matches(const MapFeature* /* F */) const
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

TagSelectorTrue::~TagSelectorTrue()
{
}

TagSelector* TagSelectorTrue::copy() const
{
	return new TagSelectorFalse();
}

TagSelectorMatchResult TagSelectorTrue::matches(const MapFeature* /* F */) const
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

TagSelectorMatchResult TagSelectorDefault::matches(const MapFeature* F) const
{
	//return (Term->matches(F) == TagSelect_Match) ? TagSelect_DefaultMatch : TagSelect_NoMatch;
	if (Term->matches(F) == TagSelect_Match)
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

