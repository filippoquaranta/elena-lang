//---------------------------------------------------------------------------
//		E L E N A   P r o j e c t:  ELENA Compiler
//
//		This file contains ELENA compiler class.
//
//                                              (C)2005-2016, by Alexei Rakov
//---------------------------------------------------------------------------

#ifndef compilerH
#define compilerH

#include "project.h"
#include "parser.h"
#include "bcwriter.h"

namespace _ELENA_
{

// --- Compiler class ---
class Compiler
{
public:
   struct Parameter
   {
      int    offset;
      ref_t  subj_ref;
      size_t class_ref;
      int    size;

      Parameter()
      {
         offset = -1;
         subj_ref = 0;
         class_ref = 0;
         size = 0;
      }
      Parameter(int offset)
      {
         this->offset = offset;
         this->subj_ref = 0;
         this->class_ref = 0;
         this->size = 0;
      }
      Parameter(int offset, ref_t subj_ref)
      {
         this->offset = offset;
         this->subj_ref = subj_ref;
         this->class_ref = 0;
         this->size = 0;
      }
      Parameter(int offset, ref_t subj_ref, size_t class_ref, int size)
      {
         this->offset = offset;
         this->subj_ref = subj_ref;
         this->class_ref = class_ref;
         this->size = size;
      }
   };

   // InheritResult
   enum InheritResult
   {
      irNone = 0,
      irSuccessfull,
      irUnsuccessfull,
      irSealed,
      irInvalid,
      irObsolete
   };

   enum MethodHint
   {
      tpMask       = 0x0F,

      tpUnknown    = 0x00,
      tpSealed     = 0x01,
      tpClosed     = 0x02,
      tpNormal     = 0x03,
      tpDispatcher = 0x04,
      tpStackSafe  = 0x10,
      tpEmbeddable = 0x20,
      tpGeneric    = 0x40,
   };

   struct Unresolved
   {
      ident_t    fileName;
      ref_t      reference;
      _Module*   module;
      size_t     row;
      size_t     col;           // virtual column

      Unresolved()
      {
         reference = 0;
      }
      Unresolved(ident_t fileName, ref_t reference, _Module* module, size_t row, size_t col)
      {
         this->fileName = fileName;
         this->reference = reference;
         this->module = module;
         this->row = row;
         this->col = col;
      }
   };

   enum ObjectKind
   {
      okUnknown = 0,
   
      okObject,                       // param - class reference
      okSymbol,                       // param - reference
      okConstantSymbol,               // param - reference, extraparam - class reference
      okConstantClass,                // param - reference, extraparam - class reference
      okLiteralConstant,              // param - reference 
      okWideLiteralConstant,          // param - reference 
      okCharConstant,                 // param - reference
      okIntConstant,                  // param - reference 
      okLongConstant,                 // param - reference 
      okRealConstant,                 // param - reference 
      okMessageConstant,              // param - reference 
      okExtMessageConstant,           // param - reference 
      okSignatureConstant,            // param - reference 
      okVerbConstant,                 // param - reference 
      okField,                        // param - field offset
      okFieldAddress,                 // param - field offset
      okOuter,                        // param - field offset
      okOuterField,                   // param - field offset, extraparam - outer field offset
      okLocal,                        // param - local / out parameter offset, extraparam : -1 indicates boxable / class reference for constructor call
      okParam,                        // param - parameter offset
      okParamField,
      okSubject,                      // param - parameter offset
      okThisParam,                    // param - parameter offset
      okNil,
      okSuper,
      okLocalAddress,                 // param - local offset, class reference
      okParams,                       // param - local offset
      okBlockLocal,                   // param - local offset
      okConstantRole,                 // param - role reference
   
      okTemplateTarget,
      okTemplateLocal,
   //   okTemplateTarget,

      okExternal,
      okInternal,
   };
   
   struct ObjectInfo
   {
      ObjectKind kind;
      ref_t      param;
      ref_t      extraparam;
      ref_t      type;
   
      ObjectInfo()
      {
         this->kind = okUnknown;
         this->param = 0;
         this->extraparam = 0;
         this->type = 0;
      }
      ObjectInfo(ObjectKind kind)
      {
         this->kind = kind;
         this->param = 0;
         this->extraparam = 0;
         this->type = 0;
      }
      ObjectInfo(ObjectKind kind, ObjectInfo copy)
      {
         this->kind = kind;
         this->param = copy.param;
         this->extraparam = copy.extraparam;
         this->type = copy.type;
      }
      ObjectInfo(ObjectKind kind, ref_t param)
      {
         this->kind = kind;
         this->param = param;
         this->extraparam = 0;
         this->type = 0;
      }
      ObjectInfo(ObjectKind kind, ref_t param, ref_t extraparam)
      {
         this->kind = kind;
         this->param = param;
         this->extraparam = extraparam;
         this->type = 0;
      }
      ObjectInfo(ObjectKind kind, ref_t param, ref_t extraparam, ref_t type)
      {
         this->kind = kind;
         this->param = param;
         this->extraparam = extraparam;
         this->type = type;
      }
   };

