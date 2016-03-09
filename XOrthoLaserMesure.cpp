#include "XOrthoLaserMesure.h"
#include "XOrthoLaserChantier.h"
#include "XOrthoLaserDalle.h"

#include "libXBase\XBase.h"

#include "libXBaseXML\XArchiXMLException.h"
#include "libXBaseXML\XArchiXMLBaseTools.h"
#include "libXBaseXML\XArchiXMLTools.h"

#include "ReaderFilePosPac\PostProcessedProjPosition.h"
#include "ReaderFilePosPac\ProjPostProcessSolution.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//-----------------------------------------------------------------------------
//*************   MESURE
//-----------------------------------------------------------------------------
XOrthoLaserMesure::XOrthoLaserMesure(XOrthoLaserPoint* Pt, XOrthoLaserDalle* img)
{
	m_bValid = false;
	m_pt = Pt;
	m_img= img;
	if(img != NULL)
		m_strImage = img->Name();
}
//-----------------------------------------------------------------------------
XOrthoLaserMesure::~XOrthoLaserMesure(void)
{
}
//-----------------------------------------------------------------------------
XError* XOrthoLaserMesure::Error()
{
	if(m_img == NULL)
		return NULL;
	return m_img->Error();
}
//-----------------------------------------------------------------------------
XOrthoLaserRxp* XOrthoLaserMesure::Rxp()
{
	if(m_img != NULL)
		return m_img->Rxp();
	return NULL;
}
//-----------------------------------------------------------------------------
double XOrthoLaserMesure::AbsoluteTime()//une heure qui permet de faire des comparaisons chronologiques
{
	if(m_img == NULL)
		return m_dTime;
	return m_img->PivotTemporel() + m_dTime;
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserMesure::strIdCouche()
{
	if(m_img == NULL)
		return m_strImage; 
	return Ortho()->strIdCouche(); 
}

//-----------------------------------------------------------------------------
ProjPostProcessSolution* XOrthoLaserMesure::Solution()
{
	if(Rxp() != NULL)
		return Rxp()->Solution();
	return NULL;
}
//-----------------------------------------------------------------------------
void  XOrthoLaserMesure::Ortho(XOrthoLaserDalle* img)
{ 
	m_img = img;
	if(img != NULL)
		m_strImage = img->Name();
}
//-----------------------------------------------------------------------------
void XOrthoLaserMesure::DeconnectOrtho()
{	
	if(m_img != NULL) 
		m_img->RemoveMesure(this);
}
//-----------------------------------------------------------------------------
XOrthoLaserPoint* XOrthoLaserMesure::Pt() 
{
	return m_pt;
}
//-----------------------------------------------------------------------------
void  XOrthoLaserMesure::Pt(XOrthoLaserPoint* pt)
{ 
	m_pt = pt;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::SetMesure(double x, double y) 
{
	m_bValid = false;
	X = x; 
	Y = y;
	if(SetValuesFromOrtholas())
		m_bValid = true;
	return m_bValid;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::SetValuesFromOrtholas() 
{
	if(m_img == NULL)
		return XErrorAlert(Error(),__FUNCTION__,"image non connectee ",m_strImage.c_str() );; //??
	
	if(!m_img->GetCoordTerrain(X,Y,&m_terrainLaser))
		return XErrorError(Error(),__FUNCTION__,"erreur de GetCoordTerrain ", m_img->Name().c_str());

	m_dTime= m_img->GetH(X,Y);
	if(m_dTime == 0)
		m_dTime= m_img->GetNearestH(X,Y);//on veut récupérer une heure pas trop loin
	if(m_dTime == 0)
		return XErrorError(Error(),__FUNCTION__,"Impossible de recuperer l'heure du point ", m_img->Name().c_str());

	return true;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserMesure::NumPoint()
{
	if(m_pt == NULL)
		return 0; 
	return m_pt->Num();
}
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserMesure::Translation()
{
	if(m_pt == NULL)
		return m_terrainLaser;
	return m_terrainLaser-*m_pt;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::SetPointNumFromTime()
{
	if(m_pt == NULL)
		return false; 

	uint32 num = floor(m_dTime*1000);
	m_pt->Num(num);
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::SetPointPosition()
{
	if(m_pt == NULL)
		return false; 
	if(m_img == NULL)
		return false; 
	
	return m_img->GetCoordTerrain(X,Y,m_pt);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::UpdateCoordFromPoint()
{
	if(m_pt == NULL) 
		return false;
	if(m_img == NULL) 
		return false;

	//une mesure générée à partir des coordonnées du point est forcement invalide
	m_bValid =false; 
	
	return m_img->GetMesure(*m_pt, this);
}
//-----------------------------------------------------------------------------
void XOrthoLaserMesure::DeconnectPoint()
{	
	if(m_pt != NULL) 
		m_pt->RemoveMesure(this);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::ConnectSousEch()
{
	if(m_pt == NULL) 
		return XErrorError(Error(),__FUNCTION__,"m_pt == NULL");
	if(m_img == NULL) 
		return XErrorError(Error(),__FUNCTION__,"m_img == NULL");
	XOrthoLaserRxp*  rxp = m_img->Rxp();
	if(rxp == NULL) 
		return XErrorError(Error(),__FUNCTION__,"rxp == NULL");
	ProjPostProcessSolution * sol = rxp->Solution();
	if(sol == NULL) 
		return XErrorError(Error(),__FUNCTION__,"sol == NULL");
	XOrthoLaserChantier* chantier  = m_img->Chantier();
	if(chantier == NULL) 
		return XErrorError(Error(),__FUNCTION__,"chantier == NULL");
	float TimeInDalle = this->Time();
	float GpsTimeInDay = TimeInDalle + chantier->CorOrthoLaserTime2Gps(rxp->Date());

	int rg;
	PostProcessedProjPosition* PosSsEch = sol->SsEchPosition(GpsTimeInDay,&rg);
	if(PosSsEch == NULL) 
	{	Valid(false);
		return XErrorError(Error(),__FUNCTION__,"PosSsEch == NULL ",InfoTxt().c_str());
	}

	PosSsEch->contrainte = (m_pt)->CreateContrainte(this); 
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::DeconnectSousEch()
{
	if(!m_bValid)
		return true;
	if(m_pt == NULL) 
		return XErrorError(Error(),__FUNCTION__,"m_pt == NULL");
	if(m_img == NULL)
		return XErrorError(Error(),__FUNCTION__,"m_img == NULL");
	if(m_img->Rxp() == NULL) 
		return XErrorError(Error(),__FUNCTION__,"m_img->Rxp() == NULL");
	if(m_img->Rxp()->Solution() == NULL) 
		return XErrorError(Error(),__FUNCTION__,"m_img->Rxp()->Solution() == NULL");
	if(m_img->Chantier() == NULL)
		return XErrorError(Error(),__FUNCTION__,"m_img->Chantier() == NULL");
	float TimeInDalle = this->Time();
	float GpsTimeInDay = TimeInDalle + m_img->Chantier()->CorOrthoLaserTime2Gps(m_img->Rxp()->Date());

	char message[1024];
	int rg;
	PostProcessedProjPosition* PosSsEch = m_img->Rxp()->Solution()->SsEchPosition(GpsTimeInDay,&rg);
	if(PosSsEch == NULL) 
	{
		sprintf(message,"Contrainte non trouvee T=%.2lf !!! ",GpsTimeInDay);
		return XErrorError(Error(),__FUNCTION__,message,m_img->Rxp()->Solution()->MissionName().c_str());
	}


	if(PosSsEch->contrainte != NULL)
		delete PosSsEch->contrainte;
	PosSsEch->contrainte = NULL; 
	sprintf(message,"Sup. contrainte %s ",m_img->Name());
	return XErrorInfo(Error(),__FUNCTION__,message,m_img->Rxp()->Solution()->MissionName().c_str());
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::EcritureVecteurViewer(std::ostream* out)//fichier vectoriel pour le Viewer
{
	if(m_pt == NULL)
		return out->good();

	if(!m_bValid)
		return out->good();

	m_pt->EcritureVecteurViewer(out);//info sur le point
	*out << X << "	" << Y << std::endl;///coordonnées de la mesure
	
	return out->good();
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserMesure::InfoTxt()
{
	std::ostringstream oss;
	oss << "Mesure du point ";
	if(m_pt == NULL)oss << "NULL";	else oss << m_pt->Num();
	oss << " sur dalle " << m_strImage ;
	if(m_img == NULL) oss << " connectee ";	else oss <<  " NON connectee ";
	oss << " time " << Time() ;
	return oss.str();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::WriteMesure(std::ostream* out)
{
	if(!m_bValid)
		return out->good();
	if(m_pt == NULL)
		return out->good();
	*out << m_pt->Num() << '\t'<< X << '\t' << Y << std::endl;//coordonnées de la mesure
	return out->good();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserMesure::XmlWrite(std::ostream* out)
{
	if(!m_bValid)
		return out->good();
	*out << "<OrthoLaserMesure>" << std::endl;
	*out << "<dalle> " << m_strImage << " </dalle>" << std::endl;
	*out << "<time> " << m_dTime << " </time>" << std::endl;
	*out << "<X> " << X << " </X>" << std::endl;
	*out << "<Y> " << Y << " </Y>" << std::endl;
	m_terrainLaser.XmlWrite(out);
	*out << "</OrthoLaserMesure>" << std::endl;

	return out->good();
}

