#include "sculpt/SUtil.h"
#include <algorithm>
#include <Windows.h>
#include <random>

tbb::mutex g_sculpt_mutex;

void SUtil::renderBoundingBox(const Point3Dd& bbmin, const Point3Dd& bbmax, double* color, bool useColor)
{
#if 0
	Point3Dd listVertex[8];
	double size[3];
	for (int i = 0; i < 3; i++)
		size[i] = bbmax[i] - bbmin[i];

	if (useColor)
		glColor3dv(color);

	listVertex[0] = bbmin;

	listVertex[1][0] = bbmin[0];
	listVertex[1][1] = bbmin[1] + size[1];
	listVertex[1][2] = bbmin[2];

	listVertex[2][0] = bbmin[0] + size[0];
	listVertex[2][1] = bbmin[1] + size[1];
	listVertex[2][2] = bbmin[2];

	listVertex[3][0] = bbmin[0] + size[0];
	listVertex[3][1] = bbmin[1];
	listVertex[3][2] = bbmin[2];

	listVertex[4][0] = bbmin[0];
	listVertex[4][1] = bbmin[1];
	listVertex[4][2] = bbmin[2] + size[2];

	listVertex[5][0] = bbmin[0] + size[0];
	listVertex[5][1] = bbmin[1];
	listVertex[5][2] = bbmin[2] + size[2];

	listVertex[6][0] = bbmin[0] + size[0];;
	listVertex[6][1] = bbmin[1] + size[1];
	listVertex[6][2] = bbmin[2] + size[2];

	listVertex[7][0] = bbmin[0];
	listVertex[7][1] = bbmin[1] + size[1];
	listVertex[7][2] = bbmin[2] + size[2];

	glBegin(GL_LINES);
	glVertex3dv(listVertex[0].v);
	glVertex3dv(listVertex[1].v);
	glVertex3dv(listVertex[1].v);
	glVertex3dv(listVertex[2].v);
	glVertex3dv(listVertex[2].v);
	glVertex3dv(listVertex[3].v);
	glVertex3dv(listVertex[3].v);
	glVertex3dv(listVertex[0].v);

	glVertex3dv(listVertex[4].v);
	glVertex3dv(listVertex[5].v);
	glVertex3dv(listVertex[5].v);
	glVertex3dv(listVertex[6].v);
	glVertex3dv(listVertex[6].v);
	glVertex3dv(listVertex[7].v);
	glVertex3dv(listVertex[7].v);
	glVertex3dv(listVertex[4].v);

	glVertex3dv(listVertex[0].v);
	glVertex3dv(listVertex[4].v);
	glVertex3dv(listVertex[1].v);
	glVertex3dv(listVertex[7].v);
	glVertex3dv(listVertex[2].v);
	glVertex3dv(listVertex[6].v);
	glVertex3dv(listVertex[3].v);
	glVertex3dv(listVertex[5].v);

	glEnd();
#endif
}



bool SUtil::planePointSideFlip(const Point3Dd& coord, const Point3Dd& planePoint, const Point3Dd& planeNorm, int flip)
{
	Point3Dd delta = coord - planePoint;
	double d = planeNorm.dot(delta);
	if (flip)
		d = -d;
	return (d < 0.0);
}

bool SUtil::planePointSideFlip(const Vector3f& coord, const Vector3f& planePoint, const Vector3f& planeNorm, int flip)
{
	Vector3f delta = coord - planePoint;
	float d = planeNorm.dot(delta);
	if (flip)
		d = -d;
	return (d < 0.0f);
}

bool SUtil::invert_m4_m4(float inverse[4][4], float mat[4][4])
{
    int i, j, k;
    double temp;
    float tempmat[4][4];
    float max;
    int maxj;

    /* Set inverse to identity */
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            inverse[i][j] = 0;
    for (i = 0; i < 4; i++)
        inverse[i][i] = 1;

    /* Copy original matrix so we don't mess it up */
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            tempmat[i][j] = mat[i][j];

    for (i = 0; i < 4; i++) {
        /* Look for row with max pivot */
        max = fabsf(tempmat[i][i]);
        maxj = i;
        for (j = i + 1; j < 4; j++) {
            if (fabsf(tempmat[j][i]) > max) {
                max = fabsf(tempmat[j][i]);
                maxj = j;
            }
        }
        /* Swap rows if necessary */
        if (maxj != i) {
            for (k = 0; k < 4; k++) {
                std::swap(tempmat[i][k], tempmat[maxj][k]);
                std::swap(inverse[i][k], inverse[maxj][k]);
            }
        }

        temp = tempmat[i][i];
        if (temp == 0)
            return 0;  /* No non-zero pivot */
        for (k = 0; k < 4; k++) {
            tempmat[i][k] = (float)((double)tempmat[i][k] / temp);
            inverse[i][k] = (float)((double)inverse[i][k] / temp);
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                temp = tempmat[j][i];
                for (k = 0; k < 4; k++) {
                    tempmat[j][k] -= (float)((double)tempmat[i][k] * temp);
                    inverse[j][k] -= (float)((double)inverse[i][k] * temp);
                }
            }
        }
    }
    return 1;
}

