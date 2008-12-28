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

//SEXY!!!!!
//
//Allows VC debug apps to use release libs...
//
#if (defined (_MSC_VER) && defined (_DEBUG))
//HACK: allow the use of mixed debug/non-debug code
extern "C" {
	void __cdecl _invalid_parameter_noinfo( void ){
		exit(0);
	}
}
#endif

void _LOG_( const string &t,const char *file,int line ){
	cout<<t<<" (file:"<<file<<", line:"<<line<<")"<<endl;
}

void _ERROR_( const string &t,const char *file,int line ){
	cout<<"ERROR:"<<t<<" (file:"<<file<<", line:"<<line<<")"<<endl;

#if (defined (_MSC_VER) && defined (_DEBUG))
	*(int*)0=0;
#endif
	exit(0);
}

void _WARNING_( const string &t,const char *file,int line ){
	cout<<"WARNING:"<<t<<" (file:"<<file<<", line:"<<line<<")"<<endl;
}

string LoadString( const string &path ){
	ifstream in( path.c_str() );
	string str,t;
	while( getline( in,t ) ){
		str+=t+'\n';
	}
	return str;
}

string operator+( const string &t,int n ){
	char buf[32];
	sprintf( buf,"%i",n );
	return t+buf;
}

string operator+( const string &t,float n ){
	char buf[32];
	sprintf( buf,"%f",n );
	return t+buf;
}

void Tokenize( const string &str,vector<string> &tokens,const string &delimiters ){
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

string toupper( string t ){
	for( int i=0;i<t.size();++i ){
		t[i]=toupper(t[i]);
	}
	return t;
}

string tolower( string t ){
	for( int i=0;i<t.size();++i ){
		t[i]=tolower(t[i]);
	}
	return t;
}