   typedef Map<ident_t, ref_t, false>     ForwardMap;
   typedef Map<ident_t, Parameter, false> LocalMap;
   typedef Map<ref_t, ref_t>              SubjectMap;
   typedef Map<ref_t, ref_t>              ClassMap;
   typedef Map<int, ref_t>                RoleMap;
   typedef List<Unresolved>               Unresolveds;
   typedef Map<ref_t, SubjectMap*>        ExtensionMap;

   struct TemplateInfo
   {
      ref_t   templateRef;
      ref_t   templateParent;
      ref_t   targetType;
      int     targetOffset;
      ref_t   messageSubject;
      ref_t   ownerRef;

      int     sourceCol, sourceRow;

      RoleMap parameters;

      TemplateInfo()
      {
         targetType = 0;
         templateRef = 0;
         templateParent = 0;
         targetOffset = -1;
         messageSubject = 0;
         ownerRef = 0;

         sourceCol = sourceRow = 0;
      }

      TemplateInfo(ref_t templateRef, ref_t targetType)
      {
         this->templateRef = templateRef;
         this->templateParent = 0;
         this->targetType = targetType;
         this->targetOffset = -1;
         this->messageSubject = 0;
         this->ownerRef = 0;

         this->sourceCol = this->sourceRow = 0;
      }

      TemplateInfo(ref_t templateRef, ref_t targetType, int targetOffset)
      {
         this->templateRef = templateRef;
         this->templateParent = 0;
         this->targetType = targetType;
         this->targetOffset = targetOffset;
         this->messageSubject = 0;
         this->ownerRef = 0;

         this->sourceCol = this->sourceRow = 0;
      }
   };

private:
   // - ModuleScope -
   struct ModuleScope
   {
      Project*       project;
      _Module*       module;
      _Module*       debugModule;

      ident_t        sourcePath;
      int            sourcePathRef;

      // default namespaces
      List<ident_t> defaultNs;
      ForwardMap    forwards;       // forward declarations

      // symbol hints
      Map<ref_t, ref_t> constantHints;

      // extensions
      SubjectMap        extensionHints; 
      ExtensionMap      extensions;

      // type hints
      MessageMap        subjects;
      SubjectMap        subjectHints;

      // action hints
      SubjectMap        actionHints;

      // templates
      SyntaxTree        templates;

      // cached references
      ref_t superReference;
      ref_t intReference;
      ref_t longReference;
      ref_t realReference;
      ref_t literalReference;
      ref_t wideReference;
      ref_t charReference;
      ref_t trueReference;
      ref_t falseReference;
      ref_t paramsReference;
      ref_t signatureReference;
      ref_t messageReference;
      ref_t verbReference;
      ref_t arrayReference;

      // cached subjects / hints
      ref_t sealedHint;
      ref_t limitedHint;
      ref_t integerHint;
      ref_t realHint;
      ref_t literalHint;
      ref_t varHint;
      ref_t signHint;
      ref_t mssgHint;
      ref_t stackHint;
      ref_t warnHint;
      ref_t dynamicHint;
      ref_t constHint;
      ref_t structHint;
      ref_t structOfHint;
      ref_t embedHint;
      ref_t extensionHint;
      ref_t extensionOfHint;
      ref_t genericHint;
      ref_t actionHint;
      ref_t nonstructHint;
      ref_t symbolHint;
      ref_t extMssgHint;
      ref_t groupHint;

      ref_t boolType;

      // warning mapiing
      bool warnOnUnresolved;
      bool warnOnWeakUnresolved;
      int  warningMask;

      // list of references to the current module which should be checked after the project is compiled
      Unresolveds* forwardsUnresolved;

      // list of typified classes which may need get&type message
      ClassMap typifiedClasses;

      ObjectInfo mapObject(TerminalInfo identifier);

      ref_t mapReference(ident_t reference, bool existing = false);

      ObjectInfo mapReferenceInfo(ident_t reference, bool existing = false);

      void defineConstantSymbol(ref_t reference, ref_t classReference)
      {
         constantHints.add(reference, classReference);
      }

      void raiseError(const char* message, int row, int col, ident_t terminal);
      void raiseWarning(int level, const char* message, int row, int col, ident_t terminal);

      void raiseError(const char* message, TerminalInfo terminal);
      void raiseWarning(int level, const char* message, TerminalInfo terminal);

      bool checkReference(ident_t referenceName);

      ref_t resolveIdentifier(ident_t name);

      ref_t mapNewSubject(ident_t terminal);

