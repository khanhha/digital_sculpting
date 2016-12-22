#include "VBQuaternion.h"
#include "Point3Dd.h"

/**************************************************************************************************
 * @fn	Quaternion::Quaternion ()
 *
 * @brief	Default constructor.
 *
 * @author	Son
 * @date	8/21/2013
 **************************************************************************************************/

VBQuaternion::VBQuaternion ()
{
}

/**************************************************************************************************
 * @fn	Quaternion::Quaternion (double w, double x, double y, double z)
 *
 * @brief	Constructor.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	w	The width.
 * @param	x	The double to process.
 * @param	y	The double to process.
 * @param	z	The double to process.
 **************************************************************************************************/

VBQuaternion::VBQuaternion (double w, double x, double y, double z)
{
    q[0] = w;
    q[1] = x;
    q[2] = y;
    q[3] = z;
}

/**************************************************************************************************
 * @fn	Quaternion::Quaternion (const Quaternion& quat)
 *
 * @brief	Constructor.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	quat	The quaternion.
 **************************************************************************************************/

VBQuaternion::VBQuaternion (const VBQuaternion& quat)
{
    this->q[0] = quat.q[0];
    this->q[1] = quat.q[1];
    this->q[2] = quat.q[2];
    this->q[3] = quat.q[3];
}

VBQuaternion::VBQuaternion(const double angle, const Point3Dd &axis)
{
    quatFromAngleAxis(angle, axis.v);
}

/**************************************************************************************************
 * @fn	Quaternion::~Quaternion()
 *
 * @brief	Destructor.
 *
 * @author	Son
 * @date	8/21/2013
 **************************************************************************************************/

VBQuaternion::~VBQuaternion()
{

}

/**************************************************************************************************
 * @fn	void Quaternion::quatFromMatrix(double rot[9])
 *
 * @brief	Quaternion from matrix.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	rot	The rot.
 **************************************************************************************************/

void VBQuaternion::quatFromMatrix(double rot[9])
{
#if 0
    double m[3][3] = { rot[0], rot[1], rot[2], rot[3], rot[4], rot[5], rot[6], rot[7], rot[8]};
    double trace = m[0][0] + m[1][1] + m[2][2];
    if (trace > 0) { 
        float S = sqrt(trace+1.0) * 2; // S=4*qw 
        q[0] = 0.25 * S;
        q[1] = (m[2][1] - m[1][2]) / S;
        q[2]= (m[0][2] - m[2][0]) / S; 
        q[3] = (m[1][0] - m[0][1]) / S; 
    } else if ((m[0][0] > m[1][1])&(m[0][0] > m[2][2])) { 
        float S = sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2; // S=4*qx 
        q[0] = (m[2][1] - m[1][2]) / S;
        q[1] = 0.25 * S;
        q[2] = (m[0][1] + m[1][0]) / S; 
        q[3] = (m[0][2] + m[2][0]) / S; 
    } else if (m[1][1] > m[2][2]) { 
        float S = sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2; // S=4*qy
        q[0] = (m[0][2] - m[2][0]) / S;
        q[1] = (m[0][1] + m[1][0]) / S; 
        q[2] = 0.25 * S;
        q[3] = (m[1][2] + m[2][1]) / S; 
    } else { 
        float S = sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2; // S=4*qz
        q[0] = (m[1][0] - m[0][1]) / S;
        q[1] = (m[0][2] + m[2][0]) / S;
        q[2] = (m[1][2] + m[2][1]) / S;
        q[3] = 0.25 * S;
    }
#endif
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    const int next[3] = { 1, 2, 0 };
    double m[3][3] = { rot[0], rot[1], rot[2], rot[3], rot[4], rot[5], rot[6], rot[7], rot[8]};
    double trace = m[0][0] + m[1][1] + m[2][2];

    double root;

    if (trace > (double)0)
    {
        // |w| > 1/2, may as well choose w > 1/2
        root = sqrt(trace + (double)1);  // 2w
        q[0] = ((double)0.5)*root;
        root = ((double)0.5)/root;  // 1/(4w)
        q[1] = (m[2][1] - m[1][2])*root;
        q[2] = (m[0][2] - m[2][0])*root;
        q[3] = (m[1][0] - m[0][1])*root;
    }
    else
    {
        // |w| <= 1/2
        int i = 0;
        if (m[1][1] > m[0][0])
        {
            i = 1;
        }
        if (m[2][2] > m[i][i])
        {
            i = 2;
        }
        int j = next[i];
        int k = next[j];

        root = sqrt(m[i][i] - m[j][j] - m[k][k] + (double)1);
        double* quat[3] = { &q[1], &q[2], &q[3] };
        *quat[i] = ((double)0.5)*root;
        root = ((double)0.5)/root;
        q[0] = (m[k][j] - m[j][k])*root;
        *quat[j] = (m[j][i] + m[i][j])*root;
        *quat[k] = (m[k][i] + m[i][k])*root;
    }
}

