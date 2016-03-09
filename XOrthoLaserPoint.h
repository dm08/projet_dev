#pragma once
#include "libXBase/XPt3D.h"
#include "libXBase/XBase.h"
#include <list>

class XOrthoLaserMesure;
class XOrthoLaserDalle;
class TiXmlNode;
class XOrthoLaserChantier;
class XError;
class ProjPostProcessSolution;
class XContrainte;
class XOrthoLaserPoint : public XPt3D //ici soit les coordonnées fixées soir une copie des coordonnées issues de la mesure maitre
{
protected:
	uint32 m_nNum; 
	TypeAppui m_Type;			// Type de point
	bool m_bControl;

	std::vector<XOrthoLaserMesure*>	m_Mesure;// Mesures effectuees sur les orthos
	XOrthoLaserMesure* m_masterMesure;
	XPt3D* m_CompPosition;//résultat de compensation volatile
	float3 *m_ecartType;
	bool m_bIsMovable;
public:
	XOrthoLaserPoint(double x, double y, double z,TypeAppui typ, bool movable) : XPt3D(x,y,z)
	{ 
		m_Type = typ; 
		m_nNum = 0;
		m_masterMesure= NULL;
		m_bControl = false;
		m_CompPosition = NULL;
		m_bIsMovable = movable;
		m_ecartType = NULL;
	}
	~XOrthoLaserPoint();

	XError* Error();

	bool IsMovable(){return m_bIsMovable;}
	void Lock(){m_bIsMovable = false;}
	void UnLock(){m_bIsMovable = true; RegeneratePositionFromMaster();}

	static float3 defaultEcartTypePoint;
	float3 EcartType(){if(m_ecartType!=NULL)return *m_ecartType;return defaultEcartTypePoint;}

	std::string StrMovable(){if(m_bIsMovable)return std::string("Libre"); return std::string("FIXE");}
	virtual std::string StrLevel(){if((m_Type == PLiaison)||(m_Type == PSift))return std::string("Relatif");return std::string("Absolu");}

	//compensation
	XPt3D* CompPosition() {return m_CompPosition;}
	void SetCompPosition(XPt3D p);
	float ResX();
	float ResY();
	float ResZ();
	float ResXY();
	float ResXYZ();
	float ResXYMesMax();
	float ResZMesMax();
	std::vector<float3> ResMesures();


	XOrthoLaserMesure* CreateMesure(XOrthoLaserDalle* dalle);
	XOrthoLaserMesure* FindMesure(XOrthoLaserDalle* dalle);
	XOrthoLaserMesure* FindMesureOnSolution(ProjPostProcessSolution* sol);
	 
	std::vector<XOrthoLaserMesure*> MesuresValid();
	uint32 NbMesuresValid(){return MesuresValid().size();}

	void AddMesure(XOrthoLaserMesure* mes);
	bool RemoveMesure(XOrthoLaserMesure* mes);

	uint32 NbMesures(){return m_Mesure.size();}
	std::vector<XOrthoLaserMesure*>*	Mesures() {return &m_Mesure;}

	virtual bool SetMasterMesure();
	XOrthoLaserMesure* MasterMesure(){return m_masterMesure;}
	std::string MasterName();
	XPt3D MasterPosition();//correspond a la position données par la dalle maitre
	bool RegeneratePositionFromMaster();

	bool ConnectSousEch(bool computeSegment);
	void DeconnectSousEch(bool computeSegment);

	TypeAppui Type(){return m_Type;}
	void Type(TypeAppui typ){m_Type = typ;}
	bool Control() { return m_bControl;}
	void Control(bool flag) { m_bControl = flag;}

	uint32 Num() {return m_nNum;} 
	void Num(uint32 val) {m_nNum = val;} 
	std::string Id();

	bool EcritureVecteurViewer(std::ostream* out);
	bool XmlWrite(std::ostream* out);
	bool LoadOrthoLaserMesureFromNode( TiXmlNode* node);
	bool ConnectOrthoLas(XOrthoLaserChantier* chantier);//après chargement xml
	virtual bool InfosMesures();
	virtual bool InfosCompensation();

	std::list<ProjPostProcessSolution*> listTrajecto();

	XContrainte* CreateContrainte(XOrthoLaserMesure* mes);
	std::vector<XContrainte*> CreateContraintes(ProjPostProcessSolution* sol);

};
