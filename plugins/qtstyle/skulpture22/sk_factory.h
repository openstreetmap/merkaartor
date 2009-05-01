/*
 * skulpture_factory.h - Classical Three-Dimensional Artwork for Qt 4
 *
 * Copyright (c) 2008 Christoph Feck <christoph@maxiom.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SKULPTURE_FACTORY_H
#define SKULPTURE_FACTORY_H 1


/*-----------------------------------------------------------------------*/

#include <QtGui/QPainterPath>
#include <QtGui/QLinearGradient>
#include <QtGui/QColor>

class AbstractFactory
{
	public:
		typedef qint8 Code;
		typedef const Code *Description;

		static const int MinVar = 1;
		static const int MaxVar = 9;

	public:
		enum OpCode
		{
			/* Values */
			MinVal = -100, MaxVal = 100,
			GetVar = 100,
			Add = 110, Sub, Mul, Div, Min, Max, Mix, Cond,

			/* Colors */
			RGB = 0, RGBA, RGBAf, Blend, Palette, Shade, Darker, Lighter,

			/* Conditions */
			EQ = 0, NE, LT, GE, GT, LE, Or, And, Not, FactoryVersion,
			OptionVersion, OptionType, OptionComplex, OptionState, OptionRTL,

			/* Instructions */
			SetVar = 100,
/* Shape */		Move = 121, Line, Quad, Cubic, Close,
/* Gradient */	ColorAt = 121,
/* Frame */
/* Panel */
/* Primitive */
/* Control */
			Begin = 118, Else = 119, End = 120, If = 126, While = 127, Nop = 0
		};

	protected:
		AbstractFactory() : p(0), opt(0) { }
		virtual ~AbstractFactory() { }

	protected:
		void setDescription(Description description) { p = description; }
		void setOption(const QStyleOption *option) { opt = option; }

		void setVar(int n, qreal value) { var[n] = value; }
		qreal getVar(int n) const { return var[n]; }

		void create();

	protected:
		virtual void QT_FASTCALL executeCode(Code code);
		virtual void QT_FASTCALL skipCode(Code code);
		virtual int QT_FASTCALL version() { return 0; }

	protected:
		qreal QT_FASTCALL evalValue();
		QColor QT_FASTCALL evalColor();
		void QT_FASTCALL skipValue();
		void QT_FASTCALL skipColor();

	private:
		bool QT_FASTCALL evalCondition();
		void QT_FASTCALL skipCondition();

	private:
		Description p;
		const QStyleOption *opt;
		qreal var[MaxVar + 1];
};


/*-----------------------------------------------------------------------*/

class ShapeFactory : public AbstractFactory
{
	public:
		static QPainterPath createShape(Description description, qreal var[]);
		static QPainterPath createShape(Description description);

	protected:
		ShapeFactory() : AbstractFactory() { }
		virtual ~ShapeFactory() { }

		void clear() { path = QPainterPath(); }
		const QPainterPath &getPath() const { return path; }

	protected:
		virtual void QT_FASTCALL executeCode(Code code);
		virtual void QT_FASTCALL skipCode(Code code);

	private:
		QPainterPath path;
};


#define Pvalue(v) int(100 * (v) + 0.5)

#define Pmove(x,y) ShapeFactory::Move, Pvalue(x), Pvalue(y)
#define Pline(x,y) ShapeFactory::Line, Pvalue(x), Pvalue(y)
#define Pquad(x,y,a,b) ShapeFactory::Quad, Pvalue(x), Pvalue(y), Pvalue(a), Pvalue(b)
#define Pcubic(x,y,a,b,c,d) ShapeFactory::Cubic, Pvalue(x), Pvalue(y), Pvalue(a), Pvalue(b), Pvalue(c), Pvalue(d)
#define Pend ShapeFactory::Close, ShapeFactory::End
#define Pclose ShapeFactory::Close


/*-----------------------------------------------------------------------*/

class GradientFactory : public AbstractFactory
{
	public:
		static QGradient createGradient(Description description, qreal var[]);
		static QGradient createGradient(Description description);

	protected:
		GradientFactory() : AbstractFactory() { }
		virtual ~GradientFactory() { }

	protected:
		void clear() { gradient.setStops(QGradientStops()); }
		const QGradient &getGradient() const { return gradient; }

	protected:
		virtual void QT_FASTCALL executeCode(Code code);
		virtual void QT_FASTCALL skipCode(Code code);

	private:
		QGradient gradient;
};


/*-----------------------------------------------------------------------*/

#endif


