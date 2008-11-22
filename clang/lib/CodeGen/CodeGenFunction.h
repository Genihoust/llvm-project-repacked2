//===-- CodeGenFunction.h - Per-Function state for LLVM CodeGen -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the internal per-function state used for llvm translation. 
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CODEGEN_CODEGENFUNCTION_H
#define CLANG_CODEGEN_CODEGENFUNCTION_H

#include "clang/AST/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"

#include <vector>
#include <map>

#include "CGBuilder.h"
#include "CGCall.h"
#include "CGValue.h"

namespace llvm {
  class BasicBlock;
  class Module;
  class SwitchInst;
  class Value;
}

namespace clang {
  class ASTContext;
  class Decl;
  class EnumConstantDecl;
  class FunctionDecl;
  class FunctionTypeProto;
  class LabelStmt;
  class ObjCInterfaceDecl;
  class ObjCIvarDecl;
  class ObjCMethodDecl;
  class ObjCPropertyImplDecl;
  class TargetInfo;
  class VarDecl;

namespace CodeGen {
  class CodeGenModule;
  class CodeGenTypes;
  class CGRecordLayout;  
  
/// CodeGenFunction - This class organizes the per-function state that is used
/// while generating LLVM code.
class CodeGenFunction {
public:
  CodeGenModule &CGM;  // Per-module state.
  TargetInfo &Target;
  
  typedef std::pair<llvm::Value *, llvm::Value *> ComplexPairTy;
  CGBuilderTy Builder;
  
  // Holds the Decl for the current function or method
  const Decl *CurFuncDecl;
  QualType FnRetTy;
  llvm::Function *CurFn;

  /// ReturnBlock - Unified return block.
  llvm::BasicBlock *ReturnBlock;
  /// ReturnValue - The temporary alloca to hold the return value. This
  /// is null iff the function has no return value.
  llvm::Instruction *ReturnValue;
  
  /// AllocaInsertPoint - This is an instruction in the entry block before which
  /// we prefer to insert allocas.
  llvm::Instruction *AllocaInsertPt;

  const llvm::Type *LLVMIntTy;
  uint32_t LLVMPointerWidth;

public:
  // FIXME: The following should be private once EH code is moved out
  // of NeXT runtime.

  // ObjCEHStack - This keeps track of which object to rethrow from
  // inside @catch blocks and which @finally block exits from an EH
  // scope should be chained through.
  struct ObjCEHEntry {
    ObjCEHEntry(llvm::BasicBlock *fb, llvm::BasicBlock *fne, 
                llvm::SwitchInst *fs, llvm::Value *dc)
      : FinallyBlock(fb), FinallyNoExit(fne), FinallySwitch(fs), 
        DestCode(dc), Exception(0) {}

    /// Entry point to the finally block.
    llvm::BasicBlock *FinallyBlock; 

    /// Entry point to the finally block which skips execution of the
    /// try_exit runtime function.
    llvm::BasicBlock *FinallyNoExit; 

    /// Switch instruction which runs at the end of the finally block
    /// to forward jumps through the finally block.
    llvm::SwitchInst *FinallySwitch; 

    /// Variable holding the code for the destination of a jump
    /// through the @finally block.
    llvm::Value *DestCode;

    /// The exception object being handled, during IR generation for a
    /// @catch block.
    llvm::Value *Exception; 
  };

  typedef llvm::SmallVector<ObjCEHEntry*, 8> ObjCEHStackType;
  ObjCEHStackType ObjCEHStack;

  /// EmitJumpThroughFinally - Emit a branch from the current insert
  /// point through the finally handling code for \arg Entry and then
  /// on to \arg Dest. It is legal to call this function even if there
  /// is no current insertion point.
  ///
  /// \param ExecuteTryExit - When true, the try_exit runtime function
  /// should be called prior to executing the finally code.
  void EmitJumpThroughFinally(ObjCEHEntry *Entry, llvm::BasicBlock *Dest,
                              bool ExecuteTryExit=true);
  
private:
  /// LabelIDs - Track arbitrary ids assigned to labels for use in
  /// implementing the GCC address-of-label extension and indirect
  /// goto. IDs are assigned to labels inside getIDForAddrOfLabel().
  std::map<const LabelStmt*, unsigned> LabelIDs;

