/*
Max3D
Copyright (c) 2008, Mark Sibly
All rights reserved.

Redistribution and use in source and binary forms, with or without
conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of Max3D's copyright owner nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>

#include "object.h"

const float PI=3.14159265359f;
const float TWOPI=PI*2;
const float HALFPI=PI/2;

struct CRect{
	float x,y,width,height;
	
	CRect():x(0),y(0),width(0),height(0){
	}
	
	CRect( float x,float y,float width,float height ):x(x),y(y),width(width),height(height){
	}
};

struct CVec2{
	float x,y;
	
	CVec2():x(0),y(0){
	}
	CVec2( float x,float y ):x(x),y(y){
	}

	float &operator[]( int i ){
		return *(&x+i);
	}
	float operator[]( int i )const{
		return *(&x+i);
	}
};

struct CVec3{
	float x,y,z;
	
	CVec3():x(0),y(0),z(0){
	}
	CVec3( float t ):x(t),y(t),z(t){
	}
	CVec3( float x,float y,float z ):x(x),y(y),z(z){
	}
	
	float &operator[]( int i ){
		return *(&x+i);
	}
	float operator[]( int i )const{
		return *(&x+i);
	}
	
	CVec3 operator-()const{
		return CVec3( -x,-y,-z ); 
	}
	CVec3 operator+( float t )const{
		return CVec3( x+t,y+t,z+t );
	}
	CVec3 operator-( float t )const{
		return CVec3( x-t,y-t,z-t );
	}
	CVec3 operator*( float t )const{
		return CVec3( x*t,y*t,z*t );
	}
	CVec3 operator/( float t )const{
		return CVec3( x/t,y/t,z/t );
	}
	CVec3 operator+( const CVec3 &t )const{
		return CVec3( x+t.x,y+t.y,z+t.z );
	}
	CVec3 operator-( const CVec3 &t )const{
		return CVec3( x-t.x,y-t.y,z-t.z );
	}
	CVec3 operator*( const CVec3 &t )const{
		return CVec3( x*t.x,y*t.y,z*t.z );
	}
	CVec3 operator/( const CVec3 &t )const{
		return CVec3( x/t.x,y/t.y,z/t.z );
	}
	
	CVec3 &operator+=( const CVec3 &v ){
		x+=v.x;y+=v.y;z+=v.z;return *this;
	}
	CVec3 &operator-=( const CVec3 &v ){
		x-=v.x;y-=v.y;z-=v.z;return *this;
	}
	CVec3 &operator*=( const CVec3 &v ){
		x*=v.x;y*=v.y;z*=v.z;return *this;
	}
	CVec3 &operator/=( const CVec3 &v ){
		x/=v.x;y/=v.y;z/=v.z;return *this;
	}
	CVec3 &operator*=( float t ){
		x*=t;y*=t;z*=t;return *this;
	}
	CVec3 &operator/=( float t ){
		x/=t;y/=t;z/=t;return *this;
	}
	bool operator==( const CVec3 &v )const{
		return (x==v.x && y==v.y && z==v.z);
	}
	
	float Dot( const CVec3 &t )const{ 
		return x*t.x+y*t.y+z*t.z;
	}
	float Length()const{
		return sqrtf( x*x+y*y+z*z );
	}
	float Distance( const CVec3 &t )const{
		return (*this-t).Length();
	}
	CVec3 Normalize()const{
		return *this/Length();
	}
	CVec3 Cross( const CVec3 &t )const{
		return CVec3( y*t.z-z*t.y,z*t.x-x*t.z,x*t.y-y*t.x );
	}
	CVec3 Blend( const CVec3 &t,float alpha )const{
		return (t-*this)*alpha+*this;
	}
	
	void Write( CStream *stream ){
		stream->WriteData( &x,12 );
	}
	static CVec3 Read( CStream *stream ){
		CVec3 v;
		stream->ReadData( &v.x,12 );
		return v;
	}
};

struct CVec4{
	float x,y,z,w;
	
	CVec4():x(0),y(0),z(0),w(0){
	}
	CVec4( float t ):x(t),y(t),z(t),w(t){
	}
	CVec4( const CVec3 &v,float w ):x(v.x),y(v.y),z(v.z),w(w){
	}
	CVec4( float x,float y,float z,float w ):x(x),y(y),z(z),w(w){
	}
	
	float &operator[]( int i ){
		return *(&x+i);
	}
	float operator[]( int i )const{
		return *(&x+i);
	}

	CVec4 operator-()const{
		return CVec4( -x,-y,-z,-w ); 
	}
	CVec4 operator+( float t )const{
		return CVec4( x+t,y+t,z+t,w+t );
	}
	CVec4 operator-( float t )const{
		return CVec4( x-t,y-t,z-t,w-t );
	}
	CVec4 operator*( float t )const{
		return CVec4( x*t,y*t,z*t,w*t );
	}
	CVec4 operator/( float t )const{
		return CVec4( x/t,y/t,z/t,w/t );
	}
	CVec4 operator+( const CVec4 &t )const{
		return CVec4( x+t.x,y+t.y,z+t.z,w+t.w );
	}
	CVec4 operator-( const CVec4 &t )const{
		return CVec4( x-t.x,y-t.y,z-t.z,w-t.w );
	}
	CVec4 operator*( const CVec4 &t )const{
		return CVec4( x*t.x,y*t.y,z*t.z,w*t.w );
	}
	CVec4 operator/( const CVec4 &t )const{
		return CVec4( x/t.x,y/t.y,z/t.z,w/t.w );
	}
	
	CVec4 &operator+=( const CVec4 &v ){
		x+=v.x;
		y+=v.y;
		z+=v.z;
		w+=v.w;
		return *this;
	}
	
	bool operator==( const CVec4 &v )const{
		return (x==v.x && y==v.y && z==v.z && w==v.w);
	}
	
	CVec3 &xyz(){
		return (CVec3&)*this;
	}
	const CVec3 &xyz()const{
		return (CVec3&)*this;
	}
	
	float Dot( const CVec4 &t )const{ 
		return x*t.x+y*t.z+z*t.z+w*t.w;
	}
	float Length()const{
		return sqrtf( x*x+y*y+z*z+w*w );
	}
	CVec4 Normalize()const{
		return operator/( Length() );
	}
};

struct CLine{
	CVec3 o,d;
	
	CLine( const CVec3 &o,const CVec3 &d ):o(o),d(d){
	}
};

struct CPlane{
	CVec3 n;
	float d;
	
	CPlane():d(0){
	}
	CPlane( const CVec3 &n,float d ):n(n),d(d){
	}
	CPlane( const CVec3 &v0,const CVec3 &v1,const CVec3 &v2 ){
		n=(v2-v1).Cross(v0-v1).Normalize();
		d=-n.Dot( v0 );
	}
	
	float SolveX( float y,float z )const{
		return (n.y*y+n.z*z+d)/-n.x;
	}
	
	float SolveY( float x,float z )const{
		return (n.x*x+n.z*z+d)/-n.y;
	}
	
	float SolveZ( float x,float y )const{
		return (n.x*x+n.y*y+d)/-n.z;
	}
	
	float Distance( const CVec3 &v )const{
		return n.Dot( v )+d;
	}
	CPlane Normalize()const{
		float t=1/n.Length();
		return CPlane( n*t,d*t );
	}
};

struct CBox{
	CVec3 a,b;
	
	CBox(){
	}

	CBox( const CVec3 &a,const CVec3 &b ):a(a),b(b){
	}

	CVec3 Corner( int i )const{
		return CVec3( (i&1) ? b.x : a.x,(i&2) ? b.y : a.y,(i&4) ? b.z : a.z );
	}
	
	CPlane Plane( int i )const{
		switch( i ){
		case 0:return CPlane( CVec3(1,0,0),-a.x );
		case 1:return CPlane( CVec3(-1,0,0),b.x );
		case 2:return CPlane( CVec3(0,1,0),-a.y );
		case 3:return CPlane( CVec3(0,-1,0),b.y );
		case 4:return CPlane( CVec3(0,0,1),-a.z );
		case 5:return CPlane( CVec3(0,0,-1),b.z );
		}
		Error( "CBox::Plane()" );
	}

	float Width()const{
		return b.x-a.x;
	}
	
	float Height()const{
		return b.y-a.y;
	}
	
	float Depth()const{
		return b.z-a.z;
	}

	bool IsEmpty()const{
		return b.x<a.x || b.y<a.y || b.z<a.z;
	}

	void Update( const CVec3 &v ){
		if( v.x<a.x ) a.x=v.x;
		if( v.x>b.x ) b.x=v.x;
		if( v.y<a.y ) a.y=v.y;
		if( v.y>b.y ) b.y=v.y;
		if( v.z<a.z ) a.z=v.z;
		if( v.z>b.z ) b.z=v.z;
	}
	
	static CBox Empty(){
		return CBox( CVec3(10000),CVec3(-10000) );
	}

	static CBox World(){
		return CBox( CVec3(-10000),CVec3(10000) );
	}
};

struct CSphere{
	CVec3 o;
	float r;
	
	CSphere():r(0){
	}

	CSphere( float r ):r(r){
	}

	CSphere( const CVec3 &o,float r ):o(o),r(r){
	}
};

struct CHull{
	vector<CPlane> planes;
	
	CHull(){
	}
	
	CHull( const CBox &box ){
		for( int i=0;i<6;++i ){
			planes.push_back( box.Plane(i) );
		}
	}
	
	bool Empty()const{
		return !planes.size();
	}

	bool Intersects( const CBox &box )const{
		for( vector<CPlane>::const_iterator it=planes.begin();it!=planes.end();++it ){
			int i;
			for( i=0;i<8;++i ){
				if( (*it).Distance( box.Corner(i) )>=0 ) break;
			}
			if( i==8 ) return false;
		}
		return true;
	}

	bool Intersects( const CSphere &CSphere )const{
		for( vector<CPlane>::const_iterator it=planes.begin();it!=planes.end();++it ){
			if( (*it).Distance( CSphere.o )<-CSphere.r ){
				return false;
			}
		}
		return true;
	}
};

struct CQuat{
	CVec3 v;
	float w;
	
	CQuat():w(1){
	}
	CQuat( const CVec3 &v,float w ):v(v),w(w){
	}

	CQuat operator-()const{
		return CQuat( -v,w );
	}
	CVec3 operator*( const CVec3 &t )const{
		float qx= w   * t.x + v.y * t.z - v.z * t.y;
		float qy= w   * t.y - v.x * t.z + v.z * t.x;
		float qz= w   * t.x + v.x * t.y - v.y * t.x;
		float qw=-v.x * t.x - v.y * t.y - v.z * t.z;
		CVec3 r;
		r.x=qz*t.y - qw*t.x - qy*t.z;
		r.y=qx*t.z - qw*t.y - qz*t.x;
		r.z=qy*t.x - qw*t.z - qx*t.y;
		return r;
		/*
		CQuaternion qtmp;

		qtmp.m_X = ( (CQuat.m_W * vecin.m_X) + (CQuat.m_Y * vecin.m_Z) - (CQuat.m_Z * vecin.m_Y));
		qtmp.m_Y = ( (CQuat.m_W * vecin.m_Y) - (CQuat.m_X * vecin.m_Z) + (CQuat.m_Z * vecin.m_X));
		qtmp.m_Z = ( (CQuat.m_W * vecin.m_Z) + (CQuat.m_X * vecin.m_Y) - (CQuat.m_Y * vecin.m_X));
		qtmp.m_W = (-(CQuat.m_X * vecin.m_X) - (CQuat.m_Y * vecin.m_Y) - (CQuat.m_Z * vecin.m_Z));

		vecout.m_X = ((qtmp.m_Z * vecin.m_Y) - (qtmp.m_W * vecin.m_X) - (qtmp.m_Y * vecin.m_Z));
		vecout.m_Y = ((qtmp.m_X * vecin.m_Z) - (qtmp.m_W * vecin.m_Y) - (qtmp.m_Z * vecin.m_X));
		vecout.m_Z = ((qtmp.m_Y * vecin.m_X) - (qtmp.m_W * vecin.m_Z) - (qtmp.m_X * vecin.m_Y));	
		*/
	}
	CQuat operator*( const CQuat &q )const{
		return CQuat( q.v.Cross(v)+q.v*w+v*q.w,w*q.w-v.Dot(q.v) );
	}	
	float Dot( const CQuat &q )const{
		return v.Dot( q.v )+w*q.w;
	}
	CQuat Slerp( const CQuat &q,float a )const{
		CQuat t=q;
		float d=Dot(q),b=1-a;
		if( d<0 ){
			t.v=-t.v;
			t.w=-t.w;
			d=-d;
		}
		if( d<1 ){
			float om=acosf( d );
			float si=sinf( om );
			a=sinf( a*om )/si;
			b=sinf( b*om )/si;
		}
		return CQuat( v*b+q.v*a,w*b+q.w*a );
	}
	CVec3 I()const{
		float xz=v.x*v.z,wy=w*v.y;
		float xy=v.x*v.y,wz=w*v.z;
		float yy=v.y*v.y,zz=v.z*v.z;
		return CVec3( 1-2*(yy+zz),2*(xy-wz),2*(xz+wy) );
	}

	CVec3 J()const{
		float yz=v.y*v.z,wx=w*v.x;
		float xy=v.x*v.y,wz=w*v.z;
		float xx=v.x*v.x,zz=v.z*v.z;
		return CVec3( 2*(xy+wz),1-2*(xx+zz),2*(yz-wx) );
	}

	CVec3 K()const{
		float xz=v.x*v.z,wy=w*v.y;
		float yz=v.y*v.z,wx=w*v.x;
		float xx=v.x*v.x,yy=v.y*v.y;
		return CVec3( 2*(xz-wy),2*(yz+wx),1-2*(xx+yy) );
	}	

	CVec3 YawPitchRoll()const{
		CVec3 r;
		float x2=v.x*v.x,y2=v.y*v.y,z2=v.z*v.z;
		r.x=atan2f( 2*v.y*w-2*v.z*v.x,1-2*y2-2*x2 );
		r.y=-asinf( 2*v.z*v.y+2*v.x*w );
		r.z=-atan2f( 2*v.z*w-2*v.y*v.x,1-2*z2-2*x2 );
		return r;
	}

	static CQuat YawPitchRollQuat( const CVec3 &e ){
		CQuat r;
		float c1=cosf(-e.z/2),s1=sinf(-e.z/2);
		float c2=cosf(-e.y/2),s2=sinf(-e.y/2);
		float c3=cosf( e.x/2),s3=sinf( e.x/2);
		float c1_c2=c1*c2,s1_s2=s1*s2;
		r.v.x=c1*s2*c3-s1*c2*s3;
		r.v.y=c1_c2*s3+s1_s2*c3;
		r.v.z=s1*c2*c3+c1*s2*s3;
		r.w=c1_c2*c3-s1_s2*s3;
		return r;
	}
};

