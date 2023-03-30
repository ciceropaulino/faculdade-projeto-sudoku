#ifndef MESSAGES_H
#define MESSAGES_H

#include <iostream>
#include <iomanip> // setw()
#include <string>
#include <sstream>

#include "text_color.h"

#define SC coms::SourceContext(__FILE__, __LINE__)

#define ERROR(msg) coms::Error( msg, SC )
#define WARNING(msg) coms::Warning( msg, SC )
#define MESSAGE(msg) coms::Message( msg )

namespace  coms{ // Message Handler
    // Holds context information for a warning or error while pre-processing scene file or executing the renderer.
    // Use macro SC to create one
    struct SourceContext {
        const char* file;
        int line;
        SourceContext(const char* file="n/a", int line=0) : file(file), line(line) { }
        SourceContext& operator=( const SourceContext& ) = default;
        SourceContext( const SourceContext& clone) = default;
        ~SourceContext() = default;
    };

    /// Prints out the error message and exits the program.
    void Error( const std::string&,  const SourceContext& );
    void Error( const std::string& );
    /// Prints out the warning, but the program keeps running.
    void Warning( const std::string&,  const SourceContext& );
    void Warning( const std::string& );
    /// Prints out a simple message, program keeps running.
    void Message( const std::string& );
} // namespace coms

#endif
//-------------------------------------------------------------------------------