  /// IndirectSwitches - Record the list of switches for indirect
  /// gotos. Emission of the actual switching code needs to be delayed
  /// until all AddrLabelExprs have been seen.
  std::vector<llvm::SwitchInst*> IndirectSwitches;

  /// LocalDeclMap - This keeps track of the LLVM allocas or globals for local C
  /// decls.
  llvm::DenseMap<const Decl*, llvm::Value*> LocalDeclMap;

  /// LabelMap - This keeps track of the LLVM basic block for each C label.
  llvm::DenseMap<const LabelStmt*, llvm::BasicBlock*> LabelMap;
  
  // BreakContinueStack - This keeps track of where break and continue 
  // statements should jump to.
  struct BreakContinue {
    BreakContinue(llvm::BasicBlock *bb, llvm::BasicBlock *cb)
      : BreakBlock(bb), ContinueBlock(cb) {}
      
    llvm::BasicBlock *BreakBlock;
    llvm::BasicBlock *ContinueBlock;
  }; 
  llvm::SmallVector<BreakContinue, 8> BreakContinueStack;

  /// SwitchInsn - This is nearest current switch instruction. It is null if
  /// if current context is not in a switch.
  llvm::SwitchInst *SwitchInsn;

  /// CaseRangeBlock - This block holds if condition check for last case 
  /// statement range in current switch instruction.
  llvm::BasicBlock *CaseRangeBlock;

public:
  CodeGenFunction(CodeGenModule &cgm);
  
  ASTContext &getContext() const;

  void GenerateObjCMethod(const ObjCMethodDecl *OMD);

  void StartObjCMethod(const ObjCMethodDecl *MD);

  /// GenerateObjCGetter - Synthesize an Objective-C property getter
  /// function.
  void GenerateObjCGetter(const ObjCPropertyImplDecl *PID);

  /// GenerateObjCSetter - Synthesize an Objective-C property setter
  /// function for the given property.
  void GenerateObjCSetter(const ObjCPropertyImplDecl *PID);

  void GenerateCode(const FunctionDecl *FD,
                    llvm::Function *Fn);
  void StartFunction(const Decl *D, QualType RetTy, 
                     llvm::Function *Fn,
                     const FunctionArgList &Args,
                     SourceLocation StartLoc);

  /// FinishFunction - Complete IR generation of the current
  /// function. It is legal to call this function even if there is no
  /// current insertion point.
  void FinishFunction(SourceLocation EndLoc=SourceLocation());

  /// EmitFunctionProlog - Emit the target specific LLVM code to load
  /// the arguments for the given function. This is also responsible
  /// for naming the LLVM function arguments.
  void EmitFunctionProlog(llvm::Function *Fn, QualType RetTy, 
                          const FunctionArgList &Args);

  /// EmitFunctionEpilog - Emit the target specific LLVM code to
  /// return the given temporary.
  void EmitFunctionEpilog(QualType RetTy, 
                          llvm::Value *ReturnValue);

  const llvm::Type *ConvertType(QualType T);

  /// LoadObjCSelf - Load the value of self. This function is only
  /// valid while generating code for an Objective-C method.
  llvm::Value *LoadObjCSelf();

  /// isObjCPointerType - Return true if the specificed AST type will map onto
  /// some Objective-C pointer type.
  static bool isObjCPointerType(QualType T);

  /// hasAggregateLLVMType - Return true if the specified AST type will map into
  /// an aggregate LLVM type or is void.
  static bool hasAggregateLLVMType(QualType T);