      // NOTE : the function returns 0 for implicit subjects
      // in any case output is set (for explicit one - the namespace is copied as well)
      ref_t mapSubject(TerminalInfo terminal, IdentifierString& output);
      ref_t mapSubject(TerminalInfo terminal, bool explicitOnly = true);
      ref_t resolveSubjectRef(ident_t name, bool explicitOnly = true);

      ref_t mapTerminal(TerminalInfo terminal, bool existing = false);

      ObjectInfo defineObjectInfo(ref_t reference, bool checkState = false);

      ref_t loadClassInfo(ClassInfo& info, ident_t vmtName, bool headerOnly = false);
      ref_t loadClassInfo(ClassInfo& info, ref_t reference, bool headerOnly = false)
      {
         return loadClassInfo(info, module->resolveReference(reference), headerOnly);
      }
      ref_t loadSymbolExpressionInfo(SymbolExpressionInfo& info, ident_t symbol);

      _Memory* loadTemplateInfo(ref_t reference, _Module* &argModule)
      {
         return loadTemplateInfo(module->resolveSubject(reference), argModule);
      }
      _Memory* loadTemplateInfo(ident_t symbol, _Module* &argModule);

//      bool recognizePrimitive(ident_t name, ident_t value, size_t& roleMask, int& size);

      int defineStructSize(ref_t classReference, bool embeddableOnly = true);
      int defineSubjectSize(ref_t type_ref, bool embeddableOnly = true);

      int checkMethod(ClassInfo& info, ref_t message, ref_t& outputType);
      int checkMethod(ref_t reference, ref_t message, bool& found, ref_t& outputType);
      int checkMethod(ref_t reference, ref_t message)
      {
         bool dummy;
         ref_t dummyRef;
         return checkMethod(reference, message, dummy, dummyRef);
      }

      void loadSubjects(_Module* module);
      void loadExtensions(TerminalInfo terminal, _Module* module);
      void loadActions(_Module* module);

      void saveSubject(ref_t type_ref, ref_t classReference, bool internalType);
      void saveTemplate(ref_t template_ref);
      bool saveExtension(ref_t message, ref_t type, ref_t role);
      void saveAction(ref_t message, ref_t reference);

      void validateReference(TerminalInfo terminal, ref_t reference);

      ref_t getBaseLazyExpressionClass();

      int getClassFlags(ref_t reference);
      int getTypeFlags(ref_t subject)
      {
         return getClassFlags(subjectHints.get(subject));
      }

      bool checkIfCompatible(ref_t typeRef, ref_t classRef);
//      ref_t defineType(ref_t classRef);

      void importClassInfo(ClassInfo& copy, ClassInfo& target, _Module* exporter, bool headerOnly);

      void loadModuleInfo(_Module* extModule)
      {
         loadSubjects(extModule);
         loadExtensions(TerminalInfo(), extModule);
         loadActions(extModule);
      }

      ref_t mapNestedExpression();
      ref_t mapNestedTemplate();

      ModuleScope(Project* project, ident_t sourcePath, _Module* module, _Module* debugModule, Unresolveds* forwardsUnresolved);
   };

   // - Scope -
   struct Scope
   {
      enum ScopeLevel
      {
         slClass,
         slSymbol,
         slMethod,
         slCode,
         slOwnerClass,
         slTemplate,
      };

      ModuleScope* moduleScope;
      Scope*       parent;

      void raiseError(const char* message, TerminalInfo terminal)
      {
         moduleScope->raiseError(message, terminal);
      }
      void raiseWarning(int level, const char* message, TerminalInfo terminal)
      {
         moduleScope->raiseWarning(level, message, terminal);
      }
      void raiseError(const char* message, SyntaxTree::Node node)
      {
         SyntaxTree::Node row = SyntaxTree::findChild(node, lxRow);
         SyntaxTree::Node col = SyntaxTree::findChild(node, lxCol);
         SyntaxTree::Node terminal = SyntaxTree::findChild(node, lxTerminal);

         moduleScope->raiseError(message, row.argument, col.argument, terminal.identifier());
      }
      void raiseWarning(int level, const char* message, SyntaxTree::Node node)
      {
         SyntaxTree::Node row = SyntaxTree::findChild(node, lxRow);
         SyntaxTree::Node col = SyntaxTree::findChild(node, lxCol);
         SyntaxTree::Node terminal = SyntaxTree::findChild(node, lxTerminal);

         moduleScope->raiseWarning(level, message, row.argument, col.argument, terminal.identifier());
      }

      virtual ObjectInfo mapObject(TerminalInfo identifier)
      {
         if (parent) {
            return parent->mapObject(identifier);
         }
         else return moduleScope->mapObject(identifier);
      }

      virtual Scope* getScope(ScopeLevel level)
      {
         if (parent) {
            return parent->getScope(level);
         }
         else return NULL;
      }

      virtual ref_t mapSubject(TerminalInfo terminal, IdentifierString& output)
      {
         if (parent) {
            return parent->mapSubject(terminal, output);
         }
         else return moduleScope->mapSubject(terminal, output);
      }

