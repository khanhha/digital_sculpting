// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteMeshFactory.h>
#include "BaseLib/MathUtil.h"

using namespace gte;


MeshFactory::~MeshFactory()
{
}

MeshFactory::MeshFactory()
    :
    mIndexSize(sizeof(unsigned int)),
    mVBUsage(Resource::IMMUTABLE),
    mIBUsage(Resource::IMMUTABLE),
    mOutside(true),
    mPositions(nullptr),
    mNormals(nullptr),
    mTangents(nullptr),
    mBinormals(nullptr)
{
    for (int i = 0; i < VA_MAX_TCOORD_UNITS; ++i)
    {
        mAssignTCoords[i] = false;
        mTCoords[i] = nullptr;
    }
}

MeshFactory::MeshBuffer MeshFactory::CreateSphere(unsigned int numZSamples,
	unsigned int numRadialSamples, float radius)
{
	// Quantities derived from inputs.
	unsigned int zsm1 = numZSamples - 1;
	unsigned int zsm2 = numZSamples - 2;
	unsigned int zsm3 = numZSamples - 3;
	unsigned int rsp1 = numRadialSamples + 1;
	float invRS = 1.0f / static_cast<float>(numRadialSamples);
	float zFactor = 2.0f / static_cast<float>(zsm1);
	unsigned int numVertices = zsm2*rsp1 + 2;
	unsigned int numTriangles = 2 * zsm2*numRadialSamples;

	// Generate geometry.
	std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numVertices);
	if (!vbuffer)
	{
		return MeshBuffer();
	}

	EVector3 pos, nor, basis[3];
	EVector2 tcd;

	// Generate points on the unit circle to be used in computing the mesh
	// points on a sphere slice.
	std::vector<float> cs(rsp1), sn(rsp1);
	for (unsigned int r = 0; r < numRadialSamples; ++r)
	{
		float angle = invRS*r*(float)2.0f * M_PI;
		cs[r] = cos(angle);
		sn[r] = sin(angle);
	}
	cs[numRadialSamples] = cs[0];
	sn[numRadialSamples] = sn[0];

	// Generate the sphere itself.
	unsigned int i = 0;
	for (unsigned int z = 1; z < zsm1; ++z)
	{
		float zFraction = -1.0f + zFactor*static_cast<float>(z);  // in (-1,1)
		float zValue = radius*zFraction;

		// Compute center of slice.
		EVector3 sliceCenter{ 0.0f, 0.0f, zValue };

		// Compute radius of slice.
		float sliceRadius = sqrt(std::abs(radius*radius - zValue*zValue));

		// Compute slice vertices with duplication at endpoint.
		for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
		{
			float radialFraction = r*invRS;  // in [0,1)
			EVector3 radial{ cs[r], sn[r], 0.0f };
			pos = sliceCenter + sliceRadius*radial;
			nor = pos.normalized();
			if (!mOutside)
			{
				nor = -nor;
			}

			basis[0] = nor;
			ComputeOrthogonalComplement(1, basis);
			tcd = { radialFraction, 0.5f*(zFraction + 1.0f) };

			SetPosition(i, pos);
			SetNormal(i, nor);
			SetTangent(i, basis[1]);
			SetBinormal(i, basis[2]);
			SetTCoord(i, tcd);
		}
	}

	// The point at the south pole.
	pos = { 0.0f, 0.0f, -radius };
	if (mOutside)
	{
		nor = { 0.0f, 0.0f, -1.0f };
	}
	else
	{
		nor = { 0.0f, 0.0f, 1.0f };
	}
	basis[0] = nor;
	ComputeOrthogonalComplement(1, basis);
	tcd = { 0.5f, 0.5f };
	SetPosition(i, pos);
	SetNormal(i, nor);
	SetTangent(i, basis[1]);
	SetBinormal(i, basis[2]);
	SetTCoord(i, tcd);
	++i;

	// The point at the north pole.
	pos = { 0.0f, 0.0f, radius };
	if (mOutside)
	{
		nor = { 0.0f, 0.0f, 1.0f };
	}
	else
	{
		nor = { 0.0f, 0.0f, -1.0f };
	}
	basis[0] = nor;
	ComputeOrthogonalComplement(1, basis);
	tcd = { 0.5f, 1.0f };
	SetPosition(i, pos);
	SetNormal(i, nor);
	SetTangent(i, basis[1]);
	SetBinormal(i, basis[2]);
	SetTCoord(i, tcd);

	// Generate indices (outside view).
	std::shared_ptr<IndexBuffer> ibuffer = CreateIBuffer(numTriangles);
	if (!ibuffer)
	{
		return MeshBuffer();
	}
	unsigned int t = 0;
	for (unsigned int z = 0, zStart = 0; z < zsm3; ++z)
	{
		unsigned int i0 = zStart;
		unsigned int i1 = i0 + 1;
		zStart += rsp1;
		unsigned int i2 = zStart;
		unsigned int i3 = i2 + 1;
		for (i = 0; i < numRadialSamples; ++i, ++i0, ++i1, ++i2, ++i3)
		{
			ibuffer->SetTriangle(t++, i0, i1, i2);
			ibuffer->SetTriangle(t++, i1, i3, i2);
		}
	}

	// The south pole triangles (outside view).
	unsigned int numVerticesM2 = numVertices - 2;
	for (i = 0; i < numRadialSamples; ++i, ++t)
	{
		ibuffer->SetTriangle(t, i, numVerticesM2, i + 1);
	}

	// The north pole triangles (outside view).
	unsigned int numVerticesM1 = numVertices - 1, offset = zsm3*rsp1;
	for (i = 0; i < numRadialSamples; ++i, ++t)
	{
		ibuffer->SetTriangle(t, i + offset, i + 1 + offset, numVerticesM1);
	}

	if (!mOutside)
	{
		ReverseTriangleOrder(ibuffer.get());
	}

	// Create the mesh.
	return MeshBuffer(vbuffer, ibuffer);
}


