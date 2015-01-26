// --- Predefined References  --
define GC_ALLOC	            10001h
define HOOK                 10010h
define INIT_RND             10012h
define INIT                 10013h
define NEWFRAME             10014h
define INIT_ET              10015h
define ENDFRAME             10016h
define RESTORE_ET           10017h
define LOAD_CLASSNAME       10018h
define OPENFRAME            10019h
define CLOSEFRAME           1001Ah
define NEWTHREAD            1001Bh
define CLOSETHREAD          1001Ch
define EXIT                 1001Dh
define LOCK                 10021h
define UNLOCK               10022h
define LOAD_CALLSTACK       10024h
define REFRESH_CALLSTACK    10025h
define CORE_EXCEPTION_TABLE 20001h
define CORE_GC_TABLE        20002h
define CORE_GC_SIZE         20003h
define CORE_STAT_COUNT      20004h
define CORE_STATICROOT      20005h
define CORE_RT_TABLE        20006h
define CORE_TLS_INDEX       20007h
define THREAD_TABLE         20008h

// CORE RT TABLE OFFSETS
define rt_Instance      0000h
define rt_loadSymbol    0004h
define rt_loadName      0008h
define rt_interprete    000Ch
define rt_lasterr       0010h
define rt_loadaddrinfo  0014h

// CORE GC SIZE
define gcs_MGSize	0000h
define gcs_YGSize	0004h
define gcs_TTSize       0008h

// GCXT TABLE
define gc_header             0000h
define gc_start              0004h
define gc_yg_start           0008h
define gc_yg_current         000Ch
define gc_yg_end             0010h
define gc_shadow             0014h
define gc_shadow_end         0018h
define gc_mg_start           001Ch
define gc_mg_current         0020h
define gc_end                0024h
define gc_promotion          0028h
define gc_mg_wbar            002Ch
define gc_lock               0030h
define gc_signal             0034h
define tt_ptr                0038h
define tt_lock               003Ch

// GCXT TLS TABLE
define tls_stack_frame       0000h
define tls_stack_bottom      0004h
define tls_catch_addr        0008h
define tls_catch_level       000Ch
define tls_catch_frame       0010h
define tls_sync_event        0014h
define tls_flags             0018h

// Page Size
define page_size               10h
define page_size_order          4h
define page_size_order_minus2   2h
define page_mask        0FFFFFFF0h

// Object header fields
define elObjectOffset        0010h
define elSyncOffset          0010h
define elSizeOffset          000Ch
define elCountOffset         0008h
define elVMTOffset           0004h 

define GC_HEAP_ATTRIBUTE 00Dh

// --- System Core Preloaded Routines --

structure %CORE_GC_TABLE

  dd 0 // ; gc_header             : +00h
  dd 0 // ; gc_start              : +04h
  dd 0 // ; gc_yg_start           : +08h
  dd 0 // ; gc_yg_current         : +0Ch
  dd 0 // ; gc_yg_end             : +10h
  dd 0 // ; gc_shadow             : +14h
  dd 0 // ; gc_shadow_end         : +18h
  dd 0 // ; gc_mg_start           : +1Ch
  dd 0 // ; gc_mg_current         : +20h
  dd 0 // ; gc_end                : +24h
  dd 0 // ; gc_promotion          : +28h
  dd 0 // ; gc_mg_wbar            : +2Ch
  dd 0 // ; gc_lock               : +30h
  dd 0 // ; gc_signal             : +34h
  dd 0 // ; tt_ptr                : +38h
  dd 0 // ; tt_lock               : +3Ch

end

// --- GC_ALLOC ---
// in: ecx - counter ; ebx - size ; out: eax - created object ; edi contains the object or zero ; ecx is not used but its value saved
procedure %GC_ALLOC

  // ; GCXT: set lock
labStart:
  mov  esi, data : %CORE_GC_TABLE + gc_lock
labWait:
  mov edx, 1
  xor eax, eax
  lock cmpxchg dword ptr[esi], edx
  jnz  short labWait

  mov  eax, [data : %CORE_GC_TABLE + gc_yg_current]
  mov  esi, ebx
  mov  edx, [data : %CORE_GC_TABLE + gc_yg_end]
  add  esi, eax
  cmp  esi, edx
  jae  short labYGCollect
  mov  [eax+4], ebx
  mov  [data : %CORE_GC_TABLE + gc_yg_current], esi
  
  // ; GCXT: clear sync field
  mov  [eax], 0                  
  mov  edx, 0FFFFFFFFh
  mov  esi, data : %CORE_GC_TABLE + gc_lock
  lea  eax, [eax + elObjectOffset]
  
  // ; GCXT: free lock
  // ; could we use mov [esi], 0 instead?
  lock xadd [esi], edx

  ret

