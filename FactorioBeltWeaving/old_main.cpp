#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <optional>
#include <unordered_set>
#include "XY.h"
#include "Timer.h"
#include "trace.h"

using namespace std;

namespace
{
   int divRoundUp( int a, int b ) { return (a + b - 1) / b; }
   int mod( int x, int m ) { int ret = x % m; return ret + (ret < 0 ? m : 0); }
}

//
//struct BeltStatus
//{   
//   int n; // belt size (4, 5, or 6)
//   int wasteAllowed;
//   int spaceLeft; // "time" until a belt is necessary
//   optional<int> firstBeltPos; 
//
//   static BeltStatus initial( int n, int tilingWidth ) { return BeltStatus{ n, mod( -tilingWidth, n ), n-1, nullopt }; }
//   bool isValid() const { return wasteAllowed >= 0 && spaceLeft >= 0; }
//   bool isValidFinish() const { return spaceLeft >= firstBeltPos; }
//
//   BeltStatus placedSpace( int /*pos*/ ) { return BeltStatus{ n, wasteAllowed, spaceLeft - 1, firstBeltPos }; }
//   BeltStatus placedABelt( int pos ) { return firstBeltPos ? BeltStatus{ n, wasteAllowed - spaceLeft, n - 1, firstBeltPos } : BeltStatus{ n, wasteAllowed, n - 1, pos }; }
//};
//
//int g_numSolutions = 0;
//void go( int pos, BeltStatus belt4, BeltStatus belt5, BeltStatus belt6, vector<int>& belt )
//{
//   if ( !belt4.isValid() ) return;
//   if ( !belt5.isValid() ) return;
//   if ( !belt6.isValid() ) return;
//
//   if ( pos == (int)belt.size() )
//   {
//      if ( !belt4.isValidFinish() ) return;
//      if ( !belt5.isValidFinish() ) return;
//      if ( !belt6.isValidFinish() ) return;
//
//      //for ( int b : belt )
//      //   trace << b;
//      //trace << endl;
//      g_numSolutions++;
//      return;
//   }
//
//   belt[pos] = 0; go( pos + 1, belt4.placedSpace( pos ), belt5.placedSpace( pos ), belt6.placedSpace( pos ), belt ); // place a space
//   belt[pos] = 4; go( pos + 1, belt4.placedABelt( pos ), belt5.placedSpace( pos ), belt6.placedSpace( pos ), belt ); // place a 4-belt
//   belt[pos] = 5; go( pos + 1, belt4.placedSpace( pos ), belt5.placedABelt( pos ), belt6.placedSpace( pos ), belt ); // place a 5-belt
//   belt[pos] = 6; go( pos + 1, belt4.placedSpace( pos ), belt5.placedSpace( pos ), belt6.placedABelt( pos ), belt ); // place a 6-belt
//}
//
//void go( int pos, BeltStatus belt5, BeltStatus belt6, vector<int>& belt )
//{
//   if ( !belt5.isValid() ) return;
//   if ( !belt6.isValid() ) return;
//
//   if ( pos == (int)belt.size() )
//   {
//      if ( !belt5.isValidFinish() ) return;
//      if ( !belt6.isValidFinish() ) return;
//
//      //for ( int b : belt )
//      //   trace << b;
//      //trace << endl;
//      g_numSolutions++;
//      return;
//   }
//
//   belt[pos] = 0; go( pos + 1, belt5.placedSpace( pos ), belt6.placedSpace( pos ), belt ); // place a space
//   belt[pos] = 5; go( pos + 1, belt5.placedABelt( pos ), belt6.placedSpace( pos ), belt ); // place a 5-belt
//   belt[pos] = 6; go( pos + 1, belt5.placedSpace( pos ), belt6.placedABelt( pos ), belt ); // place a 6-belt
//}
//
//int main()
//{
//
//   for ( int N = 7; N < 80; N++ )
//   {
//      //int min4 = divRoundUp( N, 4 );
//      int min4 = 0;
//      int min5 = divRoundUp( N, 5 );
//      int min6 = divRoundUp( N, 6 );
//      int totalBelts = min4 + min5 + min6;
//      double density = (double) totalBelts / N;
//
//      vector<int> belt( N );
//      g_numSolutions = 0;
//      //go( 0, BeltStatus::initial( 4, N ), BeltStatus::initial( 5, N ), BeltStatus::initial( 6, N ), belt );
//      go( 0, BeltStatus::initial( 5, N ), BeltStatus::initial( 6, N ), belt );
//
//      if ( g_numSolutions == 0 )
//         continue;
//      trace << "N = " << N << "\tdensity = " << density <<  "\tg_numSolutions = " << g_numSolutions << endl;
//   }
//   return 0;
//}