MeshFactory::MeshBuffer MeshFactory::CreateTorus(
    unsigned int numCircleSamples, unsigned int numRadialSamples,
    float outerRadius, float innerRadius)
{
    // Quantities derived from inputs.
    float invCS = 1.0f/static_cast<float>(numCircleSamples);
    float invRS = 1.0f/static_cast<float>(numRadialSamples);
    unsigned int numVertices = (numCircleSamples + 1)*(numRadialSamples + 1);
    unsigned int numTriangles = 2 * numCircleSamples*numRadialSamples;

    // Generate geometry.
    std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
		return MeshBuffer();
    }

    EVector3 pos, nor, basis[3];
    EVector2 tcd;

    // Generate an open cylinder that is warped into a torus.
    unsigned int i = 0;
    for (unsigned int c = 0; c < numCircleSamples; ++c)
    {
        // Compute center point on torus circle at specified angle.
        float circleFraction = static_cast<float>(c)*invCS;  // in [0,1)
        float theta = circleFraction*(float)2.0f * M_PI;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        EVector3 radial{ cosTheta, sinTheta, 0.0f };
        EVector3 torusMiddle = outerRadius*radial;

        // Compute slice vertices with duplication at endpoint.
        for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
        {
            float radialFraction = static_cast<float>(r)*invRS;  // in [0,1)
            float phi = radialFraction*(float)2.0f * M_PI;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);
            nor = cosPhi*radial + sinPhi*EVector3::Unit(2);
            pos = torusMiddle + innerRadius*nor;
            if (!mOutside)
            {
                nor = -nor;
            }

            basis[0] = nor;
            ComputeOrthogonalComplement(1, basis);
            tcd = { radialFraction, circleFraction };

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, basis[1]);
            SetBinormal(i, basis[2]);
            SetTCoord(i, tcd);
        }
    }

    // Duplicate the cylinder ends to form a torus.
    for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
    {
        Position(i) = Position(r);
        if (mNormals)
        {
            Normal(i) = Normal(r);
        }
        if (mTangents)
        {
            Tangent(i) = Tangent(r);
        }
        if (mBinormals)
        {
            Binormal(i) = Binormal(r);
        }
        for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
        {
            if (mAssignTCoords[unit])
            {
                TCoord(unit, i) = EVector2{ TCoord(unit, r)[0], 1.0f };
            }
        }
    }

    // Generate indices (outside view).
    std::shared_ptr<IndexBuffer> ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return MeshBuffer();
    }
    unsigned int t = 0;
    for (unsigned int c = 0, cStart = 0; c < numCircleSamples; ++c)
    {
        unsigned int i0 = cStart;
        unsigned int i1 = i0 + 1;
        cStart += numRadialSamples + 1;
        unsigned int i2 = cStart;
        unsigned int i3 = i2 + 1;
        for (i = 0; i < numRadialSamples; ++i, ++i0, ++i1, ++i2, ++i3)
        {
            ibuffer->SetTriangle(t++, i0, i2, i1);
            ibuffer->SetTriangle(t++, i1, i2, i3);
        }
    }

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

	return  MeshBuffer(vbuffer, ibuffer);
}