struct CMat4{
	CVec4 i,j,k,t;
	
	CMat4():i(CVec4(1,0,0,0)),j(CVec4(0,1,0,0)),k(CVec4(0,0,1,0)),t(CVec4(0,0,0,1)){
	}
	
	CMat4( const CVec4 &i,const CVec4 &j,const CVec4 &k,const CVec4 &t ):i(i),j(j),k(k),t(t){
	}

	CVec4 &operator[]( int n ){
		return *(&i+n);
	}
	
	const CVec4 &operator[]( int n )const{ 
		return *(&i+n);
	}

	CVec4 Row( int n )const{
		return operator[]( n );
	}
	
	CVec4 Column( int n )const{
		return CVec4( i[n],j[n],k[n],t[n] );
	}
	
	bool IsAffine()const{
		return !i.w && !j.w && !k.w && t.w==1;
	}

	void AssertAffine()const{
		if( !IsAffine() ) Error( "ERROR" );
	}

	float Determinant()const{
		AssertAffine();
		return i.x*(j.y*k.z-j.z*k.y) - i.y*(j.x*k.z-j.z*k.x) + i.z*(j.x*k.y-j.y*k.x);
	}
	
	CMat4 Transpose()const{
		return CMat4( Column(0),Column(1),Column(2),Column(3) );
	}
	
