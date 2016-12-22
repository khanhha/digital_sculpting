#include "sculpt/BezierCurve.h"

BezierCurve::BezierCurve(VbsDef::CURVE  type /*= CURVE_PRESET_ROOT*/)
    :
	_type(type),
    _cpoints(nullptr),
    _table(nullptr),
    _totalPoint(4),
    _resolution(256)
{
    setControlPoints(_type);
    makeTable(_resolution);
}

BezierCurve::~BezierCurve()
{
    if (_cpoints)
        delete[] _cpoints;

    if (_table)
        delete[] _table;
}

void BezierCurve::reset(VbsDef::CURVE type)
{
    setControlPoints(type);
    makeTable(_resolution);
}

bool BezierCurve::setControlPoints(int type)
{
    if (nullptr == _cpoints)
        _cpoints = new CurvePoint[4];

    switch (type)
    {
	case VbsDef::CURVE_ROUND:
        _cpoints[0].x = 0.0;
        _cpoints[0].y = 1.0;
        _cpoints[1].x = 0.5;
        _cpoints[1].y = 0.90;
        _cpoints[2].x = 0.86;
        _cpoints[2].y = 0.5;
        _cpoints[3].x = 1.0;
        _cpoints[3].y = 0.0;
        break;

	case VbsDef::CURVE_SHARP:
        _cpoints[0].x = 0;
        _cpoints[0].y = 1;
        _cpoints[1].x = 0.25;
        _cpoints[1].y = 0.50;
        _cpoints[2].x = 0.75;
        _cpoints[2].y = 0.04;
        _cpoints[3].x = 1;
        _cpoints[3].y = 0;
        break;

	case VbsDef::CURVE_ROOT:
        _cpoints[0].x = 0;
        _cpoints[0].y = 1;
        _cpoints[1].x = 0.25;
        _cpoints[1].y = 0.95;
        _cpoints[2].x = 0.75;
        _cpoints[2].y = 0.44;
        _cpoints[3].x = 1;
        _cpoints[3].y = 0;
        break;
	case VbsDef::CURVE_SMOOTH:
        _cpoints[0].x = 0;
        _cpoints[0].y = 1;
        _cpoints[1].x = 0.25;
        _cpoints[1].y = 0.94;
        _cpoints[2].x = 0.75;
        _cpoints[2].y = 0.06;
        _cpoints[3].x = 1;
        _cpoints[3].y = 0;
        break;
    }

    return true;
}

void BezierCurve::makeTable(int resolution)
{
    //http://www.drdobbs.com/forward-difference-calculation-of-bezier/184403417?pgno=5

    if (nullptr == _table)
        _table = new CurvePoint[resolution];

    double ax, ay, bx, by, cx, cy, dx, dy;
    int  i;
    double _step;
    double pointX, pointY;
    double firstFDX, firstFDY;
    double secondFDX, secondFDY;
    double thirdFDX, thirdFDY;

    ax = -_cpoints[0].x + 3 * _cpoints[1].x + -3 * _cpoints[2].x + _cpoints[3].x;
    ay = -_cpoints[0].y + 3 * _cpoints[1].y + -3 * _cpoints[2].y + _cpoints[3].y;

    bx = 3 * _cpoints[0].x + -6 * _cpoints[1].x + 3 * _cpoints[2].x;
    by = 3 * _cpoints[0].y + -6 * _cpoints[1].y + 3 * _cpoints[2].y;

    cx = -3 * _cpoints[0].x + 3 * _cpoints[1].x;
    cy = -3 * _cpoints[0].y + 3 * _cpoints[1].y;

    dx = _cpoints[0].x;
    dy = _cpoints[0].y;

    _step = 1.0 / (double)resolution;

    pointX = dx;
    pointY = dy;

    firstFDX = ax * (_step * _step * _step) + bx * (_step * _step) + cx * _step;
    firstFDY = ay * (_step * _step * _step) + by * (_step * _step) + cy * _step;

    secondFDX = 6 * ax * (_step * _step * _step) + 2 * bx * (_step * _step);
    secondFDY = 6 * ay * (_step * _step * _step) + 2 * by * (_step * _step);

    thirdFDX = 6 * ax * (_step * _step * _step);
    thirdFDY = 6 * ay * (_step * _step * _step);

    _table[0].x = (double)pointX;
    _table[0].y = (double)pointY;

    for (i = 0; i < resolution - 1; i++)
    {
        pointX += firstFDX;
        pointY += firstFDY;

        firstFDX += secondFDX;
        firstFDY += secondFDY;

        secondFDX += thirdFDX;
        secondFDY += thirdFDY;

        _table[i + 1].x = (double)pointX;
        _table[i + 1].y = (double)pointY;
    }
}

double BezierCurve::evaluate(const double par)
{
    if (par >= 0.999)
        return 0.0;

    else if (par <= 0.0001)
        return 1.0;

    double fi = par * (double)(_resolution - 1);
    int i = (int)fi;

    if (i < 0)
        return _table[0].y;

    if (i >= _resolution)
        return _table[_resolution - 1].y;

    //double test = fi;

    fi = fi - (double)i;

    //Point3Dd p0(_table[i].x, _table[i].y, 0.0);
    //Point3Dd p1((fi)* _table[i].x + (1.0 - fi)*_table[i + 1].x, (fi)* _table[i].y + (1.0 - fi) *_table[i + 1].y, 0.0);
    //Point3Dd p2(_table[i + 1].x, _table[i + 1].y, 0.0);
    //Acad::drawPoint(p0);
    //Acad::drawPoint(p2);
    //Acad::drawPoint(p1);

    //par = (fi)* _table[i].x + (1.0 - fi)*_table[i + 1].x;
    return (fi * _table[i].y + (1.0 - fi) *_table[i + 1].y);
}

void BezierCurve::render()
{
#if 0
	int resol = 400;
	double step = 1.0 / (double)resol;
	double par = 0.0;
	Point3Dd lastPoint;
	lastPoint.x = 0.0;
	lastPoint.y = evaluate(par);
	lastPoint.z = 0;

	Point3Dd curPoint(0.0, 0.0, 1.0);
	glColor3d(1.0, 0.0, .0);
	glBegin(GL_POINTS);
	for (int i = 0; i < resol; i++)
	{
		par = i * step;
		curPoint.y = evaluate(par);
		curPoint.x = par;
		glVertex3dv(curPoint.v);
	}
	glEnd();

	glColor3d(.0, 0.0, 1.0);
	glBegin(GL_POINTS);
	for (int i = 1; i < _resolution; i++)
	{
		curPoint.x = _table[i].x;
		curPoint.y = _table[i].y;
		glVertex3dv(curPoint.v);
	}
	glEnd();
#endif
}

void BezierCurve::dumpCurve()
{
    std::ofstream of("F:\\Programming\\Test Data\\sculpt\\bezier.txt");
    for (int i = 0; i < _resolution; i++)
    {
        of << _table[i].x << "\t" << _table[i].y << std::endl;
    }
    of.close();
}