labYGCollect:

  // ; GCXT: find the current thread entry
  mov  edx, fs:[2Ch]
  mov  eax, [data : %CORE_TLS_INDEX]

  // ; GCXT: save registers
  push edi
  mov  eax, [edx+eax*4]
  push ebp

  // ; GCXT: lock frame
  // ; get current thread event
  mov  esi, [eax + tls_sync_event]         
  mov  [eax + tls_stack_frame], esp

  push ecx
  push ebx

  // ; === GCXT: safe point ===
  mov  edx, [data : %CORE_GC_TABLE + gc_signal]
  // ; if it is a collecting thread, starts the GC
  test edx, edx                       
  jz   short labConinue
  // ; otherwise eax contains the collecting thread event

  // ; signal the collecting thread that it is stopped
  push 0FFFFFFFFh // -1
  mov  edi, data : %CORE_GC_TABLE + gc_lock
  push edx
  push esi

  // ; signal the collecting thread that it is stopped
  call extern 'dlls'kernel32.SetEvent

  // ; free lock
  // ; could we use mov [esi], 0 instead?
  mov  ebx, 0FFFFFFFFh
  lock xadd [edi], ebx

  // ; stop until GC is ended
  call extern 'dlls'kernel32.WaitForSingleObject

  // ; restore registers and try again
  pop  ebp
  pop  ebx
  pop  ecx
  pop  edi

  jmp  labStart

labConinue:

  mov  [data : %CORE_GC_TABLE + gc_signal], esi // set the collecting thread signal
  mov  ebp, esp

  // ; === thread synchronization ===

  // ; create list of threads need to be stopped
  mov  eax, esi
  mov  edi, [data : %CORE_GC_TABLE + tt_ptr]
  // ; get tls entry address  
  mov  esi, data : %THREAD_TABLE
labNext:
  mov  edx, [esi]
  push [edx+4]
  test [edx+8], 1
  jnz  short labSkipReset 
  cmp  [edx+4], eax
  jz   short labSkipSave
  push [edx+4]
labSkipSave:
  // ; reset all signal events
  call extern 'dlls'kernel32.ResetEvent      
labSkipReset:
  lea  esi, [esi+4]
  mov  eax, [data : %CORE_GC_TABLE + gc_signal]
  sub  edi, 1
  jnz  short labNext

  mov  esi, data : %CORE_GC_TABLE + gc_lock
  mov  edx, 0FFFFFFFFh
  mov  ebx, [data : %CORE_GC_TABLE + tt_ptr]

  // ; free lock
  // ; could we use mov [esi], 0 instead?
  lock xadd [esi], edx

  mov  ecx, esp
  sub  ebx, 1
  jz   short labSkipWait

  // ; wait until they all stopped
  push 0FFFFFFFFh // -1
  push 0FFFFFFFFh // -1
  push ecx
  push ebx
  call extern 'dlls'kernel32.WaitForMultipleObjects

  // ; remove list
  mov  esp, ebp     

labSkipWait:
  // ==== GCXT end ==============
  
  // ; create set of roots
  mov  ebp, esp
  xor  ecx, ecx
  push ecx
  push ecx                                                              

  // ; save static roots
  mov  esi, data : %CORE_STATICROOT
  mov  ecx, [data : %CORE_STAT_COUNT]
  push esi
  push ecx

  // ; == GCXT: save frames ==
  mov  ebx, [data : %CORE_GC_TABLE + tt_ptr]

labYGNextThread:  
  sub  ebx, 1
  mov  eax, data : %THREAD_TABLE
  
  // ; get tls entry address
  mov  esi, [eax+ebx*4]            
  // ; get the top frame pointer
  mov  eax, [esi + tls_stack_frame]
  mov  ecx, eax

labYGNextFrame:
  mov  esi, eax
  mov  eax, [esi]
  test eax, eax
  jnz  short labYGNextFrame
  
  push ecx
  sub  ecx, esi
  neg  ecx
  push ecx  
  
  mov  eax, [esi + 4]
  test eax, eax
  mov  ecx, eax
  jnz  short labYGNextFrame
  nop
  nop
  test ebx, ebx
  jnz  short labYGNextThread
  // ; == GCXT: end ==
  
  // ; check if major collection should be performed
  mov  edx, [data : %CORE_GC_TABLE + gc_end]
  mov  ebx, [data : %CORE_GC_TABLE + gc_yg_end]
  sub  edx, [data : %CORE_GC_TABLE + gc_mg_current] // ; mg free space
  sub  ebx, [data : %CORE_GC_TABLE + gc_yg_start]   // ; size to promote 
  cmp  ebx, edx                                     // ; currently it is presumed that all objects
  jae  short labFullCollect                         // ; can be promoted

  // === Minor collection ===

  // ; save mg -> yg roots 
  mov  ebx, [data : %CORE_GC_TABLE + gc_mg_current]
  mov  edi, [data : %CORE_GC_TABLE + gc_mg_start]
  sub  ebx, edi                                        // ; we need to check only MG region
  jz   labWBEnd                                        // ; skip if it is zero
  mov  esi, [data : %CORE_GC_TABLE + gc_mg_wbar]
  shr  ebx, page_size_order
  lea  edi, [edi + elObjectOffset]

labWBNext:
  cmp  [esi], 0
  lea  esi, [esi+4]
  jnz  short labWBMark
  sub  ebx, 4
  ja   short labWBNext
  nop
  nop
  jmp  short labWBEnd

labWBMark:
  lea  eax, [esi-4]
  sub  eax, [data : %CORE_GC_TABLE + gc_mg_wbar]
  mov  edx, [esi-4]
  shl  eax, page_size_order
  lea  eax, [edi + eax]
  
  test edx, 0FFh
  jz   short labWBMark2
  push eax
  mov  ecx, [eax-elCountOffset]
  push ecx

labWBMark2:
  lea  eax, [eax + page_size]
  test edx, 0FF00h
  jz   short labWBMark3
  push eax
  mov  ecx, [eax-elCountOffset]
  push ecx

