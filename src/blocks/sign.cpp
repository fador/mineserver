  //Sign placement
  
  if(newblock == ITEM_SIGN)
  {
    if(direction == 0)
    {
      return PACKET_OK;
    }
    
    //This will make a sign post
    else if(direction == 1)
    {
      // 0x0 -> West  West  West  West
      // 0x1 -> West  West  West  North
      // 0x2 -> West  West  North North
      // 0x3 -> West  North North North
      // 0x4 -> North North North North
      // 0x5 -> North North North East
      // 0x6 -> North North East  East
      // 0x7 -> North East  East  East
      // 0x8 -> East  East  East  East
      // 0x9 -> East  East  East  South
      // 0xA -> East  East  South South
      // 0xB -> East  South South South
      // 0xC -> South South South South
      // 0xD -> South South South West
      // 0xE -> South South West  West
      // 0xF -> South West  West  West
      
      //           The values were the signs are facing
      //
      //                          North
      //
      //      -X |                  4
      //         |              3       5
      //         |           2             6
      //         |         1       [=]       7
      //  West   |        0         |         8           East
      //         |         F                 9
      //         |           E             A
      //         |              D       B
      //      +X |                  C
      //           ----------------------------------
      //           +Z                              -Z
      //
      //                          South
      
      
      // Were it should be placed depending on the user's position
      //
      //                          North
      //
      //      -X |                  C
      //         |              B       D
      //         |           A      o      E
      //         |         9       [ ]       F
      //  West   |        8        | |         0           East
      //         |         7                 1
      //         |           6             2
      //         |              5       3
      //      +X |                  4
      //           ----------------------------------
      //           +Z                              -Z
      //
      //                          South
      
      newblock = BLOCK_SIGN_POST;
      
      // We place according to the player's position

      double mdiffX = (x + 0.5) - user->pos.x; // + 0.5 to get the middle of the square
      double mdiffZ = (z + 0.5) - user->pos.z; // + 0.5 to get the middle of the square

      double angleDegree = ((atan2(mdiffZ, mdiffX) * 180 / M_PI + 90) / 22.5);
      
      if (angleDegree < 0) 
        angleDegree += 16;
         
      metadata = (uint8)(angleDegree + 0.5);
      
      //std::cout << "mdiffX= " << mdiffX << "  mdiffZ= " << mdiffZ << "  andgleDegree= " << angleDegree << "  metadata= " << (int)metadata << std::endl;
    }
    
    //Else wall sign
    else
    {
      newblock = BLOCK_WALL_SIGN;
      metadata = direction;
    }
  }