      virtual bool isVirtualSubject(TerminalInfo terminal)
      {
         if (parent) {
            return parent->isVirtualSubject(terminal);
         }
         else return false;
      }

      virtual ref_t mapSubject(TerminalInfo terminal, bool implicitOnly = true)
      {
         if (parent) {
            return parent->mapSubject(terminal, implicitOnly);
         }
         else return moduleScope->mapSubject(terminal, implicitOnly);
      }

      Scope(ModuleScope* moduleScope)
      {
         this->parent = NULL;
         this->moduleScope = moduleScope;
      }
      Scope(Scope* parent)
      {
         this->parent = parent;
         this->moduleScope = parent->moduleScope;
      }
   };

   // - SourceScope -
   struct SourceScope : public Scope
   {
      SyntaxTree syntaxTree;

      CommandTape    tape;
      ref_t          reference;

      SourceScope(ModuleScope* parent, ref_t reference);
   };

   // - ClassScope -
   struct ClassScope : public SourceScope
   {
      ClassInfo   info;
      ref_t       extensionMode;

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      void compileClassHint(SyntaxTree::Node hint);
      //void compileFieldHints(DNode hints, int& size, ref_t& type);

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slClass || level == slOwnerClass) {
            return this;
         }
         else return Scope::getScope(level);
      }

      virtual bool validateTemplate(ref_t hintRef)
      {
         _Module* extModule = NULL;
         return moduleScope->loadTemplateInfo(hintRef, extModule) != 0;
      }

      void save()
      {
         // save class meta data
         MemoryWriter metaWriter(moduleScope->module->mapSection(reference | mskMetaRDataRef, false), 0);
         metaWriter.Memory()->trim(0);
         info.save(&metaWriter);
      }

      bool include(ref_t message)
      {
         // check if the method is inhreited and update vmt size accordingly
         ClassInfo::MethodMap::Iterator it = info.methods.getIt(message);
         if (it.Eof()) {
            info.methods.add(message, true);
         
            return true;
         }
         else {
            (*it) = true;
         
            return false;
         }
      }

