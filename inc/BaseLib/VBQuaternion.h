#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "stdio.h"
#include "math.h"
#include "defined.h"
#include "Point3Dd.h"

class Matrix3x3d;

class VBQuaternion {
	public:

	union {
		double q[4];
		struct {
			double w;
			double x;
			double y;
			double z;
		};
	};

	/**************************************************************************************************
	 * @fn	Quaternion::Quaternion();
	 *
	 * @brief	Default constructor.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 **************************************************************************************************/

	VBQuaternion();

	/**************************************************************************************************
	 * @fn	Quaternion::Quaternion(double v[4]);
	 *
	 * @brief	Constructor.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	v	The double to process.
	 **************************************************************************************************/

	VBQuaternion(double v[4]);

	/**************************************************************************************************
	 * @fn	Quaternion::Quaternion(double w, double x, double y, double z);
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

	VBQuaternion(double w, double x, double y, double z);

	/**************************************************************************************************
	 * @fn	Quaternion::Quaternion (const Quaternion& quat);
	 *
	 * @brief	Constructor.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	quat	The quaternion.
	 **************************************************************************************************/

	VBQuaternion::VBQuaternion (const VBQuaternion& quat);

    VBQuaternion(const double angle, const Point3Dd &axis);

	~VBQuaternion();

	/**************************************************************************************************
	 * @fn	void Quaternion::quatFromMatrix(double rot[9]);
	 *
	 * @brief	Quaternion from matrix.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	rot	The rot.
	 **************************************************************************************************/

	void quatFromMatrix(double rot[9]);

	/**************************************************************************************************
	 * @fn	void Quaternion::quatFromAngleAxis(const double angle, const double axis[3]);
	 *
	 * @brief	Quaternion from angle axis.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	angle	The angle.
	 * @param	axis 	The axis.
	 **************************************************************************************************/

	void quatFromAngleAxis(const double angle, const double axis[3]);

	/**************************************************************************************************
	 * @fn	void Quaternion::toAngleAxis(double &angle, double axis[3]);
	 *
	 * @brief	Converts this object to an angle axis.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param [in,out]	angle	The angle.
	 * @param	axis		 	The axis.
	 **************************************************************************************************/

	void toAngleAxis(double &angle, double axis[3]);

	/**************************************************************************************************
	 * @fn	void Quaternion::toRotateMatrix(double rotMat[9]);
	 *
	 * @brief	Converts a rotMat to a rotate matrix.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	rotMat	The rot mat.
	 **************************************************************************************************/

	void toRotateMatrix(double rotMat[9]);

	/**************************************************************************************************
	 * @fn	Quaternion Quaternion::multiply(const Quaternion &rquat);
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

	VBQuaternion multiply(const VBQuaternion &rquat);

	/**************************************************************************************************
	 * @fn	double Quaternion::squaredLength ();
	 *
	 * @brief	Gets the squared length.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @return	.
	 **************************************************************************************************/

	double squaredLength ();

	/**************************************************************************************************
	 * @fn	Quaternion Quaternion::inverse ();
	 *
	 * @brief	Gets the inverse.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @return	.
	 **************************************************************************************************/

	VBQuaternion inverse ();

	/**************************************************************************************************
	 * @fn	void Quaternion::rotateVector(const double vec[3], double r_vec[3]);
	 *
	 * @brief	Rotate vector.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @param	vec  	The vector.
	 * @param	r_vec	The vector.
	 **************************************************************************************************/

	void rotateVector(const double vec[3], double r_vec[3]);
    Vector3Dd vec() { return Vector3Dd(x, y, z); }

	/**************************************************************************************************
	 * @fn	Vector3Dd Quaternion::rotateVector(const Vector3Dd &vec);
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

	Vector3Dd rotateVector(const Vector3Dd &vec);

    Vector3Dd rotateVector(const Vector3Dd &vec, const Point3Dd &center);

	/**************************************************************************************************
	 * @fn	inline double Quaternion::length () const;
	 *
	 * @brief	Gets the length.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @return	.
	 **************************************************************************************************/

