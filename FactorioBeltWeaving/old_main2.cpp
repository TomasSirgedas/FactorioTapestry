#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <fstream>
#include "XY.h"
#include "Timer.h"
#include "trace.h"

using namespace std;

namespace
{
   int divRoundUp( int a, int b ) { return (a + b - 1) / b; }
   int mod( int x, int m ) { int ret = x % m; return ret + (ret < 0 ? m : 0); }
   string canonicalizedRotation( string s )
   {
      string ret = s;
      for ( int i = 0; i < s.length(); i++ )
      {
         rotate( s.begin(), s.begin() + 1, s.end() );
         ret = min( ret, s );
      }
      return ret;
   }
}

struct BeltState
{
   struct Hasher
   {
      size_t operator()( const BeltState& state ) const
      {
         return state.hash();
      }
   };

   size_t hash() const { return m6 * 1000 + m5 * 100 + m4 * 10 + m3; }
   bool operator==( const BeltState& rhs ) const
   {
      return hash() == rhs.hash();
   }
   int numActiveBelts() const
   {
      return (m3 != 0 ? 1 : 0)
         + (m4 != 0 ? 1 : 0)
         + (m5 != 0 ? 1 : 0)
         + (m6 != 0 ? 1 : 0);
   }
   int operator[]( int color ) const
   {
      if ( color == 3 ) return m3;
      if ( color == 4 ) return m4;
      if ( color == 5 ) return m5;
      if ( color == 6 ) return m6;
      throw runtime_error( "invalid color" );
   }
   bool isValid( int expectedActiveBelts ) const
   {
      if ( numActiveBelts() != expectedActiveBelts )
         return false;

      unordered_set<int> pos;
      if ( m3 && !pos.insert( 3 - m3 ).second ) return false;
      if ( m4 && !pos.insert( 4 - m4 ).second ) return false;
      if ( m5 && !pos.insert( 5 - m5 ).second ) return false;
      if ( m6 && !pos.insert( 6 - m6 ).second ) return false;
      //if ( m5 ) return false;
      //if ( m6 ) return false;
      return true;
   }
   bool hasBelt() const
   {
      return beltColor() > 0;
   }
   int beltColor() const
   {
      if ( m3 == 3 ) return 3;
      if ( m4 == 4 ) return 4;
      if ( m5 == 5 ) return 5;
      if ( m6 == 6 ) return 6;
      return 0;
   }
   static int dec( int x ) { return max( x - 1, 0 ); }
   vector<BeltState> nextStates() const
   {
      vector<BeltState> ret;

      // no footprint
      if ( m3 != 1 && m4 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BeltState{ dec( m3 ), dec( m4 ), dec( m5 ), dec( m6 ) } );

      // refresh belt with same color
      if ( m3 && m4 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BeltState{ 3, dec( m4 ), dec( m5 ), dec( m6 ) } );
      if ( m4 && m3 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BeltState{ dec( m3 ), 4, dec( m5 ), dec( m6 ) } );
      if ( m5 && m3 != 1 && m4 != 1 && m6 != 1 ) ret.push_back( BeltState{ dec( m3 ), dec( m4 ), 5, dec( m6 ) } );
      if ( m6 && m3 != 1 && m4 != 1 && m5 != 1 ) ret.push_back( BeltState{ dec( m3 ), dec( m4 ), dec( m5 ), 6 } );

      // refresh belt with different color
      if ( m3 && !m4 && m5 != 1 && m6 != 1 ) ret.push_back( BeltState{ 0, 4, dec( m5 ), dec( m6 ) } );
      if ( m3 && !m5 && m4 != 1 && m6 != 1 ) ret.push_back( BeltState{ 0, dec( m4 ), 5, dec( m6 ) } );
      if ( m3 && !m6 && m4 != 1 && m5 != 1 ) ret.push_back( BeltState{ 0, dec( m4 ), dec( m5 ), 6 } );
      if ( m4 && !m3 && m5 != 1 && m6 != 1 ) ret.push_back( BeltState{ 3, 0, dec( m5 ), dec( m6 ) } );
      if ( m4 && !m5 && m3 != 1 && m6 != 1 ) ret.push_back( BeltState{ dec( m3 ), 0, 5, dec( m6 ) } );
      if ( m4 && !m6 && m3 != 1 && m5 != 1 ) ret.push_back( BeltState{ dec( m3 ), 0, dec( m5 ), 6 } );
      if ( m5 && !m3 && m4 != 1 && m6 != 1 ) ret.push_back( BeltState{ 3, dec( m4 ), 0, dec( m6 ) } );
      if ( m5 && !m4 && m3 != 1 && m6 != 1 ) ret.push_back( BeltState{ dec( m3 ), 4, 0, dec( m6 ) } );
      if ( m5 && !m6 && m3 != 1 && m4 != 1 ) ret.push_back( BeltState{ dec( m3 ), dec( m4 ), 0, 6 } );
      if ( m6 && !m3 && m4 != 1 && m5 != 1 ) ret.push_back( BeltState{ 3, dec( m4 ), dec( m5 ), 0 } );
      if ( m6 && !m4 && m3 != 1 && m5 != 1 ) ret.push_back( BeltState{ dec( m3 ), 4, dec( m5 ), 0 } );
      if ( m6 && !m5 && m3 != 1 && m4 != 1 ) ret.push_back( BeltState{ dec( m3 ), dec( m4 ), 5, 0 } );

      ret.erase( std::remove_if( ret.begin(), ret.end(), [this]( BeltState s ) { return !isValid( numActiveBelts() ); } ), ret.end() );

      return ret;
   }

