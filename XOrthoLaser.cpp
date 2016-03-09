#include "XOrthoLaser.h"

#include "libXFileSystem/XSystemInfo.h"
#include "libXFileSystem/XPath.h"

#include <fstream>
//-----------------------------------------------------------------------------
bool XOrthoFloat::Load(std::string filename)
{
	uint32 nbpix = parent->OrthoIn.NumPix();
	data = new float[nbpix];
	std::ifstream fic;
	fic.open(filename.c_str(), std::ios::binary);
	if (!fic.good())//Le fichier n'existe pas
		return XErrorError(parent->m_error,__FUNCTION__," Erreur ouverture ",filename.c_str());
	fic.read((char *) data, nbpix * sizeof(float));
	if (!fic.good())
		return XErrorError(parent->m_error,__FUNCTION__," Erreur de lecture ",filename.c_str());
	fic.close();
	return true;
}
//-----------------------------------------------------------------------------
XOrthoFloat::~XOrthoFloat(void)
{
	if(data != NULL)
		delete [] data;
}
//-----------------------------------------------------------------------------
float XOrthoFloat::GetPixValue(uint16 col, uint16 lig)
{
	float* val = data + lig*parent->OrthoIn.Width() + col;
	float value = *val;
	return value;
}
//-----------------------------------------------------------------------------
bool XOrthoFloat::WriteFile(std::string filename)
{
	std::ofstream binary_file(filename.c_str(),std::ios::out|std::ios::binary);
	if (!binary_file.good())
		return XErrorError(parent->m_error,__FUNCTION__," Erreur ouverture ",filename.c_str());

	binary_file.write((char*)data, parent->OrthoIn.NumPix()*sizeof (float));
    binary_file.close();
	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------




















//********************************************************  A VOIR  !!!!!!!!!!!!!!!!!!
XOrthoLaser::XOrthoLaser(XError* error): m_Z(this),m_H(this), OrthoIn(error)
{
	m_error = error;

}
//-----------------------------------------------------------------------------
std::string XOrthoLaser::ZdataFile()
{
	XPath P;
	return P.ChangeExtension(P.InsertBeforeExt(m_filename.c_str(),"_Z").c_str(),"bin");
}
//-----------------------------------------------------------------------------
std::string XOrthoLaser::HdataFile()
{
	XPath P;
	return P.ChangeExtension(P.InsertBeforeExt(m_filename.c_str(),"_H").c_str(),"bin");
}
//-----------------------------------------------------------------------------
XOrthoLaser::~XOrthoLaser(void)
{
	OrthoIn.DeAllocate();
}
//-----------------------------------------------------------------------------
bool XOrthoLaser::Load_All(std::string filename)
{
	if(!OrthoIn.ReadFile(filename.c_str()))
		return  XErrorError(m_error,__FUNCTION__,"Erreur au chargement de l'image ",filename.c_str());

	//TODO charger un des fichiers de georef pour avoir toutes les infos

	m_filename= filename;

	XSystemInfo system;	
	if(!system.FindFile(ZdataFile().c_str()))
		return  XErrorError(m_error,__FUNCTION__," Fichier non présent ",ZdataFile().c_str());
	m_Z.Load(ZdataFile());

	if(!system.FindFile(HdataFile().c_str()))
		return  XErrorError(m_error,__FUNCTION__," Fichier non présent ",HdataFile().c_str());
	m_H.Load(HdataFile());
	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaser::GetCandidats(uint16 col, uint16 lig, uint16 range, std::vector<XOrthoPoint>& vecCandidat)
{
	bool gauche = false;
	uint16 C,L;
	for(int l=-range; l< range +1; l++)
		for(int c=-range; c< 0; c++)
			if(AddCandidat(col+c,lig+l,abs(l)+ abs(c),vecCandidat))
				gauche = true;
	
	for(int l=-range; l< 0; l++)
		if(AddCandidat(col,lig+l,abs(l),vecCandidat))
			gauche = true;
			
	if(!gauche)
		return false;

	bool droite = false;
	for(int l=-range; l< range+1; l++)
		for(int c=1; c< range+1; c++)
			if(AddCandidat(col+c,lig+l,abs(l)+ abs(c),vecCandidat))
				droite = true;	
	
	for(int l=1; l< range+1; l++)
		if(AddCandidat(col,lig+l,abs(l),vecCandidat))
			droite = true;	

	if(!droite)
		return false;

	//on a forcement encadré le point 
	return true;
}

//-----------------------------------------------------------------------------
bool XOrthoLaser::AddCandidat(uint16 C, uint16 L, float coef, std::vector<XOrthoPoint>& vecCandidat)
{
	unsigned char* pix = OrthoIn.Pixels() + L*OrthoIn.Width() + C;
	if(*pix ==0)
		return false;
	
	XOrthoPoint xbtc;
	xbtc.val = *pix;
	xbtc.coef = coef;
	xbtc.Z = m_Z.GetPixValue(C,L);
	xbtc.H = m_H.GetPixValue(C,L);
	vecCandidat.push_back(xbtc);

	return true;
}
//-----------------------------------------------------------------------------
bool XOrthoLaser::GetCandidats2(uint16 col, uint16 lig, std::vector<XOrthoPoint>& vecCandidat)
{	
	std::vector<uint16> position;
	if(AddCandidat(col-1, lig-1, 2,vecCandidat))
		position.push_back(0);
	if(AddCandidat(col, lig-1, 1,vecCandidat))
		position.push_back(1);
	if(AddCandidat(col+1, lig-1, 2,vecCandidat))
		position.push_back(2);
	if(AddCandidat(col+1, lig, 1,vecCandidat))
		position.push_back(3);
	if(AddCandidat(col+1, lig+1, 2,vecCandidat))
		position.push_back(4);
	if(AddCandidat(col, lig+1, 1,vecCandidat))
		position.push_back(5);
	if(AddCandidat(col-1, lig+1, 2,vecCandidat))
		position.push_back(6);
	if(AddCandidat(col-1, lig, 1,vecCandidat))
		position.push_back(7);
	

	if(vecCandidat.size() < 2)
		return false;

	for(uint32 i=0; i< position.size()-1; i++)
		for(uint32 j=i+1; j< position.size(); j++)
			if((position[j]-position[i] > 2)&&(position[j]-position[i] < 6))//le point est encadré 
				return true;

	return false;
}

//-----------------------------------------------------------------------------
XOrthoPoint XOrthoLaser::ProcessCandidats( std::vector<XOrthoPoint>& vecCandidat)
{
	uint32 nbcandisats = vecCandidat.size();
	//moyenne pondérée des altitudes
	double SZ =0;
	double Sdist = 0;
	for(uint32 i=0; i< vecCandidat.size(); i++)
	{
		SZ += vecCandidat[i].Z;
		Sdist += vecCandidat[i].coef;
	}
	double Zpond = 0;
	for(uint32 i=0; i< vecCandidat.size(); i++)
		Zpond += vecCandidat[i].Z * (double)vecCandidat[i].coef/Sdist;

	//On cherche le candidat le + proche en Z du Z pondéré
	XOrthoPoint plusProche = vecCandidat[0];
	double diff = abs(plusProche.Z -  vecCandidat[0].Z);
	for(uint32 i=1; i< vecCandidat.size(); i++)
		if(abs(plusProche.Z -  vecCandidat[i].Z) < diff)
			plusProche = vecCandidat[i];

	//on elimine les candidats dont le Z est trop éloigné du meilleur candidat
	double deltaZmax = 0.05;
	double Zref = plusProche.Z;
	std::vector<XOrthoPoint> vec_plan;
	for(uint32 i=0; i< vecCandidat.size(); i++)
	{
		double dZ = abs(vecCandidat[i].Z- Zref);
		if( dZ < deltaZmax)
			vec_plan.push_back(vecCandidat[i]);
	}

	//on recalcule les moyennes pondérées
	uint32 nbInPlan = vec_plan.size();
	Sdist = 0;
	for(uint32 i=0; i< vec_plan.size(); i++)
		Sdist += vec_plan[i].coef;
	
	double valPix = 0;
	double Z = 0;
	double H = 0;
	for(uint32 i=0; i< vec_plan.size(); i++)
	{
		valPix += vec_plan[i].val * (double)vec_plan[i].coef/Sdist;
		Z += vec_plan[i].Z * (double)vec_plan[i].coef/Sdist;
		H += vec_plan[i].H * (double)vec_plan[i].coef/Sdist;
	}

	XOrthoPoint result;
	result.val = (unsigned char) valPix;
	result.Z = Z;
	result.H = H;
	return result;
}
//-------------------------------------------------------------------------
bool XOrthoLaser::BoucheTrous(uint16 nbIter)
{
	uint16 range = 1;
	XOrthoPoint* buf = new XOrthoPoint[OrthoIn.NumPix()];	
	for(uint32 iter=0; iter < nbIter; iter++)
	{
		memset(buf,0,OrthoIn.NumPix()*sizeof(XOrthoPoint));
		for(uint32 lig = range; lig < OrthoIn.Height() - range-1; lig++)
		{
			unsigned char* pix = OrthoIn.Pixels() + lig*OrthoIn.Width() +range;
			for(uint32 col = range; col < OrthoIn.Width() - range-1; col++)
			{
				if(*pix == 0)
				{
					std::vector<XOrthoPoint> vecCandidat;
	//				if(GetCandidats(col,lig,range,vecCandidat))
					if(GetCandidats2(col,lig,vecCandidat))
					{
						XOrthoPoint* trou = buf +lig*OrthoIn.Width() + col;
						*trou = ProcessCandidats(vecCandidat);
					}
				}
				pix = pix+1;
			}
		}
		//Merge
		XOrthoPoint* point = buf;	
		unsigned char* img = OrthoIn.Pixels();	
		float* Z = m_Z.data;
		float* H = m_H.data;
		for(uint32 i = 0; i < OrthoIn.NumPix() ; i++)
		{
			if(point->val != 0)
			{
				*img = point->val;
				*Z = point->Z;
				*H = point->H;
			}
			point++; img++; Z++; H++;
		}
	}
	delete buf;

	return true;
}

//-----------------------------------------------------------------------------
bool XOrthoLaser::Save_All(std::string FolderOutput)
{
	XPath P;
	if(FolderOutput.empty())//on ecrase les fichiers en entrée
		FolderOutput = P.Path(m_filename.c_str());
	P.AddPathSep(FolderOutput);
	std::string filename = FolderOutput +  P.Name(OrthoIn.FileName().c_str());
	if(!OrthoIn.WriteFile(filename.c_str()))
		return XErrorError(m_error,__FUNCTION__,"Erreur écriture ",filename.c_str());

	filename =  FolderOutput +  P.NameNoExt(OrthoIn.FileName().c_str())+ "_Z.bin";
	m_Z.WriteFile(filename);
	filename =  FolderOutput +  P.NameNoExt(OrthoIn.FileName().c_str())+ "_H.bin";
	m_H.WriteFile(filename);
	return true;
}