labWBMark3:
  lea  eax, [eax + page_size]
  test edx, 0FF0000h
  jz   short labWBMark4
  push eax
  mov  ecx, [eax-elCountOffset]
  push ecx

labWBMark4:
  lea  eax, [eax + page_size]
  test edx, 0FF000000h
  jz   short labWBNext
  push eax
  mov  ecx, [eax-elCountOffset]
  push ecx
  jmp  short labWBNext

labWBEnd:
  push ebp                      // save the stack restore-point

  // ; init registers
  mov  ebx, [data : %CORE_GC_TABLE + gc_yg_start]
  mov  edx, [data : %CORE_GC_TABLE + gc_yg_end]
  mov  ebp, [data : %CORE_GC_TABLE + gc_shadow]

  // ; collect roots
  lea  eax, [esp+4]
  mov  ecx, [eax]
  mov  esi, [esp+8]
  mov  [data : %CORE_GC_TABLE + gc_yg_current], ebp

labCollectFrame:
  push eax
  call labCollectYG
  pop  eax
  lea  eax, [eax+8]
  mov  esi, [eax+4]
  test esi, esi
  mov  ecx, [eax]
  jnz short labCollectFrame 
  
  // ; save gc_yg_current to mark survived objects
  mov  [data : %CORE_GC_TABLE + gc_promotion], ebp
  mov  [data : %CORE_GC_TABLE + gc_yg_current], ebp
  
  // ; switch main YG heap with a shadow one
  mov  eax, [data : %CORE_GC_TABLE + gc_yg_start]
  mov  ebx, [data : %CORE_GC_TABLE + gc_shadow]
  mov  ecx, [data : %CORE_GC_TABLE + gc_yg_end]
  mov  edx, [data : %CORE_GC_TABLE + gc_shadow_end]

  mov  [data : %CORE_GC_TABLE + gc_yg_start], ebx
  mov  [data : %CORE_GC_TABLE + gc_yg_end], edx
  mov  ebx, [esp]
  mov  [data : %CORE_GC_TABLE + gc_shadow], eax  
  mov  ebx, [ebx]                           // ; restore object size  
  mov  [data : %CORE_GC_TABLE + gc_shadow_end], ecx

  sub  edx, ebp

  // ; check if it is enough place
  cmp  ebx, edx
  pop  ebp
  jae  short labFullCollect

  // ; free root set
  mov  esp, ebp

  // ; restore registers
  pop  ebx

  // ; try to allocate once again
  mov  eax, [data : %CORE_GC_TABLE + gc_yg_current]
  mov  [eax], ebx
  add  ebx, eax
  mov  [data : %CORE_GC_TABLE + gc_yg_current], ebx
  lea  edi, [eax + elObjectOffset]

  // ; GCXT: signal the collecting thread that GC is ended
  // ; should it be placed into critical section?
  xor  ecx, ecx
  mov  esi, [data : %CORE_GC_TABLE + gc_signal]
  // ; clear thread signal var
  mov  [data : %CORE_GC_TABLE + gc_signal], ecx
  push esi
  call extern 'dlls'kernel32.SetEvent 

  mov  eax, edi
  pop  ecx
  pop  ebp
  pop  edi  

  ret

labFullCollect:
  // ====== Major Collection ====
  push ebp                      // ; save the stack restore-point

  // ; mark both yg and mg objects
  mov  ebx, [data : %CORE_GC_TABLE + gc_yg_start]
  mov  edx, [data : %CORE_GC_TABLE + gc_mg_current]

  // ; collect roots
  lea  eax, [esp+4]
  mov  ecx, [eax]
  mov  esi, [esp+8]

labMGCollectFrame:
  push eax
  call labCollectMG
  pop  eax
  lea  eax, [eax+8]
  mov  esi, [eax+4]
  test esi, esi
  mov  ecx, [eax]
  jnz short labMGCollectFrame 

  // ; compact mg
  mov  esi, [data : %CORE_GC_TABLE + gc_mg_start]
  mov  edi, esi
  sub  edi, [data : %CORE_GC_TABLE + gc_start]
  shr  edi, page_size_order_minus2
  add  edi, [data : %CORE_GC_TABLE + gc_header]

  // ; skip the permanent part
labMGSkipNext:
  mov  ecx, [esi]
  test ecx, ecx
  jns  short labMGSkipEnd
  mov  eax, esi
  neg  ecx
  lea  eax, [eax + elObjectOffset]
  add  esi, ecx
  mov  [edi], eax
  shr  ecx, page_size_order_minus2
  add  edi, ecx
  cmp  esi, edx
  jb   short labMGSkipNext

labMGSkipEnd:
  mov  ebp, esi
  
  // ; compact
labMGCompactNext:
  add  esi, ecx
  shr  ecx, page_size_order_minus2
  add  edi, ecx
  cmp  esi, edx
  jae  short labMGCompactEnd

labMGCompactNext2:
  mov  ecx, [esi]
  test ecx, ecx
  jns  short labMGCompactNext
  mov  eax, ebp
  neg  ecx
  lea  eax, [eax + elObjectOffset]
  mov  [edi], eax
  mov  eax, ecx
  shr  eax, page_size_order_minus2
  add  edi, eax

