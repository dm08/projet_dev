#pragma once
#include "XOrthoLaserDalle.h"
#include <list>


class XError;
class XOrthoLaserPile
{
protected:
	bool DoFusionHZ_Monochrome(std::string OutputFolder,std::ofstream* outMetadata,XError* error);
	bool DoFusionHZ_Color(std::string OutputFolder,std::ofstream* outMetadata,XError* error);
	bool DoFusionHZXYZ_Monochrome(std::string OutputFolder,std::ofstream* outMetadata,XError* error);
	bool DoFusionHZXYZ_Color(std::string OutputFolder,std::ofstream* outMetadata,XError* error);

public:
	XOrthoLaserChantier* parent;
	std::vector<XOrthoLaserDalle*> vecDalle;
	XOrthoLaserDalle* m_masterDalle;// a priori toujours vecDalle[0]  -> a supprimer !!
public:
	XOrthoLaserPile(XOrthoLaserChantier* pparent);
	~XOrthoLaserPile(void);

	uint32 NbDalles(){return vecDalle.size();}
	XFrame Frame(){if(vecDalle.empty())return XFrame(); return vecDalle[0]->Frame();}
	XError* Error();

	bool InitMasterDalle();
	uint32 NbMesures();
	uint32 NbMesuresValides();
	std::vector<XOrthoLaserDalle*> VecDalle(){return vecDalle;}
	XOrthoLaserDalle* Dalle(uint32 i){if (i < vecDalle.size()) return vecDalle[i]; return NULL;}
	XOrthoLaserDalle* MasterDalle(){return m_masterDalle;}
		
	std::list<XOrthoLaserRxp*> listRxp();
	// NO TRAJ std::list<ProjPostProcessSolution*> listTrajecto();

	XPt2D Center();
	XPt2D NW();
	XPt2D SW();
	XPt2D Coord(uint16 col, uint16 lig);
	XPt3D OffsetUsefull();

	std::string Id();
	bool IsIn(XPt2D P);

	//Traitements
	bool ProcessFusion(std::string OutputFolder,bool monochrome,std::ofstream* outMetadata,XError* error);

	bool ExportToPly(double coefZ);
	bool ExportToPlyIndiv(double coefZ);

	std::string PlyFile();
	void InfosConsole();

	bool ExportMesuresDalles(std::ostream* out);

};
