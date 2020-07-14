#pragma once

//---------------------------------------
// Necessary for every movable object
//---------------------------------------
template<typename T>
class Transformable
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	T meshTrans;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual const T GetTransform() = 0;
	virtual void SetTransform(T trans) = 0;
};
