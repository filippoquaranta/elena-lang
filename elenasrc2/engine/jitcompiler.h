//---------------------------------------------------------------------------
//		E L E N A   P r o j e c t:  ELENA Compiler Engine
//
//		This file contains ELENA JIT compiler class.
//
//                                              (C)2005-2020, by Alexei Rakov
//---------------------------------------------------------------------------

#ifndef jitcompilerH
#define jitcompilerH 1

namespace _ELENA_
{

// --- ReferenceHelper ---

class _ReferenceHelper
{
public:
   virtual ref_t getLinkerConstant(ref_t constant) = 0;
   virtual SectionInfo getCoreSection(ref_t reference) = 0;
   virtual SectionInfo getSection(ref_t reference, _Module* module = nullptr) = 0;

   virtual void* getVAddress(MemoryWriter& writer, ref_t mask) = 0;

   virtual mssg_t resolveMessage(mssg_t reference, _Module* module = nullptr) = 0;

   virtual void writeReference(MemoryWriter& writer, ref_t reference, pos_t disp, _Module* module = NULL) = 0;
   virtual void writeReference(MemoryWriter& writer, void* vaddress, bool relative, pos_t disp) = 0;
   virtual void writeMTReference(MemoryWriter& writer) = 0;

   //// used for 64bit programming, currently only for mskVMTXMethodAddress and mskVMTXEntryOffset
   //virtual void writeXReference(MemoryWriter& writer, ref_t reference, ref64_t disp, _Module* module = NULL) = 0;

   virtual void addBreakpoint(pos_t position) = 0;
};

//// --- _BinaryHelper ---
//
//class _BinaryHelper
//{
//public:
//   virtual void writeReference(MemoryWriter& writer, ident_t reference, int mask) = 0;
//};

// --- JITCompiler class ---
class _JITCompiler
{
public:
   virtual size_t getObjectHeaderSize() const = 0;

   virtual bool isWithDebugInfo() const = 0;

   virtual void prepareCore(_ReferenceHelper& helper, _JITLoader* loader) = 0;

   virtual void alignCode(MemoryWriter* writer, int alignment, bool code) = 0;

   virtual void compileInt32(MemoryWriter* writer, int integer) = 0;
   virtual void compileInt64(MemoryWriter* writer, long long integer) = 0;
   virtual void compileReal64(MemoryWriter* writer, double number) = 0;
   virtual void compileLiteral(MemoryWriter* writer, const char* value) = 0;
   virtual void compileWideLiteral(MemoryWriter* writer, const wide_c* value) = 0;
   virtual void compileChar32(MemoryWriter* writer, const char* value) = 0;
   virtual void compileBinary(MemoryWriter* writer, _Memory* binary) = 0;
   virtual void compileCollection(MemoryWriter* writer, _Memory* binary) = 0;

   virtual void compileMessage(MemoryWriter* writer, mssg_t mssg) = 0;
   virtual void compileAction(MemoryWriter* writer, ref_t actionRef) = 0;
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, ref_t ref, int refOffset) = 0;
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, uintptr_t addr) = 0;

   virtual void compileSymbol(_ReferenceHelper& helper, MemoryReader& reader, MemoryWriter& codeWriter);
   virtual void compileProcedure(_ReferenceHelper& helper, MemoryReader& reader, MemoryWriter& codeWriter) = 0;

   virtual void allocateVariable(MemoryWriter& writer) = 0;
//   virtual void allocateArray(MemoryWriter& writer, size_t count) = 0;

   virtual int allocateTLSVariable(_JITLoader* loader) = 0;
   virtual void allocateThreadTable(_JITLoader* loader, int length) = 0;
   virtual int allocateVMTape(_JITLoader* loader, void* tape, pos_t length) = 0;

   virtual int allocateConstant(MemoryWriter& writer, size_t objectOffset) = 0;
   virtual void allocateVMT(MemoryWriter& vmtWriter, ref_t flags, pos_t vmtLength, pos_t staticSize) = 0;

   virtual int copyParentVMT(void* parentVMT, VMTEntry* entries) = 0;

   virtual uintptr_t findMethodAddress(void* refVMT, mssg_t messageID, size_t vmtLength) = 0;
   virtual pos_t findMethodIndex(void* refVMT, mssg_t messageID, size_t vmtLength) = 0;
   virtual ref_t findFlags(void* refVMT) = 0;
   virtual size_t findLength(void* refVMT) = 0;
//   virtual void* findClassPtr(void* refVMT) = 0;

   virtual void addVMTEntry(mssg_t message, uintptr_t codePosition, VMTEntry* entries, size_t& count) = 0;

   virtual void fixVMT(MemoryWriter& vmtWriter, uintptr_t classClassVAddress, uintptr_t packageParentVAddress, 
      size_t count, bool virtualMode) = 0;

////   virtual void loadNativeCode(_BinaryHelper& helper, MemoryWriter& writer, _Module* binary, _Memory* section) = 0;

   virtual void* getPreloadedReference(ref_t reference) = 0;

   virtual void setStaticRootCounter(_JITLoader* loader, size_t counter, bool virtualMode) = 0;
   virtual void setTLSKey(void* ptr) = 0;
   virtual void setThreadTable(void* ptr) = 0;
   virtual void setEHTable(void* ptr) = 0;
   virtual void setGCTable(void* ptr) = 0;
   virtual void setVoidParent(_JITLoader* loader, void* ptr, bool virtualMode) = 0;

   virtual void* getInvoker() = 0;

   virtual void generateProgramStart(MemoryDump& tape) = 0;
   virtual void generateSymbolCall(MemoryDump& tape, void* address) = 0;
   virtual void generateProgramEnd(MemoryDump& tape) = 0;
};

