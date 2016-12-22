#ifndef SCULPT_Transform_H
#define SCULPT_Transform_H

#include "BaseLib/Point3Dd.h"
#include <string>
#include "OpenGL/include/glew.h"
#include <Eigen/Dense>
class Transform
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
    Transform(){};
    ~Transform(){};
    void reset();
    Eigen::Vector2f project(const Eigen::Vector3f& point);
    bool unprojectScreenSpace(const int& mousex, const int& mousey, const double& depth, Point3Dd& p);
    bool unproject(const int& winx, const int& winy, const double& winz, Point3Dd& p);
    double calculateDepth(const Point3Dd& p);
    double worldRadius(const double& pixelRadius, const int& mouseX, const int& mouseY, const Point3Dd& location);
    void viewDirection(Point3Dd& viewDir);
    void nearFarPoints(int mouseX, int mouseY, Point3Dd& nearP, Point3Dd& farP);
    void nearPoint(const int& mx, const int& my, Point3Dd& nearP);
    void screenNormal(double& x, double& y, double& z);
    bool invertMatrix44(const double mat[4][4], double inverse[4][4]);
    void copyMatrix33Matrix44(double m1[3][3], double m2[4][4]);
    void mulMatrix33Vector3(const double M[3][3], const double r[3], double out[3]);
    void dump(std::string file);
    void load(std::string file);

public:
	Eigen::Matrix4f _modelViewMat;
	Eigen::Matrix4f _projectMat;

    double _modelview[4][4];
    double _projection[4][4];
    double _invModelview[4][4];
    double _invProjection[4][4];
    int   _viewport[4];
    double _viewDir[3];

};
#endif