   int m3, m4, m5, m6;
};

struct BitPattern
{
   struct Hasher
   {
      size_t operator()( const BitPattern& pattern ) const
      {
         return pattern.m[0] + pattern.m_size * 1234567ull;
      }
   };

public:
   BitPattern() {}
   bool operator==( const BitPattern& rhs ) const { return m_size == rhs.m_size && m[0] == rhs.m[0] && m[1] == rhs.m[1] && m[2] == rhs.m[2] && m[3] == rhs.m[3]; }
   void push_back( bool x ) { m[m_size >> 6] |= ( x ? 1ull : 0ull ) << (m_size & 63); m_size++; }
   void pop_back() { m_size--; m[m_size >> 6] &= ~(1ull << (m_size & 63)); }
   bool operator[]( int idx ) const { return ((m[idx >> 6] >> (idx & 63)) & 1) ? true : false; }
   string str() const { stringstream ss; for ( int i = 0; i < m_size; i++ ) ss << ((*this)[i] ? 'X' : '.'); return ss.str(); }
   string reverseStr() const { string ret = str(); reverse( ret.begin(), ret.end() ); return ret; }
   int size() const { return m_size; }

public:
   int m_size = 0;
   uint64_t m[4] = { 0 };
};

struct AllStates
{
public:
   AllStates( int numBelts )
   {
      BeltState state;
      for ( state.m3 = 0; state.m3 <= 3; state.m3++ )
         for ( state.m4 = 0; state.m4 <= 4; state.m4++ )
            for ( state.m5 = 0; state.m5 <= 5; state.m5++ )
               for ( state.m6 = 0; state.m6 <= 6; state.m6++ )
                  if ( state.isValid( numBelts ) )
                     m_allStates.push_back( state );

      m_prevStatesMap = calcPrevStatesMap();

      //m_allStates.erase( remove_if( m_allStates.begin(), m_allStates.end(), [&]( BeltState state ) { return prevStatesMap.count( state ) == 0; } ), m_allStates.end() );
   }

   const vector<BeltState>& prevStates( BeltState state ) const
   {
      return m_prevStatesMap.at( state );
   }
   
private:
   unordered_map<BeltState, vector<BeltState>, BeltState::Hasher> calcPrevStatesMap() const
   {
      unordered_map<BeltState, vector<BeltState>, BeltState::Hasher> ret;
      for ( BeltState a : m_allStates )
         for ( BeltState b : a.nextStates() )
            ret[b].push_back( a );
      return ret;
   }

public:
   vector<BeltState> m_allStates;
   unordered_map<BeltState, vector<BeltState>, BeltState::Hasher> m_prevStatesMap;
};

