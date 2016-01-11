#include "expandconfig.h"

int main(int argc, char *argv[]) {
   // Check arguments
   if (argc <= 1) {
      printf("Usage: %s [filename]\n", argv[0]);
      exit(1);
   }

   // Create new cfg object
   struct cfg_t cfg;
   
   // Try to open file
   cfg.fhandle = fopen(argv[argc-1],"r");
   if (cfg.fhandle == NULL) {
      // Error
      printf("ERROR: Could not open file %s\n",argv[argc-1]);
      exit(1);
   }
   
   // Give name
   cfg.name = argv[argc-1];
   
   // Initialize
   cfg.stat_no = 1;
   cfg.stat_no_allocd = 4;
   cfg.stat_len        = (size_t*) malloc(cfg.stat_no_allocd*sizeof(size_t));
   cfg.stat_len_allocd = (size_t*) malloc(cfg.stat_no_allocd*sizeof(size_t));
   cfg.stat_len[0] = 0;
   cfg.stat_len_allocd[0] = 16;
   cfg.stat = (char**) malloc(cfg.stat_no_allocd*sizeof(char*));
   cfg.stat[0] = (char*) malloc(cfg.stat_len_allocd[0]*sizeof(char));
   cfg.dyn_no = 0;
   cfg.dyn_no_allocd = 0;
   cfg.dyn_alt_no = NULL;
   cfg.dyn_alt_no_allocd = NULL;
   cfg.dyn_len = NULL;
   cfg.dyn_len_allocd = NULL;
   cfg.dyn = NULL;
   
   // Begin parsing
   init(&cfg);
   
   // Close file
   fclose(cfg.fhandle);
   
   #ifdef DEBUG
   // Print
   int i, j;
   size_t c;
   for (i=0; i<cfg.stat_no; i++) {
      printf("stat[%d]: ",i);
      for (c=0; c<cfg.stat_len[i]; c++) {
         printf("%c",cfg.stat[i][c]);
      }
      printf("\n");
   }
   for (i=0; i<cfg.dyn_no; i++) {
      for (j=0; j<cfg.dyn_alt_no[i]; j++) {
         printf("dyn[%d][%d]: ",i,j);
         for (c=0; c<cfg.dyn_len[i][j]; c++) {
            printf("%c",cfg.dyn[i][j][c]);
         }
         printf("\n");
      }
   }
   #endif
   
   // Write all different files
   write_dyn_files(&cfg);
   
   // Free allocated space
   cleanup(&cfg);
   
   return 0;
}

void write_dyn_files(struct cfg_t* cfg) {
   // DEBUG
   int i, j;
   int overtake;
   size_t c;
   FILE* outfile;
   char* outname;
   
   // Find out how many possibilities for different alternatives there are
   int possibilities = 1;
   for (i=0; i<cfg->dyn_no; i++) {
      possibilities = possibilities * cfg->dyn_alt_no[i];
   }
   
   // Create array that holds the index of the alternatives
   int* indices = malloc(cfg->dyn_no*sizeof(int));
   for (i=0; i<cfg->dyn_no; i++) indices[i] = 0;
   
   // Check for too many possibilities >= 10^7
   if (possibilities > 999999) {
      printf("Error: Possibilities exceed 10^7-1 = 9 999 999, aborting");
      exit(1);
   }
   
   // Loop over all possible files
   for (i=0; i<possibilities; i++) {
      
      #ifdef DEBUG
      // Print the indices
      printf("indices:");
      for (j=0; j<cfg->dyn_no; j++) printf("  %d", indices[j]);
      printf("\n");
      #endif
      
      // Open output file
                       //                  "."  #  "." exp  \0
      outname = malloc((strlen(cfg->name) + 1 + 7 + 1 + 3 + 1)*sizeof(char));
      sprintf(outname, "%s.%07d.exp", cfg->name, i);
      outfile = fopen(outname,"w+");
      free(outname);
      
      // Print the files
      for (j=0; j<cfg->stat_no; j++) {
         // Print static part
         for (c=0; c<cfg->stat_len[j]; c++)
            fprintf(outfile, "%c", cfg->stat[j][c]);
         
         // Print dynamic part (if present)
         if (j < cfg->dyn_no) {
            for (c=0; c<cfg->dyn_len[j][indices[j]]; c++)
               fprintf(outfile, "%c", cfg->dyn[j][indices[j]][c]);
         }
      }
      
      // Close file
      fclose(outfile);
      
      // Increase the index
      overtake = 1;
      for (j=0; j<cfg->dyn_no; j++) {
         if (overtake == 1) {
            indices[j] ++;
            if (indices[j] >= cfg->dyn_alt_no[j]) {
               indices[j] = 0;
               overtake = 1;
            } else {
               overtake = 0;
            }
         }
      }
   }
   
   // Free indices
   free(indices);
   
}

