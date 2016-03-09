#pragma once
#include <string>
#include "libXImage2/XRawImage.h"

class XError;
class XOrthoLaser;

struct XOrthoPoint
{
	unsigned char val;
	float coef;
	float Z;
	float H;
};

class XOrthoFloat
{
public:
	XOrthoLaser* parent;
	float* data;
public:
	XOrthoFloat(XOrthoLaser* prt){parent = prt; data= NULL;}
	~XOrthoFloat(void);
	bool Load(std::string filename);
	float GetPixValue(uint16 col, uint16 lig);
	bool WriteFile(std::string filename);

};

class XOrthoLaser
{
public:
	XError* m_error;
	XRawImage OrthoIn ;

protected:
	std::string m_filename;//nom de la dalle ortho
	XOrthoFloat m_Z;
	XOrthoFloat m_H;

	bool AddCandidat(uint16 C, uint16 L, float coef, std::vector<XOrthoPoint>& vecCandidat);
	bool GetCandidats(uint16 col, uint16 lig, uint16 range, std::vector<XOrthoPoint>& vecCandidat);
	bool GetCandidats2(uint16 col, uint16 lig, std::vector<XOrthoPoint>& vecCandidat);

	XOrthoPoint ProcessCandidats( std::vector<XOrthoPoint>& vecCandidat);

public:
	XOrthoLaser(XError* error);
	~XOrthoLaser(void);

	std::string ZdataFile();
	std::string HdataFile();

	bool Load_All(std::string filename);
	bool BoucheTrous(uint16 nbIter = 1);
	bool Save_All(std::string FolderOutput ="");
};
