#ifndef S3_DES_HPP
#define S3_DES_HPP

extern"C"{
   
#include <string.h>    
#define i_DES_UNROLL 1

#define i_ITERATIONS 16
#define i_DES_LONG unsigned long
#define i_DES_ENCRYPT	1
#define i_DES_DECRYPT	0

#define	i_ROTATE(a, n) ( ( (a) >> (n) ) + ( (a) << ( 32 - (n) ) ) )
#define i_DES_CAST(x) ((i_DES_cblock *)(x))

#define i_DES_check_key 0
#define i_DES_KEY_SZ 	(sizeof(i_DES_cblock))

#define c2l(c, l) \
			(l =((i_DES_LONG)(*((c)++)))     , \
			 l|=((i_DES_LONG)(*((c)++)))<< 8L, \
			 l|=((i_DES_LONG)(*((c)++)))<<16L, \
			 l|=((i_DES_LONG)(*((c)++)))<<24L)

#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) \
			{ \
				case 8: l2 =((i_DES_LONG)(*(--(c))))<<24L; \
				case 7: l2|=((i_DES_LONG)(*(--(c))))<<16L; \
				case 6: l2|=((i_DES_LONG)(*(--(c))))<< 8L; \
				case 5: l2|=((i_DES_LONG)(*(--(c))));     \
				case 4: l1 =((i_DES_LONG)(*(--(c))))<<24L; \
				case 3: l1|=((i_DES_LONG)(*(--(c))))<<16L; \
				case 2: l1|=((i_DES_LONG)(*(--(c))))<< 8L; \
				case 1: l1|=((i_DES_LONG)(*(--(c))));     \
			} \
		}
			
#define l2c(l, c) \
			(*((c)++)=(unsigned char)(((l)     )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24L)&0xff))

#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) \
			{ \
				case 8: *(--(c))=(unsigned char)(((l2)>>24L)&0xff); \
				case 7: *(--(c))=(unsigned char)(((l2)>>16L)&0xff); \
				case 6: *(--(c))=(unsigned char)(((l2)>> 8L)&0xff); \
				case 5: *(--(c))=(unsigned char)(((l2)     )&0xff); \
				case 4: *(--(c))=(unsigned char)(((l1)>>24L)&0xff); \
				case 3: *(--(c))=(unsigned char)(((l1)>>16L)&0xff); \
				case 2: *(--(c))=(unsigned char)(((l1)>> 8L)&0xff); \
				case 1: *(--(c))=(unsigned char)(((l1)     )&0xff); \
			} \
		}

#define PERM_OP(a, b, t, n, m) ((t)=((((a)>>(n))^(b))&(m)), (b)^=(t), (a)^=((t)<<(n)))
#define HPERM_OP(a, t, n, m) ((t)=((((a)<<(16-(n)))^(a))&(m)),	(a)=(a)^(t)^(t>>(16-(n))))
	
#define IP(l, r) \
	{ \
		register i_DES_LONG tt; \
		PERM_OP(r, l, tt,  4,0x0f0f0f0fL); \
		PERM_OP(l, r, tt, 16,0x0000ffffL); \
		PERM_OP(r, l, tt,  2,0x33333333L); \
		PERM_OP(l, r, tt,  8,0x00ff00ffL); \
		PERM_OP(r, l, tt,  1,0x55555555L); \
	}

#define FP(l, r) \
	{ \
		register i_DES_LONG tt; \
		PERM_OP(l, r, tt,  1,0x55555555L); \
		PERM_OP(r, l, tt,  8,0x00ff00ffL); \
		PERM_OP(l, r, tt,  2,0x33333333L); \
		PERM_OP(r, l, tt, 16,0x0000ffffL); \
		PERM_OP(l, r, tt,  4,0x0f0f0f0fL); \
	}

typedef unsigned char i_DES_cblock[8];
typedef unsigned char i_const_DES_cblock[8];

typedef struct i_DES_ks
{
	union
	{
		i_DES_cblock cblock;
		/* make sure things are correct size on machines with 8 byte longs */
		i_DES_LONG deslong[2];
	} ks[16];
} i_DES_key_schedule;

int i_DES_random_key(i_DES_cblock *ret);
int i_DES_key_sched(i_const_DES_cblock *key,i_DES_key_schedule *schedule);
int i_DES_set_key(i_const_DES_cblock *key,i_DES_key_schedule *schedule);
int i_DES_set_key_checked(i_const_DES_cblock *key,i_DES_key_schedule *schedule);
void i_DES_set_key_unchecked(i_const_DES_cblock *key,i_DES_key_schedule *schedule);
void i_DES_set_odd_parity(i_DES_cblock *key);
int i_DES_check_key_parity(i_const_DES_cblock *key);
int i_DES_is_weak_key(i_const_DES_cblock *key);

void i_DES_encrypt2(i_DES_LONG *data, i_DES_key_schedule *ks, int enc);
void i_DES_encrypt3(i_DES_LONG *data, i_DES_key_schedule *ks1, i_DES_key_schedule *ks2, i_DES_key_schedule *ks3);
void i_DES_decrypt3(i_DES_LONG *data, i_DES_key_schedule *ks1, i_DES_key_schedule *ks2, i_DES_key_schedule *ks3);
void i_DES_ecb3_encrypt(i_const_DES_cblock *input, i_DES_cblock *output, i_DES_key_schedule *ks1,i_DES_key_schedule *ks2, i_DES_key_schedule *ks3, int enc);
void i_DES_ede3_cbc_encrypt(const unsigned char *input,unsigned char *output, long length, i_DES_key_schedule *ks1,i_DES_key_schedule *ks2, i_DES_key_schedule *ks3,i_DES_cblock *ivec,int enc);

#define i_DES_ecb2_encrypt(i, o, k1, k2, e) i_DES_ecb3_encrypt((i), (o), (k1), (k2), (k1), (e))
#define i_DES_ede2_cbc_encrypt(i,o,l,k1,k2,iv,e) i_DES_ede3_cbc_encrypt((i),(o),(l),(k1),(k2),(k1),(iv),(e))
    
}


#endif