	CMat4 AffineInverse()const{
		AssertAffine();
		float c=1/Determinant();
		CMat4 r;
		r.i.x= c * ( j.y*k.z - j.z*k.y );
		r.i.y=-c * ( i.y*k.z - i.z*k.y );
		r.i.z= c * ( i.y*j.z - i.z*j.y );
		r.j.x=-c * ( j.x*k.z - j.z*k.x );
		r.j.y= c * ( i.x*k.z - i.z*k.x );
		r.j.z=-c * ( i.x*j.z - i.z*j.x );
		r.k.x= c * ( j.x*k.y - j.y*k.x );
		r.k.y=-c * ( i.x*k.y - i.y*k.x );
		r.k.z= c * ( i.x*j.y - i.y*j.x );
		r.t.x=-( t.x*r.i.x + t.y*r.j.x + t.z*r.k.x );
		r.t.y=-( t.x*r.i.y + t.y*r.j.y + t.z*r.k.y );
		r.t.z=-( t.x*r.i.z + t.y*r.j.z + t.z*r.k.z );
		return r;
	}
	
	CMat4 operator-()const{
		return AffineInverse();
	}

	CMat4 operator*( const CMat4 &m )const{
		CMat4 r;
		r.i.x=i.x*m.i.x + j.x*m.i.y + k.x*m.i.z + t.x*m.i.w;
		r.i.y=i.y*m.i.x + j.y*m.i.y + k.y*m.i.z + t.y*m.i.w;
		r.i.z=i.z*m.i.x + j.z*m.i.y + k.z*m.i.z + t.z*m.i.w;
		r.i.w=i.w*m.i.x + j.w*m.i.y + k.w*m.i.z + t.w*m.i.w;
		r.j.x=i.x*m.j.x + j.x*m.j.y + k.x*m.j.z + t.x*m.j.w;
		r.j.y=i.y*m.j.x + j.y*m.j.y + k.y*m.j.z + t.y*m.j.w;
		r.j.z=i.z*m.j.x + j.z*m.j.y + k.z*m.j.z + t.z*m.j.w;
		r.j.w=i.w*m.j.x + j.w*m.j.y + k.w*m.j.z + t.w*m.j.w;
		r.k.x=i.x*m.k.x + j.x*m.k.y + k.x*m.k.z + t.x*m.k.w;
		r.k.y=i.y*m.k.x + j.y*m.k.y + k.y*m.k.z + t.y*m.k.w;
		r.k.z=i.z*m.k.x + j.z*m.k.y + k.z*m.k.z + t.z*m.k.w;
		r.k.w=i.w*m.k.x + j.w*m.k.y + k.w*m.k.z + t.w*m.k.w;
		r.t.x=i.x*m.t.x + j.x*m.t.y + k.x*m.t.z + t.x*m.t.w;
		r.t.y=i.y*m.t.x + j.y*m.t.y + k.y*m.t.z + t.y*m.t.w;
		r.t.z=i.z*m.t.x + j.z*m.t.y + k.z*m.t.z + t.z*m.t.w;
		r.t.w=i.w*m.t.x + j.w*m.t.y + k.w*m.t.z + t.w*m.t.w;
		return r;
	}