      ClassScope(ModuleScope* parent, ref_t reference);
   };

   // - SymbolScope -
   struct SymbolScope : public SourceScope
   {
      bool  constant;
      ref_t typeRef;      

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slSymbol) {
            return this;
         }
         else return Scope::getScope(level);
      }

      SymbolScope(ModuleScope* parent, ref_t reference);
   };

   // - MethodScope -
   struct MethodScope : public Scope
   {
      CommandTape* tape;

      ref_t        message;
      LocalMap     parameters;
      int          reserved;           // defines inter-frame stack buffer (excluded from GC frame chain)
      int          rootToFree;         // by default is 1, for open argument - contains the list of normal arguments as well
      bool         withOpenArg;
      bool         stackSafe;
//      bool         embeddable;
      bool         generic;

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slMethod) {
            return this;
         }
         else return parent->getScope(level);
      }

      ref_t getReturningType() const
      {
         return ((ClassScope*)parent)->info.methodHints.get(ClassInfo::Attribute(message, maType));
      }

      ref_t getClassFlags(bool ownerClass = true)
      {
         ClassScope* scope = (ClassScope*)getScope(ownerClass ? slOwnerClass : slClass);

         return scope ? scope->info.header.flags : 0;
      }

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      MethodScope(ClassScope* parent);
   };

   // - ActionScope -
   struct ActionScope : public MethodScope
   {
      ActionScope(ClassScope* parent);

      virtual ObjectInfo mapObject(TerminalInfo identifier);
   };

   // - CodeScope -
   struct CodeScope : public Scope
   {
      SyntaxWriter* writer;
//      int           rootBookmark;   // !! should be removed??

      // scope local variables
      LocalMap     locals;
      int          level;

      // scope stack allocation
      int          reserved;  // allocated for the current statement
      int          saved;     // permanently allocated

      int newLocal()
      {
         level++;

         return level;
      }

      void mapLocal(ident_t local, int level, ref_t type)
      {
         locals.add(local, Parameter(level, type));
      }
      void mapLocal(ident_t local, int level, ref_t type, size_t class_ref, int size)
      {
         locals.add(local, Parameter(level, type, class_ref, size));
      }

      void freeSpace()
      {
         reserved = saved;
      }

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slCode) {
            return this;
         }
         else return parent->getScope(level);
      }

      int getMessageID()
      {
         MethodScope* scope = (MethodScope*)getScope(slMethod);

         return scope ? scope->message : 0;
      }

      ref_t getClassRefId(bool ownerClass = true)
      {
         ClassScope* scope = (ClassScope*)getScope(ownerClass ? slOwnerClass : slClass);

         return scope ? scope->reference : 0;
      }

      ref_t getFieldType(int offset, bool ownerClass = true)
      {
         ClassScope* scope = (ClassScope*)getScope(ownerClass ? slOwnerClass : slClass);

         return scope ? scope->info.fieldTypes.get(offset) : 0;
      }

      ref_t getClassFlags(bool ownerClass = true)
      {
         ClassScope* scope = (ClassScope*)getScope(ownerClass ? slOwnerClass : slClass);

         return scope ? scope->info.header.flags : 0;
      }

      CodeScope(SymbolScope* parent, SyntaxWriter* writer);
      CodeScope(MethodScope* parent, SyntaxWriter* writer);
      CodeScope(CodeScope* parent);
   };

   // - InlineClassScope -

   struct InlineClassScope : public ClassScope
   {
      struct Outer
      {
         int        reference;
         ObjectInfo outerObject;

         Outer()
         {
            reference = -1;
         }
         Outer(int reference, ObjectInfo outerObject)
         {
            this->reference = reference;
            this->outerObject = outerObject;
         }
      };

      bool                    templateMode;
      ref_t                   templateRef;
      Map<ident_t, Outer>     outers;
      ClassInfo::FieldTypeMap outerFieldTypes;

      Outer mapSelf();

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slClass) {
            return this;
         }
         else return Scope::getScope(level);
      }

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      InlineClassScope(CodeScope* owner, ref_t reference);
   };

   // --- TemplateScope ---
   struct TemplateScope : public ClassScope
   {
      enum Type
      {
         ttNone = 0,
         ttClass,
         ttField,
         ttMethod
      };

      ref_t       templateRef;
      Type        type;
      ForwardMap  parameters;

      // NOTE : reference is defined in subject namespace, so templateRef should be initialized and used
      // proper reference is 0 in this case
      TemplateScope(ModuleScope* parent, ref_t reference);

      virtual ObjectInfo mapObject(TerminalInfo identifier);

      virtual bool validateTemplate(ref_t reference);

      virtual ref_t mapSubject(TerminalInfo terminal, IdentifierString& output)
      {
         ref_t parameter = parameters.get(terminal);
         if (parameter != 0) {
            int offset = output.Length();

            output.append(TARGET_POSTFIX);
            output.appendInt((int)parameter);            

            return moduleScope->module->mapSubject(output + offset, false);
         }
         else return moduleScope->mapSubject(terminal, output);
      }

      virtual bool isVirtualSubject(TerminalInfo terminal)
      {
         return parameters.exist(terminal);
      }

      virtual ref_t mapSubject(TerminalInfo terminal, bool implicitOnly = true)
      {
         ref_t parameter = parameters.get(terminal);
         if (parameter != 0) {
            IdentifierString output;
            output.copy(TARGET_POSTFIX);
            output.appendInt((int)parameter);

            return moduleScope->module->mapSubject(output, false);
         }
         else return Scope::mapSubject(terminal, implicitOnly);
      }

      virtual Scope* getScope(ScopeLevel level)
      {
         if (level == slTemplate) {
            return this;
         }
         else return ClassScope::getScope(level);
      }

      void save()
      {
         _Memory* section = moduleScope->module->mapSection(templateRef | mskSyntaxTreeRef, false);
         section->trim(0);

         syntaxTree.save(section);
      }
   };

   ByteCodeWriter _writer;
   Parser         _parser;

   MessageMap     _verbs;                            // list of verbs
   MessageMap     _operators;                        // list of operators

   int            _optFlag;

   // optimization rules
   TransformTape _rules;

   // optmimization routines
   bool applyRules(CommandTape& tape);
   bool optimizeIdleBreakpoints(CommandTape& tape);
   bool optimizeJumps(CommandTape& tape);
   void optimizeTape(CommandTape& tape);
   
   void recordDebugStep(CodeScope& scope, TerminalInfo terminal, int stepType)
   {
      if (terminal != nsNone) {
         scope.writer->newNode(lxBreakpoint, stepType);
         scope.writer->appendNode(lxRow, terminal.row);
         scope.writer->appendNode(lxCol, terminal.disp);
         scope.writer->appendNode(lxLength, terminal.length);
         scope.writer->closeNode();
      }
   }
   void recordDebugVirtualStep(CodeScope& scope, int stepType)
   {
      scope.writer->newNode(lxBreakpoint, stepType);
      scope.writer->closeNode();
   }

   void raiseWarning(ModuleScope& scope, SNode node, ident_t message, int warningLevel, int warningMask, bool triggered = true);

   void appendObjectInfo(CodeScope& scope, ObjectInfo object);
   void writeMessage(ModuleScope& scope, SyntaxWriter& writer, ref_t messageRef);
   ref_t mapHint(DNode hint, ModuleScope& scope);

   bool checkIfCompatible(ModuleScope& scope, ref_t typeRef, SyntaxTree::Node node);
   bool checkIfImplicitBoxable(ModuleScope& scope, ref_t sourceClassRef, ClassInfo& targetInfo);
   ref_t resolveObjectReference(CodeScope& scope, ObjectInfo object);

   ref_t mapNestedExpression(CodeScope& scope);
   ref_t mapExtension(CodeScope& scope, ref_t messageRef, ObjectInfo target);

   void importCode(DNode node, ModuleScope& scope, SyntaxWriter& writer, ident_t reference, ref_t message);

   InheritResult inheritClass(ClassScope& scope, ref_t parentRef, bool ignoreSealed);

   ref_t declareInlineTemplate(ModuleScope& scope, SNode node, TemplateInfo& templateInfo, ref_t inlineTemplateRef);

   void declareParameterDebugInfo(MethodScope& scope, SyntaxWriter& writer, bool withThis, bool withSelf);

   void declareTemplateParameters(DNode hint, ModuleScope& scope, RoleMap& parameters);
   bool declareTemplateInfo(DNode hint, ClassScope& scope, ref_t hintRef, ref_t messageSubject = 0);
   void declareFieldTemplateInfo(SyntaxTree::Node node, ClassScope& scope, ref_t hintRef, int fieldOffset);
   void importTemplateInfo(SyntaxTree::Node node, ModuleScope& scope, ref_t ownerRef, _Module* templateModule, TemplateInfo& info);
   void readTemplateInfo(SyntaxTree::Node node, TemplateInfo& info);
   void copyTemplateInfo(SyntaxTree::Node node, SyntaxTree::Writer& writer);

   void compileParentDeclaration(DNode baseNode, ClassScope& scope, ref_t parentRef, bool ignoreSealed = false);
   void compileParentDeclaration(DNode node, ClassScope& scope);
   void compileFieldDeclarations(DNode& member, SyntaxWriter& writer, ClassScope& scope); 
   void compileTemplateFieldDeclaration(DNode& node, SyntaxWriter& writer, TemplateScope& scope);

   void compileSymbolHints(DNode hints, SymbolScope& scope, bool silentMode);
   bool compileClassHint(DNode hint, SyntaxWriter& writer, ClassScope& scope, bool directiveOnly);
   void compileClassHints(DNode hints, SyntaxWriter& writer, ClassScope& scope);
   void compileSingletonHints(DNode hints, SyntaxWriter& writer, ClassScope& scope);

   void compileTemplateHints(DNode hints, SyntaxWriter& writer, TemplateScope& scope);
   void compileLocalHints(DNode hints, CodeScope& scope, ref_t& type, ref_t& classRef, int& size);
   void compileFieldHints(DNode hints, SyntaxWriter& writer, ClassScope& scope);
   void compileMethodHints(DNode hints, SyntaxWriter& writer, MethodScope& scope, bool warningsOnly);
   void declareVMT(DNode member, SyntaxWriter& writer, ClassScope& scope, Symbol methodSymbol);

   bool declareImportedTemplates(ClassScope& scope, SyntaxWriter& writer);
   bool declareTemplate(ClassScope& scope, SyntaxWriter& writer, TemplateInfo& templateInfo);
   void importTemplates(ClassScope& scope, SyntaxWriter& writerf);
   void importTemplate(ClassScope& scope, SyntaxWriter& writer, TemplateInfo& templateInfo);
   void importTemplateTree(ClassScope& scope, SyntaxWriter& writer, SyntaxTree::Node node, TemplateInfo& info, _Module* templateModule);
   void importNode(ClassScope& scope, SyntaxTree::Node node, SyntaxWriter& writer, _Module* templateModule, TemplateInfo& info);
   void importTree(ClassScope& scope, SyntaxTree::Node node, SyntaxWriter& writer, _Module* templateModule, TemplateInfo& info);