labMGCopy:
  mov  eax, [esi]
  mov  [ebp], eax
  sub  ecx, 4
  lea  esi, [esi+4]
  lea  ebp, [ebp+4]
  jnz  short labMGCopy
  cmp  esi, edx
  jb   short labMGCompactNext2
labMGCompactEnd:

  // ; promote yg
  mov  ebx, [data : %CORE_GC_TABLE + gc_end]
  mov  esi, [data : %CORE_GC_TABLE + gc_yg_start]
  sub  ebx, ebp
  mov  edi, esi
  sub  edi, [data : %CORE_GC_TABLE + gc_start]
  shr  edi, page_size_order_minus2
  mov  edx, [data : %CORE_GC_TABLE + gc_yg_current]
  add  edi, [data : %CORE_GC_TABLE + gc_header]
  jmp  short labYGPromNext2

labYGPromNext:
  add  esi, ecx
  shr  ecx, page_size_order_minus2
  add  edi, ecx
  cmp  esi, edx
  jae  short labYGPromEnd
labYGPromNext2:
  mov  ecx, [esi]
  test ecx, ecx
  jns  short labYGPromNext
  mov  eax, ebp
  neg  ecx
  // ; raise an exception if it is not enough memory to promote object
  lea  eax, [eax + elObjectOffset]
  sub  ebx, ecx
  js   short labError
  mov  [edi], eax
  mov  eax, ecx
  shr  eax, page_size_order_minus2
  add  edi, eax
labYGProm:
  mov  eax, [esi]
  sub  ecx, 4
  mov  [ebp], eax
  lea  esi, [esi+4]
  lea  ebp, [ebp+4]
  jnz  short labYGProm
  cmp  esi, edx
  jb   short labYGPromNext2
labYGPromEnd:

  // ; get previous heap end
  mov  edx, [data : %CORE_GC_TABLE + gc_mg_current]

  // ; set mg_current, clear yg and survive
  mov  [data : %CORE_GC_TABLE + gc_mg_current], ebp
  mov  eax, [data : %CORE_GC_TABLE + gc_yg_start]
  mov  [data : %CORE_GC_TABLE + gc_yg_current], eax
  mov  [data : %CORE_GC_TABLE + gc_promotion], eax
  
  // ; fix roots
  lea  eax, [esp+4]
  mov  ecx, [eax]
  mov  esi, [esp+8]

  mov  ebx, [data : %CORE_GC_TABLE + gc_yg_start]
  mov  ebp, [data : %CORE_GC_TABLE + gc_start]

labFixRoot:
  push eax
  call labFixObject
  pop  eax
  lea  eax, [eax+8]
  mov  esi, [eax+4]
  test esi, esi
  mov  ecx, [eax]
  jnz  short labFixRoot 

  // ; clear WBar
  mov  esi, [data : %CORE_GC_TABLE + gc_mg_wbar]
  mov  ecx, [data : %CORE_GC_TABLE + gc_end]
  xor  eax, eax
  sub  ecx, [data : %CORE_GC_TABLE + gc_mg_start]
  shr  ecx, page_size_order

labClearWBar:
  mov  [esi], eax
  sub  ecx, 4
  lea  esi, [esi+4]
  ja   short labClearWBar
	
  // ; free root set
  mov  esp, [esp]
  pop  ebp
  pop  ebx

  // ; allocate
  mov  eax, [data : %CORE_GC_TABLE + gc_yg_current]
  mov  [eax], ebx
  mov  edx, [data : %CORE_GC_TABLE + gc_yg_end]
  add  ebx, eax
  cmp  ebx, edx
  jae  short labError2
  mov  [data : %CORE_GC_TABLE + gc_yg_current], ebx
  lea  edi, [eax + elObjectOffset]

  // ; GCXT: signal the collecting thread that GC is ended
  // ; should it be placed into critical section?
  xor  ecx, ecx
  mov  esi, [data : %CORE_GC_TABLE + gc_signal]
  // ; clear thread signal var
  mov  [data : %CORE_GC_TABLE + gc_signal], ecx
  push esi
  call extern 'dlls'kernel32.SetEvent 

  mov  eax, edi
  pop  ecx
  pop  edi  
  ret

labError:
  // ; restore stack
  mov  esp, [esp]
  pop  ebx
  pop  ecx
  pop  ebp
  pop  edi 

labError2:
  push 0
  push 0
  push 1
  push 0C0000017h
  call extern 'dlls'KERNEL32.RaiseException
  ret  

  // start collecting: esi => ebp, [ebx, edx] ; ecx - count
labCollectYG:
  push 0

  lea  ecx, [ecx+4]
  lea  esi, [esi-4]

labYGNext:
  lea  esi, [esi+4]
  sub  ecx, 4
  jz   labYGResume

