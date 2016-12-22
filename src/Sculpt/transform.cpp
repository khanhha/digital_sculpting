#include "Sculpt/transform.h"
#include <iostream>
#include <fstream>
#include <Eigen/Dense>
void Transform::reset()
{
    glGetFloatv(GL_MODELVIEW_MATRIX,  _modelViewMat.data());
    glGetFloatv(GL_PROJECTION_MATRIX, _projectMat.data());

    glGetDoublev(GL_MODELVIEW_MATRIX, &_modelview[0][0]);
    glGetDoublev(GL_PROJECTION_MATRIX, &_projection[0][0]);
    glGetIntegerv(GL_VIEWPORT, _viewport);

    invertMatrix44(_modelview, _invModelview);
    invertMatrix44(_projection, _invProjection);

    _viewDir[0] = _modelview[0][0];
    _viewDir[0] = _modelview[0][1];
    _viewDir[0] = _modelview[0][2];
}

Eigen::Vector2f Transform::project(const Eigen::Vector3f& point)
{
    //Eigen::Vector4f p(point.x(), point.y(), point.z(), 1.0f);

    //p = _projectMat * p;
    //p = _modelViewMat * p;

    //p  /= p.w();
    //p = p * 0.5f + Eigen::Vector4f(0.5f, 0.5f, 0.5f, 0.5f);
    //p.x() = p.x() * float(_viewport[2]) + float(_viewport[0]);
    //p.y() = p.y() * float(_viewport[3]) + float(_viewport[1]);

    //return Eigen::Vector2f(p.x(), p.y());

    Eigen::Vector3d proj;
    //double modelview[4][4];
    //double projection[4][4];
    //int   viewport[4];
    //glGetDoublev(GL_MODELVIEW_MATRIX, &modelview[0][0]);
    //glGetDoublev(GL_PROJECTION_MATRIX, &projection[0][0]);
    //glGetIntegerv(GL_VIEWPORT, viewport);
    //gluProject(point.x(), point.y(), point.z(), &modelview[0][0], &projection[0][0], viewport, &proj.x(), &proj.y(), &proj.z());
    
    gluProject(point.x(), point.y(), point.z(), &_modelview[0][0], &_projection[0][0], _viewport, &proj.x(), &proj.y(), &proj.z());

    return Eigen::Vector2f((float)proj.x(), (float)proj.y());
}

bool Transform::unprojectScreenSpace(const int& mousex, const int& mousey, const double& depth, Point3Dd& p)
{
    double mx = (double)mousex;
    double my = (double)mousey;
    bool ret = gluUnProject(mx, my, depth, &_modelview[0][0], &_projection[0][0], _viewport, &p.x, &p.y, &p.z);
    return ret;
}

bool Transform::unproject(const int& mousex, const int& mousey, const double& depth, Point3Dd& p)
{
    double mx = (double)mousex;
    double my = (double)_viewport[3] - (double)mousey;
    bool ret = gluUnProject(mx, my, depth, &_modelview[0][0], &_projection[0][0], _viewport, &p.x, &p.y, &p.z);
    return ret;
}

double Transform::calculateDepth(const Point3Dd& p)
{
    double winX, winY, winZ;
    gluProject(static_cast<double> (p.x), static_cast<double> (p.y), static_cast<double> (p.z), &_modelview[0][0], &_projection[0][0], _viewport, &winX, &winY, &winZ);

    if (winZ < 0.0)
        return -winZ;
    else
        return winZ;
}

void Transform::viewDirection(Point3Dd& viewDir)
{
    double viewAxis[3] = { 0.0f, 0.0f, 1.0f };
    double mat[3][3];

    copyMatrix33Matrix44(mat, _invModelview);
    mulMatrix33Vector3(mat, viewAxis, viewDir.v);

    double sqrtNorm = std::sqrt(viewDir[0] * viewDir[0] + viewDir[1] * viewDir[1] + viewDir[2] * viewDir[2]);
    if (sqrtNorm > EPSILON_VAL_)
    {
        double norm = 1.0 / std::sqrt(viewDir[0] * viewDir[0] + viewDir[1] * viewDir[1] + viewDir[2] * viewDir[2]);
        viewDir[0] = norm * viewDir[0];
        viewDir[1] = norm * viewDir[1];
        viewDir[2] = norm * viewDir[2];
    }
    return;
}