//   bool validateMethodTemplate(SyntaxTree::Node node, ref_t& targetMethod);

   ref_t mapMessage(DNode node, CodeScope& scope, size_t& count, bool& argsUnboxing);
   ref_t mapMessage(DNode node, CodeScope& scope, size_t& count)
   {
      bool dummy = false;
      return mapMessage(node, scope, count, dummy);
   }

   void compileSwitch(DNode node, CodeScope& scope, ObjectInfo switchValue);
   void compileVariable(DNode node, CodeScope& scope, DNode hints);

   ObjectInfo compileClosure(DNode node, CodeScope& ownerScope, int mode);
   ObjectInfo compileClosure(DNode node, CodeScope& ownerScope, InlineClassScope& scope, int mode);
   ObjectInfo compileCollection(DNode objectNode, CodeScope& scope, int mode);
   ObjectInfo compileCollection(DNode objectNode, CodeScope& scope, int mode, ref_t vmtReference);

   ObjectInfo compileMessageReference(DNode objectNode, CodeScope& scope);
   void writeTerminal(TerminalInfo terminal, CodeScope& scope, ObjectInfo object);

   ObjectInfo compileTerminal(DNode node, CodeScope& scope);
   ObjectInfo compileObject(DNode objectNode, CodeScope& scope, int mode);

   int mapOpArg(Compiler::ModuleScope& scope, SNode arg, ref_t& target);
   int mapOpArg(Compiler::ModuleScope& scope, SNode arg);

   ObjectInfo compileOperator(DNode& node, CodeScope& scope, ObjectInfo object, int mode, int operator_id);
   ObjectInfo compileOperator(DNode& node, CodeScope& scope, ObjectInfo object, int mode);
   ObjectInfo compileBranchingOperator(DNode& node, CodeScope& scope, ObjectInfo object, int mode, int operator_id);

   ref_t compileMessageParameters(DNode node, CodeScope& scope);

   ObjectInfo compileMessage(DNode node, CodeScope& scope, ObjectInfo object);
   ObjectInfo compileMessage(DNode node, CodeScope& scope, ObjectInfo object, int messageRef, int mode);
   ObjectInfo compileExtensionMessage(DNode node, CodeScope& scope, ObjectInfo object, ObjectInfo role/*, int mode*/);

   ObjectInfo compileNewOperator(DNode node, CodeScope& scope, int mode);
   ObjectInfo compileAssigning(DNode node, CodeScope& scope, ObjectInfo target, int mode);
   ObjectInfo compileOperations(DNode node, CodeScope& scope, ObjectInfo target, int mode);   
   ObjectInfo compileExtension(DNode& node, CodeScope& scope, ObjectInfo object, int mode);
   ObjectInfo compileExpression(DNode node, CodeScope& scope, ref_t targetType, int mode);
   ObjectInfo compileRetExpression(DNode node, CodeScope& scope, int mode);
   ObjectInfo compileAssigningExpression(DNode node, DNode assigning, CodeScope& scope, ObjectInfo target, int mode = 0);

   ObjectInfo compileBranching(DNode thenNode, CodeScope& scope/*, ObjectInfo target, int verb, int subCodinteMode*/);

   void compileLoop(DNode node, CodeScope& scope);
   void compileThrow(DNode node, CodeScope& scope, int mode);