void cleanup(struct cfg_t* cfg) {
   // Free all allocated space
   int i, j;
   
   // Static text variables
   for (i=0; i<cfg->stat_no; i++) {
      free(cfg->stat[i]);
   }
   free(cfg->stat);
   free(cfg->stat_len);
   free(cfg->stat_len_allocd);
      
   // Dynamic text variables
   for (i=0; i<cfg->dyn_no; i++) {
      for (j=0; j<cfg->dyn_alt_no[i]; j++) {
         free(cfg->dyn[i][j]);
      }
      free(cfg->dyn[i]);
      free(cfg->dyn_len[i]);
      free(cfg->dyn_len_allocd[i]);
   }
   free(cfg->dyn);
   free(cfg->dyn_len);
   free(cfg->dyn_len_allocd);
   free(cfg->dyn_alt_no);
   free(cfg->dyn_alt_no_allocd);

}

void check_dyn_len(struct cfg_t* cfg) {
   // Check length
   if (    cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]
        >= cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]) {
      // Reallocate if necessary
      if (cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] == 0) {
         cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] = 1;
      } else {
         cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] *= 2;
      }
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] =
         (char*) realloc(cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1],
                         cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]*sizeof(char));
   }
   
   return;
}

void check_dyn_alt_no(struct cfg_t* cfg) {
   // Check length
   if (    cfg->dyn_alt_no[cfg->dyn_no-1]
        >= cfg->dyn_alt_no_allocd[cfg->dyn_no-1]) {
      // Reallocate if necessary
      if (cfg->dyn_alt_no_allocd[cfg->dyn_no-1] == 0) {
         cfg->dyn_alt_no_allocd[cfg->dyn_no-1] = 1;
      } else {
         cfg->dyn_alt_no_allocd[cfg->dyn_no-1] *= 2;
      }
      cfg->dyn[cfg->dyn_no-1] =
         (char**) realloc(cfg->dyn[cfg->dyn_no-1],
                          cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(char*));
      cfg->dyn_len[cfg->dyn_no-1] =
         (size_t*) realloc(cfg->dyn_len[cfg->dyn_no-1],
                           cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(size_t));
      cfg->dyn_len_allocd[cfg->dyn_no-1] =
         (size_t*) realloc(cfg->dyn_len_allocd[cfg->dyn_no-1],
                           cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(size_t));
   }
   
   return;
}


void check_dyn_no(struct cfg_t* cfg) {
   // Check length
   if (    cfg->dyn_no
        >= cfg->dyn_no_allocd) {
      // Reallocate if necessary
      if (cfg->dyn_no_allocd == 0) {
         cfg->dyn_no_allocd = 1;
      } else {
         cfg->dyn_no_allocd *= 2;
      }
      cfg->dyn =
         (char***) realloc(cfg->dyn,
                           cfg->dyn_no_allocd*sizeof(char**));
      cfg->dyn_alt_no =
         (int*) realloc(cfg->dyn_alt_no,
                        cfg->dyn_no_allocd*sizeof(int));
      cfg->dyn_alt_no_allocd =
         (int*) realloc(cfg->dyn_alt_no_allocd,
                        cfg->dyn_no_allocd*sizeof(int));
      cfg->dyn_len =
         (size_t**) realloc(cfg->dyn_len,
                            cfg->dyn_no_allocd*sizeof(size_t*));
      cfg->dyn_len_allocd =
         (size_t**) realloc(cfg->dyn_len_allocd,
                            cfg->dyn_no_allocd*sizeof(size_t*));
   }
   
   return;
}

