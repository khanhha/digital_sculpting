#include <memory>
#include "BaseLib/VQuadric.h"
#include "BaseLib/MathUtil.h"

namespace Qdr
{
	//#define QUADRIC_FLT_TOT (sizeof(Quadric) / sizeof(double))

	//void BLI_quadric_from_plane(Quadric *q, const Vector4d &v)
	//{
	//	q->a2 = v[0] * v[0];
	//	q->b2 = v[1] * v[1];
	//	q->c2 = v[2] * v[2];

	//	q->ab = v[0] * v[1];
	//	q->ac = v[0] * v[2];
	//	q->bc = v[1] * v[2];

	//	q->ad = v[0] * v[3];
	//	q->bd = v[1] * v[3];
	//	q->cd = v[2] * v[3];

	//	q->d2 = v[3] * v[3];
	//}

	//void BLI_quadric_to_tensor_m3(const Quadric *q, Matrix3f &m)
	//{
	//	m(0,0) = (float)q->a2;
	//	m(0,1) = (float)q->ab;
	//	m(0,2) = (float)q->ac;

	//	m(1,0) = (float)q->ab;
	//	m(1,1) = (float)q->b2;
	//	m(1,2) = (float)q->bc;

	//	m(2,0) = (float)q->ac;
	//	m(2,1) = (float)q->bc;
	//	m(2,2) = (float)q->c2;
	//}

	//void BLI_quadric_to_vector_v3(const Quadric *q, Vector3f &v)
	//{
	//	v[0] = (float)q->ad;
	//	v[1] = (float)q->bd;
	//	v[2] = (float)q->cd;
	//}

	//void BLI_quadric_clear(Quadric *q)
	//{
	//	memset(q, 0, sizeof(*q));
	//}

	//void BLI_quadric_add_qu_qu(Quadric *a, const Quadric *b)
	//{
	//	MathUtil::add_vn_vn_d((double *)a, (double *)b, QUADRIC_FLT_TOT);
	//}

	//void BLI_quadric_add_qu_ququ(Quadric *r, const Quadric *a, const Quadric *b)
	//{
	//	MathUtil::add_vn_vnvn_d((double *)r, (const double *)a, (const double *)b, QUADRIC_FLT_TOT);
	//}

	//void BLI_quadric_mul(Quadric *a, const double scalar)
	//{
	//	MathUtil::mul_vn_db((double *)a, QUADRIC_FLT_TOT, scalar);
	//}

	//double BLI_quadric_evaluate(const Quadric *q, const Vector3f &v_fl)
	//{
	//	const Vector3d v = v_fl.cast<double>();
	//	return ((q->a2 * v[0] * v[0]) + (q->ab * 2 * v[0] * v[1]) + (q->ac * 2 * v[0] * v[2]) + (q->ad * 2 * v[0]) +
	//		(q->b2 * v[1] * v[1]) + (q->bc * 2 * v[1] * v[2]) + (q->bd * 2 * v[1]) +
	//		(q->c2 * v[2] * v[2]) + (q->cd * 2 * v[2]) +
	//		(q->d2));
	//}

	//bool BLI_quadric_optimize(const Quadric *q, Vector3f &v, float epsilon)
	//{
	//	Matrix3f m, invm;
	//	bool invertible;

	//	BLI_quadric_to_tensor_m3(q, m);

	//	m.computeInverseWithCheck(invm, invertible, epsilon);
	//	if (invertible){
	//		BLI_quadric_to_vector_v3(q, v);
	//		v = invm * v;
	//		v = -v;
	//		return true;
	//	}
	//	return false;
	//}

	Quadric::Quadric()
	{
		quadric_clear();
	}

	Quadric::Quadric(const Quadric &other)
		:
		a2(other.a2), b2(other.b2), c2(other.c2),
		ab(other.ab), ac(other.ac), bc(other.bc),
		ad(other.ad), bd(other.bd), cd(other.cd), d2(other.d2)
	{}

	Quadric& Quadric::operator=(const Quadric &other)
	{
		a2 = other.a2;
		b2 = other.b2;
		c2 = other.c2;

		ab = other.ab;
		ac = other.ac;
		bc = other.bc;

		ad = other.ad;
		bd = other.bd;
		cd = other.cd;

		d2 = other.d2;

		return *this;
	}

