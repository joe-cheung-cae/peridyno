#include "Vessel.h"

#include "Quat.h"

#include <GLSurfaceVisualModule.h>
#include "GLPhotorealisticInstanceRender.h"

namespace dyno
{
	template<typename TDataType>
	Vessel<TDataType>::Vessel()
		: RigidBody<TDataType>()
	{
		this->stateEnvelope()->setDataPtr(std::make_shared<TriangleSet<TDataType>>());
		this->stateMesh()->setDataPtr(std::make_shared<TriangleSet<TDataType>>());

		this->varDensity()->setRange(1.0f, 10000.0f);

		auto callback = std::make_shared<FCallBackFunc>(std::bind(&Vessel<TDataType>::transform, this));
		this->varLocation()->attach(callback);
		this->varScale()->attach(callback);
		this->varRotation()->attach(callback);


		auto EnvelopeRender = std::make_shared<GLSurfaceVisualModule>();
		EnvelopeRender->setColor(Color(0.8f, 0.8f, 0.8f));
		this->stateEnvelope()->promoteOuput()->connect(EnvelopeRender->inTriangleSet());
		this->graphicsPipeline()->pushModule(EnvelopeRender);
		EnvelopeRender->setVisible(false);


		auto texMeshRender = std::make_shared<GLPhotorealisticInstanceRender>();
		this->inTextureMesh()->connect(texMeshRender->inTextureMesh());
		this->stateInstanceTransform()->connect(texMeshRender->inTransform());
		this->graphicsPipeline()->pushModule(texMeshRender);


		auto evenlopeLoader = std::make_shared<FCallBackFunc>(
			[=]() {
				std::string name = this->varEnvelopeName()->getValue().string();
				bool succeed = mInitialEnvelope.loadObjFile(name);

				if (succeed)
				{
					auto envelope = this->stateEnvelope()->getDataPtr();

					envelope->copyFrom(mInitialEnvelope);

					envelope->scale(this->varScale()->getValue());
					envelope->rotate(this->varRotation()->getValue() * M_PI / 180);
					envelope->translate(this->varLocation()->getValue());
				}
			}
		);
		evenlopeLoader->update();

		this->varEnvelopeName()->attach(evenlopeLoader);
	}

	template<typename TDataType>
	Vessel<TDataType>::~Vessel()
	{

	}

	template<typename TDataType>
	NBoundingBox Vessel<TDataType>::boundingBox()
	{
		NBoundingBox box;

		this->stateMesh()->constDataPtr()->requestBoundingBox(box.lower, box.upper);

		return box;
	}

	template<typename TDataType>
	void Vessel<TDataType>::resetStates()
	{

		this->transform();

		auto texMesh = this->inTextureMesh()->constDataPtr();

		//Initialize states for the rigid body
		{
			auto bodyIndex = this->varBodyId()->getValue();

			Coord lo = texMesh->shapes()[bodyIndex]->boundingBox.v0;
			Coord hi = texMesh->shapes()[bodyIndex]->boundingBox.v1;

			Coord scale = this->varScale()->getValue();

			mShapeCenter = texMesh->shapes()[bodyIndex]->boundingTransform.translation() * scale;


			Real lx = hi.x - lo.x;
			Real ly = hi.y - lo.y;
			Real lz = hi.z - lo.z;

			Real rho = this->varDensity()->getData();
			Real mass = rho * lx * ly * lz;

			//Calculate mass using the bounding box
			Matrix inertia = 1.0f / 12.0f * mass
				* Mat3f(ly * ly + lz * lz, 0, 0,
					0, lx * lx + lz * lz, 0,
					0, 0, lx * lx + ly * ly);


			Coord location = this->varLocation()->getValue();
			Coord rot = this->varRotation()->getValue();

			auto quat = this->computeQuaternion();
			auto offset = this->varBarycenterOffset()->getValue();

			this->stateMass()->setValue(mass);
			this->stateCenter()->setValue(location + mShapeCenter);
			this->stateBarycenter()->setValue(location + mShapeCenter + quat.rotate(offset));
			this->stateVelocity()->setValue(Vec3f(0));
			this->stateAngularVelocity()->setValue(Vec3f(0));
			this->stateInertia()->setValue(inertia);
			this->stateQuaternion()->setValue(quat);
			this->stateInitialInertia()->setValue(inertia);
		}




		RigidBody<TDataType>::resetStates();
	}

	template<typename TDataType>
	void Vessel<TDataType>::updateStates()
	{
		RigidBody<TDataType>::updateStates();

		auto center = this->stateCenter()->getValue();
		auto quat = this->stateQuaternion()->getValue();
		auto scale = this->varScale()->getValue();

		auto offset = this->varBarycenterOffset()->getValue();

		this->stateBarycenter()->setValue(center + quat.rotate(offset));

		auto buoy = this->stateEnvelope()->getDataPtr();
		buoy->copyFrom(mInitialEnvelope);
		buoy->rotate(quat);
		buoy->scale(scale);
		buoy->translate(center - mShapeCenter);

		auto texMesh = this->inTextureMesh()->getDataPtr();
		{

			uint N = texMesh->shapes().size();

			CArrayList<Transform3f> tms;
			tms.assign(this->stateInstanceTransform()->constData());

			for (uint i = 0; i < tms.size(); i++)
			{
				auto& list = tms[i];
				for (uint j = 0; j < list.size(); j++)
				{
					list[j].translation() = center + quat.rotate(texMesh->shapes()[i]->boundingTransform.translation() * scale) - mShapeCenter; //
					list[j].rotation() = quat.toMatrix3x3();
					list[j].scale() = scale;
				}

			}

			auto instantanceTransform = this->stateInstanceTransform()->getDataPtr();
			instantanceTransform->assign(tms);

			tms.clear();

		}
	}

	template<typename TDataType>
	void Vessel<TDataType>::transform()
	{

		Coord location = this->varLocation()->getValue();
		Coord rot = this->varRotation()->getValue();
		Coord scale = this->varScale()->getValue();

		auto quat = this->computeQuaternion();

		auto envelope = this->stateEnvelope()->getDataPtr();
		envelope->copyFrom(mInitialEnvelope);
		envelope->scale(scale);
		envelope->rotate(quat);
		envelope->translate(location);

		if (this->inTextureMesh()->isEmpty())
			return;

		auto texMesh = this->inTextureMesh()->constDataPtr();
		{
			uint N = texMesh->shapes().size();

			CArrayList<Transform3f> tms;
			tms.resize(N, 1);

			for (uint i = 0; i < N; i++)
			{
				Transform3f t = texMesh->shapes()[i]->boundingTransform;

				tms[i].insert(Transform3f(t.translation() * scale + location, quat.toMatrix3x3(), t.scale() * scale));


			}

			if (this->stateInstanceTransform()->isEmpty())
			{
				this->stateInstanceTransform()->allocate();
			}

			auto instantanceTransform = this->stateInstanceTransform()->getDataPtr();
			instantanceTransform->assign(tms);

			tms.clear();

		}
	}

	DEFINE_CLASS(Vessel);
}