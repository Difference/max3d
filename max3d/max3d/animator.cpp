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

#include "std.h"

#include "entity.h"

struct CAnimKey{
	float time;
	CVec4 value;
	CAnimKey( float time,const CVec4 &value ):time(time),value(value){
	}
};

class CAnimTrack{
public:
	void SetKey( float time,const CVec4 &value ){
		for( vector<CAnimKey>::iterator it=_keys.begin();it!=_keys.end();++it ){
			CAnimKey &key=*it;
			if( time<key.time ){
				_keys.insert( it,CAnimKey( time,value ) );
				return;
			}else if( time==key.time ){
				key.time=time;
				key.value=value;
				return;
			}
		}
		_keys.push_back( CAnimKey( time,value ) );
	}
	bool GetKeys( float time,CVec4 *val1,CVec4 *val2,float *alpha ){
		if( _keys.empty() ) return false;
		if( time<=_keys.front().time ){
			*val1=*val2=_keys.front().value;
			*alpha=0;
			return true;
		}
		if( time>=_keys.back().time ){
			*val1=*val2=_keys.back().value;
			*alpha=0;
			return true;
		}
		for( int i=0;;++i ){
			CAnimKey &key0=_keys[i],&key1=_keys[i+1];
			if( time<=key1.time ){
				*val1=key0.value;
				*val2=key1.value;
				*alpha=(time-key0.time)/(key1.time-key0.time);
				return true;
			}
		}
	}
private:
	vector<CAnimKey> _keys;
};

class CAnimSequence{
public:
	void SetKey( int entity,float time,const CVec3 &trans,const CQuat &rot,const CVec3 &scale,int flags ){
		if( flags & 1 ){
			if( entity>=_trans.size() ) _trans.resize(entity+1);
			_trans[entity].SetKey( time,CVec4( trans,0 ) );
		}
		if( flags & 2 ){
			if( entity>=_rot.size() ) _rot.resize(entity+1);
			_rot[entity].SetKey( time,CVec4( rot.v,rot.w ) );
		}
		if( flags & 4 ){
			if( entity>=_scale.size() ) _scale.resize(entity+1);
			_scale[entity].SetKey( time,CVec4( scale,0 ) );
		}
	}

	void Animate( float time,const vector<CEntity*> &entities ){
		for( int i=0;i<entities.size();++i ){
			CEntity *entity=entities[i];
			CVec3 trans,scale;
			CQuat rot;
			CVec4 val1,val2;
			float alpha;
			entity->GetTRS( trans,rot,scale );
			if( i<_trans.size() && _trans[i].GetKeys( time,&val1,&val2,&alpha ) ){
				trans=val1.xyz().Blend( val2.xyz(),alpha );
			}
			if( i<_rot.size() && _rot[i].GetKeys( time,&val1,&val2,&alpha ) ){
				rot=CQuat(val1.xyz(),val1.w).Slerp( CQuat(val2.xyz(),val2.w),alpha );
			}
			if( i<_scale.size() && _scale[i].GetKeys( time,&val1,&val2,&alpha ) ){
				scale=val1.xyz().Blend( val2.xyz(),alpha );
			}
			entity->SetTRS( trans,rot,scale );
		}
	}
	void SetLength( float length ){
		_length=length;
	}
	float Legnth(){ 
		return _length; 
	}
private:
	float _length;
	vector<CAnimTrack> _trans,_rot,_scale;
};

class CAnimation : public CResource{
public:
	void SetKey( int seq,int entity,float time,const CVec3 &trans,const CQuat &rot,const CVec3 &scale,int flags ){
		if( seq>=_seqs.size() ) _seqs.resize( seq+1 );
		_seqs[seq].SetKey( entity,time,trans,rot,scale,flags );
	}
	void SetLength( int seq,float length ){
		if( seq>=_seqs.size() ) _seqs.resize( seq+1 );
		_seqs[seq].SetLength( length );
	}
	void Animate( int seq,float time,const vector<CEntity*> &entities ){
		if( seq<_seqs.size() ) _seqs[seq].Animate( time,entities );
	}

private:
	vector<CAnimSequence> _seqs;
};

CAnimator::CAnimator():_animation( new CAnimation ){
}

void CAnimator::SetKey( int seq,CEntity *entity,float time,int flags ){
	int i;
	for( i=0;i<_entities.size();++i ){
		if( _entities[i]==entity ) break;
	}
	if( i==_entities.size() ) _entities.push_back( entity );
	_animation->SetKey( seq,i,time,entity->Translation(),entity->Rotation(),entity->Scale(),flags );
}

void CAnimator::SetLength( int seq,float length ){
	_animation->SetLength( seq,length );
}

void CAnimator::Animate( int seq,float time ){
	_animation->Animate( seq,time,_entities );
}

void CAnimator::OnAttach( CEntity *entity ){
}

CAnimator *CAnimator::OnCopy( CCopier *copier ){
	CAnimator *copy=new CAnimator;
	_animation->Retain();
	copy->_animation->Release();
	copy->_animation=_animation;
	return copy;
}

REGISTERTYPE( CAnimator )
