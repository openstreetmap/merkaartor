#include "TagSelector.h"

#include "Map/MapFeature.h"

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
	while (idx < Expression.length())
	{
		if ( (Expression[idx] == '_') || (Expression[idx].isLetterOrNumber()) )
			Key += Expression[idx++];
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

TagSelector* parseTagSelector(const QString& Expression, int& idx);

TagSelector* parseFactor(const QString& Expression, int& idx)
{
	TagSelector* Current = 0;
	if (canParseSymbol(Expression, idx, '('))
	{
		Current = parseTagSelector(Expression, idx);
		canParseSymbol(Expression, idx, ')');
	}
	int Saved = idx;
	if (!Current)
		Current = parseTagSelectorIs(Expression, idx);
	if (!Current)
	{
		idx = Saved;
		Current = parseTagSelectorIsOneOf(Expression, idx);
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
}

TagSelector* TagSelectorIs::copy() const
{
	return new TagSelectorIs(Key,Value);
}

bool TagSelectorIs::matches(const MapFeature* F) const
{
	return F->tagValue(Key, "") == Value;
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
}

TagSelector* TagSelectorIsOneOf::copy() const
{
	return new TagSelectorIsOneOf(Key,Values);
}

bool TagSelectorIsOneOf::matches(const MapFeature* F) const
{
	QString V = F->tagValue(Key, "");
	for (unsigned int i=0; i<Values.size(); ++i)
		if (V == Values[i])
			return true;
	return false;
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

bool TagSelectorOr::matches(const MapFeature* F) const
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		if (Terms[i]->matches(F))
			return true;
	return false;
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


/* TAGSELECTOROR */

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

bool TagSelectorAnd::matches(const MapFeature* F) const
{
	for (unsigned int i=0; i<Terms.size(); ++i)
		if (!Terms[i]->matches(F))
			return false;
	return true;
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
