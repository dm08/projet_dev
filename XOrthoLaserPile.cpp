#include "XOrthoLaserPile.h"
#include "XOrthoLaserChantier.h"

#include "libXBase/XError.h"
#include "libXFileSystem/XPath.h"

#include "ReaderFilePosPac/ProjPostProcessSolution.h"
#include "ReaderFilePosPac/OptionsEchantillonnage.h"

#include <algorithm>

//-----------------------------------------------------------------------------
XOrthoLaserPile::XOrthoLaserPile(XOrthoLaserChantier* pparent)
{
	parent = pparent;
	m_masterDalle = NULL;
}
//-----------------------------------------------------------------------------
XOrthoLaserPile::~XOrthoLaserPile(void)
{
}
//-----------------------------------------------------------------------------
XError* XOrthoLaserPile::Error()
{
	return parent->Error();
}

//-----------------------------------------------------------------------------
std::string XOrthoLaserPile::Id()
{
	if(vecDalle.empty())
		return std::string("Pile vide !");
	return vecDalle[0]->strIdPile();
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserPile::NbMesures()
{
	uint32 count =0;
	for(uint32 i=0; i<vecDalle.size(); i++)
		count = count + vecDalle[i]->NbMesures();
	return count;
}
//-----------------------------------------------------------------------------
uint32 XOrthoLaserPile::NbMesuresValides()
{
	uint32 count =0;
	for(uint32 i=0; i<vecDalle.size(); i++)
		count += vecDalle[i]->NbMesuresValides();
	return count;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPile::IsIn(XPt2D P)
{
	if(vecDalle.empty())
		return false;
	return vecDalle[0]->IsIn(P);
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserPile::Center()
{
	if(vecDalle.empty())
		return XPt2D();
	return vecDalle[0]->Center();
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserPile::NW()
{
	if(vecDalle.empty())
		return XPt2D();
	return vecDalle[0]->NW();
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserPile::SW()
{
	if(vecDalle.empty())
		return XPt2D();
	return vecDalle[0]->SW();
}
//-----------------------------------------------------------------------------
XPt2D XOrthoLaserPile:: Coord(uint16 col, uint16 lig)
{
	if(vecDalle.empty())
		return XPt2D();
	return vecDalle[0]->Coord(col, lig);
}	
//-----------------------------------------------------------------------------
XPt3D XOrthoLaserPile::OffsetUsefull()
{
	if(parent==NULL)
		return XPt3D();
	return parent->OffsetUsefull();
}

//-----------------------------------------------------------------------------	
struct  TriChronoDalle
{
   bool operator() (XOrthoLaserDalle*  A, XOrthoLaserDalle* B)
   {
	   if(A->Date() == B->Date())
			return A->CenterTime()< B->CenterTime();
		return A->Date()< B->Date();
    };
};
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::InitMasterDalle()
{
	if(vecDalle.empty())
		return true;
	for(uint32 i=0; i<vecDalle.size(); i++)
		if(!vecDalle[i]->Initialize())
			return false;
		
	std::sort(vecDalle.begin(),vecDalle.end(),TriChronoDalle());
	m_masterDalle = vecDalle[0];
	return true;
}
//-----------------------------------------------------------------------------	
std::list<XOrthoLaserRxp*> XOrthoLaserPile::listRxp()
{
	std::list<XOrthoLaserRxp*> list;
	for(uint32 i=0; i<vecDalle.size(); i++)
		list.push_back(vecDalle[i]->Rxp());
	list.sort();
	list.unique();

	return list;
}
//-----------------------------------------------------------------------------	
std::list<ProjPostProcessSolution*> XOrthoLaserPile::listTrajecto()
{
	std::list<XOrthoLaserRxp*> rxp = listRxp();
	std::list<XOrthoLaserRxp*>::iterator iter;
	std::list<ProjPostProcessSolution*> list;
	for (iter = rxp.begin(); iter != rxp.end(); iter++)
		if(*iter != NULL)
			list.push_back((*iter)->Solution());
	list.sort();
	list.unique();
	return list;
}


//-----------------------------------------------------------------------------	
void PixelFusion(XPt2D p, byte* pix, byte* pix2,float* Z, float* Z2, float* H, float* H2, float3* XYZ, float3* XYZ2)
{
	if(*pix2 == 0)
		return;
	if(*pix == 0)
	{
		*pix = *pix2;
		*Z = *Z2;
		*H = *H2;
		*XYZ = *XYZ2;
		return;
	}
	//le point est déjà occupé
	double d = dist(p,XPt2D(XYZ->x,XYZ->y));
	double d2= dist(p,XPt2D(XYZ2->x,XYZ2->y));
	if(d < d2)//le nouveau point est vu de plus loin
		return;
	//if(*Z < *Z2)//le nouveau point est plus haut
	//	return;

	*pix = *pix2;
	*Z = *Z2;
	*H = *H2;
	*XYZ = *XYZ2;
}
//-----------------------------------------------------------------------------	
//version rapide sans test sur la distance au point
void PixelFusion2(byte* pix, byte* pix2,float* Z, float* Z2, float* H, float* H2)
{	
	*pix = *pix2;
	*Z = *Z2;
	*H = *H2;
}
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::DoFusionHZ_Monochrome(std::string OutputFolder,std::ofstream* outMetadata,XError* error)
{
	for(uint32 i=1; i<vecDalle.size(); i++)
	{
		if(!vecDalle[i]->Load_Auto())
			return false;
		byte* pix = vecDalle[0]->OrthoLas()->Pixels();
		float* Z = vecDalle[0]->Zgrid()->Pixels();
		float* H = vecDalle[0]->Hgrid()->Pixels();

		byte* pix2 = vecDalle[i]->OrthoLas()->Pixels();
		float* Z2 = vecDalle[i]->Zgrid()->Pixels();
		float* H2 = vecDalle[i]->Hgrid()->Pixels();
		for(uint32 j=0; j< vecDalle[0]->OrthoLas()->Height(); j++)
		{
			for(uint32 k=0; k< vecDalle[0]->OrthoLas()->Width(); k++)
			{
				if(*pix2 != 0)
					PixelFusion2(pix,pix2,Z,Z2,H,H2);
				pix++;Z++;H++,pix2++;Z2++;H2++;
			}
		}
		vecDalle[i]->Unload_All();
	}
	vecDalle[0]->Write_All(OutputFolder);
	vecDalle[0]->Unload_All();

	return true;
}
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::DoFusionHZ_Color(std::string OutputFolder,std::ofstream* outMetadata,XError* error)
{
	XRawImage color(vecDalle[0]->OrthoLas()->Width(),vecDalle[0]->OrthoLas()->Height(),8,3,Error());
	color.Fill0Value();

	for(uint32 i=0; i<vecDalle.size(); i++)
	{
		if(!vecDalle[i]->Load_Auto())
			return false;
		byte* pix = color.Pixels() + i%3;
		float* Z = vecDalle[0]->Zgrid()->Pixels();
		float* H = vecDalle[0]->Hgrid()->Pixels();

		byte* pix2 = vecDalle[i]->OrthoLas()->Pixels();
		float* Z2 = vecDalle[i]->Zgrid()->Pixels();
		float* H2 = vecDalle[i]->Hgrid()->Pixels();
		for(uint32 j=0; j< color.Height(); j++)
		{
			for(uint32 k=0; k< color.Width(); k++)
			{
				if(*pix2 != 0)
					PixelFusion2(pix,pix2,Z,Z2,H,H2);
				
				pix = pix +3;Z++;H++,pix2++;Z2++;H2++;
			}
		}
		if(i!=0)
			vecDalle[i]->Unload_All();
	}
	XPath p;
	std::string newname = OutputFolder + p.Name(vecDalle[0]->File().c_str()) +".tif";

	vecDalle[0]->Write_All(OutputFolder);
	vecDalle[0]->Unload_All();
	return color.WriteFile(newname.c_str());;

}
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::DoFusionHZXYZ_Monochrome(std::string OutputFolder,std::ofstream* outMetadata,XError* error)
{
	for(uint32 i=1; i<vecDalle.size(); i++)
	{
		if(!vecDalle[i]->Load_Auto())
			return false;
		byte* pix = vecDalle[0]->OrthoLas()->Pixels();
		float* Z = vecDalle[0]->Zgrid()->Pixels();
		float* H = vecDalle[0]->Hgrid()->Pixels();
		float3* XYZ = vecDalle[0]->gridXYZlaser;

		byte* pix2 = vecDalle[i]->OrthoLas()->Pixels();
		float* Z2 = vecDalle[i]->Zgrid()->Pixels();
		float* H2 = vecDalle[i]->Hgrid()->Pixels();
		float3* XYZ2 = vecDalle[i]->gridXYZlaser;
		for(uint32 j=0; j< vecDalle[0]->OrthoLas()->Height(); j++)
		{
			for(uint32 k=0; k< vecDalle[0]->OrthoLas()->Width(); k++)
			{
				if(*pix2 != 0)
				{
					XPt2D p = Coord(k,j)- SW();
					PixelFusion(p,pix,pix2,Z,Z2,H,H2,XYZ,XYZ2);
				}
				pix++;Z++;H++;XYZ++;pix2++;Z2++;H2++;XYZ2++;
			}
		}
		vecDalle[i]->Unload_All();
	}
	vecDalle[0]->Write_All(OutputFolder);
	vecDalle[0]->Unload_All();

	return true;
}
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::DoFusionHZXYZ_Color(std::string OutputFolder,std::ofstream* outMetadata,XError* error)
{

	XRawImage color(vecDalle[0]->OrthoLas()->Width(),vecDalle[0]->OrthoLas()->Height(),8,3,Error());
	color.Fill0Value();

	for(uint32 i=0; i<vecDalle.size(); i++)
	{
		if(!vecDalle[i]->Load_Auto())
			return false;
		byte* pix = color.Pixels() + i%3;
		float* Z = vecDalle[0]->Zgrid()->Pixels();
		float* H = vecDalle[0]->Hgrid()->Pixels();
		float3* XYZ = vecDalle[0]->gridXYZlaser;

		byte* pix2 = vecDalle[i]->OrthoLas()->Pixels();
		float* Z2 = vecDalle[i]->Zgrid()->Pixels();
		float* H2 = vecDalle[i]->Hgrid()->Pixels();
		float3* XYZ2 = vecDalle[i]->gridXYZlaser;
		for(uint32 j=0; j< color.Height(); j++)
		{
			for(uint32 k=0; k< color.Width(); k++)
			{
				if(*pix2 != 0)
				{
					XPt2D p = Coord(k,j)- SW();
					PixelFusion(p,pix,pix2,Z,Z2,H,H2,XYZ,XYZ2);
				}
				
				pix = pix +3;Z++;H++;XYZ++,pix2++;Z2++;H2++;XYZ2++;
			}
		}
		if(i!=0)
			vecDalle[i]->Unload_All();
	}
	XPath p;
	std::string newname = OutputFolder + p.Name(vecDalle[0]->File().c_str()) +".tif";

	vecDalle[0]->Write_All(OutputFolder);
	vecDalle[0]->Unload_All();
	return color.WriteFile(newname.c_str());
}
//-----------------------------------------------------------------------------	
bool XOrthoLaserPile::ProcessFusion(std::string OutputFolder,bool monochrome,std::ofstream* outMetadata, XError* error)
{
	if(vecDalle.size() < 2)
		return vecDalle[0]->Copy_All(OutputFolder);

	if(!vecDalle[0]->Load_Auto())
		return false;

	if(vecDalle[0]->gridXYZlaser == NULL)
	{
		if(monochrome)
			return DoFusionHZ_Monochrome(OutputFolder,outMetadata,error);
		return DoFusionHZ_Color(OutputFolder,outMetadata,error);
	}
	
	if(!monochrome)
		return DoFusionHZXYZ_Color(OutputFolder,outMetadata,error);
	return DoFusionHZXYZ_Monochrome(OutputFolder,outMetadata,error);

}
//-----------------------------------------------------------------------------	
std::vector<XTranslatExpressSegment*> XOrthoLaserPile::Segments()
{
	XErrorInfo(Error(),__FUNCTION__,"Segments sur pile ",this->Id().c_str());
	std::vector<XTranslatExpressSegment*> segs;
	char message[1024];
	for(uint32 i=0; i<vecDalle.size(); i++)
	{
		float GpsTimeInDay  = vecDalle[i]->CenterTime() + parent->CorOrthoLaserTime2Gps(vecDalle[i]->Rxp()->Date());;
		XTranslatExpressSegment* seg =vecDalle[i]->Rxp()->Solution()->FindSegment(GpsTimeInDay);
		if(seg == NULL)
		{
			sprintf(message,"Segment NULL pour Dalle %s T=%.2lf",vecDalle[i]->strIdCouche().c_str(),GpsTimeInDay);
			XErrorError(Error(),__FUNCTION__,message);
			continue;
		}	
		segs.push_back(seg);
	}
	return segs;
}
//conversion utilisée dans Translat_Express pour visualiser les données en 3D
//-----------------------------------------------------------------------------
std::string XOrthoLaserPile::PlyFile()
{
	if(vecDalle.size() ==0)
		return std::string("Error.ply");
	std::string plyfileOut = vecDalle[0]->FolderForWriting() + Id() + ".ply";
	return plyfileOut;
}
//-----------------------------------------------------------------------------
bool XOrthoLaserPile::ExportToPly(double coefZ)
{
	if(vecDalle.size() ==0)
		return XErrorError(Error(),__FUNCTION__,"Pile sans dalle !");

	XPt3D offset = OffsetUsefull();
	if(offset == XPt3D())
		offset = SW();

	uint32 nbpix = 0;
	for(uint32 i=0; i<vecDalle.size(); i++)
		nbpix += vecDalle[i]->NbPixWithValue();
	vecDalle[0]->WritePlyEntete(PlyFile(),offset,nbpix);
	for(uint32 i=0; i<vecDalle.size(); i++)
		vecDalle[i]->WritePlyDatas(PlyFile(),offset,coefZ);
	
	return true;

}
//-----------------------------------------------------------------------------
bool XOrthoLaserPile::ExportToPlyIndiv(double coefZ)
{
	if(vecDalle.size() ==0)
		return XErrorError(Error(),__FUNCTION__,"Pile sans dalle !");

	XPt3D offset = OffsetUsefull();
	if(offset == XPt3D())
		offset = SW();

	for(uint32 i=0; i<vecDalle.size(); i++)
		vecDalle[i]->ExportToPly(coefZ);
	
	return true;

}
//-----------------------------------------------------------------------------
void XOrthoLaserPile::InfosConsole()
{
	if(vecDalle.empty())
		return;
	std::cout  << "\nInfos pile : << "<< Id() <<'\n';
	for(uint32 i=0; i<vecDalle.size(); i++)
		vecDalle[i]->InfosConsole();
	//float tref = vecDalle[0]->CenterTime();
	//char message[1024];
	//for(uint32 i=1; i<vecDalle.size(); i++)
	//{
	//	sprintf(message,"");
	//}

}
//-----------------------------------------------------------------------------
bool XOrthoLaserPile::ExportMesuresDalles(std::ostream* out)
{
	if(vecDalle.empty())
		return out->good();
	if(NbMesuresValides() == 0)
		return out->good();

	*out<< '\n' << Id()<< '\n' ;
	for(uint32 i=0; i<vecDalle.size(); i++)
		vecDalle[i]->ExportMesuresDalles(out);
	return out->good();
}