labYGCheck:
  mov  eax, [esi]

  // ; check if it valid reference
  cmp  eax, ebx
  jl   short labYGNext  
  nop
  cmp  edx, eax
  jl   short labYGNext

  // ; check if it was collected
  mov  edi, [eax-elSizeOffset]
  test edi, edi
  js   labYGContinue

  // ; check if the object should be promoted
  cmp  eax, [data : %CORE_GC_TABLE + gc_promotion]
  jb   labYGPromMin

  // ; save previous ecx field
  push ecx

  // ; === GCXT: Copy Object Header ===
  // ; copy object size
  mov  [ebp+4], edi

  // ; copy sync field
  mov  ecx, [eax - elSyncOffset]
  mov  [ebp], ecx
  
  // ; copy object vmt
  mov  ecx, [eax - elVMTOffset]
  mov  [ebp + 0Ch], ecx
  
  // ; mark as collected
  or   [eax - elSizeOffset], 80000000h

  // ; reserve shadow YG
  mov  ecx, edi
  lea  edi, [ebp + elObjectOffset]
  add  ebp, ecx

  // ; update reference
  mov  [esi], edi

  // ; get object size
  mov  ecx, [eax - elCountOffset]

  // ; save ESI
  push esi
  mov  esi, eax

  // ; copy object size
  mov  [edi - elCountOffset], ecx

  // ; save new reference
  mov  [eax - elVMTOffset], edi

  // ; check if the object has fields
  cmp  ecx, 0 // probaly will be enough just test ecx, ecx and analize c flag

  // ; save original reference
  push eax

  // ; collect object fields if it has them
  jg   labYGCheck

  lea  esp, [esp+4]
  jz   short labYGSkipCopyData

  // ; copy meta data object to shadow YG
  neg  ecx  
  add  ecx, 3
  and  ecx, 0FFFFFFFCh

labYGCopyData:
  mov  eax, [esi]
  sub  ecx, 4
  mov  [edi], eax
  lea  esi, [esi+4]
  lea  edi, [edi+4]
  jnz  short labYGCopyData

labYGSkipCopyData:
  pop  esi
  pop  ecx
  jmp  labYGNext

labYGResume:
  // ; copy object to shadow YG
  pop  edi
  test edi, edi
  jz   short labYGEnd

  mov  ecx, [edi-elCountOffset]
  mov  esi, [edi - elVMTOffset]

labYGCopy:
  mov  eax, [edi]
  sub  ecx, 4
  mov  [esi], eax
  lea  esi, [esi+4]
  lea  edi, [edi+4]
  jnz  short labYGCopy

  pop  esi
  pop  ecx
  jmp  labYGNext

  nop
labYGEnd:
  ret

labYGContinue:
  // ; update reference
  mov  edi, [eax - elVMTOffset]
  mov  [esi], edi
  jmp  labYGNext

  // ; ---- minor promotion to mg ---
labYGPromMin:

  // ; save yg pointer
  push ecx
  mov  [data : %CORE_GC_TABLE + gc_yg_current], ebp
  push 0
  mov  ebp, [data : %CORE_GC_TABLE + gc_mg_current]
  mov  ecx, 4
  jmp  short labYGPromMinBegin

labYGPromMinNext:
  lea  esi, [esi+4]
  sub  ecx, 4
  jz   labYGPromMinResume

labYGPromMinCheck:
  mov  eax, [esi]

  // ; check if it valid reference
  cmp  eax, ebx
  jl   short labYGPromMinNext 
  nop
  cmp  edx, eax
  jl   short labYGPromMinNext

  // ; check if it was collected
  mov  edi, [eax-elSizeOffset]
  test edi, edi
  js   labYGPromMinContinue

labYGPromMinBegin:
  // ; save previous ecx field
  push ecx

  // ; === GCXT: Copy Object Header ===
  // ; copy object size
  mov  [ebp + 4], edi

  // ; copy sync field
  mov  ecx, [eax - elSyncOffset]
  mov  [ebp], ecx
  
  // ; copy object vmt
  mov  ecx, [eax - elVMTOffset]
  mov  [ebp + 0Ch], ecx
  
  // ; mark as collected
  or   [eax - elSizeOffset], 80000000h

  // ; reserve MG
  mov  ecx, edi
  lea  edi, [ebp + elObjectOffset]
  add  ebp, ecx

  // ; update reference
  mov  [esi], edi

  // ; get object size
  mov  ecx, [eax - elCountOffset]

  // ; save ESI
  push esi
  mov  esi, eax

  // ; copy object size
  mov  [edi - elCountOffset], ecx

  // ; save new reference
  mov  [eax - elVMTOffset], edi

  // ; check if the object has fields
  cmp  ecx, 0

  // ; save original reference
  push eax

  // ; collect object fields if it has them
  jg   labYGPromMinCheck

  lea  esp, [esp+4]
  jz   short labYGPromMinSkipCopyData

  // ; copy meta data object to MG
  neg  ecx  
  add  ecx, 3
  and  ecx, 0FFFFFFFCh

labYGPromMinCopyData:
  mov  eax, [esi]
  sub  ecx, 4
  mov  [edi], eax
  lea  esi, [esi+4]
  lea  edi, [edi+4]
  jnz  short labYGPromMinCopyData

labYGPromMinSkipCopyData:
  pop  esi
  pop  ecx
  jmp  labYGPromMinNext

labYGPromMinResume:
  // ; copy object to shadow MG
  pop  edi
  test edi, edi
  jnz   short labYGPromMinResume2

  lea  esi, [esi-4]
  mov  [data : %CORE_GC_TABLE + gc_mg_current], ebp
  pop  ecx
  mov  ebp, [data : %CORE_GC_TABLE + gc_yg_current]
  jmp  labYGNext

labYGPromMinResume2:
  mov  ecx, [edi-elCountOffset]
  mov  esi, [edi - elVMTOffset]