//   void compileTry(DNode node, CodeScope& scope);
//   void compileLock(DNode node, CodeScope& scope);

   void compileExternalArguments(DNode node, CodeScope& scope/*, ExternalScope& externalScope*/);

   int allocateStructure(bool bytearray, int& allocatedSize, int& reserved);
   int allocateStructure(ModuleScope& scope, SyntaxTree::Node node, int& size);
   bool allocateStructure(CodeScope& scope, int size, int flags, bool bytearray, ObjectInfo& exprOperand);

   ObjectInfo compileExternalCall(DNode node, CodeScope& scope, ident_t dllName, int mode);
   ObjectInfo compileInternalCall(DNode node, CodeScope& scope, ObjectInfo info);

   void compileConstructorResendExpression(DNode node, CodeScope& scope, ClassScope& classClassScope, bool& withFrame);
   void compileConstructorDispatchExpression(DNode node, SyntaxWriter& writer, CodeScope& scope);
   void compileResendExpression(DNode node, CodeScope& scope, CommandTape* tape);
   void compileDispatchExpression(DNode node, CodeScope& scope, CommandTape* tape);

   ObjectInfo compileCode(DNode node, CodeScope& scope);

   void declareArgumentList(DNode node, MethodScope& scope, DNode hints);
   ref_t declareInlineArgumentList(DNode node, MethodScope& scope);
   bool declareActionScope(DNode& node, ClassScope& scope, DNode argNode, SyntaxWriter& writer, ActionScope& methodScope, int mode, bool alreadyDeclared);

   void declareSingletonClass(DNode member, DNode parentNode, ClassScope& scope, DNode hints);
   void compileSingletonClass(DNode member, ClassScope& scope);

   void declareSingletonAction(ClassScope& scope, DNode objNode, DNode expression, DNode hints);

   void compileActionMethod(DNode member, SyntaxWriter& writer, MethodScope& scope);
   void compileLazyExpressionMethod(DNode member, SyntaxWriter& writer, MethodScope& scope);
   void compileDispatcher(DNode node, SyntaxWriter& writer, MethodScope& scope, bool withGenericMethods = false);
   
   void compileMethod(DNode node, SyntaxWriter& writer, MethodScope& scope);
   void compileDefaultConstructor(MethodScope& scope, SyntaxWriter& writer, ClassScope& classClassScope);
   void compileDynamicDefaultConstructor(MethodScope& scope, SyntaxWriter& writer, ClassScope& classClassScope);
   void compileConstructor(DNode node, SyntaxWriter& writer, MethodScope& scope, ClassScope& classClassScope, ref_t embeddedMethodRef = 0);
