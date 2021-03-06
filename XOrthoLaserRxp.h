#pragma once
#include <string>
#include "XBase.h"

// NO TRAJ class ProjPostProcessSolution;
class XOrthoLaserChantier;
class XOrthoLaserRxp
{
protected:
	XOrthoLaserChantier* m_parent;
	std::string m_rxpName;
	uint32 m_date;
	float m_hmin;//valeurs issues des dalles 
	float m_hmax;
	// NO TRAJ ProjPostProcessSolution* m_solution;
public:
	XOrthoLaserRxp(XOrthoLaserChantier* parent,std::string name, uint32 date);
	~XOrthoLaserRxp(void);
	
	std::string Name(){ return m_rxpName;}
	//float Hmin(){return m_hmin;}
	//float Hmax(){return m_hmax;}
	uint32 Date(){return m_date;}
	float GpsStartTimeInDay();
	float GpsEndTimeInDay();


	// NO TRAJ void Solution(ProjPostProcessSolution* sol);
	// NO TRAJ ProjPostProcessSolution*  Solution();

	uint32 Length(){return 0;}//longueur developpée en m
	void AddTime(float time);

};