  /// createBasicBlock - Create an LLVM basic block.
  llvm::BasicBlock *createBasicBlock(const char *Name="", 
                                     llvm::Function *Parent=0,
                                     llvm::BasicBlock *InsertBefore=0) {
#ifdef NDEBUG
    return llvm::BasicBlock::Create("", Parent, InsertBefore);
#else
    return llvm::BasicBlock::Create(Name, Parent, InsertBefore);
#endif
  }
                                    
  /// getBasicBlockForLabel - Return the LLVM basicblock that the specified
  /// label maps to.
  llvm::BasicBlock *getBasicBlockForLabel(const LabelStmt *S);
  
  /// EmitBlock - Emit the given block \arg BB and set it as the
  /// insert point, adding a fall-through branch from the current
  /// insert block if necessary. It is legal to call this function
  /// even if there is no current insertion point.
  ///
  /// IsFinished - If true, indicates that the caller has finished
  /// emitting branches to the given block and does not expect to emit
  /// code into it. This means the block can be ignored if it is
  /// unreachable.
  void EmitBlock(llvm::BasicBlock *BB, bool IsFinished=false);

  /// EmitBranch - Emit a branch to the specified basic block from the
  /// current insert block, taking care to avoid creation of branches
  /// from dummy blocks. It is legal to call this function even if
  /// there is no current insertion point.
  ///
  /// This function clears the current insertion point. The caller
  /// should follow calls to this function with calls to Emit*Block
  /// prior to generation new code.
  void EmitBranch(llvm::BasicBlock *Block);

  /// HaveInsertPoint - True if an insertion point is defined. If not,
  /// this indicates that the current code being emitted is
  /// unreachable.
  bool HaveInsertPoint() const { 
    return Builder.GetInsertBlock() != 0;
  }

  /// EnsureInsertPoint - Ensure that an insertion point is defined so
  /// that emitted IR has a place to go. Note that by definition, if
  /// this function creates a block then that block is unreachable;
  /// callers may do better to detect when no insertion point is
  /// defined and simply skip IR generation.
  void EnsureInsertPoint() {
    if (!HaveInsertPoint())
      EmitBlock(createBasicBlock());
  }
  
  /// ErrorUnsupported - Print out an error that codegen doesn't support the
  /// specified stmt yet.
  void ErrorUnsupported(const Stmt *S, const char *Type,
                        bool OmitOnError=false);

  //===--------------------------------------------------------------------===//
  //                                  Helpers
  //===--------------------------------------------------------------------===//
  
  /// CreateTempAlloca - This creates a alloca and inserts it into the entry
  /// block.
  llvm::AllocaInst *CreateTempAlloca(const llvm::Type *Ty,
                                     const char *Name = "tmp");
  
  /// EvaluateExprAsBool - Perform the usual unary conversions on the specified
  /// expression and compare the result against zero, returning an Int1Ty value.
  llvm::Value *EvaluateExprAsBool(const Expr *E);

  /// EmitAnyExpr - Emit code to compute the specified expression which can have
  /// any type.  The result is returned as an RValue struct.  If this is an
  /// aggregate expression, the aggloc/agglocvolatile arguments indicate where
  /// the result should be returned.
  RValue EmitAnyExpr(const Expr *E, llvm::Value *AggLoc = 0, 
                     bool isAggLocVolatile = false);

  /// EmitAnyExprToTemp - Similary to EmitAnyExpr(), however, the result
  /// will always be accessible even if no aggregate location is
  /// provided.
  RValue EmitAnyExprToTemp(const Expr *E, llvm::Value *AggLoc = 0, 
                           bool isAggLocVolatile = false);

  void EmitAggregateCopy(llvm::Value *DestPtr, llvm::Value *SrcPtr,
                         QualType EltTy);

  void EmitAggregateClear(llvm::Value *DestPtr, QualType Ty);

  /// StartBlock - Start new block named N. If insert block is a dummy block
  /// then reuse it.
  void StartBlock(const char *N);

  /// getCGRecordLayout - Return record layout info.
  const CGRecordLayout *getCGRecordLayout(CodeGenTypes &CGT, QualType RTy);

