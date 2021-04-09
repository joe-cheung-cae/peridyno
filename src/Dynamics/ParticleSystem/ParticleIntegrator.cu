#include <cuda_runtime.h>
#include "ParticleIntegrator.h"
#include "Framework/Node.h"
#include "Framework/SceneGraph.h"

namespace dyno
{
	IMPLEMENT_CLASS_1(ParticleIntegrator, TDataType)

	template<typename TDataType>
	ParticleIntegrator<TDataType>::ParticleIntegrator()
		: NumericalIntegrator()
	{
		this->inAttribute()->tagOptional(true);
	}

	template<typename TDataType>
	void ParticleIntegrator<TDataType>::begin()
	{
		if (!this->inPosition()->isEmpty())
		{
			int num = this->inPosition()->getElementCount();
			
			m_prePosition.resize(num);
			m_preVelocity.resize(num);

			m_prePosition.assign(this->inPosition()->getData());
			m_preVelocity.assign(this->inVelocity()->getData());

			this->inForceDensity()->getDataPtr()->reset();
		}
	}

	template<typename TDataType>
	void ParticleIntegrator<TDataType>::end()
	{

	}

	template<typename TDataType>
	bool ParticleIntegrator<TDataType>::initializeImpl()
	{
// 		if (!isAllFieldsReady())
// 		{
// 			std::cout << "Exception: " << std::string("DensitySummation's fields are not fully initialized!") << "\n";
// 			return false;
// 		}
// 
// 		int num = this->inPosition()->getElementCount();
// 
// 		m_prePosition.resize(num);
// 		m_preVelocity.resize(num);

		return true;
	}

	template<typename Real, typename Coord>
	__global__ void K_UpdateVelocity(
		DArray<Coord> vel,
		DArray<Coord> forceDensity,
		Coord gravity,
		Real dt)
	{
		int pId = threadIdx.x + (blockIdx.x * blockDim.x);
		if (pId >= forceDensity.size()) return;

		vel[pId] += dt * (forceDensity[pId] + gravity);
	}


	template<typename Real, typename Coord>
	__global__ void K_UpdateVelocity(
		DArray<Coord> vel,
		DArray<Coord> forceDensity,
		DArray<Attribute> atts,
		Coord gravity,
		Real dt)
	{
		int pId = threadIdx.x + (blockIdx.x * blockDim.x);
		if (pId >= forceDensity.size()) return;

		Attribute att = atts[pId];

		if (att.IsDynamic())
		{
			vel[pId] += dt * (forceDensity[pId] + gravity);
		}
	}

	template<typename TDataType>
	bool ParticleIntegrator<TDataType>::updateVelocity()
	{
		Real dt = Real(0.001);
		if(this->getParent() != NULL)
			dt = getParent()->getDt();
		Coord gravity = SceneGraph::getInstance().getGravity();

		int total_num = this->inPosition()->getElementCount();

		if (this->inAttribute()->isEmpty())
		{
			cuExecute(total_num,
				K_UpdateVelocity,
				this->inVelocity()->getData(),
				this->inForceDensity()->getData(),
				gravity,
				dt);
		}
		else
		{
			cuExecute(total_num,
				K_UpdateVelocity,
				this->inVelocity()->getData(),
				this->inForceDensity()->getData(),
				this->inAttribute()->getData(),
				gravity,
				dt);
		}
		

		return true;
	}

	template<typename Real, typename Coord>
	__global__ void K_UpdatePosition(
		DArray<Coord> pos,
		DArray<Coord> vel,
		Real dt)
	{
		int pId = threadIdx.x + (blockIdx.x * blockDim.x);
		if (pId >= pos.size()) return;

		pos[pId] += dt * vel[pId];
	}

	template<typename Real, typename Coord>
	__global__ void K_UpdatePosition(
		DArray<Coord> pos,
		DArray<Coord> vel,
		DArray<Attribute> att,
		Real dt)
	{
		int pId = threadIdx.x + (blockIdx.x * blockDim.x);
		if (pId >= pos.size()) return;

		Attribute att_i = att[pId];

		if (!att_i.IsFixed())
		{
			pos[pId] += dt * vel[pId];
		}
	}

	template<typename TDataType>
	bool ParticleIntegrator<TDataType>::updatePosition()
	{
		Real dt = 0.001;
		if (this->getParent() != NULL)
			dt = getParent()->getDt();

		int total_num = this->inPosition()->getDataPtr()->size();
		if (this->inAttribute()->isEmpty())
		{
			cuExecute(total_num,
				K_UpdatePosition,
				this->inPosition()->getData(),
				this->inVelocity()->getData(),
				dt);
		}
		else
		{
			cuExecute(total_num,
				K_UpdatePosition,
				this->inPosition()->getData(),
				this->inVelocity()->getData(),
				this->inAttribute()->getData(),
				dt);
		}


		return true;
	}

	template<typename TDataType>
	bool ParticleIntegrator<TDataType>::integrate()
	{
		if (!this->inPosition()->isEmpty())
		{
			updateVelocity();
			updatePosition();
		}

		return true;
	}

	DEFINE_CLASS(ParticleIntegrator);
}