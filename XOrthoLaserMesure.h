#pragma once
#include "libXBase/XPt3D.h"
#include "libXBase/XBase.h"
//#include "ProjPostProcessSolution.h"

#include "XOrthoLaserPoint.h"

#include <list>

class XOrthoLaserDalle;
class XError;
class XOrthoLaserRxp;
class ProjPostProcessSolution;

class XOrthoLaserMesure : public XPt2D 
{
protected:
	bool			m_bValid;		// Indique si la mesure est valide (mesurée)
	float			m_dTime;		// Heure du point récupérée dans la dalle
	XPt3D			m_terrainLaser;  //coordonnées terrain issues des ortho laser

	XOrthoLaserPoint* m_pt;
	XOrthoLaserDalle* m_img;
	std::string		m_strImage;	// nom de image sur laquellle a ete faite la mesure
public:
	XOrthoLaserMesure(XOrthoLaserPoint* Pt, XOrthoLaserDalle* img);
	~XOrthoLaserMesure();

	XError* Error();

	virtual bool XmlWrite(std::ostream* out);

	bool SetMesure(double x, double y);
	bool SetValuesFromOrtholas();

	void PositionLaser(XPt3D	p){m_terrainLaser = p;  }//coordonnées terrain issues des ottho laser
	XPt3D PositionLaser(){return m_terrainLaser;}

	inline bool Valid() const { return m_bValid;}
	void Valid(bool flag) { m_bValid = flag;}

	float Time(){return m_dTime;}// Heure Gps du point
	void Time(float val){m_dTime = val;}	
	double AbsoluteTime();//une heure qui permet de faire des comparaisons chronologiques

	XOrthoLaserDalle*  Ortho(){	return m_img;}
	XOrthoLaserRxp*  Rxp();
	ProjPostProcessSolution* Solution();

	std::string ImageName(){return m_strImage;}
	void ImageName(std::string str){m_strImage = str;}
	std::string strIdCouche();

	void  Ortho(XOrthoLaserDalle* img);
	void DeconnectOrtho();

	XOrthoLaserPoint* Pt() ;
	void  Pt(XOrthoLaserPoint* pt);
	uint32 NumPoint();
	bool SetPointNumFromTime();

	XPt3D Translation();

	bool SetPointPosition();
	bool UpdateCoordFromPoint();

	void DeconnectPoint();

	bool ConnectSousEch();
	bool DeconnectSousEch();

	bool EcritureVecteurViewer(std::ostream* out);
	std::string InfoTxt();
	bool WriteMesure(std::ostream* out);


};