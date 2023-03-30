#include "messages.h"

inline std::ostream& operator<<(std::ostream& os, const coms::SourceContext& sc) {
    os << sc.file << ":" << sc.line;
    return os;
}

namespace coms{ // message handler

    /// Prints out the warning, but the program keeps running.
    void Warning( const std::string& msg,  const SourceContext& sc ){
        std::cerr << Color::tcolor( "Warning: \""+msg+"\"", Color::BRIGHT_BLUE ) << "\n";
    }
    /// Prints out the warning, but the program keeps running, without showing the line of code that generated the warning.
    void Warning( const std::string& msg ){
        std::cerr << Color::tcolor( "Warning: \""+msg+"\"", Color::BRIGHT_BLUE ) << "\n";
    }

    /// Prints out the error message and exits the program.
    void Error( const std::string& msg, const SourceContext& sc ){
        std::cerr << Color::tcolor( "Severe error: \""+msg+"\"", Color::RED ) << "\n";
        std::exit( EXIT_FAILURE );
    }

    /// Prints out the error message and exits the program, without showing the line of code that generated the error.
    void Error( const std::string& msg ){
        std::cerr << Color::tcolor( "Severe error: \""+msg+"\"", Color::RED ) << "\n";
        std::exit( EXIT_FAILURE );
    }

    /// Prints out a regular message in green.
    void Message( const std::string& str ){
        std::cout << Color::tcolor( ">>> "+ str, Color::GREEN ) << std::endl;
    }
}
