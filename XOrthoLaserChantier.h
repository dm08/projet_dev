#pragma once
#include "libXBase/XError.h"
#include "XOrthoLaserDalle.h"
#include "XOrthoLaserFolder.h"
#include <string>

//-----------------------------------------------------------------------------
class XOrthoLaserFolder;
class XOrthoLaserPoint;
class XOrthoLaserPile;
class XOrthoLaserRxp;
class XOrthoLaserChantier
{
protected:
	XError* m_error;
	std::vector<XOrthoLaserFolder*> m_vecFolder;
	std::vector<XOrthoLaserPoint*> m_vecSift;//points de liaison sift ( ne peuvent pas être mesurés en absolu)
	std::vector<XOrthoLaserPile*> m_vecPile;
	std::vector<XOrthoLaserRxp*> m_vecRxp;
	std::vector<XOrthoLaserDalle*> m_vecDalle;//copie de l'ensemble des dalles des différents dossiers

	XPt3D m_offset;//offset utilisé pour exporter les ply
	bool m_bDoCorrectionUtcGps;

	double m_deltaMesurePileMax;

	XFrame m_Frame;
	void ComputeFrame();

public:
	XOrthoLaserChantier(XError* error);
	~XOrthoLaserChantier(void);


	XPt3D Offset(){return m_offset;}
	XPt3D OffsetUsefull();
	void  Offset(XPt3D p){m_offset = p;}

	bool DoCorrectionUtcGps(){return m_bDoCorrectionUtcGps;}
	void DoCorrectionUtcGps(bool flag){ m_bDoCorrectionUtcGps = flag;}
	uint16 CorOrthoLaserTime2Gps(uint32 date);

	void Clear();
	XFrame Frame(){return m_Frame;}

	XError* Error(){return m_error;}
	std::vector<XOrthoLaserPoint*>*  SiftPoints(){return &m_vecSift;}
	bool DeletePoint(XOrthoLaserPoint* pt);
	void AddPoint(XOrthoLaserPoint* pt){m_vecSift.push_back(pt);}


	// methodes a modifiées pour multi folder ortho
	void AddFolderInput(std::string path) ;
	bool CheckDalle(std::string name);//a virer prochainement
	bool LoadFolderOrthoLaser();
	XOrthoLaserDalle* FindDalle(std::string name);
	uint32 nbDalles();
	std::vector<XOrthoLaserDalle*>* DallesOrthos(){return &m_vecDalle;}//adresse de la copie
	//todo methodes a modifier pour multi folder ortho
	bool LoadOrthoLaserDallesFromNode( TiXmlNode* node);
	bool XmlWrite(std::ostream* out, std::string xmlFilename);

	bool ConnectImages();

	bool CreateRxp();
	std::vector<XOrthoLaserRxp*> Rxp(){return m_vecRxp;}
	XOrthoLaserRxp* FindRxp(std::string RxpName);

	bool CreatePiles();
	XOrthoLaserPile* FindPile(XFrame f);
	XOrthoLaserPile* FindPile(std::string IdPile);
	std::vector<XOrthoLaserPile*>* PilesOrthos(){return &m_vecPile;}
	uint32 nbPiles(){return m_vecPile.size();}

	XOrthoLaserPile* PileOrtho(XPt2D pTerrain);
	XOrthoLaserPile* PileOrtho(XOrthoLaserPoint* point){if(point!=NULL) return PileOrtho(*point); return NULL;}

	bool LoadSiftResults(std::string resultsFolder);
	bool AddSiftResult(std::string file);

	double DeltaMesurePileMax(){return m_deltaMesurePileMax;}

	void RegenerateTimeFromOrtholas();

	//Export divers
	bool ExportMesuresDalles(std::string filename);

};
