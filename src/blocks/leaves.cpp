/*
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cmath>
#include "leaves.h"
#include "map.h"
#include "mineserver.h"
#include "constants.h"
#include "plugin.h"


bool BlockLeaves::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  std::set<Decay>::iterator it = std::find_if(decaying.begin(), decaying.end(), DecayFinder(x,y,z,map));

  if (it != decaying.end()) decaying.erase(it);

  return true;
}

void BlockLeaves::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  if ( (oldblock != BLOCK_WOOD && oldblock != BLOCK_LEAVES)   ||
       std::find_if(decaying.begin(), decaying.end(), DecayFinder(x,y,z,map)) != decaying.end() )
  {
    return;
  }

  for (int xi = -2; xi <= 2; ++xi)
    for (int yi = -2; yi <= 2; ++yi)
      for (int zi = -2; zi <= 2; ++zi)
        if (std::abs(xi) + std::abs(yi) + std::abs(zi) <= 3)
        {
          uint8_t block, meta;

          ServerInstance->map(map)->getBlock(x + xi, y + yi, z + zi, &block, &meta);

          if (block == BLOCK_WOOD) return;
        }

  decaying.insert(Decay(time(0), x, y, z, map));
}

inline void decayIt(const Decay & decaying)
{
  uint8_t block, meta;
  const Plugin::BlockCBs & plugins =  ServerInstance->plugin()->getBlockCB();

  //this->notifyNeighbours(decaying[0].x,decaying[0].y,decaying[0].z,decaying[0].map,"onNeighbourBroken",0,BLOCK_LEAVES,0); // <--- USE THIS WHEN IT's FIXED

  for (int xoff = -1; xoff <= 1; ++xoff)
  {
    for (int yoff = -1; yoff <= 1; ++yoff)
    {
      for (int zoff = -1; zoff <= 1; ++zoff)
      {
        ServerInstance->map(decaying.map)->getBlock(decaying.x + xoff, decaying.y + yoff, decaying.z + zoff, &block, &meta);

        for (Plugin::BlockCBs::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        {
          if (*i != NULL && (*i)->affectedBlock(block))
          {
            (*i)->onNeighbourBroken(0, BLOCK_LEAVES, decaying.x + xoff, decaying.y + yoff, decaying.z + zoff, decaying.map, 0);
          }
        }
      }
    }
  }

  for (Plugin::BlockCBs::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
  {
    if ((*i)->affectedBlock(BLOCK_LEAVES))
    {
      (*i)->onBroken(0, 0, decaying.x, decaying.y, decaying.z, decaying.map, 0);
    }
  }
}

void BlockLeaves::timer200()
{
  while (!decaying.empty())
  {
    Decay d = *decaying.begin(); // copy! might get deleted by decayIt()

    if ((time(NULL) - d.decayStart) >= 5)
    {
      decayIt(d);
      decaying.erase(d); // Erasing by value is safe, erases either 0 or 1 elements.
    }
    else
    {
      return;
    }
  }
}
