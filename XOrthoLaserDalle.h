#pragma once
#include "XBase.h"
#include "XFrame.h"
#include "XImageTiffLoader.h"
#include "XRawImage.h"

#include "XOrthoLaserMesure.h"
#include "XOrthoLaserRxp.h"

class XOrthoLaserChantier;
class XOrthoLaserMesure;
class XOrthoLaserDalle;
class XOrthoLaserFolder;
//------------------------------------------------------------------
class XOrthoLaserGrille{
protected :
	char m_value;
	XOrthoLaserDalle* parent;
	float* m_Image;
	float minValue;
	float maxValue;
	float meanValue;

public :
	XOrthoLaserGrille(XOrthoLaserDalle* pparent, char value) ;
	~XOrthoLaserGrille();

	bool IsLoaded(){return (m_Image != NULL);}
	bool Load();
	void UnLoad();
	float GetValue(double x, double y);
	float GetNearestValue(double x, double y);
	float GetMeanValue();
	float GetMinValue();
	float GetMaxValue();
	float* Pixels(){return m_Image;}
	uint32 NbPixWithValue();//nombre de pixel ayant une valeur
	bool WriteBin(std::string OutFilename);
};

//------------------------------------------------------------------
class XOrthoLaserDalle
{
protected:
	XOrthoLaserFolder* parent;
	std::string m_strName;

	XRawImage m_OrthoLas ;//image pas forcement chargée

	XFrame m_frame;

	//décodé de la nomenclature
	double m_dResol;
	uint32 m_nDate;
	std::string m_strRxpName;
	std::string m_strProjection;
	uint16 m_nIncrement;

	XOrthoLaserGrille* gridZ;
	XOrthoLaserGrille* gridH;

	//implementation
	float m_centerTime;//a faire évoluer avec hmin hmax
	float m_centerZ;

	std::vector<XOrthoLaserMesure*>	m_Mesure;// Mesures effectuees dans l'orthos
	XPt3D OffsetUsefull(); //à priori c'est l'offset du chantier

	XOrthoLaserRxp* rxp;

	//cuisine interne
	uint32 ExtractDate(std::string& s);


public:
	XOrthoLaserDalle(XOrthoLaserFolder* pparent);
	~XOrthoLaserDalle(void);

	//methodes de transition entre dalles et chantier
	XError* Error();
	XOrthoLaserChantier* Chantier() ;
	std::string Folder();
	std::string FolderForWriting();

	bool SetName(std::string nameNoExt);
	bool SetDimension(unsigned int w, unsigned int h, unsigned int bps, unsigned int numChannel);
	bool InitSize();//initialisation taille uniquement
	bool Initialize();//initialisation complète : taille + Z =T
	void LoadTime();

	bool WriteInfoXml(std::ostream* out);

	uint32 Width(){return m_OrthoLas.Width();}
	uint32 Height(){return m_OrthoLas.Height();}
	double Resolution(){return m_dResol;}

	uint32 NbPixWithValue();
	

	std::string File();
	std::string OrthoFile();
	std::string Hfile();
	std::string Zfile();
	std::string xyzLaserfile();
	std::string PlyFile();

	XFrame Frame();
	XPt2D Center(){return m_frame.Center();}
	XPt2D NW(){return m_frame.NW();}
	XPt2D SW(){return m_frame.SW();}

	XPt2D Coord(uint16 col, uint16 lig);
	XPt2D Coord(float3* XYZlaser);
	XPt2D CoordPix(XPt2D pTerrain);

	uint16 CorOrthoLaserTime2Gps();

	std::string Name(){return m_strName;} //0624900-6854400-040_Casqyb_07_LAMB93_0
	std::string strIdCouche(){return m_strName.substr(m_strName.find('_')+1);}//Casqyb_07_LAMB93_0
	std::string strIdPile(){ return m_strName.substr(0,m_strName.find('_'));}//0624900-6854400-040
	std::string RxpName(){ return m_strRxpName;}//décodé de la nomenclature
	// NO TRAJ std::string TrajectoName();

	bool IsIn(XPt2D P){return m_frame.IsIn(P);}

	XOrthoLaserMesure* GetMesure(XOrthoLaserPoint* P);
	bool GetMesure(XPt3D pTerrain, XPt2D* pImage);
	bool AddMesure(XOrthoLaserMesure* mes);
	bool RemoveMesure(XOrthoLaserMesure* mes);
	XOrthoLaserMesure* FindMesure(XOrthoLaserPoint* P);
	uint32 NbMesures(){return m_Mesure.size();}
	std::vector<XOrthoLaserMesure*>* ListMesures(){return &m_Mesure;}
	uint32 NbMesuresValides();
	std::vector<XOrthoLaserMesure*> ListMesuresValides();
	uint32 NbLiaisonsValides(XOrthoLaserDalle* dalle);

	bool ManageLoading(bool *HtoUnload, bool* ZtoUnload);
	void ManageUnLoading(bool HtoUnload, bool ZtoUnload);

	std::string InfosPointe(XPt2D mesure);
	void InfosConsole();

	bool LoadGrilles(bool all_grids = false);
	bool LoadGrillesAuto();
	void UnLoadGrilles();
	bool GetCoordTerrain(double col, double lig, XPt3D* pt);
	bool GetCoordTerrain(XOrthoLaserMesure* mes, XPt3D* pt){return GetCoordTerrain(mes->X,mes->Y,pt);}

	float GetBestZ(double col, double lig);
	float GetZ(double col, double lig);
	float GetNearestZ(double col, double lig);
	float CenterZ();

	float GetBestH(double col, double lig );
	float GetH(double col, double lig);
	float GetNearestH(double col, double lig);

	uint32 Date(){return m_nDate;}
	float CenterTime();
	void CenterTime(float val){m_centerTime = val;}
	bool SetTime(XOrthoLaserMesure* mes);

	//implementation TE avec connexion via rxp
	XOrthoLaserRxp* Rxp(){return rxp;}
	void Rxp(XOrthoLaserRxp* xolrxp){rxp = xolrxp;}


	//conversion diverses
	bool WritePlyEntete(std::string plyfilename,XPt3D offset,uint32 nbPts);
	bool WritePlyDatas(std::string plyfilename,XPt3D offset,float coefZ);
	bool ExportToPly(float coefZ);
	bool Bouchage();

	//methodes utilisées pour la fusion
	bool Load_All(bool all_grids= false);
	bool Load_Auto();//chargement automatique des grilles présentes
	void Unload_All();
	bool LoadXYZlaser();//les grilles inutilisé pour le recalage sont gérées différemment !	
	bool WriteXYZlaser(std::string OutFilename);
	XRawImage* OrthoLas(){return &m_OrthoLas ;}//image pas forcement chargée
	XOrthoLaserGrille* Zgrid(){return gridZ;}
	XOrthoLaserGrille* Hgrid(){return gridH;}

	bool Write_All(std::string OutputFolder);
	bool Copy_All(std::string OutputFolder);
	bool Copy_Georef(std::string OutputFolder);
	bool ExportMesuresDalles(std::ostream* out);

	float3* gridXYZlaser;
	bool m_bgridXYZlaserAvailable;

};
