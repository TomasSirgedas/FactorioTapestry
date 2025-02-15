#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <limits>

using namespace std;

namespace
{
   string canonicalRotation( string s )
   {
      string ret = s;
      for ( int i = 1; i < (int)s.length(); i++ )
      {
         rotate( s.begin(), s.begin() + 1, s.end() );
         ret = min( ret, s );
      }
      return ret;
   }
}

struct BraidState
{
   struct Hasher
   {
      size_t operator()( const BraidState& state ) const
      {
         return state.hash();
      }
   };

   size_t hash() const { return m6 * 1000 + m5 * 100 + m4 * 10 + m3; }
   bool operator==( const BraidState& rhs ) const
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

      if ( (m3 > 1) + (m4 > 1) + (m5 > 1) + (m6 > 1) + 1 < expectedActiveBelts ) return false;
      if ( (m3 > 2) + (m4 > 2) + (m5 > 2) + (m6 > 2) + 2 < expectedActiveBelts ) return false;

      unordered_set<int> pos;
      if ( m3 && !pos.insert( 3 - m3 ).second ) return false;
      if ( m4 && !pos.insert( 4 - m4 ).second ) return false;
      if ( m5 && !pos.insert( 5 - m5 ).second ) return false;
      if ( m6 && !pos.insert( 6 - m6 ).second ) return false;
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
   vector<BraidState> nextStates() const
   {
      vector<BraidState> ret;

      // no footprint
      if ( m3 != 1 && m4 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BraidState{ dec( m3 ), dec( m4 ), dec( m5 ), dec( m6 ) } );

      // refresh belt with same color
      if ( m3 && m4 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BraidState{ 3, dec( m4 ), dec( m5 ), dec( m6 ) } );
      if ( m4 && m3 != 1 && m5 != 1 && m6 != 1 ) ret.push_back( BraidState{ dec( m3 ), 4, dec( m5 ), dec( m6 ) } );
      if ( m5 && m3 != 1 && m4 != 1 && m6 != 1 ) ret.push_back( BraidState{ dec( m3 ), dec( m4 ), 5, dec( m6 ) } );
      if ( m6 && m3 != 1 && m4 != 1 && m5 != 1 ) ret.push_back( BraidState{ dec( m3 ), dec( m4 ), dec( m5 ), 6 } );

      // refresh belt with different color
      if ( m3 && !m4 && m5 != 1 && m6 != 1 ) ret.push_back( BraidState{ 0, 4, dec( m5 ), dec( m6 ) } );
      if ( m3 && !m5 && m4 != 1 && m6 != 1 ) ret.push_back( BraidState{ 0, dec( m4 ), 5, dec( m6 ) } );
      if ( m3 && !m6 && m4 != 1 && m5 != 1 ) ret.push_back( BraidState{ 0, dec( m4 ), dec( m5 ), 6 } );
      if ( m4 && !m3 && m5 != 1 && m6 != 1 ) ret.push_back( BraidState{ 3, 0, dec( m5 ), dec( m6 ) } );
      if ( m4 && !m5 && m3 != 1 && m6 != 1 ) ret.push_back( BraidState{ dec( m3 ), 0, 5, dec( m6 ) } );
      if ( m4 && !m6 && m3 != 1 && m5 != 1 ) ret.push_back( BraidState{ dec( m3 ), 0, dec( m5 ), 6 } );
      if ( m5 && !m3 && m4 != 1 && m6 != 1 ) ret.push_back( BraidState{ 3, dec( m4 ), 0, dec( m6 ) } );
      if ( m5 && !m4 && m3 != 1 && m6 != 1 ) ret.push_back( BraidState{ dec( m3 ), 4, 0, dec( m6 ) } );
      if ( m5 && !m6 && m3 != 1 && m4 != 1 ) ret.push_back( BraidState{ dec( m3 ), dec( m4 ), 0, 6 } );
      if ( m6 && !m3 && m4 != 1 && m5 != 1 ) ret.push_back( BraidState{ 3, dec( m4 ), dec( m5 ), 0 } );
      if ( m6 && !m4 && m3 != 1 && m5 != 1 ) ret.push_back( BraidState{ dec( m3 ), 4, dec( m5 ), 0 } );
      if ( m6 && !m5 && m3 != 1 && m4 != 1 ) ret.push_back( BraidState{ dec( m3 ), dec( m4 ), 5, 0 } );

      ret.erase( std::remove_if( ret.begin(), ret.end(), [this]( BraidState s ) { return !s.isValid( numActiveBelts() ); } ), ret.end() );

      return ret;
   }
   string str() const { return to_string( m3 ) + to_string( m4 ) + to_string( m5 ) + to_string( m6 ); }