void alloc_next_dyn(struct cfg_t* cfg) {
          cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] = 0;
   cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] = 8;
   cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1] =
      (char*) malloc(cfg->dyn_len_allocd[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]*sizeof(char));
   return;
}

void alloc_next_dyn_alt(struct cfg_t* cfg) {
   cfg->dyn_alt_no[cfg->dyn_no-1] = 0;
   cfg->dyn_alt_no_allocd[cfg->dyn_no-1] = 4;
   cfg->dyn[cfg->dyn_no-1] =
      (char**) malloc(cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(char*));
   cfg->dyn_len[cfg->dyn_no-1] = 
      (size_t*) malloc(cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(size_t));
   cfg->dyn_len_allocd[cfg->dyn_no-1] = 
      (size_t*) malloc(cfg->dyn_alt_no_allocd[cfg->dyn_no-1]*sizeof(size_t));
   return;
}

void check_stat_len(struct cfg_t* cfg) {
   // Check length
   if (     cfg->stat_len[cfg->stat_no-1]
         >= cfg->stat_len_allocd[cfg->stat_no-1]) {
      // Reallocate if necessary
      cfg->stat_len_allocd[cfg->stat_no-1] *= 2;
      cfg->stat[cfg->stat_no-1] =
         (char *) realloc(cfg->stat[cfg->stat_no-1],
                          cfg->stat_len_allocd[cfg->stat_no-1]*sizeof(char));
   }
   
   return;
}

void check_stat_no(struct cfg_t* cfg) {
   // Check number
   if ( cfg->stat_no >= cfg->stat_no_allocd) {
      // Reallocate if necessary
      cfg->stat_no_allocd *= 2;
      cfg->stat = (char**) realloc(cfg->stat, cfg->stat_no_allocd*sizeof(char*));
      cfg->stat_len = (size_t*) realloc(cfg->stat_len, cfg->stat_no_allocd*sizeof(size_t));
      cfg->stat_len_allocd = (size_t*) realloc(cfg->stat_len_allocd, cfg->stat_no_allocd*sizeof(size_t));
   }
   
   return;
}

void alloc_next_stat(struct cfg_t* cfg) {
   cfg->stat_len[cfg->stat_no-1] = 0;
   cfg->stat_len_allocd[cfg->stat_no-1] = 16;
   cfg->stat[cfg->stat_no-1] = (char*) malloc(cfg->stat_len_allocd[cfg->stat_no-1]*sizeof(char));   
   return;
}

void init(struct cfg_t* cfg) {
   // Read one character from the file
   char character;
   if (fscanf(cfg->fhandle, "%c", &character) < 1) {
      // Successfully read everything
      return;
   }
   
   #ifdef DEBUG
   printf("%c: ", character);
   #endif
   
   // Test the character
   if (character == '[') {
      #ifdef DEBUG
      printf("init und [\n");
      #endif
      
      have_bracket(cfg);
   }
   else {
      #ifdef DEBUG
      printf("init und kein [\n");
      #endif
      
      // Character is static
      check_stat_len(cfg);
      // Save character
      cfg->stat[cfg->stat_no-1][cfg->stat_len[cfg->stat_no-1]] = character;
      cfg->stat_len[cfg->stat_no-1]++;  
      init(cfg);
   }
      
   return;
}