  /// GetAddrOfStaticLocalVar - Return the address of a static local variable.
  llvm::Constant *GetAddrOfStaticLocalVar(const VarDecl *BVD);

  /// GetAddrOfLocalVar - Return the address of a local variable.
  llvm::Value *GetAddrOfLocalVar(const VarDecl *VD);
  
  /// getAccessedFieldNo - Given an encoded value and a result number, return
  /// the input field number being accessed.
  static unsigned getAccessedFieldNo(unsigned Idx, const llvm::Constant *Elts);

  unsigned GetIDForAddrOfLabel(const LabelStmt *L);

  /// EmitMemSetToZero - Generate code to memset a value of the given type to 0;
  void EmitMemSetToZero(llvm::Value *DestPtr, QualType Ty);

  // EmitVAArg - Generate code to get an argument from the passed in pointer
  // and update it accordingly. The return value is a pointer to the argument.
  // FIXME: We should be able to get rid of this method and use the va_arg
  // instruction in LLVM instead once it works well enough.  
  llvm::Value *EmitVAArg(llvm::Value *VAListAddr, QualType Ty);
  
  //===--------------------------------------------------------------------===//
  //                            Declaration Emission
  //===--------------------------------------------------------------------===//
  
  void EmitDecl(const Decl &D);
  void EmitBlockVarDecl(const VarDecl &D);
  void EmitLocalBlockVarDecl(const VarDecl &D);
  void EmitStaticBlockVarDecl(const VarDecl &D);

  /// EmitParmDecl - Emit a ParmVarDecl or an ImplicitParamDecl.
  void EmitParmDecl(const VarDecl &D, llvm::Value *Arg);
  
  //===--------------------------------------------------------------------===//
  //                             Statement Emission
  //===--------------------------------------------------------------------===//

  /// EmitStopPoint - Emit a debug stoppoint if we are emitting debug
  /// info.
  void EmitStopPoint(const Stmt *S);

  /// EmitStmt - Emit the code for the statement \arg S. It is legal
  /// to call this function even if there is no current insertion
  /// point.
  /// 
  /// This function may clear the current insertion point; callers
  /// should use EnsureInsertPoint if they wish to subsequently
  /// generate code without first calling EmitBlock, EmitBranch, or
  /// EmitStmt.
  void EmitStmt(const Stmt *S);

  /// EmitSimpleStmt - Try to emit a "simple" statement which does not
  /// necessarily require an insertion point or debug information;
  /// typically because the statement amounts to a jump or a container
  /// of other statements.
  ///
  /// \return True if the statement was handled.
  bool EmitSimpleStmt(const Stmt *S);

  RValue EmitCompoundStmt(const CompoundStmt &S, bool GetLast = false,
                          llvm::Value *AggLoc = 0, bool isAggVol = false);

  /// EmitLabel - Emit the block for the given label. It is legal
  /// to call this function even if there is no current insertion
  /// point.
  void EmitLabel(const LabelStmt &S); // helper for EmitLabelStmt.

  void EmitLabelStmt(const LabelStmt &S);
  void EmitGotoStmt(const GotoStmt &S);
  void EmitIndirectGotoStmt(const IndirectGotoStmt &S);
  void EmitIfStmt(const IfStmt &S);
  void EmitWhileStmt(const WhileStmt &S);
  void EmitDoStmt(const DoStmt &S);
  void EmitForStmt(const ForStmt &S);
  void EmitReturnStmt(const ReturnStmt &S);
  void EmitDeclStmt(const DeclStmt &S);
  void EmitBreakStmt(const BreakStmt &S);
  void EmitContinueStmt(const ContinueStmt &S);
  void EmitSwitchStmt(const SwitchStmt &S);
  void EmitDefaultStmt(const DefaultStmt &S);
  void EmitCaseStmt(const CaseStmt &S);
  void EmitCaseStmtRange(const CaseStmt &S);
  void EmitAsmStmt(const AsmStmt &S);
  
