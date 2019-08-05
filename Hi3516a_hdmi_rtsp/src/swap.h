#ifndef _SWAP_H_
#define _SWAP_H_

#include <assert.h>

typedef unsigned char byte;

#ifndef WIN32
#define _ASSERT assert
#endif

#ifndef axis_max
#define axis_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef axis_min
#define axis_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

inline void swap(byte * a, byte * b){
	byte t = *a;
	*a = *b;
	*b = t;
}

template<class T>
inline T swap(T i){
	byte * b = (byte *)&i;
	int len = sizeof(T);
	for(int j = 0; j < len / 2; ++j){
		swap(&b[j], &b[sizeof(T) - j - 1]);
	}
	return i;
}

#endif
