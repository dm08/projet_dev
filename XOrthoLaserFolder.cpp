#include "XOrthoLaserFolder.h"
#include "XOrthoLaserChantier.h"

#include "XArchiXMLException.h"
#include "XArchiXMLBaseTools.h"
#include "XArchiXMLTools.h"

#include "XPath.h"
#include "XSystemInfo.h"

//-----------------------------------------------------------------------------
XOrthoLaserFolder::XOrthoLaserFolder(XOrthoLaserChantier* parent, std::string path)
{
	p_parent = parent;
	m_strFolder = path;
	m_bWrintingRights = true;

}
//-----------------------------------------------------------------------------
XOrthoLaserFolder::~XOrthoLaserFolder(void)
{
	Clear();
}
//-----------------------------------------------------------------------------
void XOrthoLaserFolder::Clear()
{
	for(uint32 i=0; i< m_vecDalle.size(); i++)
		delete m_vecDalle[i];
	m_vecDalle.clear();
	m_Frame = XFrame();
}
//-----------------------------------------------------------------------------
XError* XOrthoLaserFolder::Error(){return p_parent->Error();}
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserFolder::OffsetUsefull(){return p_parent->OffsetUsefull();}
//-----------------------------------------------------------------------------
uint16 XOrthoLaserFolder::CorOrthoLaserTime2Gps(uint32 date){return p_parent->CorOrthoLaserTime2Gps(date);}
//-----------------------------------------------------------------------------
XOrthoLaserChantier* XOrthoLaserFolder::Chantier() {return p_parent;}