void SUtil::mul_v3_m4v3(float r[3], const float mat[4][4], const float vec[3])
{
    const float x = vec[0];
    const float y = vec[1];

    r[0] = x * mat[0][0] + y * mat[1][0] + mat[2][0] * vec[2] + mat[3][0];
    r[1] = x * mat[0][1] + y * mat[1][1] + mat[2][1] * vec[2] + mat[3][1];
    r[2] = x * mat[0][2] + y * mat[1][2] + mat[2][2] * vec[2] + mat[3][2];
}

void SUtil::scale_m4_fl(float m[4][4], float scale)
{
    m[0][0] = m[1][1] = m[2][2] = scale;
    m[3][3] = 1.0;
    m[0][1] = m[0][2] = m[0][3] = 0.0;
    m[1][0] = m[1][2] = m[1][3] = 0.0;
    m[2][0] = m[2][1] = m[2][3] = 0.0;
    m[3][0] = m[3][1] = m[3][2] = 0.0;
}

void SUtil::mul_m4_m4m4(float m1[4][4], float m3_[4][4], float m2_[4][4])
{
    float m2[4][4], m3[4][4];

    /* copy so it works when m1 is the same pointer as m2 or m3 */
    copy_m4_m4(m2, m2_);
    copy_m4_m4(m3, m3_);

    /* matrix product: m1[j][k] = m2[j][i].m3[i][k] */
    m1[0][0] = m2[0][0] * m3[0][0] + m2[0][1] * m3[1][0] + m2[0][2] * m3[2][0] + m2[0][3] * m3[3][0];
    m1[0][1] = m2[0][0] * m3[0][1] + m2[0][1] * m3[1][1] + m2[0][2] * m3[2][1] + m2[0][3] * m3[3][1];
    m1[0][2] = m2[0][0] * m3[0][2] + m2[0][1] * m3[1][2] + m2[0][2] * m3[2][2] + m2[0][3] * m3[3][2];
    m1[0][3] = m2[0][0] * m3[0][3] + m2[0][1] * m3[1][3] + m2[0][2] * m3[2][3] + m2[0][3] * m3[3][3];

    m1[1][0] = m2[1][0] * m3[0][0] + m2[1][1] * m3[1][0] + m2[1][2] * m3[2][0] + m2[1][3] * m3[3][0];
    m1[1][1] = m2[1][0] * m3[0][1] + m2[1][1] * m3[1][1] + m2[1][2] * m3[2][1] + m2[1][3] * m3[3][1];
    m1[1][2] = m2[1][0] * m3[0][2] + m2[1][1] * m3[1][2] + m2[1][2] * m3[2][2] + m2[1][3] * m3[3][2];
    m1[1][3] = m2[1][0] * m3[0][3] + m2[1][1] * m3[1][3] + m2[1][2] * m3[2][3] + m2[1][3] * m3[3][3];

    m1[2][0] = m2[2][0] * m3[0][0] + m2[2][1] * m3[1][0] + m2[2][2] * m3[2][0] + m2[2][3] * m3[3][0];
    m1[2][1] = m2[2][0] * m3[0][1] + m2[2][1] * m3[1][1] + m2[2][2] * m3[2][1] + m2[2][3] * m3[3][1];
    m1[2][2] = m2[2][0] * m3[0][2] + m2[2][1] * m3[1][2] + m2[2][2] * m3[2][2] + m2[2][3] * m3[3][2];
    m1[2][3] = m2[2][0] * m3[0][3] + m2[2][1] * m3[1][3] + m2[2][2] * m3[2][3] + m2[2][3] * m3[3][3];

    m1[3][0] = m2[3][0] * m3[0][0] + m2[3][1] * m3[1][0] + m2[3][2] * m3[2][0] + m2[3][3] * m3[3][0];
    m1[3][1] = m2[3][0] * m3[0][1] + m2[3][1] * m3[1][1] + m2[3][2] * m3[2][1] + m2[3][3] * m3[3][1];
    m1[3][2] = m2[3][0] * m3[0][2] + m2[3][1] * m3[1][2] + m2[3][2] * m3[2][2] + m2[3][3] * m3[3][2];
    m1[3][3] = m2[3][0] * m3[0][3] + m2[3][1] * m3[1][3] + m2[3][2] * m3[2][3] + m2[3][3] * m3[3][3];
}

