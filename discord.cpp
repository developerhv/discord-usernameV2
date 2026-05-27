#include "discord.hpp"

#include <windows.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

#include "memory/memory.hpp"

namespace {
    std::string g_discord_username;
}

std::string extract_username_from_json_file ( const std::string& filepath ) {
    std::ifstream file ( filepath );
    if ( !file.is_open ( ) ) return "";

    std::stringstream buffer;
    buffer << file.rdbuf ( );
    std::string content = buffer.str ( );
    file.close ( );

    std::string search =  ( "\"username\":\"" );
    size_t pos = content.find ( search );
    if ( pos == std::string::npos ) return "";

    pos += search.length ( );
    size_t end_pos = content.find ( "\"", pos );
    if ( end_pos == std::string::npos ) return "";

    return content.substr ( pos, end_pos - pos );
}

std::string get_discord_username ( ) {
    std::string username;

    std::string users_path =  ( "C:\\Users\\" );

    for ( const auto& user_dir : std::filesystem::directory_iterator ( users_path ) ) {
        if ( user_dir.is_directory ( ) ) {
            std::string sentinel_path = user_dir.path ( ).string ( ) +  ( "\\AppData\\Roaming\\discord\\sentry\\scope_v3.json" );

            if ( std::filesystem::exists ( sentinel_path ) ) {
                username = extract_username_from_json_file ( sentinel_path );
                if ( !username.empty ( ) ) break;
            }
        }
    }

    if ( username.empty ( ) ) {
        auto mem = std::make_shared<c_memory> (  ( "Discord.exe" ) );
        if ( mem->valid ( ) ) {
            username = mem->find_username ( );
        }
    }

    if ( username.empty ( ) ) {
      std::cout << "failed to find discord username" << std::endl;
    }

    g_discord_username = username;
    return g_discord_username;
}
