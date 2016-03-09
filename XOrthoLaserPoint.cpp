#include "XOrthoLaserPoint.h"
#include "XOrthoLaserChantier.h"
#include "XOrthoLaserDalle.h"

#include "libXBase/XBase.h"
#include "libXBaseXML/XArchiXMLException.h"
#include "libXBaseXML/XArchiXMLBaseTools.h"
#include "libXBaseXML/XArchiXMLTools.h"

#include "ReaderFilePosPac/PostProcessedProjPosition.h"
#include "ReaderFilePosPac/ProjPostProcessSolution.h"
#include "ReaderFilePosPac/XContrainte.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//-----------------------------------------------------------------------------
//**************   POINT
float3 XOrthoLaserPoint::defaultEcartTypePoint = float3(0.002,0.002,0.002);

//-----------------------------------------------------------------------------
XOrthoLaserPoint::~XOrthoLaserPoint(void)
{
	//c'est le point qui fait le ménage
	for (uint32 i = 0; i < m_Mesure.size(); i++){
		m_Mesure[i]->DeconnectOrtho();
		delete m_Mesure[i];
	}
	m_Mesure.clear();

	if(m_CompPosition != NULL)
		delete m_CompPosition;
	if(m_ecartType != NULL)
		delete m_ecartType;	
	
}	
//-----------------------------------------------------------------------------
XError* XOrthoLaserPoint::Error()
{
	if(this->m_masterMesure == NULL)
		return NULL;
	return m_masterMesure->Error();
}
//-----------------------------------------------------------------------------
void XOrthoLaserPoint::SetCompPosition(XPt3D p)
{
	if(m_CompPosition == NULL)
		m_CompPosition = new XPt3D();
	*m_CompPosition = p;
}
//-----------------------------------------------------------------------------
float XOrthoLaserPoint::ResX()
{
	if(m_CompPosition == NULL)
		return 0;
	return m_CompPosition->X - X;
}
float XOrthoLaserPoint::ResY()
{
	if(m_CompPosition == NULL)
		return 0;
	return m_CompPosition->Y - Y;
}
float XOrthoLaserPoint::ResZ()
{
	if(m_CompPosition == NULL)
		return 0;
	return m_CompPosition->Z - Z;
}
float XOrthoLaserPoint::ResXY()
{
	return XPt2D(ResX(),ResY()).Norme();
}
float XOrthoLaserPoint::ResXYZ()
{
	return XPt3D(ResX(),ResY(),ResZ()).Norme();
}
float XOrthoLaserPoint::ResXYMesMax()
{
	if(m_CompPosition == NULL)
		return 0;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	if(valid.empty())
		return 0;

	XPt2D diff;
	XPt2D diffMax = XPt2D(m_CompPosition->X-valid[0]->PositionLaser().X,m_CompPosition->Y-valid[0]->PositionLaser().Y);
	float norme = diffMax.Norme();
	for (uint32 i = 1; i < valid.size(); i++)
	{
		diff = XPt2D(m_CompPosition->X-valid[i]->PositionLaser().X,m_CompPosition->Y-valid[i]->PositionLaser().Y);
		if(diff.Norme() > norme)
			diffMax = diff;
	}

	return norme;
}
float XOrthoLaserPoint::ResZMesMax()
{
	if(m_CompPosition == NULL)
		return 0;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	if(valid.empty())
		return 0;

	float diff;
	float diffMax = m_CompPosition->Z-valid[0]->PositionLaser().Z;
	for (uint32 i = 1; i < valid.size(); i++)
	{
		diff = m_CompPosition->Z-valid[i]->PositionLaser().Z;
		if(diff > diffMax)
			diffMax = diff;
	}

	return diffMax;
}
//-----------------------------------------------------------------------------
std::vector<float3> XOrthoLaserPoint::ResMesures()
{
	std::vector<float3> vec;
	if(m_CompPosition == NULL)
		return vec;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	for (uint32 i = 1; i < valid.size(); i++)
	{
		float3 f3;
		f3.x = m_CompPosition->X-valid[i]->PositionLaser().X;
		f3.y = m_CompPosition->Y-valid[i]->PositionLaser().Y;
		f3.z = m_CompPosition->Z-valid[i]->PositionLaser().Z;
		vec.push_back(f3);
	}
	return vec;
}