void SUtil::copy_m4_m4(float m1[4][4], float m2[4][4])
{
    memcpy(m1, m2, sizeof(float[4][4]));
}
/**
* axis angle to 3x3 matrix
*
* This takes the angle with sin/cos applied so we can avoid calculating it in some cases.
*
* \param axis rotation axis (must be normalized).
* \param angle_sin sin(angle)
* \param angle_cos cos(angle)
*/
void SUtil::axis_angle_normalized_to_mat3_ex(float mat[3][3], const float axis[3], const float angle_sin, const float angle_cos)
{
    float nsi[3], ico;
    float n_00, n_01, n_11, n_02, n_12, n_22;

    /* now convert this to a 3x3 matrix */
    ico = (1.0f - angle_cos);
    nsi[0] = axis[0] * angle_sin;
    nsi[1] = axis[1] * angle_sin;
    nsi[2] = axis[2] * angle_sin;

    n_00 = (axis[0] * axis[0]) * ico;
    n_01 = (axis[0] * axis[1]) * ico;
    n_11 = (axis[1] * axis[1]) * ico;
    n_02 = (axis[0] * axis[2]) * ico;
    n_12 = (axis[1] * axis[2]) * ico;
    n_22 = (axis[2] * axis[2]) * ico;

    mat[0][0] = n_00 + angle_cos;
    mat[0][1] = n_01 + nsi[2];
    mat[0][2] = n_02 - nsi[1];
    mat[1][0] = n_01 - nsi[2];
    mat[1][1] = n_11 + angle_cos;
    mat[1][2] = n_12 + nsi[0];
    mat[2][0] = n_02 + nsi[1];
    mat[2][1] = n_12 - nsi[0];
    mat[2][2] = n_22 + angle_cos;
}

void SUtil::mul_v3_m3v3(float r[3], float M[3][3], const float a[3])
{
    r[0] = M[0][0] * a[0] + M[1][0] * a[1] + M[2][0] * a[2];
    r[1] = M[0][1] * a[0] + M[1][1] * a[1] + M[2][1] * a[2];
    r[2] = M[0][2] * a[0] + M[1][2] * a[1] + M[2][2] * a[2];
}


size_t SUtil::random()
{
    LARGE_INTEGER val;
    assert(QueryPerformanceCounter(&val));
    std::mt19937 mt((size_t)val.LowPart);
    return  mt();
}

bool SUtil::isVertexInsideBrush(const Point3Dd& center, const double& sqrRad, const Point3Dd& coord)
{
	if ((coord.squareDistanceToPoint(center) <= sqrRad)) 
		return true;
	else 
		return false;
}
bool SUtil::isVertexInsideBrush(const Vector3f& center, const float& sqrRad, const  Vector3f& coord)
{
	return (center - coord).squaredNorm() <= sqrRad;
}

bool SUtil::planePointSide(const Point3Dd& coord, const Point3Dd& planePoint, const Point3Dd& planeNorm)
{
	Point3Dd delta = coord - planePoint;
	double d = planeNorm.dot(delta);
	return (d < 0.0);
}

bool SUtil::planePointSide(const Vector3f& coord, const Vector3f& planePoint, const Vector3f& planeNorm)
{
	Vector3f delta = coord - planePoint;
	double d = planeNorm.dot(delta);
	return (d < 0.0f);
}

void SUtil::closestToLine(Point3Dd& out, const Point3Dd& in, const Point3Dd& start, const Point3Dd& end)
{
	Point3Dd u = end - start;
	Point3Dd h = in - start;
	double lambda = u.dot(h) / u.dot(u);
	out.x = start.x + u.x * lambda;
	out.y = start.y + u.y * lambda;
	out.z = start.z + u.z * lambda;
	return;
}
