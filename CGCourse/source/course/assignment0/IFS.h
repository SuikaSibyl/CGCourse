#pragma once

#include <string>
#include <image.h>
#include <memory>
#include <vector>
#include <matrix.h>
#include <Random/RandomModule.h>

using std::string;

template<class T>
class ProbabilityPair
{
public:
	void PushPair(float p, T t);
	T& RandTarget();

private:
	vector<float>	probabilities;
	vector<T>		Targets;
	RandomModule	rand;
};

template<class T>
void ProbabilityPair<T>::PushPair(float p, T t)
{
	float lastp = probabilities.empty() ? 0 : probabilities.back();
	probabilities.push_back(p + lastp);
	Targets.push_back(t);
}

template<class T>
T& ProbabilityPair<T>::RandTarget()
{
	float p = rand.RandFloat01();
	int index = 0;
	for (auto iter = probabilities.begin(); iter != probabilities.end(); iter++)
	{
		if (p <= *iter)
			break;
		index++;
	}
	return Targets[index];
}

class IFS
{
public:
	IFS(int niter, int npoint, int nsize, string sname, ProbabilityPair<Matrix>* ppair);
	~IFS();


protected:
	std::unique_ptr<Image> pImg = nullptr;
	int nIter;
	int nPoint;
	int nSize;
	string sName;

private:
	void Execute();
	void ImWrite();
	ProbabilityPair<Matrix>* Matrixs;
};