class PathDescriber
{
   struct State
   {
      int color;
      string str;
   };
public:
   PathDescriber() {}
   PathDescriber( const vector<BeltState>& path )
   {
      for ( const BeltState& state : path )
         if ( state.hasBelt() )
            m_cost++;

      if ( path.back().m3 > 0 ) m_belts.push_back( { 3 } );
      if ( path.back().m4 > 0 ) m_belts.push_back( { 4 } );
      if ( path.back().m5 > 0 ) m_belts.push_back( { 5 } );
      if ( path.back().m6 > 0 ) m_belts.push_back( { 6 } );

      auto initialBeltColors = beltColors();

      while ( true )
      {
         for ( int i = 0; i < path.size(); i++ )
         {
            int whichBeltColorStarted = path[i].beltColor();
            int whichBeltColorEnded = whichBeltColorStarted;

            for ( State& beltState : m_belts )
            {
               if ( path[i][beltState.color] == 0 )
                  whichBeltColorEnded = beltState.color;
            }
            if ( whichBeltColorStarted > 0 )
               beltWithColor( whichBeltColorEnded ).color = whichBeltColorStarted;
            for ( State& beltState : m_belts )
            {
               beltState.str += beltState.color == whichBeltColorStarted ? char( '0' + whichBeltColorStarted ) : '.';
            }
         }
         if ( beltColors() == initialBeltColors )
            break;
      }
   }
   vector<int> beltColors() const
   {
      vector<int> ret;
      for ( const State& beltState : m_belts )
         ret.push_back( beltState.color );
      return ret;
   }
   State& beltWithColor( int color )
   {
      for ( State& beltState : m_belts )
         if ( beltState.color == color )
            return beltState;
      throw runtime_error( "beltWithColor" );
   }
   string str() const
   {
      string ret;
      for ( const State& beltState : m_belts )
         ret += beltState.str + "\n";
      return ret;
   }

public:
   int m_cost = 0;
   vector<State> m_belts;
};

class Path
{
public:

public:
   vector<BeltState> v;
};

class PathFinder
{
public:
   static constexpr int INF = 9999;
   struct Item
   {
      Item() {}
      Item( int cost ) : cost( cost ) {}
      int cost = INF;
      BeltState prevState;
   };

   PathFinder( int maxPathLen, const AllStates& allStates ) 
      : m_allStates( allStates )
   {
      m.resize( maxPathLen + 1 );
      for ( BeltState state : allStates.m_allStates )
         m[0][state][state] = Item( 0 );

      for ( int pathLen = 1; pathLen <= maxPathLen; pathLen++ )
      {
         for ( BeltState initialState : allStates.m_allStates )
         {
            for ( BeltState prevState : allStates.m_allStates )
            {
               for ( BeltState state : prevState.nextStates() )
               {
                  int newCost = m[pathLen - 1][initialState][prevState].cost + (state.hasBelt() ? 1 : 0);
                  if ( newCost < m[pathLen][initialState][state].cost )
                  {
                     m[pathLen][initialState][state].cost = newCost;
                     m[pathLen][initialState][state].prevState = prevState;
                  }
               }
            }
         }
      }
   }

   int cost( int pathLen, BeltState initialState, BeltState state ) const
   {
      return m[pathLen].at( initialState ).at( state ).cost;
   }
   int bestLoopCost( int pathLen ) const
   {
      int ret = INF;
      for ( BeltState state : m_allStates.m_allStates )
      {
         ret = min( ret, cost( pathLen, state, state ) );
      }
      return ret;
   }

   void findABestPath( int pathLen, BeltState initialState, BeltState state, vector<BeltState>& path ) const
   {
      if ( pathLen == 0 )
         return;

      //for ( BeltState prevState : m_allStates.prevStates( state ) )
      findABestPath( pathLen - 1, initialState, m[pathLen].at( initialState ).at( state ).prevState, path );
      path.push_back( state );
   }

   vector<BeltState> findABestPath( int pathLen ) const
   {
      int bestCost = bestLoopCost( pathLen );

      for ( BeltState state : m_allStates.m_allStates )
         if ( cost( pathLen, state, state ) <= bestCost )
         {
            vector<BeltState> path;
            findABestPath( pathLen, state, state, path );
            return path;
         }

      return vector<BeltState>();
   }

