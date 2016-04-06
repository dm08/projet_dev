#include "XOrthoLaserRxp.h"
#include "XOrthoLaserChantier.h"

#include <limits>

//-----------------------------------------------------------------------------
XOrthoLaserRxp::XOrthoLaserRxp(XOrthoLaserChantier* parent,std::string name, uint32 date)
{
	m_parent = parent;
	m_rxpName = name;
	m_date = date;
	m_hmin = std::numeric_limits<float>::max();
	m_hmax = 0.;
	// NO TRAJ m_solution = NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserRxp::~XOrthoLaserRxp(void)
{
}
//-----------------------------------------------------------------------------
// NO TRAJ void XOrthoLaserRxp::Solution(ProjPostProcessSolution* sol)
//{
//	m_solution= sol;
//}
//-----------------------------------------------------------------------------
// NO TRAJ ProjPostProcessSolution*  XOrthoLaserRxp::Solution()
//{
//	return m_solution;
//}
//-----------------------------------------------------------------------------
void XOrthoLaserRxp::AddTime(float time)
{
	m_hmin = XMin(m_hmin, time);
	m_hmax = XMax(m_hmax, time);
}
//-----------------------------------------------------------------------------
float XOrthoLaserRxp::GpsStartTimeInDay()
{
	return m_hmin + m_parent->CorOrthoLaserTime2Gps(Date());
}
//-----------------------------------------------------------------------------
float XOrthoLaserRxp::GpsEndTimeInDay()
{
	return m_hmax + m_parent->CorOrthoLaserTime2Gps(Date());
}