void Transform::nearFarPoints(int mouseX, int mouseY, Point3Dd& nearP, Point3Dd& farP)
{
    unproject(mouseX, mouseY, 0.0001, nearP);
    unproject(mouseX, mouseY, 0.9999, farP);
    return;
}

void Transform::nearPoint(const int& mx, const int& my, Point3Dd& nearP)
{
    unproject(mx, my, 0.0001, nearP);
    return;
}

double Transform::worldRadius(const double& pixelRadius, const int& mouseX, const int& mouseY, const Point3Dd& location)
{
    double depth = calculateDepth(location);

    double x, y, z;
    Point3Dd boundary;
    unproject(mouseX, mouseY + pixelRadius, depth, boundary);

    return (location - boundary).length();
}

void Transform::dump(std::string file)
{
    std::ofstream of(file);
    double tmp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            of << _modelview[i][j] << " ";

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            of << _projection[i][j] << " ";

    for (int i = 0; i < 4; i++)
        of << _viewport[i] << " ";
    of.close();
}

void Transform::load(std::string file)
{
    std::ifstream of(file);
    double* m = new double[16];
    double tmp;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            of >> tmp;
            _modelview[i][j] = tmp;
        }
    }


    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            of >> tmp;
            _projection[i][j] = tmp;
        }
    }

    for (int i = 0; i < 4; m++, i++)
        of >> _viewport[i];

    invertMatrix44(_modelview, _invModelview);
    invertMatrix44(_projection, _invProjection);

    of.close();
}

void Transform::screenNormal(double& x, double& y, double& z)
{
    //double iden[3] = { 0.0, 0.0, 1.0 };
    //norm[0] = (iden[0] * _modelview[0][0] +
    //    iden[1] * _modelview[0][1] +
    //    iden[2] * _modelview[0][2]);
    //norm[1] = (iden[0] * _modelview[1][0] +
    //    iden[1] * _modelview[1][1] +
    //    iden[2] * _modelview[1][2]);
    //norm[2] = (iden[0] * _modelview[2][0] +
    //    iden[1] * _modelview[2][1] +
    //    iden[2] * _modelview[2][2]);
}

/*
* invertmat -
*      computes the inverse of mat and puts it in inverse.  Returns
*  true on success (i.e. can always find a pivot) and false on failure.
*  Uses Gaussian Elimination with partial (maximal column) pivoting.
*
*					Mark Segal - 1992
*/

bool Transform::invertMatrix44(const double mat[4][4], double inverse[4][4])
{
    int i, j, k;
    double temp;
    double tempmat[4][4];
    double max;
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
            tempmat[i][k] = (double)((double)tempmat[i][k] / temp);
            inverse[i][k] = (double)((double)inverse[i][k] / temp);
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                temp = tempmat[j][i];
                for (k = 0; k < 4; k++) {
                    tempmat[j][k] -= (double)((double)tempmat[i][k] * temp);
                    inverse[j][k] -= (double)((double)inverse[i][k] * temp);
                }
            }
        }
    }
    return 1;
}

void Transform::copyMatrix33Matrix44(double m1[3][3], double m2[4][4])
{
    m1[0][0] = m2[0][0];
    m1[0][1] = m2[0][1];
    m1[0][2] = m2[0][2];

    m1[1][0] = m2[1][0];
    m1[1][1] = m2[1][1];
    m1[1][2] = m2[1][2];

    m1[2][0] = m2[2][0];
    m1[2][1] = m2[2][1];
    m1[2][2] = m2[2][2];
}

void Transform::mulMatrix33Vector3(const double M[3][3], const double in[3], double out[3])
{
    out[0] = M[0][0] * in[0] + M[1][0] * in[1] + M[2][0] * in[2];
    out[1] = M[0][1] * in[0] + M[1][1] * in[1] + M[2][1] * in[2];
    out[2] = M[0][2] * in[0] + M[1][2] * in[1] + M[2][2] * in[2];
}