  void EmitObjCForCollectionStmt(const ObjCForCollectionStmt &S);
  void EmitObjCAtTryStmt(const ObjCAtTryStmt &S);
  void EmitObjCAtThrowStmt(const ObjCAtThrowStmt &S);
  void EmitObjCAtSynchronizedStmt(const ObjCAtSynchronizedStmt &S);
  
  //===--------------------------------------------------------------------===//
  //                         LValue Expression Emission
  //===--------------------------------------------------------------------===//

  /// EmitUnsupportedLValue - Emit a dummy l-value using the type of E
  /// and issue an ErrorUnsupported style diagnostic (using the
  /// provided Name).
  LValue EmitUnsupportedLValue(const Expr *E,
                               const char *Name);

  /// EmitLValue - Emit code to compute a designator that specifies the location
  /// of the expression.
  ///
  /// This can return one of two things: a simple address or a bitfield
  /// reference.  In either case, the LLVM Value* in the LValue structure is
  /// guaranteed to be an LLVM pointer type.
  ///
  /// If this returns a bitfield reference, nothing about the pointee type of
  /// the LLVM value is known: For example, it may not be a pointer to an
  /// integer.
  ///
  /// If this returns a normal address, and if the lvalue's C type is fixed
  /// size, this method guarantees that the returned pointer type will point to
  /// an LLVM type of the same size of the lvalue's type.  If the lvalue has a
  /// variable length type, this is not possible.
  ///
  LValue EmitLValue(const Expr *E);
  
  /// EmitLoadOfLValue - Given an expression that represents a value lvalue,
  /// this method emits the address of the lvalue, then loads the result as an
  /// rvalue, returning the rvalue.
  RValue EmitLoadOfLValue(LValue V, QualType LVType);
  RValue EmitLoadOfExtVectorElementLValue(LValue V, QualType LVType);
  RValue EmitLoadOfBitfieldLValue(LValue LV, QualType ExprType);
  RValue EmitLoadOfPropertyRefLValue(LValue LV, QualType ExprType);
  RValue EmitLoadOfKVCRefLValue(LValue LV, QualType ExprType);

  
  /// EmitStoreThroughLValue - Store the specified rvalue into the specified
  /// lvalue, where both are guaranteed to the have the same type, and that type
  /// is 'Ty'.
  void EmitStoreThroughLValue(RValue Src, LValue Dst, QualType Ty);
  void EmitStoreThroughExtVectorComponentLValue(RValue Src, LValue Dst,
                                                QualType Ty);
  void EmitStoreThroughPropertyRefLValue(RValue Src, LValue Dst, QualType Ty);
  void EmitStoreThroughKVCRefLValue(RValue Src, LValue Dst, QualType Ty);

  /// EmitStoreThroughLValue - Store Src into Dst with same
  /// constraints as EmitStoreThroughLValue. 
  ///
  /// \param Result [out] - If non-null, this will be set to a Value*
  /// for the bit-field contents after the store, appropriate for use
  /// as the result of an assignment to the bit-field.
  void EmitStoreThroughBitfieldLValue(RValue Src, LValue Dst, QualType Ty,
                                      llvm::Value **Result=0);
   
  // Note: only availabe for agg return types
  LValue EmitBinaryOperatorLValue(const BinaryOperator *E);
  // Note: only availabe for agg return types
  LValue EmitCallExprLValue(const CallExpr *E);
  LValue EmitDeclRefLValue(const DeclRefExpr *E);
  LValue EmitStringLiteralLValue(const StringLiteral *E);
  LValue EmitPredefinedFunctionName(unsigned Type);
  LValue EmitPredefinedLValue(const PredefinedExpr *E);
  LValue EmitUnaryOpLValue(const UnaryOperator *E);
  LValue EmitArraySubscriptExpr(const ArraySubscriptExpr *E);
  LValue EmitExtVectorElementExpr(const ExtVectorElementExpr *E);
  LValue EmitMemberExpr(const MemberExpr *E);
  LValue EmitCompoundLiteralLValue(const CompoundLiteralExpr *E);