	inline double length () const;

	/**************************************************************************************************
	 * @fn	double Quaternion::normalize (double epsilon);
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

	double normalize (double epsilon);

	/**************************************************************************************************
	 * @fn	Quaternion Quaternion::conjugate () const;
	 *
	 * @brief	Gets the conjugate.
	 *
	 * @author	Son
	 * @date	8/21/2013
	 *
	 * @return	.
	 **************************************************************************************************/

	VBQuaternion conjugate () const;

	/**************************************************************************************************
	 * @brief	The quaternion operator.
	 **************************************************************************************************/

	inline VBQuaternion& VBQuaternion::operator= (const VBQuaternion& quat)
	{ 
		q[0] = quat.q[0];
		q[1] = quat.q[1];
		q[2] = quat.q[2];
		q[3] = quat.q[3];
		return *this;
	}

	/**************************************************************************************************
	 * @brief	The =.
	 **************************************************************************************************/

	inline bool VBQuaternion::operator== (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) == 0;
	}
	
	inline bool VBQuaternion::operator!= (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) != 0;
	}
	
	inline bool VBQuaternion::operator< (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) < 0;
	}
	
	inline bool VBQuaternion::operator<= (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) <= 0;
	}
	
	inline bool VBQuaternion::operator> (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) > 0;
	}
	
	inline bool VBQuaternion::operator>= (const VBQuaternion& quat) const
	{
		return memcmp(this->q, quat.q, 4*sizeof(double)) >= 0;
	}
	
	inline VBQuaternion VBQuaternion::operator+ (const VBQuaternion& quat)
		const
	{
		VBQuaternion result;
		for (int i = 0; i < 4; ++i)
		{
			result.q[i] = q[i] + quat.q[i];
		}
		return result;
	}
	
	inline VBQuaternion VBQuaternion::operator- (const VBQuaternion& quat)
		const
	{
		VBQuaternion result;
		for (int i = 0; i < 4; ++i)
		{
			result.q[i] = q[i] - quat.q[i];
		}
		return result;
	}

	VBQuaternion operator*(const VBQuaternion& rquat)
	{
		return multiply(rquat);
	}

	inline VBQuaternion VBQuaternion::operator* (double scalar) const
	{
		VBQuaternion result;
		for (int i = 0; i < 4; ++i)
		{
			result.q[i] = scalar*q[i];
		}
		return result;
	}

	inline VBQuaternion VBQuaternion::operator/ (double scalar) const
	{
		VBQuaternion result;
		int i;

		if (scalar != (double)0)
		{
			double invScalar = ((double)1)/scalar;
			for (i = 0; i < 4; ++i)
			{
				result.q[i] = invScalar*q[i];
			}
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				result.q[i] = DBL_MAX;
			}
		}

		return result;
	}

	inline VBQuaternion VBQuaternion::operator- () const
	{
		VBQuaternion result;
		for (int i = 0; i < 4; ++i)
		{
			result.q[i] = -q[i];
		}
		return result;
	}

	inline VBQuaternion& VBQuaternion::operator+= (const VBQuaternion& quat)
	{
		for (int i = 0; i < 4; ++i)
		{
			q[i] += quat.q[i];
		}
		return *this;
	}

	inline VBQuaternion& VBQuaternion::operator-= (const VBQuaternion& quat)
	{
		for (int i = 0; i < 4; ++i)
		{
			q[i] -= quat.q[i];
		}
		return *this;
	}

	inline VBQuaternion& VBQuaternion::operator*= (double scalar)
	{
		for (int i = 0; i < 4; ++i)
		{
			q[i] *= scalar;
		}
		return *this;
	}

	inline VBQuaternion& VBQuaternion::operator/= (double scalar)
	{
		int i;

		if (scalar != (double)0)
		{
			double invScalar = ((double)1)/scalar;
			for (i = 0; i < 4; ++i)
			{
				q[i] *= invScalar;
			}
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				q[i] = DBL_MAX;
			}
		}

		return *this;
	}

	private:
};

#endif