labYGPromMinCopy:
  mov  eax, [edi]
  sub  ecx, 4
  mov  [esi], eax
  lea  esi, [esi+4]
  lea  edi, [edi+4]
  jnz  short labYGPromMinCopy

  pop  esi
  pop  ecx
  jmp  labYGPromMinNext
  
labYGPromMinContinue:
  // ; bad luck, the referred object cannot be promoted
  // ; we have to mark in WB card
  push ecx
  mov  ecx, [esp+8]
  // ; get the promoted object (the referree object) address
  mov  ecx, [ecx]
  sub  ecx, [data : %CORE_GC_TABLE + gc_start]
  shr  ecx, page_size_order
  add  ecx, [data : %CORE_GC_TABLE + gc_header]
  mov  byte ptr [ecx], 1  
  pop  ecx

  // ; update reference
  mov  edi, [eax - elVMTOffset]
  mov  [esi], edi
  jmp  labYGPromMinNext

  // ---- start collecting: esi => ebp, [ebx, edx] ; ecx - count ---
labCollectMG:

  lea  ecx, [ecx+4]
  push 0
  lea  esi, [esi-4]
  push 0

labMGNext:
  sub  ecx, 4
  lea  esi, [esi+4]
  jz   short labMGResume

labMGCheck:
  mov  eax, [esi]

  // ; check if it valid reference
  cmp  eax, ebx
  jl   short labMGNext  
  nop
  cmp  edx, eax
  jl   short labMGNext

  // ; check if it was collected
  mov  edi, [eax-elSizeOffset]
  test edi, edi
  js   short labMGNext

  // ; mark as collected
  neg  edi
  cmp  [eax - elCountOffset], 0
  mov  [eax - elSizeOffset], edi

  jle  short labMGNext

  // ; save previous ecx field
  push ecx

  // ; get object size
  mov  ecx, [eax - elCountOffset]

  // ; save ESI
  push esi
  mov  esi, eax

  // ; collect object fields if it has them
  jmp   short labMGCheck

labMGResume:
  pop  esi
  pop  ecx
  test esi, esi
  jnz  short labMGNext

  nop
labMGEnd:
  ret

labFixObject:

  lea  ecx, [ecx+4]
  push 0
  lea  esi, [esi-4]
  push 0

labFixNext:
  sub  ecx, 4
  lea  esi, [esi+4]
  jz   short labFixResume

labFixCheck:
  mov  eax, [esi]

  // ; check if it valid reference
  cmp  eax, ebx
  jl   short labFixNext
  nop
  cmp  edx, eax
  jl   short labFixNext

  lea  edi, [eax-elObjectOffset]

  sub  edi, ebp
  shr  edi, page_size_order_minus2
  add  edi, [data : %CORE_GC_TABLE + gc_header]

  mov  eax, [edi]
  mov  [esi], eax

  // ; make sure the object was not already fixed
  mov  edi, [eax - elSizeOffset]
  test edi, edi
  jns  short labFixNext

  neg  edi
  mov  [eax - elSizeOffset], edi

  cmp  [eax - elCountOffset], 0
  jle  short labFixNext

  // ; save previous ecx field
  push ecx

  // ; get object size
  mov  ecx, [eax - elCountOffset]

  // ; save ESI
  push esi
  mov  esi, eax

  // ; collect object fields if it has them
  jmp   short labFixCheck

labFixResume:
  pop  esi
  pop  ecx
  test esi, esi
  jnz  short labFixNext
  nop

  ret

end

// --- System Core Functions --

procedure % INIT
  // GCXT: initialize signal
  xor  ebx, ebx
  mov  [data : %CORE_GC_TABLE + gc_signal], ebx

  // ; initialize
  mov  ecx, [data : %CORE_STAT_COUNT]
  mov  edi, data : %CORE_STATICROOT
  test ecx, ecx
  jz   short labNext
  xor  eax, eax

clear:
  mov  [edi], eax     
  sub  ecx, 4
  lea  edi, [edi+4]
  jnz  short clear