   void findAllPaths( int pathLen, int maxCost, BeltState initialState, BeltState state, vector<BeltState>& path ) const
   {
      if ( m[pathLen].at( initialState ).at( state ).cost > maxCost )
         return;

      if ( pathLen == 0 )
      {
         vector<BeltState> reversedPath( path.rbegin(), path.rend() );
         trace << PathDescriber( reversedPath ).str() << endl;
         return;
      }

      path.push_back( state );

      for ( BeltState prevState : m_allStates.prevStates( state ) )
         findAllPaths( pathLen - 1, maxCost - (state.hasBelt() ? 1 : 0), initialState, prevState, path);

      path.pop_back();
   }

   void findAllPaths( int pathLen, int maxCost ) const
   {
      vector<BeltState> path;
      for ( BeltState state : m_allStates.m_allStates )
         findAllPaths( pathLen, maxCost, state, state, path );
   }

   void findAllBitPatterns( int pathLen, int maxCost, BeltState initialState, BeltState state, BitPattern& pattern, vector<BeltState>& path, unordered_set<BitPattern, BitPattern::Hasher>& usedBitPatterns, const function<void( BitPattern, vector<BeltState> )>& func ) const
   {
      if ( m[pathLen].at( initialState ).at( state ).cost > maxCost )
         return;

      if ( pathLen == 0 )
      {
         if ( !usedBitPatterns.insert( pattern ).second )
            return;

         vector<BeltState> reversedPath( path.rbegin(), path.rend() );
         //trace << pattern.reverseStr() << endl;
         //trace << PathDescriber( reversedPath ).str() << " #" << usedBitPatterns.size() << endl;
         func( pattern, reversedPath );
         return;
      }

      pattern.push_back( state.hasBelt() );
      path.push_back( state );

      for ( BeltState prevState : m_allStates.prevStates( state ) )
         findAllBitPatterns( pathLen - 1, maxCost - (state.hasBelt() ? 1 : 0), initialState, prevState, pattern, path, usedBitPatterns, func );

      path.pop_back();
      pattern.pop_back();
   }

   void findAllBitPatterns( int pathLen, int maxCost, function<void( BitPattern, vector<BeltState> )> func ) const
   {
      BitPattern pattern;
      vector<BeltState> path;
      unordered_set<BitPattern, BitPattern::Hasher> usedBitPatterns;
      for ( BeltState state : m_allStates.m_allStates )
      {
         findAllBitPatterns( pathLen, maxCost, state, state, pattern, path, usedBitPatterns, func );
      }
   }

public:
   const AllStates& m_allStates;
   vector<unordered_map<BeltState, unordered_map<BeltState, Item, BeltState::Hasher>, BeltState::Hasher>> m; //[pathLen][initialState][state]
};

class FilteredResults
{
public:
   void add( string s )
   {
      m_results.insert( canonicalizedRotation( s ) );
      int numX = count( s.begin(), s.end(), 'X' );
      m_maxX = max( m_maxX, numX );
      m_minX = min( m_minX, numX );
   }
   void foreachSorted( function<void( string )> func )
   {
      vector<string> vresults( m_results.begin(), m_results.end() );
      sort( vresults.begin(), vresults.end() );
      for ( string result : vresults )
         func( result );
   }
   static void forEachDominatingString( int idx, string& s, int maxXsToRemove, const function<void( string )>& func )
   {
      func( s );
      if ( maxXsToRemove == 0 )
         return;

      for ( idx; idx < s.length(); idx++ ) if ( s[idx] == 'X' )
      {
         s[idx] = '.';
         forEachDominatingString( idx, s, maxXsToRemove - 1, func );
         s[idx] = 'X';
      }
   }
   static void forEachDominatingString( string s, int maxXsToRemove, const function<void( string )>& func )
   {
      for ( int idx = 0; idx < s.length(); idx++ ) if ( s[idx] == 'X' )
      {
         s[idx] = '.';
         forEachDominatingString( idx, s, maxXsToRemove - 1, func );
         s[idx] = 'X';
      }
   }

   bool isDominated( string s )
   {
      int numXs = count( s.begin(), s.end(), 'X' );
      int maxXsToRemove = numXs - m_minX;
      bool ret = false;      
      forEachDominatingString( s, maxXsToRemove, [&]( string ss ) {
         if ( m_results.count( canonicalizedRotation( ss ) ) )
            ret = true;
      } );
      return ret;
   }
   void removeDominated()
   {
      vector<string> removeList;
      for ( string s : m_results )
         if ( isDominated( s ) )
            removeList.push_back( s );
      for ( string s : removeList )
         m_results.erase( s );
   }

public:
   unordered_set<string> m_results;
   int m_maxX = 0;
   int m_minX = 999;
};