// --- JITCompiler32 class ---
class JITCompiler32 : public _JITCompiler
{
public:
   virtual void compileInt32(MemoryWriter* writer, int integer);
   virtual void compileInt64(MemoryWriter* writer, long long integer);
   virtual void compileReal64(MemoryWriter* writer, double number);
   virtual void compileLiteral(MemoryWriter* writer, const char* value);
   virtual void compileWideLiteral(MemoryWriter* writer, const wide_c* value);
   virtual void compileChar32(MemoryWriter* writer, const char* value);
   virtual void compileBinary(MemoryWriter* writer, _Memory* binary);
   virtual void compileCollection(MemoryWriter* writer, _Memory* binary);

   virtual void compileMessage(MemoryWriter* writer, mssg_t mssg);
   virtual void compileAction(MemoryWriter* writer, ref_t actionRef);
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, ref_t ref, int refOffset);
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, uintptr_t addr);

   virtual void allocateVariable(MemoryWriter& writer);
   virtual void allocateArray(MemoryWriter& writer, size_t count);

   // return VMT field position
   virtual int allocateConstant(MemoryWriter& writer, size_t objectOffset);

   virtual ref_t findFlags(void* refVMT);
   virtual size_t findLength(void* refVMT);
   virtual uintptr_t findMethodAddress(void* refVMT, mssg_t messageID, size_t vmtLength);
   virtual pos_t findMethodIndex(void* refVMT, mssg_t messageID, size_t vmtLength);
   virtual void* findClassPtr(void* refVMT);

   virtual void allocateVMT(MemoryWriter& vmtWriter, size_t flags, size_t vmtLength, size_t staticSize);
   virtual int copyParentVMT(void* parentVMT, VMTEntry* entries);
   virtual void addVMTEntry(mssg_t message, size_t codePosition, VMTEntry* entries, size_t& count);
   virtual void fixVMT(MemoryWriter& vmtWriter, uintptr_t classClassVAddress, uintptr_t packageParentVAddress, 
      size_t count, bool virtualMode);

   virtual void generateProgramStart(MemoryDump& tape);
   virtual void generateProgramEnd(MemoryDump& tape);
};

// --- JITCompiler64 class ---
class JITCompiler64 : public _JITCompiler
{
public:
   virtual void compileInt32(MemoryWriter* writer, int integer);
   virtual void compileInt64(MemoryWriter* writer, long long integer);
   virtual void compileReal64(MemoryWriter* writer, double number);
   virtual void compileLiteral(MemoryWriter* writer, const char* value);
   virtual void compileWideLiteral(MemoryWriter* writer, const wide_c* value);
   virtual void compileChar32(MemoryWriter* writer, const char* value);
   virtual void compileBinary(MemoryWriter* writer, _Memory* binary);
   virtual void compileCollection(MemoryWriter* writer, _Memory* binary);

   virtual void compileMessage(MemoryWriter* writer, mssg_t mssg);
   virtual void compileAction(MemoryWriter* writer, ref_t actionRef);
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, ref_t ref, int refOffset);
   virtual void compileMssgExtension(MemoryWriter* writer, mssg_t low, uintptr_t addr);

   virtual void allocateVariable(MemoryWriter& writer);
   virtual void allocateArray(MemoryWriter& writer, size_t count);

   // return VMT field position
   virtual int allocateConstant(MemoryWriter& writer, size_t objectOffset);

   virtual ref_t findFlags(void* refVMT);
   virtual size_t findLength(void* refVMT);
   virtual uintptr_t findMethodAddress(void* refVMT, mssg_t messageID, size_t vmtLength);
   virtual uintptr_t findMethodAddressX(void* refVMT, mssg64_t messageID, size_t vmtLength);
   virtual pos_t findMethodIndex(void* refVMT, mssg_t messageID, size_t vmtLength);
   virtual pos_t findMethodIndexX(void* refVMT, mssg64_t messageID, size_t vmtLength);
   virtual void* findClassPtr(void* refVMT);

   virtual void allocateVMT(MemoryWriter& vmtWriter, size_t flags, size_t vmtLength, size_t staticSize);
   virtual int copyParentVMT(void* parentVMT, VMTEntry* entries);
   virtual int copyParentVMTX(void* parentVMT, VMTXEntry* entries);
   virtual void addVMTEntry(mssg_t message, uintptr_t codePosition, VMTEntry* entries, size_t& count);
   virtual void addVMTXEntry(mssg64_t message, uintptr_t codePosition, VMTXEntry* entries, size_t& entryCount);
   virtual void fixVMT(MemoryWriter& vmtWriter, uintptr_t classClassVAddress, uintptr_t packageParentVAddress, size_t count, bool virtualMode);

   virtual void generateProgramStart(MemoryDump& tape);
   virtual void generateProgramEnd(MemoryDump& tape);
};

} // _ELENA_

#endif // jitcompilerH
