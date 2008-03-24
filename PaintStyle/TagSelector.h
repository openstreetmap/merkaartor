#ifndef MERKAARTOR_STYLE_TAGSELECTOR_H_
#define MERKAARTOR_STYLE_TAGSELECTOR_H_

class MapFeature;

#include <QtCore/QString>

#include <vector>

class TagSelector
{
	public:
		virtual ~TagSelector() = 0;

		virtual TagSelector* copy() const = 0;
		virtual bool matches(const MapFeature* F) const = 0;
		virtual QString asExpression() const = 0;

		static TagSelector* parse(const QString& Expression);
};

class TagSelectorIs : public TagSelector
{
	public:
		TagSelectorIs(const QString& key, const QString& value);

		virtual TagSelector* copy() const;
		virtual bool matches(const MapFeature* F) const;
		virtual QString asExpression() const;

		static TagSelectorIs* parse(const QString& Expression, int& idx);
	private:
		QString Key, Value;
};

class TagSelectorIsOneOf : public TagSelector
{
	public:
		TagSelectorIsOneOf(const QString& key, const std::vector<QString>& values);

		virtual TagSelector* copy() const;
		virtual bool matches(const MapFeature* F) const;
		virtual QString asExpression() const;

		static TagSelectorIsOneOf* parse(const QString& Expression, int& idx);
	private:
		QString Key;
		std::vector<QString> Values;
};

class TagSelectorOr : public TagSelector
{
	public:
		TagSelectorOr(const std::vector<TagSelector*> Terms);
		~TagSelectorOr();

		virtual TagSelector* copy() const;
		virtual bool matches(const MapFeature* F) const;
		virtual QString asExpression() const;

	private:
		std::vector<TagSelector*> Terms;
};

#endif
