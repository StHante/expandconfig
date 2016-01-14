#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cfg_t {
   // File handle
   FILE* fhandle;
   // Name
   char* name;
   // Number of static parts
   int stat_no;
   int stat_no_allocd;
   // Lengths of static parts in the file
   size_t* stat_len;
   size_t* stat_len_allocd;
   // Static parts
   char**  stat;
   // Number of dynamic parts
   int dyn_no;
   int dyn_no_allocd;
   // Number of alternatives in dynamic part
   int* dyn_alt_no;
   int* dyn_alt_no_allocd;
   // Length of alternatives of dynamic parts
   size_t** dyn_len;
   size_t** dyn_len_allocd;
   // Dynamic parts
   char***  dyn;
};

int main(int argc, char *argv[]);
void write_dyn_files(struct cfg_t* cfg);
void cleanup(struct cfg_t* cfg);
void check_dyn_len(struct cfg_t* cfg);
void check_dyn_alt_no(struct cfg_t* cfg);
void check_dyn_no(struct cfg_t* cfg);
void alloc_next_dyn(struct cfg_t* cfg);
void alloc_next_dyn_alt(struct cfg_t* cfg);
void check_stat_len(struct cfg_t* cfg);
void check_stat_no(struct cfg_t* cfg);
void alloc_next_stat(struct cfg_t* cfg);
void init(struct cfg_t* cfg);
void have_bracket(struct cfg_t* cfg);
void in_alternative(struct cfg_t* cfg);
void have_delimiter(struct cfg_t* cfg);
void have_end(struct cfg_t* cfg);
