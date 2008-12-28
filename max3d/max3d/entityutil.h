
#ifndef ENTITYUTIL_H
#define ENTITYUTIL_H

#include "entity.h"

class CEntityUtil{
public:
	CEntity *LoadEntity( const string &path,int collType,float mass,CShader *sahder );
	CEntity *CreateSphere( CMaterial *material,float radius,int collType,float mass );
	CEntity *CreateCapsule( CMaterial *material,float radius,float length,int collType,float mass );
	CEntity *CreateCylinder( CMaterial *material,float radius,float length,int collType,float mass );
	CEntity *CreateBox( CMaterial *material,float width,float height,float depth,int collType,float mass );
};

#endif
