/*
  The MIT License

  Copyright (c) 2009, 2010 Matvei Stefarov <me@matvei.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
 */

#ifndef _FORESTER_H
#define _FORESTER_H

class Forester
{
public:
  const int MAXTRIES = 1000;

  class ForesterArgs 
  {
  public:
    Operation OPERATION = Replant;
    int TREECOUNT = 15; // 0 = no limit if op=conserve/replant
    TreeShape SHAPE = Procedural;
    int HEIGHT = 25;
    int HEIGHTVARIATION = 15;
    bool WOOD = true;
    float TRUNKTHICKNESS = 1;
    float TRUNKHEIGHT = .7f;
    float BRANCHDENSITY = 1;
    RootMode ROOTS = Normal;
    bool ROOTBUTTRESSES = true;
    bool FOLIAGE = true;
    float FOLIAGEDENSITY = 1;
    bool MAPHEIGHTLIMIT = true;
    Block PLANTON = BLOCK_GRASS;
    Map inMap;
    Map outMap;

    Block bGroundSurface = BLOCK_GRASS;
    Block bTrunk = BLOCK_GRASS;
    Block bFoliage = BLOCK_GRASS;

    void Validate() 
    {
        if( TREECOUNT < 0 ) TREECOUNT = 0;
        if( HEIGHT < 1 ) HEIGHT = 1;
        if( HEIGHTVARIATION > HEIGHT ) HEIGHTVARIATION = HEIGHT;
        if( TRUNKTHICKNESS < 0 ) TRUNKTHICKNESS = 0;
        if( TRUNKHEIGHT < 0 ) TRUNKHEIGHT = 0;
        if( FOLIAGEDENSITY < 0 ) FOLIAGEDENSITY = 0;
        if( BRANCHDENSITY < 0 ) BRANCHDENSITY = 0;
    }
  };
  
  class Tree 
  {
  public:
    Vector3i pos;
    int height = 1;
    ForesterArgs args;

    Tree() { }

    virtual void Prepare() { }

    virtual void MakeTrunk() { }

    virtual void MakeFoliage() { }

    void Copy( Tree other ) 
    {
      args = other.args;
      pos = other.pos;
      height = other.height;
    }
  };
  
  class StickTree: public Tree 
  {
  public:
    void MakeTrunk() 
    {
      for( int i = 0; i < height; i++ ) 
      {
        args.outMap.SetBlock( pos.x, pos.z, pos.y + i, args.bTrunk );
      }
    }
  };


  class NormalTree: public StickTree 
  {
  public:
    void MakeFoliage() 
    {
      int topy = pos[1] + height - 1;
      int start = topy - 2;
      int end = topy + 2;

      for( int y = start; y < end; y++ ) 
      {
        int rad;
        if( y > start + 1 ) 
          rad = 1;
        else 
          rad = 2;
        
        for( int xoff = -rad; xoff < rad + 1; xoff++ ) 
        {
          for( int zoff = -rad; zoff < rad + 1; zoff++ ) 
          {
            if( args.rand.NextDouble() > .618 &&
                Math.Abs( xoff ) == Math.Abs( zoff ) &&
                Math.Abs( xoff ) == rad ) 
              continue;
            
            args.outMap.SetBlock( pos[0] + xoff, pos[2] + zoff, y, args.bFoliage );
          }
        }
      }
    }
  };


  class BambooTree: public StickTree 
  {
  public:
    void MakeFoliage()
    {
      int start = pos[1];
      int end = start + height + 1;
      for( int y = start; y < end; y++ ) 
      {
        for( int i = 0; i < 2; i++ ) 
        {
          int xoff = args.rand.Next( 0, 2 ) * 2 - 1;
          int zoff = args.rand.Next( 0, 2 ) * 2 - 1;
          args.outMap.SetBlock( pos[0] + xoff, pos[2] + zoff, y, args.bFoliage );
        }
      }
    }
  };
  
  class PalmTree: public StickTree 
  {
  public:
    void MakeFoliage() 
    {
      int y = pos[1] + height;
      for( int xoff = -2; xoff < 3; xoff++ ) 
      {
        for( int zoff = -2; zoff < 3; zoff++ ) 
        {
          if( Math.Abs( xoff ) == Math.Abs( zoff ) ) 
          {
            args.outMap.SetBlock( pos[0] + xoff, pos[2] + zoff, y, args.bFoliage );
          }
        }
      }
    }
  };


};

#endif