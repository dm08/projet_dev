#include "XOrthoLaserChantier.h"
#include "XOrthoLaserPile.h"
#include "XOrthoLaserRxp.h"
#include "XOrthoLaserMesure.h"
#include "XOrthoLaserPoint.h"

#include "XStringTools.h"

#include "XSystemInfo.h"
#include "XPath.h"

//#include "XGpsTools.h"

#include <algorithm>

//-----------------------------------------------------------------------------
XOrthoLaserChantier::XOrthoLaserChantier(XError* error)
{
	m_error = error;
	m_deltaMesurePileMax = 100; //100 pixels à 4cm = 4m!
	m_bDoCorrectionUtcGps = true;

}
//-----------------------------------------------------------------------------
XOrthoLaserChantier::~XOrthoLaserChantier(void)
{
	Clear();
}
//-----------------------------------------------------------------------------
uint16 XOrthoLaserChantier::CorOrthoLaserTime2Gps(uint32 date)
{
	if(!m_bDoCorrectionUtcGps)
		return 0;
	return 0;
	//return XGpsTools(date).CorrectionUtcToGps();
}
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserChantier::OffsetUsefull()//un offset utilisable 
{
	if(m_offset != XPt3D())
		return m_offset;

	//offset non renseigné
	return XPt3D(m_Frame.Xmin,m_Frame.Ymin, 0);
}
//-----------------------------------------------------------------------------
void XOrthoLaserChantier::Clear()
{
	//les points font le ménage sur les mesures
	for(uint32 i=0; i< m_vecSift.size(); i++)
		delete m_vecSift[i];
	m_vecSift.clear();

	for(uint32 i=0; i< m_vecFolder.size(); i++)
		delete m_vecFolder[i];
	m_vecFolder.clear();
	m_vecDalle.clear();//copie

	for(uint32 i=0; i< m_vecPile.size(); i++)
		delete m_vecPile[i];
	m_vecPile.clear();

	for(uint32 i=0; i< m_vecRxp.size(); i++)
		delete m_vecRxp[i];

	m_vecRxp.clear();
	m_Frame = XFrame();
	m_offset = XPt3D();

}
//-----------------------------------------------------------------------------
XOrthoLaserPile*  XOrthoLaserChantier::PileOrtho(XPt2D pTerrain)
{
	for(uint32 i=0; i<m_vecPile.size(); i++)
		if(m_vecPile[i]->IsIn(pTerrain))
			return m_vecPile[i];
	return NULL;
}
//-----------------------------------------------------------------------------
//std::vector<XOrthoLaserDalle*> XOrthoLaserChantier::DallesOrtho(XOrthoLaserPoint* point)
//{
//	XOrthoLaserPile* pile = PileOrtho(point);
//	if(pile != NULL)
//		return pile->vecDalle;
//	std::vector<XOrthoLaserDalle*> vec;
//	for(uint32 i=0; i< m_vecDalle.size(); i++)
//		if(m_vecDalle[i]->IsIn(*point))
//			vec.push_back(m_vecDalle[i]);
//
//	return vec;
//}