	CVec3 operator*( const CVec3 &v )const{
		AssertAffine();
		CVec3 r;
		r.x=i.x*v.x + j.x*v.y + k.x*v.z + t.x;
		r.y=i.y*v.x + j.y*v.y + k.y*v.z + t.y;
		r.z=i.z*v.x + j.z*v.y + k.z*v.z + t.z;
		return r;
	}

	CVec4 operator*( const CVec4 &v )const{
		CVec4 r;
		r.x=i.x*v.x + j.x*v.y + k.x*v.z + t.x*v.w;
		r.y=i.y*v.x + j.y*v.y + k.y*v.z + t.y*v.w;
		r.z=i.z*v.x + j.z*v.y + k.z*v.z + t.z*v.w;
		r.w=i.w*v.x + j.w*v.y + k.w*v.z + t.w*v.w;
		return r;
	}

	CPlane operator*( const CPlane &p )const{
		AssertAffine();
		CMat4 m=operator-();
		return CPlane( CVec3( 
			m.i.x*p.n.x + m.i.y*p.n.y + m.i.z*p.n.z + m.i.w*p.d,
			m.j.x*p.n.x + m.j.y*p.n.y + m.j.z*p.n.z + m.j.w*p.d,
			m.k.x*p.n.x + m.k.y*p.n.y + m.k.z*p.n.z + m.k.w*p.d ),
			m.t.x*p.n.x + m.t.y*p.n.y + m.t.z*p.n.z + m.t.w*p.d ).Normalize();
	}