int main()
{
   Timer t;
   const int NUM_BELTS = 3;

   AllStates allStates( NUM_BELTS );

   int MAX_PATH_LEN = 100;
   PathFinder pathFinder( MAX_PATH_LEN, allStates );

   for ( int pathLen = 1; pathLen <= MAX_PATH_LEN; pathLen++ )
      trace << pathFinder.bestLoopCost( pathLen ) << "/" << pathLen << " " << (double)pathFinder.bestLoopCost( pathLen ) / pathLen << endl;
   for ( int pathLen = 3; pathLen <= MAX_PATH_LEN; pathLen++ )
      trace << pathFinder.bestLoopCost( pathLen ) << endl;

   for ( int pathLen = 0; pathLen <= MAX_PATH_LEN; pathLen++ )
      trace << pathFinder.bestLoopCost( pathLen ) << ",";
   trace << endl;

   //auto path = pathFinder.findABestPath( 15 );
   //trace << PathDescriber( path ).str();
   //pathFinder.findAllPaths( 8, 4 );

   //for ( int LENGTH = 3; LENGTH <= MAX_PATH_LEN; LENGTH++ )
   //for ( int LENGTH = 34*2; LENGTH <= 34*2; LENGTH++ )
   for ( int LENGTH : { 46 } )
   {
      int COST = pathFinder.bestLoopCost( LENGTH ) + 0;
      
      stringstream filename;
      //filename << "46_nomix.txt";
      //filename << "..\\results\\" << NUM_BELTS << " belt\\" << LENGTH << "_" << COST << "_nomixed.txt";
      //filename << "..\\results_BRY\\" << NUM_BELTS << " belt\\" << LENGTH << "_" << COST << ".txt";
      filename << "..\\results_origami\\" << NUM_BELTS << " belt\\" << LENGTH << "_" << COST << ".txt";
      ofstream f( filename.str().c_str() );
            
      FilteredResults results;

      pathFinder.findAllBitPatterns( LENGTH, COST, [&]( BitPattern pattern, vector<BeltState> path ) {
         results.add( pattern.reverseStr() );
         //f     << pattern.reverseStr() << endl;
         //f     << PathDescriber( path ).str() << endl;
      } );

      results.removeDominated();
      results.foreachSorted( [&]( string result ) {
         f << result << endl;
      } );

      trace << "LENGTH = " << LENGTH << "\t" << results.m_results.size() << endl;

      //trace << endl;
      //trace << endl;
      //trace << endl;
   }
   trace << "time = " << t.ElapsedTime() << endl;


   return 0;
}


//int main()
//{
//   const int NUM_BELTS = 3;
//
//   AllStates allStates( NUM_BELTS );
//
//   int MAX_PATH_LEN = 100;
//   PathFinder pathFinder( MAX_PATH_LEN, allStates );
//
//   for ( int pathLen = 1; pathLen <= MAX_PATH_LEN; pathLen++ )
//      trace << pathFinder.bestLoopCost( pathLen ) << "/" << pathLen << " " << (double)pathFinder.bestLoopCost( pathLen ) / pathLen << endl;
//
//
//   //auto path = pathFinder.findABestPath( 15 );
//   //trace << PathDescriber( path ).str();
//   //pathFinder.findAllPaths( 8, 4 );
//
//   double best = 1;
//   for ( int LENGTH = 3; LENGTH <= MAX_PATH_LEN; LENGTH++ )
//   {
//      int COST = pathFinder.bestLoopCost( LENGTH );
//
//      string extra;
//      if ( (double)COST / LENGTH < best )
//      {
//         best = (double)COST / LENGTH;
//         extra = " ***";
//      }
//
//      trace << COST << "/" << LENGTH << "\t" << (double) COST / LENGTH << extra << endl;
//
//      //if ( !extra.empty() )
//         trace << PathDescriber( pathFinder.findABestPath( LENGTH ) ).str() << endl;
//   }
//
//   return 0;
//}