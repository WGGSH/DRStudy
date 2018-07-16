#pragma once

// Error Check Macro
#define ERROR_CHECK( ret )                                        \
    if( FAILED( ret ) ){                                          \
        std::stringstream ss;                                     \
        ss << "failed " #ret " " << std::hex << ret << std::endl; \
        throw std::runtime_error( ss.str().c_str() );             \
    }
