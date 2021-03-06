#include "converdemo.h"
#include <windows.h>
#include <gdiplus.h>
#include <GdiPlus.h>
using namespace Gdiplus;

//#include <QGradientStop>

#ifdef Q_OS_WIN

KPT_BEGIN_NAMESPACE	
//-------------------------------------------------------------------------
// 点的转换
void convertQPointF2GpPointF(const QPointF &qPoint, OUT PointF* pPoint)
{
	Q_ASSERT(NULL == pPoint); 
	pPoint->X = qPoint.x();
	pPoint->Y = qPoint.y();
}
PointF 	convertQPointF2GpPointF(const QPointF &qPoint)
{
	return PointF(qPoint.x(), qPoint.y());
}
//-------------------------------------------------------------------------
// 矩形的转换
void convertQRectF2GpRectF(const QRectF &qRect, OUT RectF* pRect)
{
	Q_ASSERT(NULL == pRect);
	//初始化
	qreal x(0), y(0), width(0), height(0);
	qRect.getCoords(&x, &y, &width, &height);
	pRect->X = x;
	pRect->Y = y;
	pRect->Width = width;
	pRect->Height = height;
}
RectF convertQRectF2GpRectF(const QRectF &qRect)
{
	//初始化
	qreal x(0), y(0), width(0), height(0);
	qRect.getCoords(&x, &y, &width, &height);
	return RectF(x, y, width, height);
}
//-------------------------------------------------------------------------
//颜色的转换
void convertQColor2GpColor(const QColor &qColor, OUT Color* pColor)
{
	Q_ASSERT(pColor);
	//初始化
	qreal red(0), green(0), blue(0), alpha(0);
	qColor.getRgbF(&red, &green, &blue, &alpha);

	//移位来组合ARGB的值
	ARGB argb = (int)(255 * alpha);
// 用下面的代码得到的结果不对
// 	argb = argb << 8 + (int)(255 * red);
// 	argb = argb << 8 + (int)(255 * green);
// 	argb = argb << 8 + (int)(255 * blue);
	
	argb = argb<<8;
	argb += (int)(255 * red);
	argb = argb<<8;
	argb += (int)(255 * green);
	argb = argb<<8;
	argb += (int)(255 * blue);
	pColor->SetValue(argb);
}
Color convertQColor2GpColor(const QColor &qColor)
 {
	//初始化
	qreal red(0), green(0), blue(0), alpha(0);
	qColor.getRgbF(&red, &green, &blue, &alpha);
	return Color(255 * alpha, 255 * red, 255 * green, 255 * blue);
 }
//-------------------------------------------------------------------------
// QTransform 转换成 Matrix,原本想把 QMatrix 转成 Matrix的，不过QT文档说QMatrix是过时的类
inline void converQTransform2GpMatrix(const QTransform &qTransform, OUT Matrix *pMatrix)
{
	Q_ASSERT(pMatrix);

	pMatrix->SetElements(qTransform.m11(), qTransform.m12(), qTransform.m21(), qTransform.m22(), qTransform.dx(), qTransform.dy());
}
 /*
 //因为 Matrix的复制构造函数为私有的，所以下面的转换函数不可以使用
 Matrix converQTransform2GpMatrix(const QTransform &qTransform)
 {
	Matrix matrix;
	matrix.Translate(qTransform.dx(), qTransform.dy());
	matrix.Shear(qTransform.m21(), qTransform.m12());
	matrix.Scale(qTransform.m11(), qTransform.m22());
	return matrix;
 }
 */
