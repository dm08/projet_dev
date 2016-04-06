#include "XOrthoLaserDalle.h"
#include "XOrthoLaserFolder.h"
#include "XOrthoLaserChantier.h"
#include "XOrthoLaser.h"

#include "XPath.h"
#include "XSystemInfo.h"

#include "XStringTools.h"

#include "XRawImage.h"

//#include "ProjPostProcessSolution.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <limits>

XOrthoLaserGrille::XOrthoLaserGrille(XOrthoLaserDalle* pparent, char value) 
{
	parent = pparent; 
	m_Image = NULL;
	m_value = value;
	minValue = -1;
	maxValue = -1;
	meanValue = -1;
}
//-----------------------------------------------------------------------------
XOrthoLaserGrille::~XOrthoLaserGrille()
{
	UnLoad();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserGrille::Load()
{
	std::string datafile = parent->File() + '_' + m_value + ".bin";
	std::ifstream data(datafile.c_str(),std::ios::binary);
	if(!data.good())
		return XErrorError(parent->Error(),__FUNCTION__," Erreur de chargement du fichier ",datafile.c_str());
	
	data.seekg (0, std::ios::end);	
	std::streamoff posData = data.tellg();
	uint32 sizeToLoad = parent->Width()* parent->Height()*sizeof(float);
	if(sizeToLoad != posData)
		return XErrorError(parent->Error(),__FUNCTION__,"La taille du fichier binaire ne correspond pas à l'ortho associée ",datafile.c_str());
	
	m_Image = new float[parent->Width()* parent->Height()];
	data.seekg (0, std::ios::beg);
	data.read((char*)m_Image,sizeToLoad);
	data.close();
	return true;
}
//-----------------------------------------------------------------------------
void XOrthoLaserGrille::UnLoad()
{
	if(m_Image!=NULL)
		delete m_Image;
	m_Image = NULL;
}
//-----------------------------------------------------------------------------
float XOrthoLaserGrille::GetValue(double x, double y)
{
	x = XMin((double)parent->Width()-1,XMax(0.,x));
	y = XMin((double)parent->Height()-1,XMax(0.,y));
	uint16 col1 = floor(x);
	uint16 lig1 = floor(y);
	double dcol = x-col1; 
	double dlig = y-lig1;
	uint16 col1plus1 = XMin((uint16)parent->Width()-1,col1+1);
	uint16 lig1plus1 = XMin((uint16)parent->Height()-1,lig1+1);

	float l1c1 = *(m_Image + (lig1*parent->Width() + col1));
	float l2c2 = *(m_Image + (lig1plus1*parent->Width() + col1plus1));
	float l2c1 = *(m_Image + (lig1plus1*parent->Width() + col1));
	float l1c2 = *(m_Image + (lig1*parent->Width() + col1plus1));
	if(l1c1 == 0)
		l1c1 = l1c2;
	if(l1c2 == 0)
		l1c2 = l1c1;
	float interl1 = l1c1 + dcol * (l1c2-l1c1);

	if(l2c2 == 0)
		l2c2 = l2c1;
	if(l2c1 == 0)
		l2c1 = l2c2;
	float interl2 = l2c1 + dcol * (l2c2-l2c1);
	if(interl1 == 0)
		interl1 = interl2;
	if(interl2 == 0)
		interl2 = interl1;

	float res = interl1 + dlig * (interl2 - interl1);
	
	return res;
}
//-----------------------------------------------------------------------------
float XOrthoLaserGrille::GetNearestValue(double x, double y)//ramene la valeur la plus proche
{
	x = XMin((double)parent->Width()-1,XMax(0.,x));
	y = XMin((double)parent->Height()-1,XMax(0.,y));
	XPt2D origine(x,y);
	uint32 col0 = floor(x);
	uint32 lig0 = floor(y);

	uint32 range = 10;
	uint32 colini = XMin(parent->Width()-1,XMax((uint32)0,col0-range));
	uint32 colfin = XMin(parent->Width()-1,XMax((uint32)0,col0+range));
	uint32 ligini = XMin(parent->Width()-1,XMax((uint32)0,lig0-range));
	uint32 ligfin = XMin(parent->Width()-1,XMax((uint32)0,lig0+range));

	float val;
	std::vector<float> vecval;;
	std::vector<float> vecdist;
	for(uint32 lig = ligini; lig< ligfin+1; lig++)
	{
		for(uint32 col = colini; col< colfin+1; col++)
		{
			val = *(m_Image + (lig*parent->Width() + col));
			if(val != 0)
			{
				vecval.push_back(val);
				vecdist.push_back(dist(origine,XPt2D(col,lig)));
			}
		}
	}
	if(vecval.empty())
		return 0;
	val  = vecval[0];
	float dmin = vecdist[0];
	for(uint32 i=1; i<vecval.size(); i++)
		if(vecdist[i]<dmin)
		{
			dmin= vecdist[i];
			val = vecval[i];
		}
	
	return val;
}
//-----------------------------------------------------------------------------
float XOrthoLaserGrille::GetMeanValue()
{
	if(meanValue != -1)
		return meanValue;

	if((m_Image==NULL)&&(!Load()))
		return 0;
	float* pix = m_Image;
	uint32 sizeImage = parent->Width()*parent->Height();
	uint32 count = 0;
	double somme = 0.;
	float first = 0;
	for(uint32 i=0; i<sizeImage; i++)
	{
		if(*pix != 0)
		{
			if(first ==0)
				first = *pix;
			somme += (*pix)-first;
			count++;
		}
		pix++;
	}
	float moy = somme/count;
	moy = moy+first;
	meanValue = moy;
	return moy;
}
//-----------------------------------------------------------------------------
float XOrthoLaserGrille::GetMinValue()
{
	if(minValue != -1)
		return minValue;
	if((m_Image==NULL)&&(!Load()))
		return 0;
	float* pix = m_Image;
	float valMin = (std::numeric_limits<float>::max)();
	uint32 sizeImage = parent->Width()*parent->Height();
	for(uint32 i=0; i<sizeImage; i++)
	{
		if((*pix != 0)&&(*pix < valMin))
			valMin = *pix;
		pix++;
	}
	minValue = valMin;
	return valMin;
}
//-----------------------------------------------------------------------------
float XOrthoLaserGrille::GetMaxValue()
{
	if(maxValue != -1)
		return maxValue;
	if((m_Image==NULL)&&(!Load()))
		return 0;
	float* pix = m_Image;
	float valMax = *pix;
	//float valMax = std::numeric_limits<float>::max();
	uint32 sizeImage = parent->Width()*parent->Height();
	for(uint32 i=0; i<sizeImage; i++)
	{
		if((*pix != 0)&&(*pix > valMax))
			valMax = *pix;
		pix++;
	}
	maxValue = valMax;
	return valMax;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserGrille::WriteBin(std::string OutFilename)
{
	std::ofstream binary_file(OutFilename.c_str(),std::ios::out|std::ios::binary);
	if (!binary_file.good())
		return XErrorError(parent->Error(),__FUNCTION__," Erreur ouverture ",OutFilename.c_str());

	uint32 nbpix = parent->Width()*parent->Height();
	binary_file.write((char*)m_Image, nbpix*sizeof (float));
    binary_file.close();
	return true;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserGrille::NbPixWithValue()//nombre de pixel ayant une valeur
{
	if((m_Image==NULL)&&(!Load()))
		return 0;
	float* pix = m_Image;
	uint32 count=0;
	for(uint32 i=0; i<parent->Width()*parent->Height(); i++)
	{
		if(*pix != 0)
			count++;
		pix++;
	}
	return count;
}

//-----------------------------------------------------------------------------
//**************** XOrthoLaserDalle *******************************************
//-----------------------------------------------------------------------------
XOrthoLaserDalle::XOrthoLaserDalle(XOrthoLaserFolder* pparent)
{
	gridH = new XOrthoLaserGrille(this,'H');
	gridZ = new XOrthoLaserGrille(this,'Z');

	gridXYZlaser = NULL;
	parent=pparent;
	m_nDate = 0;
	m_centerTime = 0;//Heure au centre de la dalle ou Heure moyenne
	m_centerZ = 0;//Z au centre de la dalle ou Z moyen
	rxp= NULL;//implémentation dans Translat Express
}
//-----------------------------------------------------------------------------
XOrthoLaserDalle::~XOrthoLaserDalle(void)
{
	delete gridH;
	delete gridZ;
	if(gridXYZlaser != NULL) 
		delete gridXYZlaser;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::LoadXYZlaser()
{
	uint32 sizeToLoad = Width()* Height()* sizeof(float3);
	if(gridXYZlaser == NULL)
		gridXYZlaser = new float3[sizeToLoad];
	std::string datafile = xyzLaserfile();
	std::ifstream data(datafile.c_str(),std::ios::binary);
	if(!data.good())
		return XErrorError(parent->Error(),__FUNCTION__," Erreur de chargement du fichier ",datafile.c_str());
	
	data.seekg (0, std::ios::end);	
	std::streamoff posData = data.tellg();
	if(sizeToLoad != posData)
		return XErrorError(parent->Error(),__FUNCTION__,"La taille du fichier binaire ne correspond pas à l'ortho associée ",datafile.c_str());
	data.seekg (0, std::ios::beg);
	data.read((char*)gridXYZlaser,sizeToLoad);
	data.close();
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::WriteXYZlaser(std::string OutFilename)
{
	std::ofstream binary_file(OutFilename.c_str(),std::ios::out|std::ios::binary);
	if (!binary_file.good())
		return XErrorError(Error(),__FUNCTION__," Erreur ouverture ",OutFilename.c_str());

	uint32 sizeToWrite = Width()* Height()* sizeof(float3);
	binary_file.write((char*)gridXYZlaser, sizeToWrite);
    binary_file.close();
	return true;
}
//-----------------------------------------------------------------------------
XError* XOrthoLaserDalle::Error(){	return parent->Error();}
//-----------------------------------------------------------------------------
XOrthoLaserChantier* XOrthoLaserDalle::Chantier() {return parent->Chantier();}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::Folder(){return parent->Folder();}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::FolderForWriting(){return parent->FolderForWriting();}

//-----------------------------------------------------------------------------
uint32 XOrthoLaserDalle::ExtractDate(std::string& s)
{
	if(!isdigit((char)s[0]) )
		return 0;
			
	uint32 count = 0;
	//Les TS mettent la date comme nom de fichier RXP 
	//on limite l'extraction de la date aux 6 premiers caractères cf XRieglFichier::Date() qui encode la date
	for(uint32 i= 0; i< 6; i++)
	{
		if(!isdigit((char)s[i]) )
			break;
		count++;
	}
	if(count != 6)
		XErrorAlert(Error(),__FUNCTION__,"Nomenclature non conforme sur encodage de la date");
	std::string strInteger = s.substr(0,count);
	return atoi(strInteger.c_str());
}
//-----------------------------------------------------------------------------
XFrame XOrthoLaserDalle::Frame()
{
	return m_frame;
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserDalle::Coord(uint16 col, uint16 lig)
{
	return XPt2D(m_frame.Xmin + col*m_dResol, m_frame.Ymax - lig*m_dResol);
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserDalle::Coord(float3* XYZlaser)
{
	return XPt2D( SW().X + XYZlaser->x, SW().Y + XYZlaser->y);
}
//-----------------------------------------------------------------------------
//renvoie les coordonnées en pixels d'un point terrain
XPt2D XOrthoLaserDalle::CoordPix(XPt2D pTerrain)
{
	if(!m_frame.IsIn(pTerrain))
		return XPt2D();
	double x = (pTerrain.X-m_frame.Xmin)/m_dResol;
	double y = (m_frame.Ymax-pTerrain.Y)/m_dResol;
	return XPt2D(x,y);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::SetName(std::string nameNoExt)
{
	//on décode la nomenclature :   0663450-6861300-040_140702LePerreux3_LAMB93_0
	XPath p;
	std::string::size_type pos = nameNoExt.find('_');
	if (pos == std::string::npos)
		return false;
	std::string bloc1 = nameNoExt.substr(0, pos);
	std::string bloc2 = nameNoExt.substr(pos + 1);
	pos = bloc1.find('-');
	if (pos == std::string::npos)
		return false;
	std::string str = bloc1.substr(0, pos);
	uint32 val;
	sscanf(str.c_str(),"%d", &val); 
	m_frame.Xmin = val;
	bloc1 = bloc1.substr(pos + 1);
	pos = bloc1.find('-');
	if (pos == std::string::npos)
		return false;
	str = bloc1.substr(0, pos);
	sscanf(str.c_str(),"%d", &val); 
	m_frame.Ymin = val;
	str = bloc1.substr(pos + 1);
	sscanf(str.c_str(),"%d", &val); 
	m_dResol = (double)val/1000.;

	if(Width() !=0)
		m_frame.Xmax = m_frame.Xmin + Width() * m_dResol;
	if(Height() !=0)
		m_frame.Ymax = m_frame.Ymin + Height() * m_dResol;

	pos = bloc2.rfind('_');
	if (pos == std::string::npos)
		return false;
	str = bloc2.substr(pos + 1);
	sscanf(str.c_str(),"%d", &val); 
	m_nIncrement = val;

	bloc2 =  bloc2.substr(0,pos);
	pos = bloc2.rfind('_');
	if (pos == std::string::npos)
		return false;
	m_strProjection = bloc2.substr(pos+1);
	m_strRxpName = bloc2.substr(0, pos);
	m_nDate = ExtractDate(m_strRxpName);
	if(m_nDate !=0)
	{
		XStringTools st;
		m_strRxpName = m_strRxpName.substr(st.numberOfDigit(m_nDate));
	}
		
	m_strName=nameNoExt;
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::SetDimension(unsigned int w, unsigned int h, unsigned int bps, unsigned int numChannel)
{
	OrthoLas()->SetDimension(w,h,bps,numChannel);
	if(Width() !=0)
		m_frame.Xmax = m_frame.Xmin + Width() * m_dResol;
	if(Height() !=0)
		m_frame.Ymax = m_frame.Ymin + Height() * m_dResol;
	return true;
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::File()
{
	return parent->Folder() + m_strName;
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::OrthoFile()
{
	return File() + ".tif";
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::Hfile()
{
	return File() + "_H.bin";
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::xyzLaserfile()
{
	return File() + "_xyzLaser.bin";
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::Zfile()
{
	return File() + "_Z.bin";
}
//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::PlyFile()
{
	std::string plyfileOut = FolderForWriting()+ m_strName + ".ply";
	return plyfileOut;
}
//-----------------------------------------------------------------------------
XOrthoLaserMesure* XOrthoLaserDalle::GetMesure(XOrthoLaserPoint* P)
{
	if(P == NULL)
		return NULL;
	XOrthoLaserMesure* xolm = FindMesure(P);
	if(xolm == NULL)
	{
		xolm = new XOrthoLaserMesure(P,this);
		m_Mesure.push_back(xolm);
		P->AddMesure(xolm);
	}
	if(xolm->Valid())//mesure déjà faite
		return xolm;
	
	xolm->X = (P->X-m_frame.Xmin)/this->m_dResol;
	xolm->Y = (m_frame.Ymax-P->Y)/this->m_dResol;
	if(P->Z == 0)
		P->Z = GetBestZ(xolm->X,xolm->Y);
	
	return xolm;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::GetMesure(XPt3D pTerrain, XPt2D* pImage)
{
	if (!m_frame.IsIn(pTerrain))
		return false;// Le point est hors image

	pImage->X = (pTerrain.X-m_frame.Xmin)/this->m_dResol;
	pImage->Y = (m_frame.Ymax-pTerrain.Y)/this->m_dResol;
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::ManageLoading(bool *HtoUnload, bool* ZtoUnload)
{
	*HtoUnload = gridH->IsLoaded();
	if((!gridH->IsLoaded())&& (!gridH->Load()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille H ",Hfile().c_str());
	*ZtoUnload = gridZ->IsLoaded();
	if((!gridZ->IsLoaded())&& (!gridZ->Load()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille Z ",Zfile().c_str());
	return true;
}
//-----------------------------------------------------------------------------
void XOrthoLaserDalle::ManageUnLoading(bool HtoUnload, bool ZtoUnload)
{
	if(HtoUnload)
		gridH->UnLoad();
	if(ZtoUnload)
		gridZ->UnLoad();

}
//-----------------------------------------------------------------------------
void XOrthoLaserDalle::InfosConsole()
{
	bool HtoUnload,ZtoUnload;
	if(!ManageLoading(&HtoUnload,&ZtoUnload))
		return;

	std::cout  << "\nDalle << "<< Name();
	float zmin = gridZ->GetMinValue();
	float zmax = gridZ->GetMaxValue();
	float zmoy = gridZ->GetMeanValue();
	float tmin = gridH->GetMinValue();
	float tmax = gridH->GetMaxValue();
	float tmoy = gridH->GetMeanValue();
	char message[1024];
	sprintf(message,"\nT min= %.0lf max= %.0lf (%.0lf s) %d",tmin,tmax,tmax-tmin,Rxp()->Date());
	std::cout  << message;
	sprintf(message,"\nZ min= %.1lf max= %.1lf (%.1lf m)",zmin,zmax,zmax-zmin);
	std::cout  << message;
	std::cout << '\n';

	ManageUnLoading(HtoUnload,ZtoUnload);
}

//-----------------------------------------------------------------------------
std::string XOrthoLaserDalle::InfosPointe(XPt2D mesure)
{
	//LoadTime(); 
	std::ostringstream oss;
	oss << "\nPointe sur dalle " << Name() << '\n';
	if((mesure.X > Width())||(mesure.Y > Height()))
	{
		oss << "mesure hors image";
		return oss.str();
	}

	float Z = GetZ(mesure.X,mesure.Y);
	if(Z!= 0)
		oss << "  Altitude pointe : " << Z << " m\n";		
	if(Z== 0)
	{
		Z = GetNearestZ(mesure.X,mesure.Y);
		if(Z!= 0)
			oss << "  Altitude proche du pointé : " << Z << " m\n";		
	}
	if(Z== 0)
		oss << "  Altitude moyenne de la dalle : " << m_centerZ << " m\n";	

	float H = GetH(mesure.X,mesure.Y);
	if(H!= 0)
		oss << "  Heure pointe : " << H << '\n';		
	if(H== 0)
	{
		H = GetNearestH(mesure.X,mesure.Y);
		if(H!= 0)
			oss << "  Heure proche du pointe : " << H << '\n';	
	}
	if(H== 0)
		oss << "  Heure moyenne de la dalle : " << m_centerTime << '\n';	

	return oss.str();
}

//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::RemoveMesure(XOrthoLaserMesure* mes)
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
XOrthoLaserMesure* XOrthoLaserDalle::FindMesure(XOrthoLaserPoint* P)
{
	for(uint32 i=0; i< m_Mesure.size(); i++)
		if(m_Mesure[i]->Pt() == P)
			return m_Mesure[i];

	return NULL;
}	
//-----------------------------------------------------------------------------
uint32 XOrthoLaserDalle::NbMesuresValides()
{
	uint32 count =0;
	for(uint32 i=0; i< m_Mesure.size(); i++)
		if(m_Mesure[i]->Valid() )
			count++;
	return count;
}	 
//-----------------------------------------------------------------------------
std::vector<XOrthoLaserMesure*> XOrthoLaserDalle::ListMesuresValides()
{
	std::vector<XOrthoLaserMesure*> vec;
	for(uint32 i=0; i< m_Mesure.size(); i++)
		if(m_Mesure[i]->Valid() )
			vec.push_back(m_Mesure[i]);
	return vec;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserDalle::NbLiaisonsValides(XOrthoLaserDalle* dalle)
{
	if((dalle== NULL)||(dalle== this))
		return 0;
	uint32 count = 0;
	for(uint32 i=0; i< m_Mesure.size(); i++)
	{	XOrthoLaserMesure* mes = m_Mesure[i]->Pt()->FindMesure(dalle);
		if((mes != NULL)&&(mes->Valid()))
			count++;
	}
	return count;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::AddMesure(XOrthoLaserMesure* mes)
{
	if((mes->Pt() != NULL) && (mes->Pt()->X == 0))
		if(!GetCoordTerrain(mes->X,mes->Y,mes->Pt()))
			return XErrorError(Error(),__FUNCTION__," Erreur de GetCoordTerrain ");
	
	if((mes->Time()==0.)&&(!SetTime(mes)))
		return XErrorError(Error(),__FUNCTION__," Erreur de SetTime ");

	m_Mesure.push_back(mes);
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::SetTime(XOrthoLaserMesure* mes)
{
	if(mes->X > Width())
		return XErrorError(Error(),__FUNCTION__," mesure hors image en X");
	if(mes->Y > Height())
		return XErrorError(Error(),__FUNCTION__," mesure hors image en Y");

	mes->Time(GetH(mes->X, mes->Y));
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Load_All(bool all_grids)
{
	if(!this->m_OrthoLas.ReadFile(OrthoFile().c_str()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de l'image ",OrthoFile().c_str());

	return LoadGrilles(all_grids);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Load_Auto()
{
	if(!this->m_OrthoLas.ReadFile(OrthoFile().c_str()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de l'image ",OrthoFile().c_str());

	return LoadGrillesAuto();
}
//-----------------------------------------------------------------------------
void XOrthoLaserDalle::Unload_All()
{
	m_OrthoLas.DeAllocate();
	this->UnLoadGrilles();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::LoadGrilles(bool all_grids)
{
	bool HtoUnload,ZtoUnload;
	if(!ManageLoading(&HtoUnload,&ZtoUnload))
		return false;

	if(!all_grids)
		return true;
	if(!m_bgridXYZlaserAvailable)
		return  XErrorError(Error(),__FUNCTION__,"manque la grille xyzLaser ",xyzLaserfile().c_str());

	if(!LoadXYZlaser())
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille xyzLaser ",xyzLaserfile().c_str());
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::LoadGrillesAuto()
{
	bool HtoUnload,ZtoUnload;
	if(!ManageLoading(&HtoUnload,&ZtoUnload))
		return false;

	if(!m_bgridXYZlaserAvailable)
		return true;

	if(!LoadXYZlaser())
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille xyzLaser ",xyzLaserfile().c_str());
	return true;
}
//-----------------------------------------------------------------------------
void XOrthoLaserDalle::UnLoadGrilles()
{
	gridZ->UnLoad();
	gridH->UnLoad();
	if(gridXYZlaser != NULL)
	{
		delete gridXYZlaser;
		gridXYZlaser = NULL;
	}

}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::GetCoordTerrain(double col, double lig, XPt3D* pt)
{
	XPt3D p;
	if(col > Width())
		return XErrorError(Error(),__FUNCTION__," mesure hors image en X");
	if(lig > Height())
		return XErrorError(Error(),__FUNCTION__," mesure hors image en Y");
	pt->X = m_frame.Xmin + col*m_dResol;
	pt->Y = m_frame.Ymax - lig*m_dResol;
	pt->Z = GetBestZ(col, lig);
	return true;
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetBestZ(double col, double lig )
{
	if(!gridZ->IsLoaded())
		gridZ->Load();

	float Z = GetZ(col,lig);
	if(Z== 0)//pas d'alti a cet endroit
		Z = GetNearestZ(col,lig);
	if(Z!= 0)
		return Z;
	
	return  CenterZ();
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetZ(double col, double lig)
{
	if(!gridZ->IsLoaded())
		gridZ->Load();

	return  gridZ->GetValue(col,lig);
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetNearestZ(double col, double lig)
{
	if(!gridZ->IsLoaded())
		gridZ->Load();

	return  gridZ->GetNearestValue(col,lig);
}

//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetBestH(double col, double lig )
{
	if(!gridH->IsLoaded())
		gridH->Load();

	float H = GetH(col,lig);
	if(H== 0)//pas d'heure a cet endroit
		H = GetNearestH(col,lig);
	if(H != 0)
		return H;

	return CenterTime();
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::CenterZ()
{
	if(m_centerZ == 0)
	{
		m_centerZ = gridZ->GetMeanValue();
		gridZ->UnLoad();
	}
	
	return m_centerZ;
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::CenterTime()
{
	if(m_centerTime == 0)
	{
		m_centerTime = gridH->GetMeanValue();
		 gridH->UnLoad();
	}
	
	return m_centerTime;
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetH(double col, double lig)
{
	if(!gridH->IsLoaded())
		gridH->Load();

	return  gridH->GetValue(col,lig);
}
//-----------------------------------------------------------------------------
float XOrthoLaserDalle::GetNearestH(double col, double lig)
{
	if(!gridH->IsLoaded())
		gridH->Load();

	return  gridH->GetNearestValue(col,lig);
}

//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::WriteInfoXml(std::ostream* out)
{
	*out << "<Dalle name=\""<< Name() ;
	*out <<"\" time=\"" << CenterTime() ;
	*out <<"\" w=\"" << Width() <<"\" h=\"" << Height() ;
	*out <<"\" bps=\"" << OrthoLas()->BPS() <<"\" nchan=\"" << OrthoLas()->NChannel() ;
	*out <<"\" ></Dalle>\n";
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::InitSize()
{
	if(m_OrthoLas.Width() != 0)
		return true; //on a déjà les infos via le fichier xml

	//on lit l'entete de l'image pour connaitre sa taille et calculer l'emprise
	if(!m_OrthoLas.ReadHeader(OrthoFile().c_str()))
		return XErrorError(Error(),__FUNCTION__," Erreur lecture de l'ortho ",OrthoFile().c_str());

	m_frame.Xmax = m_frame.Xmin + Width() * m_dResol;
	m_frame.Ymax = m_frame.Ymin + Height() * m_dResol;
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Initialize()
{
	if(!InitSize())
		return false;

	XSystemInfo system;
	if(!system.FindFile(Hfile().c_str()))
		return XErrorError(Error(),__FUNCTION__,"Fichier _H manquant ",Name().c_str());
	
	if(!system.FindFile(Zfile().c_str()))
		return XErrorError(Error(),__FUNCTION__,"Fichier _Z manquant ",Name().c_str());

	m_bgridXYZlaserAvailable = false;
	if(system.FindFile(xyzLaserfile().c_str()))
		m_bgridXYZlaserAvailable = true;
	

	if(m_centerTime ==0)
		LoadTime();

	return true;
}
//-----------------------------------------------------------------------------
void XOrthoLaserDalle::LoadTime()
{
	std::cout << '*';//patience
	m_centerTime = gridH->GetMeanValue();
	gridH->UnLoad();
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Write_All(std::string OutputFolder)
{
	XPath P;
	P.AddPathSep(OutputFolder);
	std::string filename = OutputFolder + P.Name(OrthoFile().c_str());

	if(!m_OrthoLas.WriteFile(filename.c_str()))
		return XErrorError(Error(),__FUNCTION__,"Erreur écriture ",filename.c_str());

	filename =  OutputFolder +  P.Name(Zfile().c_str());
	if(!gridZ->WriteBin(filename))
		return XErrorError(Error(),__FUNCTION__,"Erreur écriture ",filename.c_str());

	filename =  OutputFolder +  P.Name(Hfile().c_str());
	if(!gridH->WriteBin(filename))
		return XErrorError(Error(),__FUNCTION__,"Erreur écriture ",filename.c_str());

	if(gridXYZlaser != NULL)
	{
		filename =  OutputFolder +  P.Name(xyzLaserfile().c_str());
		if(!WriteXYZlaser(filename))
			return XErrorError(Error(),__FUNCTION__,"Erreur écriture ",filename.c_str());
	}
	return Copy_Georef(OutputFolder);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Copy_All(std::string OutputFolder)
{
	XPath P;
	XSystemInfo system;
	P.AddPathSep(OutputFolder);
	std::string filename = OutputFolder + P.Name(OrthoFile().c_str());
	if(!system.Copy_File(OrthoFile().c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	filename =  OutputFolder +  P.Name(Zfile().c_str());
	if(!system.Copy_File(Zfile().c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	filename =  OutputFolder +  P.Name(Hfile().c_str());
	if(!system.Copy_File(Hfile().c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	if(m_bgridXYZlaserAvailable)
	{
		filename =  OutputFolder +  P.Name(xyzLaserfile().c_str());
		if(!system.Copy_File(xyzLaserfile().c_str(),filename.c_str(),false))
			return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());
	}
	return Copy_Georef(OutputFolder);
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Copy_Georef(std::string OutputFolder)
{
	XPath P;
	XSystemInfo system;
	P.AddPathSep(OutputFolder);
	std::string inputfile = P.ChangeExtension(OrthoFile().c_str(),"gxt");
	std::string filename = OutputFolder + P.Name(inputfile.c_str());
	if(!system.Copy_File(inputfile.c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	inputfile = P.ChangeExtension(OrthoFile().c_str(),"tfw");
	filename = OutputFolder + P.Name(inputfile.c_str());
	if(!system.Copy_File(inputfile.c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	inputfile = P.ChangeExtension(OrthoFile().c_str(),"grf");
	filename = OutputFolder + P.Name(inputfile.c_str());
	if(!system.Copy_File(inputfile.c_str(),filename.c_str(),false))
		return XErrorError(Error(),__FUNCTION__,"Erreur recopie ",filename.c_str());

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::ExportMesuresDalles(std::ostream* out)
{
	if(m_Mesure.empty())
		return out->good();
	*out << Name() << std::endl;
	for(uint32 i=0; i< m_Mesure.size(); i++)
		m_Mesure[i]->WriteMesure(out);
	
	return out->good();
}
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserDalle::OffsetUsefull()
{
	if(parent == NULL)
		return XPt3D();
	return parent->OffsetUsefull();
}
//-----------------------------------------------------------------------------
uint16 XOrthoLaserDalle::CorOrthoLaserTime2Gps()
{
	if(parent == NULL)
		return 0;
	return parent->CorOrthoLaserTime2Gps(this->Date());
}

//conversion utilisée dans Translat_Express pour visualiser les données en 3D
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::WritePlyEntete(std::string plyfilename,XPt3D offset,uint32 nbPts)
{
	std::ofstream out(plyfilename.c_str(),std::ios::out);
	if(!out.good())
		return XErrorError(Error(),__FUNCTION__,"Erreur de creation fichier ", plyfilename.c_str());

	out << "ply" << '\n';
	out << "format binary_little_endian 1.0" << '\n';
	out << "comment offset position " << offset.X << ' ' << offset.Y << ' ' <<0 << '\n';
	out << "element vertex " <<std::setw(12)<<std::setfill('0')<< nbPts << '\n';
    out<<"property float32 x"<<'\n';
    out<<"property float32 y"<<'\n';
    out<<"property float32 z"<<'\n';
    out<<"property uchar red"<<'\n';
    out<<"property uchar green"<<'\n';
    out<<"property uchar blue"<<'\n';  
	out << "end_header"<< '\n';
	out.close();
	return true;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserDalle::NbPixWithValue()//nombre de pixel ayant une vraie valeur
{
	//On regarde dans la grille des heures 
	//(l'image pouvant être en couleur, la valeur serait x3 )
	//actuellement les heures sont interpolées en même temps que le reste.
	//on récupère donc le même nombre de valeur que pour l'image

	bool HtoUnload = gridH->IsLoaded();
	if((!gridH->IsLoaded())&& (!gridH->Load()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille H ",Hfile().c_str());

	uint32 count = gridH->NbPixWithValue();
	if(HtoUnload)
		gridH->UnLoad();
	return count;

}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::WritePlyDatas(std::string plyfilename,XPt3D offset,float coefZ)
{
	bool ZtoUnload = gridZ->IsLoaded();
	if((!gridZ->IsLoaded())&& (!gridZ->Load()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement de la grille Z ",Zfile().c_str());

	bool OrthoToUnload = m_OrthoLas.IsLoaded();
	if((!m_OrthoLas.IsLoaded())&&(!m_OrthoLas.ReadFile(OrthoFile().c_str())))
		return XErrorError(Error(),__FUNCTION__,"Erreur de chargement de l'image ",OrthoFile().c_str());

	std::ofstream out;
	out.open(plyfilename.c_str(), std::ios::binary | std::ios::app);
	XPt3D pt;
	unsigned char* val;
	uint32 count = 0;
	for(uint32 col=0; col< m_OrthoLas.Width(); col++)
	{
		for(uint32 lig=0; lig< m_OrthoLas.Height(); lig++)
		{
			val = m_OrthoLas.GetPix(col,lig);
			if(*val==0)
				continue;
			count++;
			GetCoordTerrain(col,lig,&pt);
			float dif = pt.X-offset.X;
			out.write((char*)&dif,sizeof(float)) ;
			dif = pt.Y-offset.Y;
			out.write((char*)&dif,sizeof(float)) ;
			dif = (pt.Z-offset.Z)*coefZ;
			out.write((char*)&dif,sizeof(float)) ;
			out.write((char*)val,sizeof(unsigned char)) ;
			out.write((char*)val,sizeof(unsigned char)) ;
			out.write((char*)val,sizeof(unsigned char)) ;
		}
	}
	out.close();
	//std::cout << "nombre de pixels ecrits" << count << '\n';

	//on remet dans l'état de chargement initial
	if(ZtoUnload)
		gridZ->UnLoad();
	if(OrthoToUnload)
		m_OrthoLas.DeAllocate();
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::ExportToPly(float coefZ)
{
	std::cout << "Export Ply " << Name() <<'\n';

	XPt3D offset = OffsetUsefull();
	if(offset == XPt3D())
		offset = SW();
	if(coefZ != 1)
		offset.Z = CenterZ(); //ne pas utiliser coefZ si plusieurs dalles !!

	if(!WritePlyEntete(PlyFile(),offset,NbPixWithValue()))
		return false;
	return WritePlyDatas(PlyFile(),offset,coefZ);

}
//-----------------------------------------------------------------------------
bool XOrthoLaserDalle::Bouchage()
{
	if(m_OrthoLas.NChannel() == 3)
		return XErrorError(Error(),__FUNCTION__,"Erreur de conversion ",OrthoFile().c_str());

	//çà c'est pas terrible.... je réutilise la classe utilisée dans BoucheOrtho !
	//il faudrait mutualiser tout çà dans la  classe XOrthoLaserDalle
	XOrthoLaser xol(Error());
	if(!xol.Load_All(OrthoFile().c_str()))
		return  XErrorError(Error(),__FUNCTION__,"Erreur au chargement des données ",OrthoFile().c_str());
	if(!xol.BoucheTrous())
		return false;
	if(!xol.Save_All())
		return false;
	return m_OrthoLas.ReadFile(OrthoFile().c_str());
}
//-----------------------------------------------------------------------------
// NO TRAJ std::string XOrthoLaserDalle::TrajectoName()
//{
//	if(rxp == NULL)
//		return std::string("RXP NULL !!");
//	if(rxp->Solution() == NULL)
//		return std::string("RXP->Traj NULL !!");
//	return rxp->Solution()->MissionName();
//}
