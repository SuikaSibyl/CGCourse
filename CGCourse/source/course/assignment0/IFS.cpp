#include "IFS.h"

IFS::IFS(int niter, int npoint, int nsize, string sname, ProbabilityPair<Matrix>* ppair) :
	nIter(niter), nPoint(npoint), nSize(nsize), sName(sname), Matrixs(ppair)
{
	// Init image with white background
	pImg = std::make_unique<Image>(nSize, nSize);
	pImg->SetAllPixels(Vec3f(1, 1, 1));
	// Execute IFS process
	Execute();
	// Output the image to local fie
	ImWrite();
}

IFS::~IFS()
{
	pImg = nullptr;
	delete(Matrixs);
}

void IFS::Execute()
{
	RandomModule rand;
	const Vec3f black(0, 0, 0);
	for (int i = 0; i < nPoint; i++)
	{
		Vec3f pos = rand.RandVec2f0101();
		for (int j = 0; j < nIter; j++)
		{
			Matrix& m = Matrixs->RandTarget();
			m.Transform(pos);
		}
		pImg->SetPixel(int(pos.x() * nSize), int(pos.y() * nSize), black);
	}
}

void IFS::ImWrite()
{
	pImg->SaveTGA((string("resource/output/") + sName).c_str());
}