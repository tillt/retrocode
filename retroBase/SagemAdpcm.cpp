/*\
 * SagemAdpcm.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
/*
 CompressIRATAdpcm proc near             ; CODE XREF: sub_403130+31p
 var_4= dword ptr -4
 arg_0= dword ptr  4
 arg_4= dword ptr  8
 arg_8= dword ptr  0Ch
 arg_C= dword ptr  10h
 arg_10= dword ptr  14h
 arg_14= dword ptr  18h
 arg_18= dword ptr  1Ch

 push    ecx
 mov     eax, [esp+4+arg_0]
 mov     ecx, [esp+4+arg_C]
 mov     edx, [esp+4+arg_4]
 push    ebx
 push    ebp
 push    esi
 push    edi
 lea     edi, [ecx+eax*2]
 mov     ecx, edx
 lea     ebx, [edx+edx]
 imul    ecx, [esp+14h+arg_10]
 mov     [esp+14h+arg_10], ebx
 lea     ebp, ds:0FFFFFFFCh[edx*4]
 lea     esi, [edi+ecx*2]
 mov     [esp+14h+arg_4], ebp
 movsx   ecx, word ptr [edi]
 add     edi, ebx
 mov     [esp+14h+var_4], esi
 movsx   ebx, word ptr [esp+14h+arg_8]
 mov     [esp+14h+arg_8], ebx
 mov     ebx, [esp+14h+arg_18]
 lea     eax, [ebx+eax*4]
 xor     ebx, ebx
 */
step = CAdpcm::pnGetIMAStepSizeTable()[index];				//mov     eax, ds:dword_405DE4[ebp*4]

outp[0]=(unsigned char)(valpred&0xFF);			//mov     [eax], cl
outp[1]=(unsigned char)((valpred>>8)&0xFF);		//sar     ecx, 8
												//mov     [eax], cl
outp[2]=(unsigned char)step;
outp[3]=(unsigned char)0x01;					
outp+=4;


//LOOP START 

do
{
	//get sample
	val = *inp++;												//movsx   ecx, word ptr [edi]

	/* Step 1 - compute difference with previous value */		//mov     edx, [esp+14h+arg_8]
	diff = val - valpred;										//sub     ecx, edx
	/*
	 mov     edx, 0
	 setns   dl
	 dec     edx
	 and     edx, 8
	 mov     [esp+14h+arg_C], edx
	 jz      short loc_403216
	*/
	sign = (diff < 0) ? 8 : 0;
	if ( sign ) 
		diff = (-diff);											//neg     ecx

	/* Step 2 - Divide and clamp */
	vpdiff = (step >> 3);										//sar     esi, 3
	delta = 0;													//xor     edx, edx

	if ( diff >= step )											//cmp     ecx, eax
	{															//jl      short loc_40322A
		delta = 4;												//mov     edx, 4
		diff -= step;											//sub     ecx, eax
		vpdiff += step;											//add     esi, eax
	}
	step >>= 1;													//sar     eax, 1
	if ( diff >= step  )										//cmp     ecx, eax
	{															//jl      short loc_403237
		delta |= 2;												//or      edx, 2
		diff -= step;											//sub     ecx, eax
		vpdiff += step;											//add     esi, eax
	}
	step >>= 1;													//sar     eax, 1
	if ( diff >= step )											//cmp     ecx, eax
	{															//jl      short loc_403242
		delta |= 1;												//or      edx, 1
		vpdiff += step;											//add     esi, eax
	}

	/* Step 3 - Update previous value */
	//mov     eax, [esp+14h+arg_C]
	//mov     ecx, [esp+14h+arg_8]		
	if ( sign )							//test    eax, eax
	{									//jz      short loc_403252
		valpred -= vpdiff;				//sub     ecx, esi
	}									
	else
	{
		valpred += vpdiff;				//add     ecx, esi
	}

	/* Step 4 - Clamp previous value to 16 bits */
	if ( valpred > 32767 )				//cmp     ecx, 7FFFh
	{									//jle     short loc_40326A
		valpred = 32767;				//mov     [esp+14h+arg_8], 7FFFh
	}
	else if ( valpred < -32768 )		//cmp     ecx, 0FFFF8000h
	{									//jge     short loc_40327A
		valpred = -32768;				//mov     [esp+14h+arg_8], 0FFFF8000h
	}

	/* Step 5 - Assemble value, update index and step values */
	delta |= sign;						//or      edx, eax
	index += m_nIMAIndexTable[delta];	//add     ebp, ds:dword_405DA4[edx*4]
	if ( index < 0 )					//jns     short loc_403289
		index = 0;						//xor     ebp, ebp
	if ( index > 88 )					//cmp     ebp, 58h
		index = 88;						//mov     ebp, 58h

	/* Step 6 - Output value */
	//mov     eax, ds:dword_405DE4[ebp*4]
	if (bufferstep & 1)					//test    bl, 1
	{									//jz      short loc_4032BC
	 //mov     ecx, [esp+14h+arg_0]
	 //and     dl, 0Fh
	 //or      [ecx], dl
	// inc     ecx
		*outp++ = outputbuffer | delta & 0x0f;;

		if (bufferstep 
	//cmp     ebx, 7
	//mov     [esp+14h+arg_0], ecx
	//jnz     short loc_4032C5
	//add     ecx, [esp+14h+arg_4]
	//mov     [esp+14h+arg_0], ecx
	}
	else 
	{
	//mov     ecx, [esp+14h+arg_0]
	//shl     dl, 4
	//mov     [ecx], dl
		outputbuffer = (delta << 4) & 0xf0);
	}
/*
mov     edx, [esp+14h+arg_10]
mov     ecx, [esp+14h+var_4]
inc     ebx
add     edi, edx
and     ebx, 7
cmp     edi, ecx
jb      loc_4031F9
*/
}
	/*
//LOOP END

 mov     edx, [esp+14h+arg_14]
 pop     edi
 pop     esi
 mov     [edx], ebp
 pop     ebp
 pop     ebx
 pop     ecx
 retn
 ; ---------------------------------------------------------------------------

 loc_4032E7:                             ; CODE XREF: CompressIRATAdpcm+73j
 pop     edi
 mov     [ecx], ebp
 pop     esi
 pop     ebp
 pop     ebx
 pop     ecx
 retn
 CompressIRATAdpcm endp
*/