	CBox operator*( const CBox &t )const{
		AssertAffine();
		CBox r=CBox::Empty();
		if( t.IsEmpty() ) return r;
		for( int i=0;i<8;++i ){
			r.Update( operator*(t.Corner(i)) );
		}
		return r;
	}

	CHull operator*( const CHull &t )const{
		AssertAffine();
		CHull r;
		r.planes.reserve( t.planes.size() );
		for( vector<CPlane>::const_iterator it=t.planes.begin();it!=t.planes.end();++it ){
			r.planes.push_back( operator*( *it ) );
		}
		return r;
	}

	CVec3 Translation()const{
		AssertAffine();
		return t.xyz();
	}

	CQuat Rotation()const{
		AssertAffine();
		CVec3 sc=Scale();
		CVec3 iv=i.xyz()/sc;
		CVec3 jv=j.xyz()/sc;
		CVec3 kv=k.xyz()/sc;
		float t=iv.x+jv.y+kv.z+1;
		CVec3 v;
		float w;
		if( t>0 ){
			t=sqrtf(t)*2;
			v.x=(kv.y-jv.z)/t;
			v.y=(iv.z-kv.x)/t;
			v.z=(jv.x-iv.y)/t;
			w=t/4;
		}else if( iv.x>jv.y && iv.x>kv.z ){
			t=sqrtf(iv.x-jv.y-kv.z+1)*2;
			v.x=t/4;
			v.y=(jv.x+iv.y)/t;
			v.z=(iv.z+kv.x)/t;
			w=(kv.y-jv.z)/t;
		}else if( jv.y>kv.z ){
			t=sqrtf(jv.y-kv.z-iv.x+1)*2;
			v.x=(jv.x+iv.y)/t;
			v.y=t/4;
			v.z=(kv.y+jv.z)/t;
			w=(iv.z-kv.x)/t;
		}else{
			t=sqrtf(kv.z-jv.y-iv.x+1)*2;
			v.x=(iv.z+kv.x)/t;
			v.y=(kv.y+jv.z)/t;
			v.z=t/4;
			w=(jv.x-iv.y)/t;
		}
		return CQuat( v,w );
	}