/**************************************************************************************************
 * @fn	void Quaternion::quatFromAngleAxis(const double angle, const double axis[3])
 *
 * @brief	Quaternion from angle axis.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	angle	The angle.
 * @param	axis 	The axis.
 **************************************************************************************************/

void VBQuaternion::quatFromAngleAxis(const double angle, const double axis[3])
{
    double sinA, cosA;
    Vector3Dd ax(axis);
    ax.unit();
    sinA = sin( angle / 2 );
    cosA = cos( angle / 2 );
    q[0]   = cosA;
    q[1]   = ax.x * sinA;
    q[2]   = ax.y * sinA;
    q[3]   = ax.z * sinA;
}

/**************************************************************************************************
 * @fn	void Quaternion::toAngleAxis(double &angle, double axis[3])
 *
 * @brief	Converts this object to an angle axis.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param [in,out]	angle	The angle.
 * @param	axis		 	The axis.
 **************************************************************************************************/

void VBQuaternion::toAngleAxis(double &angle, double axis[3])
{
    double cosA, sinA;
#if 1
    if(false == FLOAT_EQUAL(q[0] - 1, 0) && 
         q[0] > 1)
    {
        /* need to normalize quat first */
        normalize(EPSILON_VAL_);
    }
    cosA = q[0];
    angle = acos( cosA ) * 2;
    sinA = sqrt( 1.0 - cosA * cosA );
    if ( fabs( sinA ) < 0.0005 ) sinA = 1;
    axis[0] = q[1] / sinA;
    axis[1] = q[2] / sinA;
    axis[2] = q[3] / sinA;
#else
    // The quaternion representing the rotation is
    //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

    double sqrLength = q[1]*q[1] + q[2]*q[2]
    + q[3]*q[3];

    if (sqrLength > EPSILON_VAL_)
    {
        angle = ((double)2)*acos(q[0]);
        double invLength = 1/sqrt(sqrLength);
        axis[0] = q[1]*invLength;
        axis[1] = q[2]*invLength;
        axis[2] = q[3]*invLength;
    }
    else
    {
        // Angle is 0 (mod 2*pi), so any axis will do.
        angle = (double)0;
        axis[0] = (double)1;
        axis[1] = (double)0;
        axis[2] = (double)0;
    }
#endif
    angle = fabs(angle) > PI_VAL_ ? 
            (angle > 0 ? angle - PI_VAL_ * 2 : angle + PI_VAL_ * 2):
            angle;  
}

/**************************************************************************************************
 * @fn	void Quaternion::toRotateMatrix(double rotMat[9])
 *
 * @brief	Converts a rotMat to a rotate matrix.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	rotMat	The rot mat.
 **************************************************************************************************/

void VBQuaternion::toRotateMatrix(double rotMat[9])
{
    double xx = q[1] * q[1];
    double yy = q[2] * q[2];
    double zz = q[3] * q[3];
    double xy = q[1] * q[2];
    double yz = q[2] * q[3];
    double xz = q[1] * q[3];
    double xw = q[1] * q[0];
    double yw = q[2] * q[0];  
    double zw = q[3] * q[0];

    rotMat[0]  = 1 - 2 * ( yy + zz );
    rotMat[1]  =     2 * ( xy - zw );
    rotMat[2]  =     2 * ( xz + yw );
    rotMat[3]  =     2 * ( xy + zw );
    rotMat[4]  = 1 - 2 * ( xx + zz );
    rotMat[5]  =     2 * ( yz - xw );
    rotMat[6]  =     2 * ( xz - yw );
    rotMat[7]  =     2 * ( yz + xw );
    rotMat[8] = 1 - 2 * ( xx + yy );
    /*mat[3]  = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
    mat[15] = 1;*/
}

/**************************************************************************************************
 * @fn	Quaternion Quaternion::multiply(const Quaternion &rquat)
 *
 * @brief	Multiplies the given rquat.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	rquat	The rquat.
 *
 * @return	.
 **************************************************************************************************/