//-----------------------------------------------------------------------------
void XOrthoLaserChantier::ComputeFrame()
{
	m_Frame = XFrame();
	for (uint32 i = 0; i < m_vecPile.size(); i++)
		m_Frame += m_vecPile[i]->Frame();
}
//-----------------------------------------------------------------------------
XOrthoLaserPile* XOrthoLaserChantier::FindPile(XFrame f)
{
	for(uint32 i=0; i< m_vecPile.size(); i++)
		if(m_vecPile[i]->Frame() == f)
			return m_vecPile[i];
	return NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserPile* XOrthoLaserChantier::FindPile(std::string IdPile)
{
	for(uint32 i=0; i< m_vecPile.size(); i++)
	{
		std::string idp = m_vecPile[i]->Id();
		if(m_vecPile[i]->Id() == IdPile)
			return m_vecPile[i];
	}
	return NULL;
}//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::CreatePiles()
{
	XErrorCommentaire(m_error,__FUNCTION__,"Creation des piles ");
	char message[1024];
	if(m_vecDalle.empty())
		return XErrorAlert(m_error,__FUNCTION__,"Aucune dalle !");
		
	XOrthoLaserPile* pile = new XOrthoLaserPile(this);
	pile->vecDalle.push_back(m_vecDalle[0]);
	m_vecPile.push_back(pile);
	for(uint32 i=1; i< m_vecDalle.size(); i++)
	{
		//plus sur d'utiliser les frames si la taille des dalles a été modifiées
		//XOrthoLaserPile* pil = FindPile(m_vecDalle[i]->Frame());
		XOrthoLaserPile* pil = FindPile(m_vecDalle[i]->strIdPile());
		if(pil == NULL)
		{
			pil = new XOrthoLaserPile(this);
			m_vecPile.push_back(pil);
		}
		pil->vecDalle.push_back(m_vecDalle[i]);
	}
	
	sprintf(message,"%d piles pour %d dalles",m_vecPile.size(),m_vecDalle.size());
	XErrorInfo(m_error,__FUNCTION__,message);

	//determination des dalles maitres
	XErrorCommentaire(m_error,__FUNCTION__,"Initialisation des dalles maitres ");
	for(uint32 i=0; i< m_vecPile.size(); i++)
		m_vecPile[i]->InitMasterDalle();

	//l'emprise des dalles est calculée à partir du nombre de lignes colonnes des dalles ortho 
	//le compute frame doit être placé ici !
	XErrorCommentaire(m_error,__FUNCTION__,"\nCalcul des emprises ");
	ComputeFrame();

	for(uint32 i=0; i< m_vecFolder.size(); i++)
		m_vecFolder[i]->XmlWriteMetafile(true);//on force l'écriture du fichier pour sauvegarder les infos récupérées lors de l'initialisation

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::CreateRxp()
{
	XOrthoLaserDalle* dalle;
	XOrthoLaserRxp* rxp;
	for(uint32 i=0; i< m_vecDalle.size(); i++)
	{
		 dalle = m_vecDalle[i];
		 rxp = FindRxp(dalle->RxpName());
		 if(rxp == NULL)
		 {
			 rxp = new XOrthoLaserRxp(this,dalle->RxpName(),dalle->Date());
			 m_vecRxp.push_back(rxp);
		 }
		 rxp->AddTime(dalle->CenterTime());
		 dalle->Rxp(rxp);

	}
	return true;
}
//-----------------------------------------------------------------------------
XOrthoLaserRxp* XOrthoLaserChantier::FindRxp(std::string RxpName)
{
	for(uint32 i=0; i<m_vecRxp.size(); i++)
		if(m_vecRxp[i]->Name().compare(RxpName) == 0)
			return m_vecRxp[i];

	return NULL;
}

//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::AddSiftResult(std::string file)
{
	//0624900-6854400-040_Casqyb_07_LAMB93_0.-.0624900-6854400-040_Casqyb_08_LAMB93_0.result
	XStringTools ST;
	XPath P;
	std::vector<std::string> tokens;
	ST.Tokenize(P.Name(file.c_str()),tokens,std::string("."));
	if (tokens.size() != 4 )
		return XErrorError(m_error,__FUNCTION__,"Nomenclature non conforme pour un fichier result ",file.c_str());

	XOrthoLaserDalle* dalle1 = FindDalle(tokens[0]);
	if(dalle1 == NULL)
		return XErrorError(m_error,__FUNCTION__,"La dalle n'appartient pas au chantier ",tokens[0].c_str());
	XOrthoLaserDalle* dalle2 = FindDalle(tokens[2]);
	if(dalle1 == NULL)
		return XErrorError(m_error,__FUNCTION__,"La dalle n'appartient pas au chantier ",tokens[2].c_str());

	dalle1->LoadGrilles();
	dalle2->LoadGrilles();
	std::ifstream in(file.c_str());
	if (!in.good())
		return XErrorError(m_error,__FUNCTION__,"erreur ouverture fichier ",file.c_str());

	char ligne[1024];
	while( (in.good()) && (!in.eof())) 
	{
		in.getline(ligne,1023);
		if(!in.good())
			break;
		std::vector<std::string> strcoord;
		ST.Tokenize(std::string(ligne),strcoord,std::string("\t"));
		if(strcoord.size() != 4)
			continue;
		XOrthoLaserPoint* point = new XOrthoLaserPoint(0,0,0,TypeAppui::PSift,true);
		XOrthoLaserMesure* mesure1 = point->CreateMesure(dalle1); 
		XOrthoLaserMesure* mesure2 = point->CreateMesure(dalle2);
		mesure1->X = atof(strcoord[0].c_str());
		mesure1->Y = atof(strcoord[1].c_str());
		mesure1->Valid(true);
		mesure2->X = atof(strcoord[2].c_str());
		mesure2->Y = atof(strcoord[3].c_str());
		mesure2->Valid(true);
		dalle1->AddMesure(mesure1);
		dalle2->AddMesure(mesure2);

		if(point->SetMasterMesure())
		{
			delete point;
			XErrorAlert(m_error,__FUNCTION__,"master = NULL");
			continue;
		}
		m_vecSift.push_back(point);
	}
	dalle1->UnLoadGrilles();
	dalle2->UnLoadGrilles();

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::LoadSiftResults(std::string resultsFolder)
{
	XErrorCommentaire(m_error,__FUNCTION__,"Chargement des Sifts inter-pano");
	XSystemInfo system;
	XPath p;
	std::vector<std::string> listResult;
	system.GetFileListInFolder(resultsFolder,listResult,"*.result");
	if(listResult.size() == 0)
		return XErrorError(m_error,__FUNCTION__,"Aucun fichier result dans le répertoire  ",resultsFolder.c_str());

	for(uint32 i=0; i< listResult.size(); i++)
		if(system.GetFileSize1(listResult[i].c_str()) > 0)
			AddSiftResult(listResult[i]);
	
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::DeletePoint(XOrthoLaserPoint* pt)
{
	std::vector<XOrthoLaserPoint*>::iterator iter;
	iter = std::find(m_vecSift.begin(), m_vecSift.end(), pt);
	if(iter == m_vecSift.end())//le point n'est ni dans les sifts ni dans les points manuels
		return false;

	delete *iter;
	iter = std::remove (m_vecSift.begin(), m_vecSift.end(), pt);
	m_vecSift.resize(iter - m_vecSift.begin());
	return true;

}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::XmlWrite(std::ostream* out, std::string xmlFilename)
{
	XPath P;
	for(uint32 i=0; i< m_vecFolder.size(); i++)
	{
		std::string path =  P.FolderRelative(P.Path(xmlFilename.c_str()).c_str(), m_vecFolder[i]->Folder().c_str());
		P.AddPathSep(path);
		*out << "<folder_ortho_laser>" << path  << "</folder_ortho_laser>\n";
	}

	//lorsque la mécanique des pivots temporels sera en place ...
	//virer cette option qui indique si on doit faire la correction UTC to GPS entre dalles et trajectos
	*out << "<do_correction_UTC_GPS>" << DoCorrectionUtcGps()  << "</do_correction_UTC_GPS>\n";

	if(m_offset != XPt3D())
	{
		*out<< "<Offset>\n";
		m_offset.XmlWrite(out);
		*out<< "</Offset>\n";
	}

	if(m_vecSift.empty())
		return out->good();
	*out << "<OrthoLaserPoints>\n";
	for(uint32 i=0; i< m_vecSift.size(); i++)
		m_vecSift[i]->XmlWrite(out);
	*out << "</OrthoLaserPoints>\n";

	return out->good();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::ConnectImages()//connexion des images après chargement xml
{
	if(m_vecSift.empty())
		return true;
	char message[1024];
	sprintf(message,"Connection de %d points de liaisons automatiques",m_vecSift.size());
	XErrorCommentaire(m_error,__FUNCTION__,message);
	for(uint32 i=0; i<m_vecSift.size(); i++)
	{
		m_vecSift[i]->ConnectOrthoLas(this);
		m_vecSift[i]->SetMasterMesure();
	}

	return true;
}
//-----------------------------------------------------------------------------
void XOrthoLaserChantier::RegenerateTimeFromOrtholas()
{
	XErrorCommentaire(m_error,__FUNCTION__,"\nRegeneration des heures issues des dalles");
	for(uint32 i=0; i< this->m_vecDalle.size(); i++)
		m_vecDalle[i]->LoadTime();	
}

//-----------------------------------------------------------------------------
void XOrthoLaserChantier::AddFolderInput(std::string path) 
{
	m_vecFolder.push_back(new XOrthoLaserFolder(this,path));
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::CheckDalle(std::string name)
{	//ne devrait plus etre utilisé si on n'impose plus la présence des dalles
	for(uint32 i=0; i< m_vecFolder.size(); i++)
		if(m_vecFolder[i]->CheckDalle(name))
			return true;//la dalle est disponible
	return false;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserChantier::nbDalles()
{
	uint32 count =0;
	for(uint32 i=0; i< m_vecFolder.size(); i++)
		count += m_vecFolder[i]->nbDalles();
	return count;
}

//-----------------------------------------------------------------------------
XOrthoLaserDalle* XOrthoLaserChantier::FindDalle(std::string name)
{
	for(uint32 i=0; i< m_vecFolder.size(); i++)
	{
		XOrthoLaserDalle* dalle = m_vecFolder[i]->FindDalle(name);
		if(dalle != NULL)
			return dalle;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::LoadFolderOrthoLaser()
{
	for(uint32 i=0; i< m_vecFolder.size(); i++)
	{
		if(!m_vecFolder[i]->LoadFolderOrthoLaser())
			return false;
		std::vector<XOrthoLaserDalle*> tmp = *m_vecFolder[i]->DallesOrthos();
		m_vecDalle.insert(m_vecDalle.end(), tmp.begin(), tmp.end());
	}
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::LoadOrthoLaserDallesFromNode( TiXmlNode* node)
{
	//a priori si on passe ici on est dans un mode de compatibilité ascendante
	//on ne doit avoir q'un seul repertoire dont les infos de contenu était stockés dans le fichier xml
	if(m_vecFolder.size() == 0)
		return XErrorError(Error(),__FUNCTION__,"m_vecFolder.size() == 0");
	if(m_vecFolder.size() > 1)
		return XErrorError(Error(),__FUNCTION__,"m_vecFolder.size() > 1");
	if(!m_vecFolder[0]->LoadOrthoLaserDallesFromNode(node))
		return false;
	//on réécrit  les infos dans le dossier en question pour être rechargé avec la nouvelle méthodologie
	m_vecFolder[0]->XmlWriteMetafile(false);
	return true;
}

//-----------------------------------------------------------------------------
bool XOrthoLaserChantier::ExportMesuresDalles(std::string filename)
{
	std::ofstream mes(filename.c_str());
	mes.precision(2);
	mes.setf(std::ios::fixed);
	for(uint32 i=0; i< m_vecPile.size(); i++)
		m_vecPile[i]->ExportMesuresDalles(&mes);

	mes.close();
	return true;
}