	CVec3 Scale()const{
		AssertAffine();
		return CVec3( i.xyz().Length(),j.xyz().Length(),k.xyz().Length() );
	}
	
	float Yaw()const{
		return -atan2f( k.x,k.z );
	}
	
	float Pitch()const{
		return -atan2f( k.y,sqrtf( k.x*k.x+k.z*k.z ) );
	}
	
	float Roll()const{
		return atan2f( i.y,j.y );
	}
	
	CVec3 YawPitchRoll()const{
		return CVec3( Yaw(),Pitch(),Roll() );
	}
	
	static CMat4 TranslationMatrix( const CVec3 &v ){
		CMat4 r;
		r.t.xyz()=v;
		return r;
	}
	
	static CMat4 RotationMatrix( const CQuat &q ){
		CMat4 r;
		float xx=q.v.x*q.v.x,yy=q.v.y*q.v.y,zz=q.v.z*q.v.z;
		float xy=q.v.x*q.v.y,xz=q.v.x*q.v.z,yz=q.v.y*q.v.z;
		float wx=q.w*q.v.x,wy=q.w*q.v.y,wz=q.w*q.v.z;
		r.i=CVec4( 1-2*(yy+zz),  2*(xy-wz),  2*(xz+wy),0 );
		r.j=CVec4(   2*(xy+wz),1-2*(xx+zz),  2*(yz-wx),0 );
		r.k=CVec4(   2*(xz-wy),  2*(yz+wx),1-2*(xx+yy),0 );
		return r;
	}
	
