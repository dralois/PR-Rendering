#pragma once

//---------------------------------------
// Necessary for every movable object
//---------------------------------------
template<typename Trans, typename Vec, typename Quat>
class Transformable
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Trans meshTrans;
	Vec meshPos;
	Quat meshRot;
	Vec meshScale;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	virtual const Trans GetTransform() = 0;
	virtual void SetTransform(Trans trans) = 0;
	virtual const Vec GetPosition() = 0;
	virtual void SetPosition(Vec pos) = 0;
	virtual const Quat GetRotation() = 0;
	virtual void SetRotation(Quat rot) = 0;
	virtual const Vec GetScale() = 0;
	virtual void SetScale(Vec scale) = 0;
};
