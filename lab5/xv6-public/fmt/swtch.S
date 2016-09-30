2950 # Context switch
2951 #
2952 #   void swtch(struct context **old, struct context *new);
2953 #
2954 # Save current register context in old
2955 # and then load register context from new.
2956 
2957 .globl swtch
2958 swtch:
2959   movl 4(%esp), %eax
2960   movl 8(%esp), %edx
2961 
2962   # Save old callee-save registers
2963   pushl %ebp
2964   pushl %ebx
2965   pushl %esi
2966   pushl %edi
2967 
2968   # Switch stacks
2969   movl %esp, (%eax)
2970   movl %edx, %esp
2971 
2972   # Load new callee-save registers
2973   popl %edi
2974   popl %esi
2975   popl %ebx
2976   popl %ebp
2977   ret
2978 
2979 
2980 
2981 
2982 
2983 
2984 
2985 
2986 
2987 
2988 
2989 
2990 
2991 
2992 
2993 
2994 
2995 
2996 
2997 
2998 
2999 