//-----------------------------------------------------------------------------
XOrthoLaserMesure* XOrthoLaserPoint::CreateMesure(XOrthoLaserDalle* dalle)
{
	XOrthoLaserMesure* mes = new XOrthoLaserMesure(this,dalle);
	m_Mesure.push_back(mes);
	return mes;
}
//-----------------------------------------------------------------------------
XOrthoLaserMesure* XOrthoLaserPoint::FindMesure(XOrthoLaserDalle* dalle)
{
	for (uint32 i = 0; i < m_Mesure.size(); i++)
		if(m_Mesure[i]->Ortho()== dalle)
			return m_Mesure[i];
	return NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserMesure* XOrthoLaserPoint::FindMesureOnSolution(ProjPostProcessSolution* sol)
{
	for (uint32 i = 0; i < m_Mesure.size(); i++)
		if(m_Mesure[i]->Ortho()->Rxp()->Solution() == sol)
			return m_Mesure[i];

	return NULL;
}
//-----------------------------------------------------------------------------
void XOrthoLaserPoint::AddMesure(XOrthoLaserMesure* mes)
{
	m_Mesure.push_back(mes);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::RemoveMesure(XOrthoLaserMesure* mes)
{
	std::vector<XOrthoLaserMesure*>::iterator iter;
	iter = std::find(m_Mesure.begin(), m_Mesure.end(), mes);
	if(iter == m_Mesure.end())
		return false;
	
	iter = std::remove (m_Mesure.begin(), m_Mesure.end(), mes);
	m_Mesure.resize(iter - m_Mesure.begin());
	return true;
}
//-----------------------------------------------------------------------------
std::vector<XOrthoLaserMesure*> XOrthoLaserPoint::MesuresValid()
{
	std::vector<XOrthoLaserMesure*> vec;
	for (uint32 i = 0; i < m_Mesure.size(); i++)
		if(m_Mesure[i]->Valid())
			vec.push_back(m_Mesure[i]);
	return vec;
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserPoint::Id()
{
	char strId[1024];
	sprintf(strId,"TEP_%d",Num());
	return std::string(strId);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::SetMasterMesure()//la master mesure pourrait venir de la première mesure valide dans la pile (triée)
{
	m_masterMesure = NULL;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	if(valid.empty())
		return XErrorError(Error(),__FUNCTION__," Aucune mesure valide ",Id().c_str());

	m_masterMesure = valid[0];
	for (uint32 i = 1; i < valid.size(); i++)
		if(valid[i]->Time() < m_masterMesure->Time())
			m_masterMesure = valid[i];

	if(Num() == 0)//si le point a déjà un numéro ...on le conserve
		m_masterMesure->SetPointNumFromTime();

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::ConnectSousEch(bool computeSegment)
{
	//pour un point totalement connécté : mise à jour des trajectos sous ech
	//mettre computeSegment = false lors du chargement initial pour ne pas recalculer les segments à chaque point !

	bool ok = true;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	for (uint32 i = 0; i < valid.size(); i++)
		if(!valid[i]->ConnectSousEch())
			ok = false;

	if(!computeSegment)
		return ok;

	std::list<ProjPostProcessSolution*> list = listTrajecto();
	std::list<ProjPostProcessSolution*>::iterator iter;
	for (iter = list.begin(); iter != list.end(); iter++)
		(*iter)->ComputeSegment();

	return ok;
}
//-----------------------------------------------------------------------------
void XOrthoLaserPoint::DeconnectSousEch(bool computeSegment)
{
	for (uint32 i = 0; i < m_Mesure.size(); i++)
		m_Mesure[i]->DeconnectSousEch();

	if(!computeSegment)
		return ;
	std::list<ProjPostProcessSolution*> list = listTrajecto();
	std::list<ProjPostProcessSolution*>::iterator iter;
	for (iter = list.begin(); iter != list.end(); iter++)
		(*iter)->ComputeSegment();
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserPoint::MasterName()
{
	if(m_masterMesure == NULL) 
		return std::string("master inconnu");
	if(m_masterMesure->Ortho() == NULL) 
		return std::string("master ortho nulle");
	return m_masterMesure->Ortho()->strIdCouche();
}
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserPoint::MasterPosition()
{//correspond a la position données par la dalle maitre 
	if(m_masterMesure == NULL) 
		return *this;// position d'un point APP non mesué ?
	return m_masterMesure->PositionLaser();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::EcritureVecteurViewer(std::ostream* out)
{
	*out << "1" << std::endl;
	*out << "POINT" << std::endl;

	if (Type() == PLiaison) 
		*out <<	"1	5	65280	0	80"<< std::endl;//vert
	else
		*out << "1	5	255	0	80" << std::endl;//rouge
	*out << std::endl;

	*out << Num() << std::endl;
	*out <<	"1" << std::endl;	
	return out->good();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::XmlWrite(std::ostream* out)
{
	*out << "<OrthoLaserPoint>\n";
	*out << "<number> " << m_nNum << " </number>" << std::endl;
	*out << "<type> " << (int)m_Type << " </type>" << std::endl;
	*out << "<X> " << X << " </X>" << std::endl;
	*out << "<Y> " << Y << " </Y>" << std::endl;
	*out << "<Z> " << Z << " </Z>" << std::endl;
	if(m_bControl)
		*out << "<control> " << m_bControl << " </control>" << std::endl;

	for (uint32 i = 0; i < m_Mesure.size(); i++)
		m_Mesure[i]->XmlWrite(out);
	*out << "</OrthoLaserPoint>\n";

	return out->good();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::LoadOrthoLaserMesureFromNode( TiXmlNode* node)
{
	std::string dalle = XArchiXML::ReadAssertNodeAsString(node,std::string("dalle"));
	float time = XArchiXML::ReadAssertNodeAsDouble(node,std::string("time"));
	double X = XArchiXML::ReadAssertNodeAsDouble(node,std::string("X"));
	double Y = XArchiXML::ReadAssertNodeAsDouble(node,std::string("Y"));

	if(time == 0)//compatibilité ascendante : certaines mesures invalides sont filtés lors de la relecture
		return false;

	XOrthoLaserMesure* mesure = new XOrthoLaserMesure(this,NULL);
	mesure->ImageName(dalle);
	mesure->Time(time);
	mesure->X = X;
	mesure->Y = Y;
	mesure->Valid(true);//les mesures non valides ne sont pas écrites dans le xml
	mesure->PositionLaser(XArchiXML::XPt3D_LoadFromNode(node));
	this->m_Mesure.push_back(mesure);

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::ConnectOrthoLas(XOrthoLaserChantier* chantier)
{
	bool valid = true;
	for (uint32 i = 0; i < m_Mesure.size(); i++)
	{
		std::string imageName = m_Mesure[i]->ImageName();
		XOrthoLaserDalle* dalle = chantier->FindDalle(m_Mesure[i]->ImageName());
		m_Mesure[i]->Ortho(dalle);
		if(dalle != NULL)
			dalle->AddMesure(m_Mesure[i]);
		else
		{
			char message[1024];
			sprintf(message,"Point %d : mesure sur dalle inexistante ", Num());
			XErrorError(chantier->Error(),__FUNCTION__,message,m_Mesure[i]->ImageName().c_str());
			valid = false;
		}
	}	
	return valid;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::InfosMesures()
{
	if(m_masterMesure==NULL)//jamais normal
		return XErrorError(Error(),__FUNCTION__," m_masterMesure==NULL",Id().c_str());

	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	char info[1024];
	sprintf(info,"Pt %d MESURES ortho : %d",Num(),valid.size());
	std::cout << info << '\n';

	XPt3D ref = *this;
	std::string infoRef = "Lock";
	if(IsMovable())
	{
		ref = m_masterMesure->PositionLaser();
		infoRef = "Master";
	}

	for (uint32 i = 0; i < valid.size(); i++)
	{
		if((IsMovable())&&(valid[i] == m_masterMesure))
			continue;
		
		XPt3D Tr = valid[i]->PositionLaser()- ref;
		sprintf(info,"Ortho/%s.%6.2lfm (%6.2lf %6.2lf %6.2lf) %s ", infoRef.c_str(),Tr.Norme(),Tr.X,Tr.Y,Tr.Z,valid[i]->strIdCouche().c_str());
		XErrorInfo(Error(),__FUNCTION__,info);
	}
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::InfosCompensation()
{
	std::ostringstream oss;
	oss << "COMPENSATION : Ecart type de position : ";
	if(m_ecartType == NULL)
		oss << "par defaut";
	else
		oss << m_ecartType->x <<' '<<m_ecartType->x<<' '<<m_ecartType->x <<' ';
	if(m_CompPosition == NULL)
	{
		oss << "/ non compense\n";
		std::cout << oss.str();
		return true;//normal
	}
	if(m_masterMesure==NULL)//jamais normal
		return XErrorError(Error(),__FUNCTION__,Id().c_str()," m_masterMesure==NULL");

	oss << "\nEcarts sur position " << StrMovable() <<"\n";
	std::cout << oss.str();
	
	XPt3D ref ;
	std::string infoRef;
	char info[1024];
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	for (uint32 i = 0; i < valid.size(); i++)
	{
		if(IsMovable())
		{
			ref = valid[i]->PositionLaser();
			infoRef = "Ortho";
		}
		else
		{
			ref = *this;
			infoRef = "Lock";
		}

		XPt3D Tr = *m_CompPosition- ref;
		sprintf(info,"Comp/%s.%6.2lfm (%5.2lf,%5.2lf,%5.2lf) %s ", infoRef.c_str(),Tr.Norme(),Tr.X,Tr.Y,Tr.Z,valid[i]->strIdCouche().c_str());
		XErrorInfo(Error(),__FUNCTION__,info);
	}	return true;
}
//-----------------------------------------------------------------------------
std::list<ProjPostProcessSolution*> XOrthoLaserPoint::listTrajecto()
{
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();
	std::list<ProjPostProcessSolution*> list;
	for(uint32 i = 0; i < valid.size(); i++)
	{
		ProjPostProcessSolution* sol = valid[i]->Solution();
		if(sol != NULL)
			list.push_back(sol);
	}
	list.sort();
	list.unique();
	return list;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPoint::RegeneratePositionFromMaster()
{
	if(!this->IsMovable())
		return XErrorInfo(Error(),__FUNCTION__," Methode inactive sur un point fixe ",Id().c_str());
	if(m_masterMesure == NULL)
		return XErrorInfo(Error(),__FUNCTION__," m_masterMesure == NULL ",Id().c_str());
	if(!m_masterMesure->Valid())
		return XErrorInfo(Error(),__FUNCTION__," m_masterMesure non valide ",Id().c_str());
	
	XPt3D old = m_masterMesure->PositionLaser();
	if(!m_masterMesure->SetValuesFromOrtholas())
	{
		m_masterMesure->Valid(false);
		XErrorInfo(Error(),__FUNCTION__," changement de masterMesure ",Id().c_str());
		SetMasterMesure();
		if(m_masterMesure==NULL)
			return XErrorError(Error(),__FUNCTION__," masterMesure NULL",Id().c_str());

		m_masterMesure->SetValuesFromOrtholas();
		if(m_masterMesure->Ortho() != NULL)
			m_masterMesure->Ortho()->UnLoadGrilles();
		return true;
	}
	
	XPt3D diff = old-m_masterMesure->PositionLaser();
	char message[1024];
	sprintf(message,"%d %s %.2lf %.2lf %.2lf",Num(),m_masterMesure->ImageName().c_str(),diff.X,diff.Y,diff.Z);
	XErrorCommentaire(Error(),__FUNCTION__,message);

	if(m_masterMesure->Ortho() != NULL)
		m_masterMesure->Ortho()->UnLoadGrilles();

	return true;
}
//-----------------------------------------------------------------------------
XContrainte* XOrthoLaserPoint::CreateContrainte(XOrthoLaserMesure* mes)
{
	if(mes == NULL)
	{
		XErrorError(Error(),__FUNCTION__,"mes == NULL");
		return NULL;
	}
	if(mes->Ortho() == NULL)
	{
		XErrorError(Error(),__FUNCTION__,"mes->Ortho() == NULL");
		return NULL;
	}
	if(mes->Ortho()->Rxp() == NULL)
	{
		XErrorError(Error(),__FUNCTION__,"mes->Ortho()->Rxp() == NULL");
		return NULL;
	}
	if(mes->Ortho()->Rxp()->Solution() == NULL)
	{
		XErrorError(Error(),__FUNCTION__,"mes->Ortho()->Rxp()->Solution() == NULL");
		return NULL;
	}

	XContrainte::eTypeContrainte typ;
	XPt3D Translat;
	if(!IsMovable())// On connait les coordonnées définitives du point
	{
		typ = XContrainte::Event;
		Translat =  *this - mes->PositionLaser();
	}
	else
	{
		typ = XContrainte::TgpsDecal;
		Translat = m_masterMesure->PositionLaser() - mes->PositionLaser() ;
	}
	XContrainte* contrainte = new XContrainte(*this,typ,this->Id());
	contrainte->Translation = Translat;
	contrainte->time = mes->Time() + mes->Ortho()->CorOrthoLaserTime2Gps() + mes->Ortho()->Rxp()->Solution()->TimeCorDay2Week();
	//les points de controle doivent être transférés pour être compensés 
	//ils sont inactifs pour le recalage
	contrainte->actif = !this->Control();

	return contrainte;
}
//-----------------------------------------------------------------------------
std::vector<XContrainte*> XOrthoLaserPoint::CreateContraintes(ProjPostProcessSolution* sol)
{
	std::vector<XContrainte*> vec;
	std::vector<XOrthoLaserMesure*> valid = MesuresValid();

	for (uint32 i = 0; i < valid.size(); i++)
	{	
		if(valid[i]->Ortho()->Rxp()->Solution() != sol)
			continue;
		XContrainte* c = CreateContrainte(valid[i]);
		if(c!=NULL)
			vec.push_back(c);	
	}	
	return vec;
}