MeshFactory::MeshBuffer gte::MeshFactory::CreateBox(float xExtent, float yExtent, float zExtent)
{
	// Quantities derived from inputs.
	int numVertices = 8;
	int numTriangles = 12;

	// Generate geometry.
	std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numVertices);
	if (!vbuffer)
	{
		return MeshBuffer();
	}

	EVector3 pos, nor, basis[3];
	EVector2 tcd;

	// Choose vertex normals in the diagonal directions.
	EVector3 diag(xExtent, yExtent, zExtent); diag.normalize();
	if (!mOutside){
		diag = -diag;
	}

	for (unsigned int z = 0, v = 0; z < 2; ++z){

		float fz = static_cast<float>(z), omfz = 1.0f - fz;
		float zSign = 2.0f*fz - 1.0f;
		pos[2] = zSign*zExtent;
		nor[2] = zSign*diag[2];
		for (unsigned int y = 0; y < 2; ++y){

			float fy = static_cast<float>(y);
			float ySign = 2.0f*fy - 1.0f;
			pos[1] = ySign*yExtent;
			nor[1] = ySign*diag[1];
			tcd[1] = (1.0f - fy)*omfz + (0.75f - 0.5f*fy)*fz;
			
			for (unsigned int x = 0; x < 2; ++x, ++v){
				float fx = static_cast<float>(x);
				float xSign = 2.0f*fx - 1.0f;
				pos[0] = xSign*xExtent;
				nor[0] = xSign*diag[0];
				tcd[0] = fx*omfz + (0.25f + 0.5f*fx)*fz;

				basis[0] = nor;
				ComputeOrthogonalComplement(1, basis);

				SetPosition(v, pos);
				SetNormal(v, nor);
				SetTangent(v, basis[1]);
				SetBinormal(v, basis[2]);
				SetTCoord(v, tcd);
			}
		}
	}

	// Generate indices (outside view).
	std::shared_ptr<IndexBuffer> ibuffer = CreateIBuffer(numTriangles);
	if (!ibuffer)
	{
		return MeshBuffer();
	}

	ibuffer->SetTriangle(0, 0, 2, 3);
	ibuffer->SetTriangle(1, 0, 3, 1);
	ibuffer->SetTriangle(2, 0, 1, 5);
	ibuffer->SetTriangle(3, 0, 5, 4);
	ibuffer->SetTriangle(4, 0, 4, 6);
	ibuffer->SetTriangle(5, 0, 6, 2);
	ibuffer->SetTriangle(6, 7, 6, 4);
	ibuffer->SetTriangle(7, 7, 4, 5);
	ibuffer->SetTriangle(8, 7, 5, 1);
	ibuffer->SetTriangle(9, 7, 1, 3);
	ibuffer->SetTriangle(10, 7, 3, 2);
	ibuffer->SetTriangle(11, 7, 2, 6);
	if (!mOutside){
		ReverseTriangleOrder(ibuffer.get());
	}

	return MeshBuffer(vbuffer, ibuffer);
}



gte::MeshFactory::MeshBuffer gte::MeshFactory::CreateLine(gte::EVector3 &p1, gte::EVector3 &p2)
{
	std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(3);
	SetPosition(0, p1);
	SetPosition(1, 0.5f * (p1 + p2));
	SetPosition(2, p2);


	std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_POLYSEGMENT_DISJOINT, 2, mIndexSize));
	if (ibuffer) ibuffer->SetUsage(mIBUsage);
	
	ibuffer->SetSegment(0, 0, 1);
	ibuffer->SetSegment(1, 1, 2);
	
	return MeshBuffer(vbuffer, ibuffer);
}

MeshFactory::MeshBuffer MeshFactory::CreateCircle(const gte::EVector3 &normal, float radius, unsigned int numCircleSamples)
{
	float angleStep= 2.0f * M_PI / (float)numCircleSamples;
	Vector3f orthoBasis[2];

	std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numCircleSamples);

	MathUtil::ortho_basis_v3v3_v3(orthoBasis[0], orthoBasis[1], normal);
	orthoBasis[0] *= radius;
	orthoBasis[1] *= radius;
	
	for (size_t i = 0; i < numCircleSamples; ++i){
		float angle = (float)i * angleStep;
		Vector3f p = std::cos(angle) * orthoBasis[0] + std::sin(angle) * orthoBasis[1];
		SetPosition(i, p);
	}

	std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_POLYSEGMENT_DISJOINT, 2 * numCircleSamples, mIndexSize));
	for (size_t i = 0; i < numCircleSamples; ++i){
		ibuffer->SetSegment(i, i, (i + 1) % numCircleSamples);
	}

	return MeshBuffer(vbuffer, ibuffer);
}