labNext:
  // ;calculate total heap size
  // ; mg size
  mov  edi, [data : %CORE_GC_SIZE]             
  and  edi, 0FFFFFF80h     // align to 128
  mov  eax, edi
  
  // ; yg size
  mov  esi, [data : %CORE_GC_SIZE + gcs_YGSize]
  and  esi, 0FFFFFF80h    // align to 128
  add  eax, esi
  add  eax, esi
  
  // ; add header
  mov  ebx, eax
  shl  eax, page_size_order   
  shl  ebx, 2
  push ebx
  add  eax, ebx

  // ; create heap
  push eax                
  push GC_HEAP_ATTRIBUTE
  call extern 'dlls'KERNEL32.GetProcessHeap
  push eax 
  call extern 'dlls'KERNEL32.HeapAlloc

  shl  esi, page_size_order
  shl  edi, page_size_order

  // ; initialize gc table
  pop  ecx
  mov  [data : %CORE_GC_TABLE + gc_header], eax

  // ; skip header
  add  eax, ecx           

  // ; initialize yg
  mov  [data : %CORE_GC_TABLE + gc_start], eax
  mov  [data : %CORE_GC_TABLE + gc_yg_start], eax
  mov  [data : %CORE_GC_TABLE + gc_yg_current], eax
  mov  [data : %CORE_GC_TABLE + gc_promotion], eax

  // ; initialize gc end
  mov  ecx, eax
  add  ecx, esi
  add  ecx, esi
  add  ecx, edi
  mov  [data : %CORE_GC_TABLE + gc_end], ecx
  
  add  eax, esi
  mov  [data : %CORE_GC_TABLE + gc_yg_end], eax
  mov  [data : %CORE_GC_TABLE + gc_shadow], eax

  add  eax, esi
  mov  [data : %CORE_GC_TABLE + gc_shadow_end], eax
  mov  [data : %CORE_GC_TABLE + gc_mg_start], eax
  mov  [data : %CORE_GC_TABLE + gc_mg_current], eax
  
  // ; initialize wbar start
  mov  edx, [data : %CORE_GC_TABLE + gc_mg_start]
  sub  edx, [data : %CORE_GC_TABLE + gc_start]
  shr  edx, page_size_order
  add  edx, [data : %CORE_GC_TABLE + gc_header]
  mov  [data : %CORE_GC_TABLE + gc_mg_wbar], edx
  
  // ; GCXT: assign tls entry
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  ecx, fs:[2Ch]
  mov  esi, [ecx + ebx*4]

  // ; init thread flags  
  // mov  [esi + tls_flags], 0       
  
  // ; set thread event handle
  push 0
  push 0
  push 0FFFFFFFFh // -1
  push 0
  call extern 'dlls'KERNEL32.CreateEventW  
  mov  [esi + tls_sync_event], eax     

  mov  eax, data : %THREAD_TABLE
  mov  [eax], esi       // ; save tls reference 

  mov  ecx, [data : %CORE_GC_SIZE + gcs_TTSize]
  mov  [eax-4], ecx

  ret

end

procedure % NEWFRAME

  // ; put frame end and move procedure returning address
  pop  edx           

  // ; GCXT                                                               
  // ; get thread table entry from tls
  mov  ecx, [data : %CORE_TLS_INDEX]
  mov  esi, fs:[2Ch]
  mov  esi, [esi+ecx*4]

  xor  ebx, ebx
  push ebp
  push ebx
  push ebx

  // ; GCXT
  // ; set stack frame pointer / bottom stack pointer
  mov  ebp, esp 
  mov  [esi + tls_stack_bottom], esp
  push edx
  mov  [esi + tls_stack_frame], ebx
  
  // ; GCXT
  // ; set thread table length
  mov  ebx, 1
  mov  [data : %CORE_GC_TABLE + tt_ptr], ebx   
  

  ret

end

procedure % ENDFRAME

  // ; save return pointer
  pop  ecx  
  
  xor  edx, edx
  lea  esp, [esp+8]
  pop  ebp

  // ; restore return pointer
  push ecx   
  ret

end

// ; init exception table, ebx contains default handler address
procedure % INIT_ET

  pop esi

  mov edx, ebx

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  ecx, fs:[2Ch]
  mov  ebx, [ecx+ebx*4]

  // ; set default exception handler
  mov  [ebx + tls_catch_addr], edx
  mov  [ebx + tls_catch_level], esp
  mov  [ebx + tls_catch_frame], ebp

  push esi
  ret

end 

procedure % RESTORE_ET

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  ecx, fs:[2Ch]
  mov  ebx, [ecx+ebx*4]

  pop  edx
  mov  esp, [ebx + tls_catch_level]
  push edx

  ret

end 

procedure % OPENFRAME

  // ; GCXT: get thread table entry from tls
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  esi, fs:[2Ch]
  xor  edi, edi
  mov  esi, [esi+ebx*4]
                                        
  // ; save return pointer
  pop  ecx  

  // ; GCXT: get thread table entry from tls
  // ; save previous pointer / size field
  mov  esi, [esi + tls_stack_frame]
  push ebp
  push esi                                
  push edi                              
  mov  ebp, esp
  
  // ; restore return pointer
  push ecx   
  ret

end

procedure % CLOSEFRAME

  // ; GCXT: get thread table entry from tls
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  esi, fs:[2Ch]
  xor  edi, edi
  mov  esi, [esi+ebx*4]
                                        
  // ; save return pointer
  pop  ecx  

  // ; GCXT
  lea  esp, [esp+8]
  pop  ebp
  
  // ; restore return pointer
  push ecx   
  ret

end

// ; presave eax
procedure % NEWTHREAD

  push eax
  
  // ; GCXT
  mov  edx, data : %THREAD_TABLE
  mov  esi, data : %CORE_GC_TABLE + tt_lock
  mov  ecx, [edx - 4]

labWait:
  // ; set lock
  xor  eax, eax
  mov  edx, 1
  lock cmpxchg dword ptr[esi], edx
  jnz  short labWait

  mov  ebx, [data : %CORE_GC_TABLE + tt_ptr]
  sub  ecx, ebx
  jz   short labSkipSave

  add  ebx, 1
  mov  [data : %CORE_GC_TABLE + tt_ptr], ebx

