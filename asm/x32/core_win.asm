// --- Predefined References  --
define GC_ALLOC	            10001h
define HOOK                 10010h
define INVOKER              10011h
define INIT_RND             10012h
define INIT_ET              10015h
define ENDFRAME             10016h
define RESTORE_ET           10017h
define OPENFRAME            10019h
define CLOSEFRAME           1001Ah
define LOCK                 10021h
define UNLOCK               10022h
define LOAD_CALLSTACK       10024h
define BREAK                10026h
define EXPAND_HEAP          10028h

define GC_HEAP_ATTRIBUTE    00Dh

// ; in - eax - heap, ebx - size
// ; out - eax - heap
procedure % EXPAND_HEAP

  push 4
  push 00001000h
  push ecx
  push eax
  call extern 'dlls'KERNEL32.VirtualAlloc

  ret

end

// ; ebx - exception code
procedure % BREAK

  push 0
  push 0
  push 0
  push ebx
  call extern 'dlls'KERNEL32.RaiseException

end

procedure % INIT_RND

  sub  esp, 8h
  mov  eax, esp
  sub  esp, 10h
  lea  ebx, [esp]
  push eax 
  push ebx
  push ebx
  call extern 'dlls'KERNEL32.GetSystemTime
  call extern 'dlls'KERNEL32.SystemTimeToFileTime
  add  esp, 10h
  pop  eax
  pop  edx
  ret
  
end

// INVOKER(prevFrame, function, arg)
procedure % INVOKER

  // ; save registers
  mov  eax, [esp+8]   // ; function
  push esi
  mov  esi, [esp+8]   // ; prevFrame
  push edi
  mov  edi, [esp+20]  // ; arg
  push ecx
  push ebx
  push ebp

  // declare new frame
  push esi            // ; FrameHeader.previousFrame
  push 0              // ; FrameHeader.reserved
  mov  ebp, esp       // ; FrameHeader
  push edi            // ; arg

  call eax
  add  esp, 12        // ; clear FrameHeader+arg

  // ; restore registers
  pop  ebp
  pop  ebx
  pop  ecx
  pop  edi
  pop  esi
  ret

end