  llvm::Value *EmitIvarOffset(ObjCInterfaceDecl *Interface,
                              const ObjCIvarDecl *Ivar);
  LValue EmitLValueForField(llvm::Value* Base, FieldDecl* Field,
                            bool isUnion, unsigned CVRQualifiers);
  LValue EmitLValueForIvar(llvm::Value* Base, const ObjCIvarDecl *Ivar,
                           unsigned CVRQualifiers);

  LValue EmitCXXConditionDeclLValue(const CXXConditionDeclExpr *E);

  LValue EmitObjCMessageExprLValue(const ObjCMessageExpr *E);
  LValue EmitObjCIvarRefLValue(const ObjCIvarRefExpr *E);
  LValue EmitObjCPropertyRefLValue(const ObjCPropertyRefExpr *E);
  LValue EmitObjCKVCRefLValue(const ObjCKVCRefExpr *E);
  LValue EmitObjCSuperExpr(const ObjCSuperExpr *E);

  //===--------------------------------------------------------------------===//
  //                         Scalar Expression Emission
  //===--------------------------------------------------------------------===//

  /// EmitCall - Generate a call of the given function, expecting the
  /// given result type, and using the given argument list which
  /// specifies both the LLVM arguments and the types they were
  /// derived from.
  RValue EmitCall(llvm::Value *Callee,
                  QualType ResultType,
                  const CallArgList &Args);

  RValue EmitCallExpr(const CallExpr *E);

  RValue EmitCallExpr(Expr *FnExpr, CallExpr::const_arg_iterator ArgBeg,
                      CallExpr::const_arg_iterator ArgEnd);

  RValue EmitCallExpr(llvm::Value *Callee, QualType FnType,
                      CallExpr::const_arg_iterator ArgBeg,
                      CallExpr::const_arg_iterator ArgEnd);
  
  RValue EmitBuiltinExpr(unsigned BuiltinID, const CallExpr *E);

  /// EmitTargetBuiltinExpr - Emit the given builtin call. Returns 0
  /// if the call is unhandled by the current target.
  llvm::Value *EmitTargetBuiltinExpr(unsigned BuiltinID, const CallExpr *E);

  llvm::Value *EmitX86BuiltinExpr(unsigned BuiltinID, const CallExpr *E);
  llvm::Value *EmitPPCBuiltinExpr(unsigned BuiltinID, const CallExpr *E);
  
  llvm::Value *EmitShuffleVector(llvm::Value* V1, llvm::Value *V2, ...);
  llvm::Value *EmitVector(llvm::Value * const *Vals, unsigned NumVals,
                          bool isSplat = false);
  
  llvm::Value *EmitObjCProtocolExpr(const ObjCProtocolExpr *E);
  llvm::Value *EmitObjCStringLiteral(const ObjCStringLiteral *E);
  llvm::Value *EmitObjCSelectorExpr(const ObjCSelectorExpr *E);
  RValue EmitObjCMessageExpr(const ObjCMessageExpr *E);
  RValue EmitObjCPropertyGet(const Expr *E);
  void EmitObjCPropertySet(const Expr *E, RValue Src);


  //===--------------------------------------------------------------------===//
  //                           Expression Emission
  //===--------------------------------------------------------------------===//

  // Expressions are broken into three classes: scalar, complex, aggregate.
  
  /// EmitScalarExpr - Emit the computation of the specified expression of
  /// LLVM scalar type, returning the result.
  llvm::Value *EmitScalarExpr(const Expr *E);
  
  /// EmitScalarConversion - Emit a conversion from the specified type to the
  /// specified destination type, both of which are LLVM scalar types.
  llvm::Value *EmitScalarConversion(llvm::Value *Src, QualType SrcTy,
                                    QualType DstTy);
  