VBQuaternion VBQuaternion::multiply(const VBQuaternion &rquat)
{
    // NOTE:  Multiplication is not generally commutative, so in most
    // cases p*q != q*p.
    VBQuaternion result;

    result.q[0] =
        q[0]*rquat.q[0] -
        q[1]*rquat.q[1] -
        q[2]*rquat.q[2] -
        q[3]*rquat.q[3];

    result.q[1] =
        q[0]*rquat.q[1] +
        q[1]*rquat.q[0] +
        q[2]*rquat.q[3] -
        q[3]*rquat.q[2];

    result.q[2] =
        q[0]*rquat.q[2] +
        q[2]*rquat.q[0] +
        q[3]*rquat.q[1] -
        q[1]*rquat.q[3];

    result.q[3] =
        q[0]*rquat.q[3] +
        q[3]*rquat.q[0] +
        q[1]*rquat.q[2] -
        q[2]*rquat.q[1];

    return result;
}

/**************************************************************************************************
 * @fn	double Quaternion::squaredLength()
 *
 * @brief	Gets the squared length.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @return	.
 **************************************************************************************************/

double VBQuaternion::squaredLength()
{
    return (q[0]*q[0] + q[1]*q[1] +
            q[2]*q[2] + q[3]*q[3]);
}

/**************************************************************************************************
 * @fn	Quaternion Quaternion::inverse()
 *
 * @brief	Gets the inverse.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @return	.
 **************************************************************************************************/

VBQuaternion VBQuaternion::inverse()
{
    VBQuaternion inverse;
    double norm = squaredLength();
    if (norm > (double)0)
    {
        double invNorm = ((double)1)/norm;
        inverse.q[0] =  q[0]*invNorm;
        inverse.q[1] = -q[1]*invNorm;
        inverse.q[2] = -q[2]*invNorm;
        inverse.q[3] = -q[3]*invNorm;
    }
    else
    {
        // Return an invalid result to flag the error.
        for (int i = 0; i < 4; ++i)
        {
            inverse.q[i] = (double)0;
        }
    }

    return inverse;
}

/**************************************************************************************************
 * @fn	void Quaternion::rotateVector(const double vec[3], double r_vec[3])
 *
 * @brief	Rotate vector.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	vec  	The vector.
 * @param	r_vec	The vector.
 **************************************************************************************************/

void VBQuaternion::rotateVector(const double vec[3], double r_vec[3])
{
    double rotMat[9];
    toRotateMatrix(rotMat);
    for (int i = 0; i < 3; i++)
    {
        r_vec[i] = 0;
        for (int k = 0; k < 3; k++)
        {
            r_vec[i] += rotMat[3*i + k] * vec[k];
        }
    }

}

/**************************************************************************************************
 * @fn	Vector3Dd Quaternion::rotateVector(const Vector3Dd &vec)
 *
 * @brief	Rotate vector.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	vec	The vector.
 *
 * @return	.
 **************************************************************************************************/

Vector3Dd VBQuaternion::rotateVector(const Vector3Dd &vec)
{
    Vector3Dd uv = this->vec().cross(vec);
    uv += uv;
    return vec + this->w * uv + this->vec().cross(uv);
}

Vector3Dd VBQuaternion::rotateVector(const Vector3Dd &vec, const Point3Dd &center)
{
    Vector3Dd v(vec - center);
    v = rotateVector(v);

    return (v + center);
}

/**************************************************************************************************
 * @fn	inline double Quaternion::length () const
 *
 * @brief	Gets the length.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @return	.
 **************************************************************************************************/

inline double VBQuaternion::length () const
{
    return sqrt(q[0]*q[0] + q[1]*q[1] +
        q[2]*q[2] + q[3]*q[3]);
}

/**************************************************************************************************
 * @fn	double Quaternion::normalize (double epsilon)
 *
 * @brief	Normalizes.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @param	epsilon	The epsilon.
 *
 * @return	.
 **************************************************************************************************/

double VBQuaternion::normalize (double epsilon)
{
    double length = this->length();

    if (length > epsilon)
    {
        double invLength = ((double)1)/length;
        q[0] *= invLength;
        q[1] *= invLength;
        q[2] *= invLength;
        q[3] *= invLength;
    }
    else
    {
        length = (double)0;
        q[0] = (double)0;
        q[1] = (double)0;
        q[2] = (double)0;
        q[3] = (double)0;
    }

    return length;
}

/**************************************************************************************************
 * @fn	Quaternion Quaternion::conjugate () const
 *
 * @brief	Gets the conjugate.
 *
 * @author	Son
 * @date	8/21/2013
 *
 * @return	.
 **************************************************************************************************/

VBQuaternion VBQuaternion::conjugate () const
{
    return VBQuaternion(q[0], -q[1], -q[2], -q[3]);
}