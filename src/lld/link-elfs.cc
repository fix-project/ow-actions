#include "lld/Common/Driver.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/VirtualFileSystem.h"

#include <vector>

using namespace llvm;

LLD_HAS_DRIVER( elf )

namespace lld {
namespace elf {
  void useVFS(llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs);
}
}

std::pair<bool, std::string> link_elfs( std::vector<char*> dep_files, std::vector<size_t> dep_file_sizes )
{
  // Create File System
  IntrusiveRefCntPtr<vfs::InMemoryFileSystem> InMemFS( new vfs::InMemoryFileSystem() );
  std::vector<const char*> args = { "ld.lld", "-r", "-o", "-" };
  std::vector<std::string> file_names;
  std::string work_directory = "/fix/";
  for ( size_t i = 0; i < dep_files.size(); i++ ) {
    std::string file_name = work_directory + std::string( "file" ) + std::to_string( (int)i ) + ".o";
    file_names.push_back( file_name );
    InMemFS->addFile( file_name, 0, MemoryBuffer::getMemBuffer( { dep_files[i], dep_file_sizes[i] } ) );
  }
  for ( size_t i = 0; i < dep_files.size(); i++ ) {
    args.push_back( file_names[i].c_str() );
  }
  InMemFS->setCurrentWorkingDirectory( "/fix" );

  std::string llvm_stdout;
  raw_string_ostream llvm_stdoutOS( llvm_stdout );
  std::string llvm_stderr;
  raw_string_ostream llvm_stderrOS( llvm_stderr );

  lld::elf::useVFS( InMemFS );
  bool r = lld::elf::link( args, llvm_stdoutOS, llvm_stderrOS, false, false );

  if ( r ) {
    return make_pair( true, llvm_stdout );
  } else {
    return make_pair( false, llvm_stderr );
  }
}