  /// EmitComplexToScalarConversion - Emit a conversion from the specified
  /// complex type to the specified destination type, where the destination
  /// type is an LLVM scalar type.
  llvm::Value *EmitComplexToScalarConversion(ComplexPairTy Src, QualType SrcTy,
                                             QualType DstTy);
  
  
  /// EmitAggExpr - Emit the computation of the specified expression of
  /// aggregate type.  The result is computed into DestPtr.  Note that if
  /// DestPtr is null, the value of the aggregate expression is not needed.
  void EmitAggExpr(const Expr *E, llvm::Value *DestPtr, bool VolatileDest);
  
  /// EmitComplexExpr - Emit the computation of the specified expression of
  /// complex type, returning the result.
  ComplexPairTy EmitComplexExpr(const Expr *E);
  
  /// EmitComplexExprIntoAddr - Emit the computation of the specified expression
  /// of complex type, storing into the specified Value*.
  void EmitComplexExprIntoAddr(const Expr *E, llvm::Value *DestAddr,
                               bool DestIsVolatile);

  /// StoreComplexToAddr - Store a complex number into the specified address.
  void StoreComplexToAddr(ComplexPairTy V, llvm::Value *DestAddr,
                          bool DestIsVolatile);
  /// LoadComplexFromAddr - Load a complex number from the specified address.
  ComplexPairTy LoadComplexFromAddr(llvm::Value *SrcAddr, bool SrcIsVolatile);

  /// GenerateStaticBlockVarDecl - return the the static
  /// declaration of local variable. 
  llvm::GlobalValue *GenerateStaticBlockVarDecl(const VarDecl &D,
                                                bool NoInit,
                                                const char *Separator);

  // GenerateStaticBlockVarDecl - return the static declaration of
  // a local variable. Performs initialization of the variable if necessary.
  llvm::GlobalValue *GenerateStaticCXXBlockVarDecl(const VarDecl &D);

  //===--------------------------------------------------------------------===//
  //                             Internal Helpers
  //===--------------------------------------------------------------------===//
 
  /// ContainsLabel - Return true if the statement contains a label in it.  If
  /// this statement is not executed normally, it not containing a label means
  /// that we can just remove the code.
  static bool ContainsLabel(const Stmt *S, bool IgnoreCaseStmts = false);
  
  /// ConstantFoldsToSimpleInteger - If the specified expression does not fold
  /// to a constant, or if it does but contains a label, return 0.  If it
  /// constant folds to 'true' and does not contain a label, return 1, if it
  /// constant folds to 'false' and does not contain a label, return -1.
  int ConstantFoldsToSimpleInteger(const Expr *Cond);
    
  /// EmitBranchOnBoolExpr - Emit a branch on a boolean condition (e.g. for an
  /// if statement) to the specified blocks.  Based on the condition, this might
  /// try to simplify the codegen of the conditional based on the branch.
  ///
  void EmitBranchOnBoolExpr(const Expr *Cond, llvm::BasicBlock *TrueBlock,
                            llvm::BasicBlock *FalseBlock);
private:
  
  /// EmitIndirectSwitches - Emit code for all of the switch
  /// instructions in IndirectSwitches.
  void EmitIndirectSwitches();

  void EmitReturnOfRValue(RValue RV, QualType Ty);

  /// ExpandTypeFromArgs - Reconstruct a structure of type \arg Ty
  /// from function arguments into \arg Dst. See ABIArgInfo::Expand.
  ///
  /// \param AI - The first function argument of the expansion.
  /// \return The argument following the last expanded function
  /// argument.
  llvm::Function::arg_iterator 
  ExpandTypeFromArgs(QualType Ty, LValue Dst,
                     llvm::Function::arg_iterator AI);

  /// ExpandTypeToArgs - Expand an RValue \arg Src, with the LLVM type
  /// for \arg Ty, into individual arguments on the provided vector
  /// \arg Args. See ABIArgInfo::Expand.
  void ExpandTypeToArgs(QualType Ty, RValue Src, 
                        llvm::SmallVector<llvm::Value*, 16> &Args);
};
}  // end namespace CodeGen
}  // end namespace clang

#endif
