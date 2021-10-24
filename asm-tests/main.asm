.386
.model flat

.code
	main proc

	mov  eax, 15

	pusha

         mov  eax, 3
         mov  ebx, 4
         mul  ebx
         mov  ebx, 2
         add  ebx, eax
         mov  eax, 6
         mov  ecx, 7
         mul  ecx
         mov  ecx, 8
         mul  ecx
         add  ebx, eax
         mov  eax, 1
         add  eax, ebx

	popa

	main endp

end main