   int m3, m4, m5, m6;
};

struct BraidStates
{
public:
   struct Node
   {
      BraidState braidState;
      bool hasBelt;
      vector<int> nextStates;
      vector<int> prevStates;
      vector<int> prevStatesWithBelt;
      vector<int> prevStatesWithoutBelt;
   };

public:
   BraidStates( int numBelts )
      : m_numBelts( numBelts )
   {
      BraidState state;
      for ( state.m3 = 0; state.m3 <= 3; state.m3++ )
         for ( state.m4 = 0; state.m4 <= 4; state.m4++ )
            for ( state.m5 = 0; state.m5 <= 5; state.m5++ )
               for ( state.m6 = 0; state.m6 <= 6; state.m6++ )
                  if ( state.isValid( numBelts ) )
                     m_nodes.push_back( { state, state.hasBelt() } );

      for ( int i = 0; i < (int)m_nodes.size(); i++ )
         m_toIndex[m_nodes[i].braidState] = i;

      for ( Node& node : m_nodes )
      {
         for ( BraidState& nextState : node.braidState.nextStates() )
         {
            node.nextStates.push_back( toIndex( nextState ) );
            m_nodes[toIndex( nextState )].prevStates.push_back( toIndex( node.braidState ) );
         }
      }
      for ( Node& node : m_nodes )
      {
         sort( node.nextStates.begin(), node.nextStates.end() );
         sort( node.prevStates.begin(), node.prevStates.end() );
      }
      for ( Node& node : m_nodes )
         for ( int prevState : node.prevStates )
            (m_nodes[prevState].hasBelt ? node.prevStatesWithBelt : node.prevStatesWithoutBelt).push_back( prevState );
   }   
   int toIndex( const BraidState& state ) const
   {
      return m_toIndex.at( state );
   }
   const Node& operator[]( int idx ) const
   {
      return m_nodes[idx];
   }
   int numNodes() const
   {
      return (int) m_nodes.size();
   }

public:
   int m_numBelts;
   unordered_map<BraidState, int, BraidState::Hasher> m_toIndex;
   vector<Node> m_nodes;
};

class PathTable
{
public:
   typedef uint16_t CostType;
   CostType INF = std::numeric_limits<CostType>::max() - 1;

public:
   PathTable( const BraidStates& braidStates )
      : m_braidStates( braidStates )
   {
      extendTable();
      for ( int idx = 0; idx < m_braidStates.numNodes(); idx++ )
         m_table[0][idx][idx] = 0;
   }
   int cheapestCycle( int pathLen )
   {
      ensurePathLenAvailable( pathLen );

      int ret = INF;
      for ( int state = 0; state < m_braidStates.numNodes(); state++ )
         ret = min<int>( ret, m_table[pathLen][state][state] );
      return ret;
   }

   void ensurePathLenAvailable( int pathLen )
   {
      while ( m_table.size() <= pathLen )
         extendTable();
   }
   void extendTable()
   {
      int pathLen = (int) m_table.size();
      m_table.push_back( vector<vector<CostType>>( m_braidStates.numNodes(), vector<CostType>( m_braidStates.numNodes(), INF ) ) );
      if ( pathLen == 0 )
         return;

      for ( int initialState = 0; initialState < m_braidStates.numNodes(); initialState++ )
      {
         auto& m = m_table[pathLen - 1][initialState];
         auto& mm = m_table[pathLen][initialState];
         for ( int state = 0; state < m_braidStates.numNodes(); state++ )
            for ( int nextState : m_braidStates[state].nextStates )
               mm[nextState] = min<CostType>( mm[nextState], m[state] + (m_braidStates[nextState].hasBelt ? 1 : 0) );
      }
   }

public:
   const BraidStates& m_braidStates;
   vector<vector<vector<CostType>>> m_table; //[pathLen][initialState][state] = minimum path cost
};

enum class PrintPatternsMode : int
{
   ALL, UNIQUE, NONE
};