//------------------------------------------------------------------------
inline HatchStyle getHatchStyle(Qt::BrushStyle style)
{
	switch(style)
	{
	case Qt::Dense1Pattern:
		return HatchStyle50Percent;
	case Qt::Dense2Pattern:
		return HatchStyle40Percent;
	case Qt::Dense3Pattern:
		return  HatchStyle30Percent;
	case Qt::Dense4Pattern:	
		return  HatchStyle25Percent;
	case Qt::Dense5Pattern:
		return  HatchStyle20Percent;
	case Qt::Dense6Pattern:
		return  HatchStyle10Percent;
	case Qt::Dense7Pattern:
		return  HatchStyle05Percent;
	case Qt::HorPattern:
		return HatchStyleHorizontal;
	case Qt::VerPattern:
		return HatchStyleVertical;
	case Qt::CrossPattern:
		return HatchStyleCross;
	case Qt::BDiagPattern:
		return HatchStyleBackwardDiagonal;
	case Qt::FDiagPattern:
		return HatchStyleForwardDiagonal;
	case Qt::DiagCrossPattern:
		return HatchStyleDiagonalCross;
	default:
		Q_ASSERT(false);
	}
	return HatchStyleHorizontal;
}


inline LinearGradientBrush* newLineGradientBrush(const QBrush &qBrush)
{
	QGradient* qgradient = const_cast<QGradient*>(qBrush.gradient());
	QLinearGradient* qlineGradinet = static_cast<QLinearGradient*>(qgradient);
	Q_ASSERT(qlineGradinet);

	//得到两个点的坐标
	PointF pointSart(convertQPointF2GpPointF(qlineGradinet->start())),
		pointStop(convertQPointF2GpPointF(qlineGradinet->finalStop()));
	//初始化两种空颜色
	Color colorStart, colorStop;
	LinearGradientBrush* brush = new LinearGradientBrush(pointSart, pointStop, colorStart, colorStop);
	brush->SetWrapMode(WrapModeTileFlipXY);
	QGradientStops gradientStops = qgradient->stops();
	int itemsCount = gradientStops.count();
	Q_ASSERT(itemsCount);
	Color* colors = new Color[itemsCount];
	REAL* reals = new REAL[itemsCount];
	Q_ASSERT(colors);
	Q_ASSERT(reals);

	int index =0;
	for (auto iter = gradientStops.begin(); iter != gradientStops.end(); ++iter)
	{
		convertQColor2GpColor(iter->second, &colors[index]);
		reals[index] = iter->first;
		++index;
	}
	brush->SetInterpolationColors(colors, reals, itemsCount);
	delete [] colors;
	delete [] reals;

	Matrix matrix;
	converQTransform2GpMatrix(qBrush.transform(), &matrix);
	brush->MultiplyTransform(&matrix, MatrixOrderAppend);
	return brush;
}

inline PathGradientBrush * newPathGradientBrush(const QBrush &qBrush)
{
	QGradient* qgradient = const_cast<QGradient*>(qBrush.gradient());
	QPathGradient* qpathGradinet = static_cast<QPathGradient*>(qgradient);
	Q_ASSERT(qpathGradinet);
	//qpathGradinet
	return NULL;

}

Brush* convertQBrush2GpBrush(const QBrush &qBrush)
{
	Qt::BrushStyle style = qBrush.style();

	if (style > 1 && style < 15)
	{
		HatchStyle hatchStyle = getHatchStyle(style);
		Color color(convertQColor2GpColor(qBrush.color()));
		return (new HatchBrush(hatchStyle, color, color));
	}
	else
	{
		switch(style)
		{
			case Qt::NoBrush:
				Q_ASSERT(false);
			case Qt::SolidPattern:  
				return (new SolidBrush(convertQColor2GpColor(qBrush.color())));	
			case Qt::LinearGradientPattern:
				return newLineGradientBrush(qBrush);
			case Qt::ConicalGradientPattern:
			case Qt::RadialGradientPattern:
			case Qt::PathGradientPattern:
				return newPathGradientBrush(qBrush);
			case Qt::TexturePattern:
				return NULL;
			default:
				Q_ASSERT(false);
		} 
		return NULL;
	}
}

KPT_END_NAMESPACE

#endif // Q_OS_WIN