//-----------------------------------------------------------------------------
std::string XOrthoLaserFolder::Metafile()
{
	std::string meta = m_strFolder + "OrthoLaserFolder.xml";
	return meta;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserFolder::LoadOrthoLaserDallesFromNode( TiXmlNode* node)
{
	TiXmlHandle hdl(node);
    TiXmlElement* dalle =  hdl.FirstChild("Dalle").ToElement();
	while(dalle)
	{
		double time = XArchiXML::ReadAssertAttributeOrNodeAsDouble(dalle,std::string("time"));
		std::string name = XArchiXML::ReadAssertAttributeOrNodeAsString(dalle,std::string("name"));
		uint16 w = XArchiXML::ReadAttributeOrNodeAsUint32(dalle,std::string("w"),0);
		uint16 h = XArchiXML::ReadAttributeOrNodeAsUint32(dalle,std::string("h"),0);
		uint16 bps = XArchiXML::ReadAttributeOrNodeAsUint32(dalle,std::string("bps"),0);
		uint16 nchan = XArchiXML::ReadAttributeOrNodeAsUint32(dalle,std::string("nchan"),0);
		AddDalle(name,true,time,w,h,bps,nchan);
		dalle = dalle->NextSiblingElement("Dalle");
	}
	return true;
}//-----------------------------------------------------------------------------
bool XOrthoLaserFolder::LoadMetafile()
{
	try
	{
		TiXmlDocument doc( Metafile().c_str() );
		if (!doc.LoadFile() )
			return XErrorError(Error(),__FUNCTION__,"erreur de lecture xml", doc.ErrorDesc());

		TiXmlElement* root = doc.RootElement();
		XArchiXML::AssertRoot(root,"OrthoLaserFolder");

		TiXmlNode* olds =  XArchiXML::FindAssertNode(root,"OrthoLaserDalles");
		LoadOrthoLaserDallesFromNode(olds);
	}
	catch(XArchiXML::XmlException e)
	{
		return XErrorError(Error(),__FUNCTION__,e.Erreur().c_str(),Metafile().c_str());
	}

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserFolder::XmlWriteMetafile(bool force)
{
	std::string metafolder = Metafile();
	XSystemInfo system;

	//on n'ecrase pas un fichier de meta donnes en place qui pourrait avoir ete ecrit pas rieglexport
	if((!force)&&(system.FindFile(metafolder.c_str())))
		return XErrorAlert(Error(),__FUNCTION__,"Fichier de metadonnees deja existant !",metafolder.c_str());

	std::ofstream out(metafolder.c_str(),std::ios::out);
	if(!out.good())
		return XErrorError(Error(),__FUNCTION__,"Erreur de création fichier ", Metafile().c_str());
	
	out << "<OrthoLaserFolder>\n";
	out << "<OrthoLaserDalles>\n";
	for(uint32 i=0; i< this->m_vecDalle.size(); i++)
		m_vecDalle[i]->WriteInfoXml(&out);
	out << "</OrthoLaserDalles>\n";
	out << "</OrthoLaserFolder>\n";

	out.close();
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserFolder::LoadFolderOrthoLaser()
{
	if(m_strFolder.empty())
		return XErrorError(Error(),__FUNCTION__,"Dossier orthos laser non renseigné ! ");

	XErrorInfo(Error(),__FUNCTION__,"Chargement Orthos laser ",m_strFolder.c_str());
	XSystemInfo system;
	if(!system.FindFolder(m_strFolder))
		return XErrorError(Error(),__FUNCTION__,"Le répertoire n'est pas accessible ",m_strFolder.c_str());

	XPath p;
	p.AddPathSep(m_strFolder);

	//on commence par regarder si le fichier de meta-données des dalles est présent
	if(system.FindFile(Metafile().c_str()))
		LoadMetafile();

	std::vector<std::string> listeTif;
	system.GetFileListInFolder(m_strFolder,listeTif,"*.tif");
	uint32 nbTif = listeTif.size();
	if(nbTif == 0)
		return XErrorError(Error(),__FUNCTION__,"Aucune image tif dans le répertoire ortho laser ",m_strFolder.c_str());

	XErrorCommentaire(Error(),__FUNCTION__,"Chargement des dalles ");
	for(uint32 i=0; i< listeTif.size(); i++)
		AddDalle(listeTif[i],false);

	//on test le droits d'ecriture;
	m_bWrintingRights = system.TestWrintingOnFolder(m_strFolder);
	return true;
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserFolder::FolderForWriting() 
{
	if(m_bWrintingRights)
		return m_strFolder;
	XSystemInfo system;
	std::string temppath = system.GetTempFolder();
	XPath P;
	P.AddPathSep(temppath);
	return temppath;
}

//-----------------------------------------------------------------------------
//Vérifie simplement la présence de la dalle tif dans le dossier 
bool XOrthoLaserFolder::CheckDalle(std::string name)
{
	XSystemInfo system;
	std::string filename = this->m_strFolder + name + ".tif";
	if(system.FindFile(filename.c_str()))
		return true;
	return false;
	//return XErrorError(m_error,__FUNCTION__,"Fichier non present ", filename.c_str());
}
//-----------------------------------------------------------------------------
void XOrthoLaserFolder::InitSize()
{
	for(uint32 i=0; i< m_vecDalle.size(); i++)
	{
		m_vecDalle[i]->InitSize();	
		m_Frame += m_vecDalle[i]->Frame();
	}
	XErrorInfo(Error(),__FUNCTION__,"Emprise du dossier ",m_Frame.InfoTexte().c_str());
}
//-----------------------------------------------------------------------------
XOrthoLaserDalle* XOrthoLaserFolder::AddDalle(std::string TifFilename, bool check, double centerTime,	uint16 w, uint16 h, uint16 bps, uint16 nchan)
{
	XPath P;
	std::string name = P.NameNoExt(TifFilename.c_str());
	XOrthoLaserDalle* dalle = FindDalle(name);
	if(dalle != NULL)
		return dalle;

	if(check)
	{
		XSystemInfo system;
		std::string file = Folder()+ name +".tif";
		if(!system.FindFile(file.c_str()))
		{
			XErrorError(Error(),__FUNCTION__,"Dalle manquante ",file.c_str());
			return NULL;
		}
	}
	
	dalle = new XOrthoLaserDalle(this);
	if(!dalle->SetName(name))
	{
		delete dalle;
		XErrorError(Error(),__FUNCTION__,"Mauvaise nomenclature ",name.c_str());
		return NULL;
	}
	if(centerTime != 0.)
		dalle->CenterTime(centerTime);
	if(w != 0)
		dalle->SetDimension(w,h,bps,nchan);
	
	m_vecDalle.push_back(dalle);
	return dalle;
}

//-----------------------------------------------------------------------------
XOrthoLaserDalle* XOrthoLaserFolder::FindDalle(std::string name)
{
	for(uint32 i=0; i<m_vecDalle.size(); i++)
		if(m_vecDalle[i]->Name().compare(name) == 0)
			return m_vecDalle[i];
	
	return NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserDalle* XOrthoLaserFolder::FindDalleWithPrefix(std::string prefixe, uint32* count)
{
	for(uint32 i=0; i<m_vecDalle.size(); i++)
	{
		std::string test  = m_vecDalle[i]->Name().substr(0,prefixe.size());
		if(test.compare(prefixe) == 0)
		{
			*count = i;
			return m_vecDalle[i];
		}
	}
	//XErrorAlert(Error(),__FUNCTION__,"prefixe non trouve ",prefixe.c_str());
	return NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserDalle* XOrthoLaserFolder::FindDalle(XPt2D p)
{
	for(uint32 i=0; i<m_vecDalle.size(); i++)
	{
		XFrame test = m_vecDalle[i]->Frame();
		if(m_vecDalle[i]->Frame().IsIn(p))
			return m_vecDalle[i];
	}
	
	return NULL;
}