	static CMat4 ScaleMatrix( const CVec3 &s ){
		CMat4 r;
		r.i.x=s.x;
		r.j.y=s.y;
		r.k.z=s.z;
		return r;
	}

	static CMat4 YawMatrix( float q ){
		return CMat4( CVec4(cosf(q),0,sinf(q),0),CVec4(0,1,0,0),CVec4(-sinf(q),0,cosf(q),0),CVec4(0,0,0,1) );
	}

	static CMat4 PitchMatrix( float q ){
		return CMat4( CVec4(1,0,0,0),CVec4(0,cosf(q),sinf(q),0),CVec4(0,-sinf(q),cosf(q),0),CVec4(0,0,0,1) );
	}

	static CMat4 RollMatrix( float q ){
		return CMat4( CVec4(cosf(q),sinf(q),0,0),CVec4(-sinf(q),cosf(q),0,0),CVec4(0,0,1,0),CVec4(0,0,0,1) );
	}
	
	static CMat4 YawPitchRollMatrix( const CVec3 &r ){
		return YawMatrix( r.x ) * PitchMatrix( r.y ) * RollMatrix( r.z );
	}
	
	static CMat4 OrthoMatrix( float left,float right,float bottom,float top,float znear,float zfar ){
		CMat4 r;
		r.i.x=2/(right-left);
		r.j.y=2/(top-bottom);
		r.k.z=2/(zfar-znear);
		r.t.x=-(right+left)/(right-left);
		r.t.y=-(top+bottom)/(top-bottom);
		r.t.z=-(zfar+znear)/(zfar-znear);
		return r;
	}

	static CMat4 FrustumMatrix( float left,float right,float bottom,float top,float znear,float zfar ){
		CMat4 r;
		float znear2=znear*2;
		float w=right-left;
		float h=top-bottom;
		r.i.x=znear2/w;
		r.j.y=znear2/h;
		r.k.x=(right+left)/w;
		r.k.y=(top+bottom)/h;
		if( zfar ){
			float d=zfar-znear;
			r.k.z=(zfar+znear)/d;
			r.t.z=-(zfar*znear2)/d;
		}else{
			float eps=.001f;
			r.k.z=1-eps;
			r.t.z=znear*(eps-1);
		}
		r.k.w=1;
		r.t.w=0;
		return r;
	}
	
	static CMat4 PerspectiveMatrix( float fovy,float aspect,float znear,float zfar ){
			float t=znear * tan( fovy/2 );
			return FrustumMatrix( -t * aspect,t * aspect,-t,t,znear,zfar );
	}
};

ostream &operator<<( ostream &o,const CVec2 &v );
ostream &operator<<( ostream &o,const CVec3 &v );
ostream &operator<<( ostream &o,const CVec4 &v );
ostream &operator<<( ostream &o,const CPlane &p );
ostream &operator<<( ostream &o,const CMat4 &m );

#endif
