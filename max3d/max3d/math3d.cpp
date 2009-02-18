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

#include "math3d.h"

ostream &operator<<( ostream &o,const CVec2 &v ){
	o<<"CVec2("<<v.x<<","<<v.y<<")";
	return o;
}

ostream &operator<<( ostream &o,const CVec3 &v ){
	o<<"CVec3("<<v.x<<","<<v.y<<","<<v.z<<")";
	return o;
}

ostream &operator<<( ostream &o,const CVec4 &v ){
	o<<"CVec4("<<v.x<<","<<v.y<<","<<v.z<<","<<v.w<<")";
	return o;
}

ostream &operator<<( ostream &o,const CPlane &p ){
	o<<"CPlane("<<p.n.x<<","<<p.n.y<<","<<p.n.z<<","<<p.d<<")";
	return o;
}

ostream &operator<<( ostream &o,const CMat4 &m ){
	o<<"CMat4("<<m.i<<","<<m.j<<","<<m.k<<","<<m.t<<")";
	return o;
}

ostream &operator<<( ostream &o,const CLine &t ){
	o<<"CLine("<<t.o<<","<<t.d<<")";
	return o;
}