class PathEnumerator
{
public:
   struct Config
   {
      int initialState;
      int state;
      int costAvailable;
   };

public:
   PathEnumerator( PathTable& pathTable, int pathLen, int cost, PrintPatternsMode printMode )
      : m_pathTable( pathTable )
      , m_printMode( printMode )
   {
      pathTable.ensurePathLenAvailable( pathLen );
      vector<Config> configs;
      for ( int state = 0; state < pathTable.m_braidStates.numNodes(); state++ )
      {
         Config config{ state, state, cost };
         if ( isViable( config, pathLen ) )
            configs.push_back( config );
      }
      go( configs, pathLen );
   }
   bool isViable( const Config& config, int pathLen ) const
   {
      return m_pathTable.m_table[pathLen][config.initialState][config.state] <= config.costAvailable;
   }

   void go( const vector<Config>& configs, int pathLen )
   {
      if ( pathLen == 0 )
      {
         // NOTE: `m_bitPattern` is actually backwards, but it works, and it's sorted nicer this way

         bool printIt = m_printMode == PrintPatternsMode::ALL;
         if ( m_printMode == PrintPatternsMode::UNIQUE )
            if ( m_usedPatterns.insert( canonicalRotation( m_bitPattern ) ).second )
               printIt = true;

         if ( printIt )
         {
            cout << m_bitPattern << endl;
            m_numPathsPrinted++;
         }

         m_numPaths++;

         if ( m_numPaths % 100'000 == 0 && m_printMode == PrintPatternsMode::NONE )
            cout << "# paths found so far = " << m_numPaths << endl;

         return;
      }

      // with belt
      {
         vector<Config> prevConfigs;
         for ( const Config& config : configs )
         for ( int prevState : m_pathTable.m_braidStates[config.state].prevStatesWithBelt )
         {
            Config prevConfig{ config.initialState, prevState, config.costAvailable - (m_pathTable.m_braidStates[config.state].hasBelt ? 1 : 0) };
            if ( isViable( prevConfig, pathLen - 1 ) )
               prevConfigs.push_back( prevConfig );
         }
         if ( !prevConfigs.empty() )
         {
            m_bitPattern.push_back( 'X' );
            go( prevConfigs, pathLen - 1 );
            m_bitPattern.pop_back();
         }
      }
      // without belt
      {
         vector<Config> prevConfigs;
         for ( const Config& config : configs )
         for ( int prevState : m_pathTable.m_braidStates[config.state].prevStatesWithoutBelt )
         {
            Config prevConfig{ config.initialState, prevState, config.costAvailable - (m_pathTable.m_braidStates[config.state].hasBelt ? 1 : 0) };
            if ( isViable( prevConfig, pathLen - 1 ) )
               prevConfigs.push_back( prevConfig );
         }
         if ( !prevConfigs.empty() )
         {
            m_bitPattern.push_back( '.' );
            go( prevConfigs, pathLen - 1 );
            m_bitPattern.pop_back();
         }
      }
   }

public:
   PathTable& m_pathTable;
   string m_bitPattern;
   PrintPatternsMode m_printMode;
   int64_t m_numPaths = 0;
   int64_t m_numPathsPrinted = 0;
   unordered_set<string> m_usedPatterns;
};

void printBraidBitPatterns( int numberOfBelts, int length, int cost, PrintPatternsMode printMode )
{
   cout << "numberOfBelts = " << numberOfBelts << endl;
   cout << "length = " << length << endl;
   cout << "cost = " << cost << endl;
   cout << endl;

   BraidStates braidStates( numberOfBelts );
   PathTable pathTable( braidStates );
   PathEnumerator pathEnumerator( pathTable, length, cost, printMode );

   cout << endl;
   cout << "number of patterns, including rotations = " << pathEnumerator.m_numPaths << endl;
   cout << "number of patterns printed = " << pathEnumerator.m_numPathsPrinted << endl;
}

int main()
{
   int numberOfBelts = 3; // 2 or 3
   int loopLength = 34;
   int cost = 23; // # of placements
   auto printMode = PrintPatternsMode::UNIQUE; // NONE, ALL, or UNIQUE (which avoids printing cyclical duplicates -- uses extra memory)

   printBraidBitPatterns( numberOfBelts, loopLength, cost, printMode );
   return 0;
}