labSkipSave:

  // ; free lock
  // ; could we use mov [esi], 0 instead?
  mov  edx, -1
  lock xadd [esi], edx

  xor  eax, eax

  // check if there is a place in thread table
  test ecx, ecx
  jz   short lErr
  
  sub  ebx, 1
  mov  esi, data : %THREAD_TABLE
  lea  esi, [esi+ebx*4]

  // ; assign tls entry
  mov  ebx, [data : %CORE_TLS_INDEX]
  push 0

  mov  eax, fs:[2Ch]
  push 0

  mov  eax, [eax + ebx*4] 
  push 0FFFFFFFFh //-1

  mov  [esi], eax               // ; save tls entry
  push 0
  mov  esi, eax

  call extern 'dlls'kernel32.CreateEventW

  // ; initialize thread entry
  mov  [esi + tls_sync_event], eax     // ; set thread event handle

  mov  [esi+tls_flags], 0              // ; init thread flags  

  pop  eax
  pop  edx                             // ; put frame end and move procedure returning address

  xor  ebx, ebx
  push ebp
  push ebx                      
  push ebx

  // ; set stack frame pointer  
  mov  ebp, esp 
  mov  [esi + tls_stack_frame], ebx
  mov  [esi + tls_stack_bottom], esp

  // ; restore return pointer
  push edx              
  ret

lErr:
  add esp, 4 
  ret

end

procedure % CLOSETHREAD

  // ; GCXT
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  edi, fs:[2Ch]

  mov  esi, data : %CORE_GC_TABLE + tt_lock

labWait:
  // ; set lock
  xor  eax, eax  
  mov  edx, 1
  lock cmpxchg dword ptr[esi], edx
  jnz  short labWait

  // ; tls reference
  mov  eax, [edi + ebx*4]           
  mov  ecx, [data : %CORE_GC_TABLE + tt_ptr]
  mov  edi, data : %THREAD_TABLE

  // ; find the current thread entry
labSearch:
  test ecx, ecx
  jz   short labNotFound

  cmp  [edi], eax
  lea  edi, [edi+4]
  lea  ecx, [ecx-1]
  jnz   short labSearch

  // ; delete the entry
  test ecx, ecx
  jz   short labSkipDelete

labDelete:
  mov  ebx, [edi]
  mov  [edi-4], ebx
  lea  edi, [edi+4]
  sub  ecx, 1
  jnz   short labDelete
  
labSkipDelete:
  mov  ecx, [data : %CORE_GC_TABLE + tt_ptr]
  mov  edi, eax
  sub  ecx, 1
  mov  ebx, [edi + tls_sync_event]
  mov  [data : %CORE_GC_TABLE + tt_ptr], ecx

  // ; close handle

  push ebx
  call extern 'dlls'kernel32.CloseHandle

labNotFound:

  // ; free lock
  // ; could we use mov [esi], 0 instead?
  mov  edx, -1
  lock xadd [esi], edx

  pop  edx
  lea  esp, [esp+0Ch]
  push edx

lErr:
  ret
  
end

procedure % EXIT
  
  // ; GCXT: get thread table entry from tls
  mov  ecx, [data : %CORE_TLS_INDEX]
  mov  esi, fs:[2Ch]
  mov  esi, [esi+ecx*4]  

  // ; GCXT: terminate process
  mov  esp, [esi + tls_catch_level]
  mov  eax, 1                         // exit error code
  push eax
  call  extern 'dlls'kernel32.ExitThread

end

procedure % LOCK

  ret

end

procedure % UNLOCK

  ret

end

// ; ==== Command Set ==

// ; throw
inline % 7

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  edx, fs:[2Ch]
  mov  ebx, [edx+ebx*4]
  jmp  [ebx + tls_catch_addr]

end

// ; set

inline % 19h
                                
   mov  ebx, esi
   shl  ebx, 2
    
   // ; calculate write-barrier address
   mov  ecx, edi
   sub  ecx, [data : %CORE_GC_TABLE + gc_start]
   mov  edx, [data : %CORE_GC_TABLE + gc_header]
   shr  ecx, page_size_order
   mov  byte ptr [ecx + edx], 1  
   mov  [edi + ebx], eax
lEnd:

end

// ; unhook
inline % 1Dh

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  edx, fs:[2Ch]
  mov  ebx, [edx+ebx*4]

  mov  esp, [ebx + tls_catch_level]  
  mov  ebp, [ebx + tls_catch_frame]
  pop  edx
  mov  [ebx + tls_catch_frame], edx
  pop  edx
  mov  [ebx + tls_catch_level], edx
  pop  edx
  mov  [ebx + tls_catch_addr], edx

end

// ; ifheap - part of the command

inline % 096h

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  ecx, fs:[2Ch]
  mov  ebx, [ecx+ebx*4]
  lea  ebx, [ebx + tls_stack_bottom]

end

// ; hook label (ecx - offset)

inline % 0A6h

  // ; GCXT: get current thread frame
  mov  ebx, [data : %CORE_TLS_INDEX]
  mov  edx, fs:[2Ch]
  mov  ebx, [edx+ebx*4]

  call code : %HOOK
  push ebp
  push [ebx + tls_catch_addr]
  push [ebx + tls_catch_frame]
  push [ebx + tls_catch_level]  
  mov  [ebx + tls_catch_addr], ecx
  mov  [ebx + tls_catch_level], esp
  mov  [ebx + tls_catch_frame], ebp

end

// ; asavebi
inline %0C0h

  mov  esi, edi                     
  // calculate write-barrier address
  sub  esi, [data : %CORE_GC_TABLE + gc_start]
  mov  edx, [data : %CORE_GC_TABLE + gc_header]
  shr  esi, page_size_order
  mov  byte ptr [esi + edx], 1  

  mov [edi + __arg1], eax

end