	Quadric Quadric::operator+(const Quadric &lhs)
	{
		Quadric q;
		q.a2 = a2 + lhs.a2;
		q.b2 = b2 + lhs.b2;
		q.c2 = c2 + lhs.c2;

		q.ab = ab + lhs.ab;
		q.ac = ac + lhs.ac;
		q.bc = bc + lhs.bc;

		q.ad = ad + lhs.ad;
		q.bd = bd + lhs.bd;
		q.cd = cd + lhs.cd;

		q.d2 = d2 + lhs.d2;
		
		return q;
	}

	Quadric& Quadric::operator+=(const Quadric &other)
	{
		a2 += other.a2;
		b2 += other.b2;
		c2 += other.c2;

		ab += other.ab;
		ac += other.ac;
		bc += other.bc;

		ad += other.ad;
		bd += other.bd;
		cd += other.cd;

		d2 += other.d2;

		return *this;
	}


	Quadric& Quadric::operator*=(const float &val)
	{
		a2 *= val;
		b2 *= val;
		c2 *= val;

		ab *= val;
		ac *= val;
		bc *= val;

		ad *= val;
		bd *= val;
		cd *= val;

		d2 *= val;

		return *this;
	}


	void Quadric::add_plane(const Vector4d &v)
	{
		a2 += v[0] * v[0];
		b2 += v[1] * v[1];
		c2 += v[2] * v[2];

		ab += v[0] * v[1];
		ac += v[0] * v[2];
		bc += v[1] * v[2];

		ad += v[0] * v[3];
		bd += v[1] * v[3];
		cd += v[2] * v[3];

		d2 += v[3] * v[3];
	}

	void Quadric::quadric_from_plane(const Vector4d &v)
	{
		a2 = v[0] * v[0];
		b2 = v[1] * v[1];
		c2 = v[2] * v[2];

		ab = v[0] * v[1];
		ac = v[0] * v[2];
		bc = v[1] * v[2];

		ad = v[0] * v[3];
		bd = v[1] * v[3];
		cd = v[2] * v[3];

		d2 = v[3] * v[3];
	}

	void Quadric::quadric_to_tensor_m3(Matrix3f &m) const 
	{
		m(0, 0) = (float)a2;
		m(0, 1) = (float)ab;
		m(0, 2) = (float)ac;

		m(1, 0) = (float)ab;
		m(1, 1) = (float)b2;
		m(1, 2) = (float)bc;

		m(2, 0) = (float)ac;
		m(2, 1) = (float)bc;
		m(2, 2) = (float)c2;
	}

	void Quadric::quadric_to_vector_v3(Vector3f &v) const
	{
		v[0] = (float)ad;
		v[1] = (float)bd;
		v[2] = (float)cd;
	}

	void Quadric::quadric_clear()
	{
		a2 = 0;
		b2 = 0;
		c2 = 0;

		ab = 0;
		ac = 0;
		bc = 0;

		ad = 0;
		bd = 0;
		cd = 0;

		d2 = 0;
	}

	bool Quadric::quadric_optimize(Vector3f &v, float epsilon) const
	{
		Matrix3f m, invm;
		bool invertible;

		quadric_to_tensor_m3(m);

		m.computeInverseWithCheck(invm, invertible, epsilon);
		if (invertible){
			quadric_to_vector_v3(v);
			v = invm * v;
			v = -v;
			return true;
		}

		return false;
	}

	double Quadric::quadric_evaluate(const Vector3f &v_fl) const
	{
		const Vector3d v = v_fl.cast<double>();
		return (
			(a2 * v[0] * v[0]) + (ab * 2 * v[0] * v[1]) + (ac * 2 * v[0] * v[2]) + (ad * 2 * v[0]) +
			(b2 * v[1] * v[1]) + (bc * 2 * v[1] * v[2]) + (bd * 2 * v[1]) +
			(c2 * v[2] * v[2]) + (cd * 2 * v[2]) +
			(d2));
	}

	

}