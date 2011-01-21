#include "binlog.h"
#include <vector>
#include <stdio.h>

int main (int argc, const char* argv[] ) 
{
  if(argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  std::vector<event_t> logs;
  Binlog::get(argv[1]).getLogs(&logs);
  std::vector<event_t>::iterator event;
  for(event = logs.begin(); event != logs.end(); event++) 
  {
    printf("{timestamp:%d, nick:%s, x:%i, y:%i, z:%i, old_type:%#x, old_meta:%#x, new_type:%#x, new_meta:%#x}\n", 
      event->timestamp, event->nick, event->x, event->y, event->z, 
      (int) event->otype, (int) event->ometa, (int) event->ntype, (int) event->nmeta );
  }
  return 0;
}
