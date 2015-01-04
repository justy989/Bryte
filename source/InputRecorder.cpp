#include "InputRecorder.hpp"
#include "Utils.hpp"

InputRecorder::InputRecorder ( ) :
     m_recording    ( false ),
     m_playing_back ( false )
{

}

Bool InputRecorder::start_recording ( const Char8* path )
{
     ASSERT ( !m_recording );
     ASSERT ( !m_playing_back );

     LOG_INFO ( "Recording input to '%s'\n", path );

     m_file.open ( path, std::ios::out | std::ios::binary );

     if ( !m_file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open '%s' to load record input.\n", path );
          return false;
     }

     m_recording = true;

     return true;
}

Bool InputRecorder::stop_recording ( )
{
     ASSERT ( m_recording );
     ASSERT ( !m_playing_back );

     LOG_INFO ( "Done recording input.\n" );

     m_file.close ( );

     m_recording = false;

     return true;
}

Bool InputRecorder::start_playing_back ( const Char8* path )
{
     ASSERT ( !m_recording );
     ASSERT ( !m_playing_back );

     LOG_INFO ( "Playing back input from '%s'\n", path );

     m_file.open ( path, std::ios::in | std::ios::binary );

     if ( !m_file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open '%s' to load record input.\n", path );
          return false;
     }

     m_playing_back = true;

     return true;
}

Bool InputRecorder::stop_playing_back ( )
{
     ASSERT ( !m_recording );
     ASSERT ( m_playing_back );

     LOG_INFO ( "Done playing back input.\n" );

     m_file.close ( );

     m_playing_back = false;

     return true;
}

Void InputRecorder::write_frame ( const GameInput& game_input )
{
     ASSERT ( m_recording );
     ASSERT ( !m_playing_back );

     m_file.write ( reinterpret_cast<const char*>( &game_input ), sizeof ( game_input ) );
}

Bool InputRecorder::read_frame ( GameInput& game_input )
{
     ASSERT ( !m_recording );
     ASSERT ( m_playing_back );

     if ( m_file.eof ( ) ) {
          m_file.clear ( );
          m_file.seekg ( m_file.beg );

          m_file.read ( reinterpret_cast<char*>( &game_input ), sizeof ( game_input ) );

          return false;
     }

     m_file.read ( reinterpret_cast<char*>( &game_input ), sizeof ( game_input ) );

     return true;
}