//   void compileEmbeddableConstructor(DNode node, SyntaxWriter& writer, MethodScope& scope, ClassScope& classClassScope);

   void compileSymbolCode(ClassScope& scope);
   void compileVirtualTypecastMethod(SyntaxWriter& writer, MethodScope& scope, LexicalType target, int argument = 0);
   void compileVirtualDispatchMethod(SyntaxWriter& writer, MethodScope& scope, LexicalType target, int argument = 0);

   void compileAction(DNode node, ClassScope& scope, DNode argNode, int mode, bool alreadyDeclared = false);
   void compileNestedVMT(DNode node, DNode parent, InlineClassScope& scope);

   void compileVMT(DNode member, SyntaxWriter& writer, ClassScope& scope, bool warningsOnly = true);

   void compileVirtualMethods(SyntaxWriter& writer, ClassScope& scope);
   void declareVirtualMethods(ClassScope& scope);

   ref_t generateTemplate(ModuleScope& scope, TemplateInfo& templateInfo, ref_t reference);

   void generateClassFlags(ClassScope& scope, SyntaxTree::Node root);
   void generateClassFields(ClassScope& scope, SyntaxTree::Node root);
   void generateMethodHints(ClassScope& scope, SyntaxTree::Node node, ref_t message);
   void generateMethodDeclarations(ClassScope& scope, SyntaxTree::Node root, bool closed);
   void generateClassDeclaration(ClassScope& scope, bool closed);
   void generateInlineClassDeclaration(ClassScope& scope, bool closed);

   void generateClassImplementation(ClassScope& scope);

   void compileClassDeclaration(DNode node, ClassScope& scope, DNode hints);
   void compileClassImplementation(DNode node, ClassScope& scope);
   void compileTemplateDeclaration(DNode node, TemplateScope& scope, DNode hints);
   void compileClassClassDeclaration(DNode node, ClassScope& classClassScope, ClassScope& classScope);
   void compileClassClassImplementation(DNode node, ClassScope& classClassScope, ClassScope& classScope);
   void compileSymbolDeclaration(DNode node, SymbolScope& scope, DNode hints);
   void compileSymbolImplementation(DNode node, SymbolScope& scope, DNode hints, bool isStatic);
   void compileIncludeModule(DNode node, ModuleScope& scope, DNode hints);
   void declareSubject(DNode& member, ModuleScope& scope, DNode hints);
   void compileSubject(DNode& member, ModuleScope& scope, DNode hints);

   void compileDeclarations(DNode member, ModuleScope& scope);
   void compileImplementations(DNode member, ModuleScope& scope);
   void compileIncludeSection(DNode& node, ModuleScope& scope);

   virtual void compileModule(DNode node, ModuleScope& scope);

   void compile(ident_t source, MemoryDump* buffer, ModuleScope& scope);

   bool validate(Project& project, _Module* module, int reference);
   void validateUnresolved(Unresolveds& unresolveds, Project& project);

//   void compileWarningHints(ModuleScope& scope, DNode hints, SyntaxWriter& writer);

   int tryTypecasting(ModuleScope& scope, ref_t targetType, SNode& node, SNode& object, bool& typecasted, int mode);

   void optimizeAssigning(ModuleScope& scope, SyntaxTree::Node node, int warningLevel);
   bool boxPrimitive(ModuleScope& scope, SyntaxTree::Node& node, ref_t targetRef, int warningLevel, int mode);
   void optimizeExtCall(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeInternalCall(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeDirectCall(ModuleScope& scope, SyntaxTree::Node node, int warningLevel);
   void optimizeCall(ModuleScope& scope, SyntaxTree::Node node, int warningLevel);
   void optimizeEmbeddableCall(ModuleScope& scope, SyntaxTree::Node& assignNode, SyntaxTree::Node& callNode);
   bool optimizeOp(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeNewOp(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeBoolOp(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);

   void defineTargetSize(ModuleScope& scope, SNode& node);

   void optimizeBoxing(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeTypecast(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeSyntaxNode(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode);
   void optimizeSyntaxExpression(ModuleScope& scope, SyntaxTree::Node node, int warningLevel, int mode = 0);
   void optimizeClassTree(ClassScope& scope);
   void optimizeSymbolTree(SourceScope& scope);

   bool recognizeEmbeddableGet(ModuleScope& scope, SyntaxTree& tree, SyntaxTree::Node node, ref_t returningType, ref_t& subject);
   bool recognizeEmbeddableIdle(SyntaxTree& tree, SyntaxTree::Node node);
   void defineEmbeddableAttributes(ClassScope& scope, SyntaxTree::Node node);

public:
   void loadRules(StreamReader* optimization);
   void turnOnOptimiation(int level)
   {
      _optFlag |= level;
   }

   bool run(Project& project);

   Compiler(StreamReader* syntax);
};

} // _ELENA_

#endif // compilerH
