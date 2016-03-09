#pragma once
#include "libXBase/XBase.h"
#include "libXBase/XFrame.h"
#include <vector>
#include "XOrthoLaserDalle.h"
#include "XOrthoLaserFolder.h"

class TiXmlNode;
class XOrthoLaserChantier;
class XError;
class XPt3D;
class XOrthoLaserFolder
{
protected:
	XOrthoLaserChantier* p_parent;
	std::string m_strFolder;//nom du repertoire
	bool m_bWrintingRights;
	std::vector<XOrthoLaserDalle*> m_vecDalle;
	XFrame m_Frame;//permet de stocker une emprise des dalles du dossier 

	std::string m_strIdSolution;//un repertoire est obligatoirement associé à une seule trajecto
public:
	XOrthoLaserFolder(XOrthoLaserChantier* parent,std::string path);
	~XOrthoLaserFolder(void);

	void Clear();

	std::vector<XOrthoLaserDalle*>* DallesOrthos(){return &m_vecDalle;}
	uint32 nbDalles(){return m_vecDalle.size();}

	//methodes de transition entre dalles et chantier
	XError* Error();
	XPt3D OffsetUsefull();
	uint16 CorOrthoLaserTime2Gps(uint32 date);
	XOrthoLaserChantier* Chantier();

	std::string Folder() {return m_strFolder;}
	std::string FolderForWriting() ;

	bool LoadFolderOrthoLaser();
	XFrame Frame(){return m_Frame;}//emprise globale des dalles du dossier 

	bool LoadMetafile();
	std::string Metafile();

	bool CheckDalle(std::string name);
	void InitSize();//initialisation simplifiée : uniquement la taille des dalles pour faire des calculs d'intersection)

	XOrthoLaserDalle* Dalle(uint32 rg){if(rg>m_vecDalle.size()-1)return NULL; return m_vecDalle[rg];}
	XOrthoLaserDalle* FindDalle(std::string name);
	XOrthoLaserDalle* FindDalle(XPt2D p);//Point en coordonnées terrain
	XOrthoLaserDalle* FindDalleWithPrefix(std::string prefixe,uint32* count);

	XOrthoLaserDalle* AddDalle(std::string TifFilename, bool check, double centerTime = 0.,	uint16 w=0, uint16 h=0, uint16 bps=0, uint16 nchan=0);
	bool LoadOrthoLaserDallesFromNode( TiXmlNode* node);
	bool XmlWriteMetafile(bool force);


};
