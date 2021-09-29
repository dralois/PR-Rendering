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

	Trans objTrans;
	Vec objPos;
	Quat objRot;
	Vec objScl;

	Transformable(
		Trans initTrans,
		Vec initPos,
		Quat initRot,
		Vec initScale
	) :
		objTrans(initTrans),
		objPos(initPos),
		objRot(initRot),
		objScl(initScale)
	{
	}

	Transformable(const Transformable& copy):
		objTrans(copy.objTrans),
		objPos(copy.objPos),
		objRot(copy.objRot),
		objScl(copy.objScl)
	{
	}

	Transformable(Transformable&& other)
	{
		std::swap(objTrans, other.objTrans);
		std::swap(objPos, other.objPos);
		std::swap(objRot, other.objRot);
		std::swap(objScl, other.objScl);
	}

	virtual ~Transformable()
	{
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	virtual const Trans GetTransform() const = 0;
	virtual void SetTransform(Trans trans) = 0;
	virtual const Vec GetPosition() const = 0;
	virtual void SetPosition(Vec pos) = 0;
	virtual const Quat GetRotation() const = 0;
	virtual void SetRotation(Quat rot) = 0;
	virtual const Vec GetScale() const = 0;
	virtual void SetScale(Vec scale) = 0;
};