std::shared_ptr<VertexBuffer> MeshFactory::CreateVBuffer(
    unsigned int numVertices)
{
    std::shared_ptr<VertexBuffer> vbuffer(
        new VertexBuffer(mVFormat, numVertices));
    if (vbuffer)
    {
        // Get the position channel.
        mPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
        if (!mPositions)
        {
            vbuffer = nullptr;
            return false;
        }

        // Get the optional geometric channels.
        mNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
        mTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
        mBinormals = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

        // Get texture coordinate channels that are to be assigned values.
        // Clear the mAssignTCoords element in case any elements were set by a
        // previous mesh factory creation call.
        std::set<DFType> required;
        required.insert(DF_R32G32_FLOAT);
        for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
        {
            mTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit,
                required);
            if (mTCoords[unit])
            {
                mAssignTCoords[unit] = true;
            }
            else
            {
                mAssignTCoords[unit] = false;
            }
        }

        vbuffer->SetUsage(mVBUsage);
    }
    return vbuffer;
}

std::shared_ptr<IndexBuffer> MeshFactory::CreateIBuffer(
    unsigned int numTriangles)
{
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRIMESH,
        numTriangles, mIndexSize));
    if (ibuffer)
    {
        ibuffer->SetUsage(mIBUsage);
    }
    return ibuffer;
}

char* MeshFactory::GetGeometricChannel(
    std::shared_ptr<VertexBuffer> const& vbuffer, VASemantic semantic,
    float w)
{
    char* channel = nullptr;
    int index = mVFormat.GetIndex(semantic, 0);
    if (index >= 0)
    {
        channel = vbuffer->GetChannel(semantic, 0, std::set<DFType>());
        if (mVFormat.GetType(index) == DF_R32G32B32A32_FLOAT)
        {
            // Fill in the w-components.
            int const numVertices = vbuffer->GetNumElements();
            for (int i = 0; i < numVertices; ++i)
            {
                float* tuple4 = reinterpret_cast<float*>(
                    channel + i*mVFormat.GetVertexSize());
                tuple4[3] = w;
            }
        }
    }
    return channel;
}

void MeshFactory::SetPosition(unsigned int i, EVector3 const& pos)
{
    Position(i) = pos;
}

void MeshFactory::SetNormal(unsigned int i, EVector3 const& nor)
{
    if (mNormals)
    {
        Normal(i) = nor;
    }
}

void MeshFactory::SetTangent(unsigned int i, EVector3 const& tan)
{
    if (mTangents)
    {
        Tangent(i) = tan;
    }
}

void MeshFactory::SetBinormal(unsigned int i, EVector3 const& bin)
{
    if (mBinormals)
    {
        Binormal(i) = bin;
    }
}

void MeshFactory::SetTCoord(unsigned int i, EVector2 const& tcd)
{
    for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
    {
        if (mAssignTCoords[unit])
        {
            TCoord(unit, i) = tcd;
        }
    }
}

void MeshFactory::SetPlatonicTCoord(unsigned int i, EVector3 const& pos)
{
#if 0
	EVector2 tcd;
	if (std::abs(pos[2]) < 1.0f)
	{
		tcd[0] = 0.5f*(1.0f +
			Function<float>::ATan2(pos[1], pos[0])* (float)GTE_C_INV_PI);
	}
	else
	{
		tcd[0] = 0.5f;
	}

	tcd[1] = Function<float>::ACos(pos[2]) * (float)GTE_C_INV_PI;

	for (int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
	{
		if (mAssignTCoords[unit])
		{
			TCoord(unit, i) = tcd;
		}
	}
#endif
}

void MeshFactory::ReverseTriangleOrder(IndexBuffer* ibuffer)
{
    unsigned int const numTriangles = ibuffer->GetNumPrimitives();
    for (unsigned int t = 0; t < numTriangles; ++t)
    { 
        unsigned int v0, v1, v2;
        ibuffer->GetTriangle(t, v0, v1, v2);
        ibuffer->SetTriangle(t, v0, v2, v1);
    }
}

float gte::MeshFactory::ComputeOrthogonalComplement(int numInputs, EVector3* v)
{
	if (numInputs == 1)
	{
		if (std::abs(v[0][0]) > std::abs(v[0][1]))
		{
			v[1] = { -v[0][2], (float)0, +v[0][0] };
		}
		else
		{
			v[1] = { (float)0, +v[0][2], -v[0][1] };
		}
		numInputs = 2;
	}

	if (numInputs == 2)
	{
		v[2] = v[0].cross(v[1]);
		
		float minLength = (v[0]).norm(); v[0].normalize();
		for (int i = 1; i < numInputs; ++i)
		{
			for (int j = 0; j < i; ++j)
			{
				float dot = v[i].dot(v[j]);
				v[i] -= v[j] * dot;
			}
			float length = (v[i]).norm(); v[i].normalize();
			if (length < minLength)
			{
				minLength = length;
			}
		}
		return minLength;
	}

	return (float)0;
}