struct TrackState
{
   int color = -1;
   int time = -1; // time until new footprint is needed
   int firstFootprintPos = -1;

   bool isUsed() const { return color >= 0; }
   bool isValid() const { return !isUsed() || time >= 0; }
};

struct State
{
   State( int numTracks, int footprintsRemaining )
      : tracks( numTracks ), footprintsRemaining( footprintsRemaining ), colorIsUsed( 7 )
   {
   }

   bool isValid( int pos ) const
   {
      if ( footprintsRemaining < 0 )
         return false;

      for ( const TrackState& track : tracks )
         if ( !track.isValid() )
            return false;
            
      if ( pos >= 6 ) // should have footprint by now
         for ( const TrackState& track : tracks )
            if ( !track.isUsed() )
               return false;

      return true;
   }
   bool canFinish( int pos, int weaveLength ) const
   {
      int totalFootprintsRequired = 0;
      for ( const TrackState& track : tracks )
      {
         if ( !track.isUsed() ) 
            return true;
         
         int spacingRemaining = weaveLength - pos + track.firstFootprintPos;
         int footprintsRequired = divRoundUp( spacingRemaining - track.time, 6 );
         totalFootprintsRequired += footprintsRequired;
      }

      if ( pos == weaveLength )
      {
         if ( totalFootprintsRequired > 0 )
            return false;
      }

      return footprintsRemaining >= totalFootprintsRequired;
   }
   int nextUnusedTrack() const
   { 
      for ( int i = 0; i < (int)tracks.size(); i++ )
         if ( tracks[i].color < 0 )
            return i;
      return -1;
   }
   void setTrackColor( int trackIndex, int color )
   {
      if ( color == tracks[trackIndex].color )
         return;
      if ( tracks[trackIndex].color >= 0 )
         colorIsUsed[tracks[trackIndex].color] = false;
      colorIsUsed[color] = true;
      tracks[trackIndex].color = color;
   }
   State placing( int trackIndex, int color, int pos ) const
   {
      State ret = skipping();
      ret.footprintsRemaining--;
      ret.setTrackColor( trackIndex, color );
      ret.tracks[trackIndex].time = color - 1;
      if ( ret.tracks[trackIndex].firstFootprintPos < 0 )
         ret.tracks[trackIndex].firstFootprintPos = pos;
      return ret;
   }
   State skipping() const
   {
      State ret = *this;
      for ( int i = 0; i < (int)tracks.size(); i++ )
         ret.tracks[i].time--;
      return ret;
   }

   vector<TrackState> tracks;
   int footprintsRemaining;
   vector<bool> colorIsUsed;
};

class WeaveSearcher
{
public:
   WeaveSearcher( int numTracks, int weaveLength, int footprint )
      : m_numTracks( numTracks ), m_weaveLength( weaveLength ), m_footprint( footprint )
   {
      m_belt.resize( numTracks, vector<int>( weaveLength ) );
      search( 0, State( numTracks, footprint ) );
   }
   string tracksStr() const
   {
      stringstream ss;
      for ( int k = 0; k < (int)m_belt.size(); k++ )
      {
         for ( int i = 0; i < (int)m_belt[k].size(); i++ )
            ss << (m_belt[k][i] == 0 ? "." : to_string( m_belt[k][i] ));
         ss << endl;
      }
      return ss.str();
   }
   int footprintAt( int pos ) const
   {
      for ( int k = 0; k < (int)m_belt.size(); k++ )
         if ( m_belt[k][pos] != 0 )
            return m_belt[k][pos];
      return 0;
   }
   string oneLineTracksStr() const
   {
      stringstream ss;
      for ( int i = 0; i < (int)m_belt[0].size(); i++ )
      {
         char c = '.' + footprintAt(i);
         ss << (c == '0' ? '.' : c);
      }
      return ss.str();
   }

