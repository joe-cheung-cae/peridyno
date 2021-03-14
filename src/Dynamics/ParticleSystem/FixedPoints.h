#pragma once
#include "Framework/ModuleConstraint.h"

namespace dyno {

	template<typename TDataType>
	class FixedPoints : public ConstraintModule
	{
		DECLARE_CLASS_1(FixedPoints, TDataType)
	public:
		typedef typename TDataType::Real Real;
		typedef typename TDataType::Coord Coord;

		FixedPoints();
		~FixedPoints() override;

		void addFixedPoint(int id, Coord pt);
		void removeFixedPoint(int id);

		void clear();

		bool constrain() override;

		void constrainPositionToPlane(Coord pos, Coord dir);

	public:
		/**
		* @brief Particle position
		*/
		DeviceArrayField<Coord> m_position;

		/**
		* @brief Particle velocity
		*/
		DeviceArrayField<Coord> m_velocity;

	protected:
		virtual bool initializeImpl() override;

		FieldID m_initPosID;

	private:
		void updateContext();

		bool bUpdateRequired = false;

		std::map<int, Coord> m_fixedPts;

		std::vector<int> m_bFixed_host;
		std::vector<Coord> m_fixed_positions_host;

		GArray<int> m_bFixed;
		GArray<Coord> m_fixed_positions;
	};

#ifdef PRECISION_FLOAT
template class FixedPoints<DataType3f>;
#else
template class FixedPoints<DataType3d>;
#endif

}