void have_bracket(struct cfg_t* cfg) {
   // Read one character from the file
   char character;
   if (fscanf(cfg->fhandle, "%c", &character) < 1) {
      // Successfully read everything
      return;
   }
   
   #ifdef DEBUG
   printf("%c: ", character);
   #endif
   
   // Test the character
   if (character == '[') {
      #ifdef DEBUG
      printf("have_bracket und [\n");
      #endif
      
      // We have found a new dynamic part
      check_dyn_no(cfg); 
      cfg->dyn_no++;
      alloc_next_dyn_alt(cfg);
      check_dyn_alt_no(cfg);
      cfg->dyn_alt_no[cfg->dyn_no-1]++;
      alloc_next_dyn(cfg);      
      
      in_alternative(cfg);
   }
   else {
      #ifdef DEBUG
      printf("have_bracket und kein [\n");
      #endif
      
      // Characters '[' and character are static
      check_stat_len(cfg);
      check_stat_len(cfg);
      // Save characters
      cfg->stat[cfg->stat_no-1][cfg->stat_len[cfg->stat_no-1]] = '[';
      cfg->stat_len[cfg->stat_no-1]++;  
      cfg->stat[cfg->stat_no-1][cfg->stat_len[cfg->stat_no-1]] = character;
      cfg->stat_len[cfg->stat_no-1]++;  
      
      init(cfg);
   }  
   
   return;
}

void in_alternative(struct cfg_t* cfg) {
   // Read one character from the file
   char character;
   if (fscanf(cfg->fhandle, "%c", &character) < 1) {
      // Error
      printf("ERROR: Unmatched [[\n");
      exit(1);
   }
   
   #ifdef DEBUG
   printf("%c: ", character);
   #endif
   
   // Test the character
   if (character == '|') {
      #ifdef DEBUG
      printf("in_alternative und |\n");
      #endif
      
      have_delimiter(cfg);
   }
   else if (character == ']') {
      #ifdef DEBUG
      printf("in_alternative und ]\n");
      #endif
      
      have_end(cfg);
   }
   else {
      #ifdef DEBUG
      printf("in_alternative und kein | oder ]\n");
      #endif
      
      // Character is dynamic
      check_dyn_len(cfg);
      // Save character
      cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]++;
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1][cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]-1]
         = character;
      
      in_alternative(cfg);
   }
   
   return;
}


void have_delimiter(struct cfg_t* cfg) {
   // Read one character from the file
   char character;
   if (fscanf(cfg->fhandle, "%c", &character) < 1) {
      // Error
      printf("ERROR: Unmatched [[\n");
      exit(1);
   }
   
   #ifdef DEBUG
   printf("%c: ", character);
   #endif
   
   // Test the character
   if (character == '|') {
      #ifdef DEBUG
      printf("have_delimiter und |\n");
      #endif
      
      // Next alternative
      check_dyn_alt_no(cfg);
      cfg->dyn_alt_no[cfg->dyn_no-1]++;
      alloc_next_dyn(cfg);
      
      in_alternative(cfg);
   }
   else {
      #ifdef DEBUG
      printf("have_delimiter und kein |\n");
      #endif
      
      // Character '|' was normal text 
      check_dyn_len(cfg);
      // Save character
      cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]++;
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1][cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]-1]
         = '|';      
      // Character char is dynamic
      check_dyn_len(cfg);
      // Save character
      cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]++;
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1][cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]-1]
         = character;
      
      in_alternative(cfg);
   }  
   
   return;
}



void have_end(struct cfg_t* cfg) {
   // Read one character from the file
   char character;
   if (fscanf(cfg->fhandle, "%c", &character) < 1) {
      // Error
      printf("ERROR: Unmatched ]]\n");
      exit(1);
   }
   
   #ifdef DEBUG
   printf("%c: ", character);
   #endif
   
   // Test the character
   if (character == ']') {
      #ifdef DEBUG
      printf("have_end und ]\n");
      #endif
      
      // New static section
      check_stat_no(cfg);
      cfg->stat_no ++;
      alloc_next_stat(cfg);
      
      init(cfg);
   }
   else {
      #ifdef DEBUG
      printf("have_end und kein ]\n");
      #endif
      
      // Character ']' was normal text
      check_dyn_len(cfg);
      // Save character
      cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]++;
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1][cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]-1]
         = ']';
      // Character char is dynamic
      check_dyn_len(cfg);
      // Save character
      cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]++;
      cfg->dyn[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1][cfg->dyn_len[cfg->dyn_no-1][cfg->dyn_alt_no[cfg->dyn_no-1]-1]-1]
         = character;
      
      in_alternative(cfg);
   }  
   
   return;
}