   uint64_t bitPattern() const
   {
      uint64_t ret = 0;
      for ( int i = 0; i < (int)m_belt[0].size(); i++ )
         ret |= (footprintAt( i ) ? 1ull : 0ull) << i;
      return ret;
   }
   bool isSolutionValid( const State& state ) const
   {
      // detect this bad solution:
      // 6...5....5....5....5....
      // .....6.....6.....6.....6

      vector<int> trackColor;
      vector<bool> isColorUsed( 7 );
      for ( const TrackState& trackState : state.tracks )
      {
         trackColor.push_back( trackState.color );
         isColorUsed[trackState.color] = true;
      }
      for ( int pos = 0; pos < 6; pos++ )
      {
         for ( int trackIndex = 0; trackIndex < m_numTracks; trackIndex++ )
         {
            int color = m_belt[trackIndex][pos];
            if ( color <= 0 )
               continue;

            isColorUsed[trackColor[trackIndex]] = false;
            if ( isColorUsed[color] )
               return false; // already used by another track
            isColorUsed[color] = true;
            trackColor[trackIndex] = color;
         }
      }

      return true;
   }
   void search( int pos, State state )
   {
      if ( !state.isValid( pos ) )
         return;
      if ( !state.canFinish( pos, m_weaveLength ) )
         return;

      if ( pos == m_weaveLength )
      {
         if ( !isSolutionValid( state ) )
            return;
         m_solutionCount++;
         m_solutionBitPatterns.insert( bitPattern() );
         trace << tracksStr() << endl;
         //trace << oneLineTracksStr() << endl;
         return;
      }
      
      int nextUnusedTrack = state.nextUnusedTrack();
      for ( int trackIdx = 0; trackIdx < m_numTracks; trackIdx++ ) if ( state.tracks[trackIdx].isUsed() || trackIdx == nextUnusedTrack )
      {
         for ( int color = 3; color <= 6; color++ ) if ( state.tracks[trackIdx].color == color || !state.colorIsUsed[color] )
         {
            bool switchingTrackColor = state.tracks[trackIdx].isUsed() && color != state.tracks[trackIdx].color;
            if ( switchingTrackColor )
               continue;

            m_belt[trackIdx][pos] = color;
            search( pos + 1, state.placing( trackIdx, color, pos ) );
            m_belt[trackIdx][pos] = 0;
         }
      }

      search( pos + 1, state.skipping() );
   }

public:
   int m_numTracks;
   int m_weaveLength;
   int m_footprint;

public: // fluctuate during search
   vector<vector<int>> m_belt;
   int m_solutionCount = 0;
   unordered_set<uint64_t> m_solutionBitPatterns;
};


int main()
{
   WeaveSearcher searcher( 3, 24, 16 );
   //WeaveSearcher searcher( 2, 24, 9 );
   trace << "#sols = " << searcher.m_solutionCount << endl;
   trace << "#bitpatterns = " << searcher.m_solutionBitPatterns.size() << endl;

   vector<uint64_t> bitPatterns( searcher.m_solutionBitPatterns.begin(), searcher.m_solutionBitPatterns.end() );
   sort( bitPatterns.begin(), bitPatterns.end() );
   for ( uint64_t b : bitPatterns )
   {
      for ( int i = 0; i < searcher.m_weaveLength; i++ )
         trace << ( b & (1ull<<i) ? 'X' : '.');
      trace << endl;
   }

   return 0;
}