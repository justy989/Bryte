#ifndef INPUT_RECORDER_HPP
#define INPUT_RECORDER_HPP

#include "GameInput.hpp"

#include <fstream>

class InputRecorder {
public:

     InputRecorder ( );

     Bool start_recording    ( const Char8* path );
     Bool stop_recording     ( );
     Bool start_playing_back ( const Char8* path );
     Bool stop_playing_back  ( );

     Void write_frame ( const GameInput& game_input );
     Bool read_frame ( GameInput& game_input );

     inline Bool is_recording ( ) const;
     inline Bool is_playing_back ( ) const;

private:

     std::fstream m_file;

     Bool m_recording;
     Bool m_playing_back;

};

Bool InputRecorder::is_recording ( ) const
{
     return m_recording;
}

Bool InputRecorder::is_playing_back ( ) const
{
     return m_playing_back;
}

#endif

