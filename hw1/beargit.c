#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  int found;
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
	found = 1;
	continue;
    }
    fprintf(fnewindex, "%s\n", line);
  }

  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  found ? printf("Removed commit\n") : printf("Commit not found\n");

  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  return 1;
  int bear_index;
  for(;*msg != '\0';msg++) {
    if (bear_index == strlen(go_bears)) {
	return 1;
    }
    printf("char is %c, bearchar is %c\n", *msg, go_bears[bear_index]);
    if (go_bears[bear_index] != *msg) {
	bear_index = 0;
    } else {
	bear_index++;
    }
  }
  return 0;
}

void next_commit_id(char* commit_id) {
  do {
    if (*commit_id == '0') {
	*commit_id = '6';
	break;
    }
    else if (*commit_id == '6') {
	*commit_id = '1';
	break;
    }
    else if (*commit_id == '1') {
	*commit_id = 'c';
	break;
    }
  } while(*commit_id++ == 'c');
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  char *newdir = malloc(
    snprintf(newdir,0, ".beargit/%s", commit_id)
  );
  sprintf(newdir, ".beargit/%s", commit_id);
  fs_mkdir(newdir);

  char *indexdir = malloc(
    snprintf(indexdir,0, "%s/.index", newdir)
  );
  sprintf(indexdir, "%s/.index", newdir);

  fs_cp(".beargit/.index", indexdir);

  char *prevdir = malloc(
    snprintf(prevdir,0, "%s/.prev", newdir)
  );
  sprintf(prevdir, "%s/.prev", newdir);
  fs_cp(".beargit/.prev", prevdir);

  // copy all files over
  FILE* findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    
    char *path = malloc(
      snprintf(path,0, ".beargit/%s/%s", commit_id ,line)
    );
    sprintf(path, ".beargit/%s/%s", commit_id, line);
    
    fs_cp(line, path);

    free(path);
  }

  fclose(findex);

  // store message
  char *msgpath = malloc(
    snprintf(msgpath,0, ".beargit/%s/.msg", commit_id)
  );
  sprintf(msgpath, ".beargit/%s/.msg", commit_id);
  write_string_to_file(msgpath, msg);
  // store commit id in prev
  write_string_to_file(".beargit/.prev", commit_id);

  free(msgpath);
  free(newdir);
  free(prevdir);
  free(indexdir);

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  FILE* findex = fopen(".beargit/.index", "r");

  int count = 0;

  printf("Tracked files:\n\n");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    printf("%s\n", line);
    count++;
  }

  printf("%d files total\n", count);

  fclose(findex);

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int beargit_log() {

  // determine which path to look in first
  char lc[COMMIT_ID_SIZE];
  char msg[COMMIT_ID_SIZE];

  char initial_path[] = ".beargit/.prev";
  read_string_from_file(initial_path, lc, COMMIT_ID_SIZE);

  // while the LAST we read is not empty
  while (*lc != '0') {

    char *path = malloc(COMMIT_ID_SIZE * 2);

    // print the commit we got in the last loop
    printf("commit %s\n", lc);

    sprintf(path, ".beargit/%s/.msg", lc);

    read_string_from_file(path, msg, COMMIT_ID_SIZE);

    strtok(msg, "\n");
    printf("\t %s\n", msg);

    // get next commit
    sprintf(path, ".beargit/%s/.prev", lc);

    read_string_from_file(path, lc, COMMIT_ID_SIZE);

    free(path);
  }

  return 0;
}
