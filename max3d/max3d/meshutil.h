
#ifndef MESHUTIL_H
#define MESHUTIL_H

#include "mesh.h"
#include "model.h"

class CMeshUtil{
public:
	CMesh *CreateMesh( CMaterial *material,CModel *model );
	CMesh *CreateSphereMesh( CMaterial *material,float radius );
	CMesh *CreateCapsuleMesh( CMaterial *material,float radius,float length );
	CMesh *CreateCylinderMesh( CMaterial *material,float radius,float length );
	CMesh *CreateBoxMesh( CMaterial *material,float width,float height,float depth );
};

#endif

