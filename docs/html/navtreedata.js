/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Merkle Tree Library", "index.html", [
    [ "Merkle Tree Library Documentation", "index.html", "index" ],
    [ "CLAUDE.md", "md_CLAUDE.html", [
      [ "Build and Test Commands", "md_CLAUDE.html#autotoc_md1", [
        [ "Quick Start", "md_CLAUDE.html#autotoc_md2", null ],
        [ "Building Tests", "md_CLAUDE.html#autotoc_md3", null ],
        [ "Running Tests", "md_CLAUDE.html#autotoc_md4", null ],
        [ "Building Main Executable", "md_CLAUDE.html#autotoc_md5", null ]
      ] ],
      [ "Development Guidelines", "md_CLAUDE.html#autotoc_md6", [
        [ "Memory Management", "md_CLAUDE.html#autotoc_md7", null ],
        [ "Code Style", "md_CLAUDE.html#autotoc_md8", null ],
        [ "Common Pitfalls", "md_CLAUDE.html#autotoc_md9", null ]
      ] ],
      [ "Architecture Overview", "md_CLAUDE.html#autotoc_md10", [
        [ "Core Structure (src/)", "md_CLAUDE.html#autotoc_md11", null ],
        [ "Public API (include/)", "md_CLAUDE.html#autotoc_md12", null ],
        [ "Key Data Structures", "md_CLAUDE.html#autotoc_md13", null ],
        [ "Tree Construction Algorithm", "md_CLAUDE.html#autotoc_md14", null ],
        [ "Key Features", "md_CLAUDE.html#autotoc_md15", null ],
        [ "Dependencies", "md_CLAUDE.html#autotoc_md16", null ],
        [ "Important Implementation Details", "md_CLAUDE.html#autotoc_md17", null ]
      ] ],
      [ "Known Issues &amp; Limitations", "md_CLAUDE.html#autotoc_md18", null ],
      [ "File Locations Reference", "md_CLAUDE.html#autotoc_md19", null ]
    ] ],
    [ "API Overview", "api_overview.html", [
      [ "📋 Table of Contents", "api_overview.html#autotoc_md20", null ],
      [ "🔧 Core Functions", "api_overview.html#core-functions", [
        [ "Tree Creation and Destruction", "api_overview.html#autotoc_md21", [
          [ "create_merkle_tree()", "api_overview.html#autotoc_md22", null ],
          [ "dealloc_merkle_tree()", "api_overview.html#autotoc_md23", null ]
        ] ],
        [ "Hash Operations", "api_overview.html#autotoc_md24", [
          [ "get_tree_hash()", "api_overview.html#autotoc_md25", null ]
        ] ],
        [ "Proof Generation", "api_overview.html#autotoc_md26", [
          [ "generate_proof_from_index()", "api_overview.html#autotoc_md27", null ],
          [ "generate_proof_by_finder()", "api_overview.html#autotoc_md28", null ],
          [ "verify_proof()", "api_overview.html#autotoc_md29", null ]
        ] ]
      ] ],
      [ "📊 Data Structures", "api_overview.html#data-structures", [
        [ "merkle_tree_t", "api_overview.html#autotoc_md30", null ],
        [ "merkle_proof_t", "api_overview.html#autotoc_md31", null ],
        [ "merkle_proof_item_t", "api_overview.html#autotoc_md32", null ],
        [ "value_finder", "api_overview.html#autotoc_md33", null ]
      ] ],
      [ "⚠️ Error Codes", "api_overview.html#error-codes", [
        [ "merkle_error_t", "api_overview.html#autotoc_md34", null ]
      ] ],
      [ "🧠 Memory Management", "api_overview.html#memory-management", [
        [ "Allocation Patterns", "api_overview.html#autotoc_md35", null ],
        [ "Memory Safety Features", "api_overview.html#autotoc_md36", null ],
        [ "Custom Memory Management", "api_overview.html#autotoc_md37", null ]
      ] ],
      [ "🔒 Thread Safety", "api_overview.html#thread-safety", [
        [ "Thread-Safe Operations", "api_overview.html#autotoc_md38", null ],
        [ "Non-Thread-Safe Operations", "api_overview.html#autotoc_md39", null ],
        [ "Locking Implementation", "api_overview.html#autotoc_md40", null ]
      ] ],
      [ "⚡ Performance Considerations", "api_overview.html#performance-considerations", [
        [ "Algorithm Complexity", "api_overview.html#autotoc_md41", null ],
        [ "Memory Usage", "api_overview.html#autotoc_md42", null ],
        [ "Optimization Tips", "api_overview.html#autotoc_md43", null ],
        [ "Performance Testing", "api_overview.html#autotoc_md44", null ]
      ] ],
      [ "🔗 Constants", "api_overview.html#autotoc_md45", [
        [ "Hash Size", "api_overview.html#autotoc_md46", null ],
        [ "Build Configuration", "api_overview.html#autotoc_md47", null ]
      ] ]
    ] ],
    [ "Getting Started", "getting_started.html", [
      [ "📦 Installation", "getting_started.html#autotoc_md49", [
        [ "Prerequisites", "getting_started.html#autotoc_md50", null ],
        [ "Platform-Specific Setup", "getting_started.html#autotoc_md51", [
          [ "macOS", "getting_started.html#autotoc_md52", null ],
          [ "Ubuntu/Debian", "getting_started.html#autotoc_md53", null ],
          [ "Windows (MSYS2/MinGW)", "getting_started.html#autotoc_md54", null ]
        ] ],
        [ "Building the Library", "getting_started.html#autotoc_md55", null ],
        [ "Verify Installation", "getting_started.html#autotoc_md56", null ]
      ] ],
      [ "🔧 Basic Usage", "getting_started.html#autotoc_md57", [
        [ "1. Include Headers", "getting_started.html#autotoc_md58", null ],
        [ "2. Create Your First Tree", "getting_started.html#autotoc_md59", null ],
        [ "3. Get the Root Hash", "getting_started.html#autotoc_md60", null ],
        [ "4. Generate and Verify Proofs", "getting_started.html#autotoc_md61", null ]
      ] ],
      [ "🔄 Common Patterns", "getting_started.html#autotoc_md62", [
        [ "Error Handling", "getting_started.html#autotoc_md63", null ],
        [ "Memory Management", "getting_started.html#autotoc_md64", null ],
        [ "Thread Safety", "getting_started.html#autotoc_md65", null ]
      ] ],
      [ "🎯 Next Steps", "getting_started.html#autotoc_md66", null ],
      [ "🐛 Troubleshooting", "getting_started.html#autotoc_md67", [
        [ "Common Issues", "getting_started.html#autotoc_md68", null ],
        [ "Getting Help", "getting_started.html#autotoc_md69", null ]
      ] ]
    ] ],
    [ "Merkle Tree Implementation", "md_README.html", [
      [ "✨ Features", "md_README.html#autotoc_md83", null ],
      [ "📁 Project Structure", "md_README.html#autotoc_md84", null ],
      [ "🚀 Quick Start", "md_README.html#autotoc_md85", [
        [ "Prerequisites", "md_README.html#autotoc_md86", [
          [ "macOS Installation:", "md_README.html#autotoc_md87", null ],
          [ "Ubuntu Installation:", "md_README.html#autotoc_md88", null ]
        ] ],
        [ "Building and Testing", "md_README.html#autotoc_md89", null ],
        [ "VS Code Development", "md_README.html#autotoc_md90", null ]
      ] ],
      [ "📖 Usage", "md_README.html#autotoc_md91", [
        [ "Basic Example", "md_README.html#autotoc_md92", null ],
        [ "Advanced Configuration", "md_README.html#autotoc_md93", null ],
        [ "Error Handling", "md_README.html#autotoc_md94", null ]
      ] ],
      [ "🐍 Python Integration", "md_README.html#autotoc_md95", null ],
      [ "🧪 Testing", "md_README.html#autotoc_md96", [
        [ "Running Tests", "md_README.html#autotoc_md97", null ]
      ] ],
      [ "📚 Documentation", "md_README.html#autotoc_md98", [
        [ "Key API Functions", "md_README.html#autotoc_md99", null ],
        [ "Error Codes", "md_README.html#autotoc_md100", null ]
      ] ],
      [ "🏗️ Algorithm Overview", "md_README.html#autotoc_md101", null ],
      [ "🔧 Configuration", "md_README.html#autotoc_md102", [
        [ "Compile-Time Options", "md_README.html#autotoc_md103", null ],
        [ "Memory Management", "md_README.html#autotoc_md104", null ]
      ] ],
      [ "🤝 Contributing", "md_README.html#autotoc_md105", null ],
      [ "🐛 Troubleshooting", "md_README.html#autotoc_md106", [
        [ "Build Issues", "md_README.html#autotoc_md107", null ],
        [ "Runtime Issues", "md_README.html#autotoc_md108", null ]
      ] ],
      [ "📄 License", "md_README.html#autotoc_md109", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ]
      ] ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"MerkleQueue_8h.html",
"namespacepython__ctypes__example.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';