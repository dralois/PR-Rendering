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
	Vec meshScl;

	Transformable(
		Trans initTrans,
		Vec initPos,
		Quat initRot,
		Vec initScale
	) :
		meshTrans(initTrans),
		meshPos(initPos),
		meshRot(initRot),
		meshScl(initScale)
	{
	}

	Transformable(const Transformable& copy):
		meshTrans(copy.meshTrans),
		meshPos(copy.meshPos),
		meshRot(copy.meshRot),
		meshScl(copy.meshScl)
	{
	}

	Transformable(Transformable&& other)
	{
		std::swap(meshTrans, other.meshTrans);
		std::swap(meshPos, other.meshPos);
		std::swap(meshRot, other.meshRot);
		std::swap(meshScl, other.meshScl);
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
