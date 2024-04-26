#include <clang/Basic/FileManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/BackendUtil.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/TargetParser/Host.h>

#include "cc1args.h"
#include "depfile.hh"
#include "wasm-rt-content.h"

using namespace std;
using namespace clang;
using namespace llvm;

pair<bool, string> c_to_elf( const vector<char*>& system_dep_files,
                             const vector<char*>& clang_dep_files,
                             char* function_c_buffer,
                             char* function_h_impl_buffer,
                             char* function_h_buffer )
{
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  // Create compiler instance
  clang::CompilerInstance compilerInstance;

  // Create diagnostic engine
  string diagOutput;
  raw_string_ostream diagOS( diagOutput );
  auto diagPrinter = make_unique<TextDiagnosticPrinter>( diagOS, new DiagnosticOptions() );

  IntrusiveRefCntPtr<DiagnosticsEngine> diagEngine
    = CompilerInstance::createDiagnostics( new DiagnosticOptions(), diagPrinter.get(), false );

  // Create File System
  IntrusiveRefCntPtr<vfs::InMemoryFileSystem> InMemFS( new vfs::InMemoryFileSystem() );

  for ( size_t i = 0; i < system_deps.size(); i++ ) {
    InMemFS->addFile( system_deps.at( i ), 0, MemoryBuffer::getMemBuffer( system_dep_files.at( i ) ) );
  }

  for ( size_t i = 0; i < clang_deps.size(); i++ ) {
    InMemFS->addFile( clang_deps.at( i ), 0, MemoryBuffer::getMemBuffer( clang_dep_files.at( i ) ) );
  }

  InMemFS->addFile( "/fix/function.c", 0, MemoryBuffer::getMemBuffer( function_c_buffer ) );
  InMemFS->addFile( "/fix/function-impl.h", 0, MemoryBuffer::getMemBuffer( function_h_impl_buffer ) );
  InMemFS->addFile( "/fix/function.h", 0, MemoryBuffer::getMemBuffer( function_h_buffer ) );
  InMemFS->addFile( "/fix/wasm-rt.h", 0, MemoryBuffer::getMemBuffer( wasm_rt_content ) );

  InMemFS->setCurrentWorkingDirectory( "/fix" );

  FileManager* FM = new FileManager( FileSystemOptions {}, InMemFS );
  compilerInstance.setFileManager( FM );

  auto compilerInvocation = std::make_shared<CompilerInvocation>();
  if ( !CompilerInvocation::CreateFromArgs( *compilerInvocation, cc1args, *diagEngine ) ) {
    return make_pair( false, diagOS.str() + "\nFailed to create compiler invocation.\n" );
  }

  LLVMContext context;
  std::unique_ptr<CodeGenAction> action( new EmitLLVMOnlyAction( &context ) );
  compilerInstance.createDiagnostics( diagPrinter.get(), false );
  compilerInstance.setInvocation( compilerInvocation );

  auto& codegenOptions = compilerInstance.getCodeGenOpts();
  codegenOptions.CodeModel = "large";
  codegenOptions.RelocationModel = llvm::Reloc::Static;

  if ( !compilerInstance.ExecuteAction( *action ) ) {
    return make_pair( false, diagOS.str() + "\nFailed to emit llvm\n" );
  }

  std::unique_ptr<llvm::Module> module = action->takeModule();
  if ( !module ) {
    return make_pair( false, "\nFailed to take module\n" );
  }

  std::string llvm_res;
  raw_string_ostream Str_OS( llvm_res );
  auto OS = make_unique<buffer_ostream>( Str_OS );

  EmitBackendOutput( compilerInstance.getDiagnostics(),
                     compilerInstance.getHeaderSearchOpts(),
                     compilerInstance.getCodeGenOpts(),
                     compilerInstance.getTargetOpts(),
                     compilerInstance.getLangOpts(),
                     compilerInstance.getTarget().getDataLayoutString(),
                     module.get(),
                     Backend_EmitObj,
                     InMemFS,
                     std::move( OS ) );

  return make_pair( compilerInstance.getDiagnostics().getNumErrors() == 0, llvm_res );
}
