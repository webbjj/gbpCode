#include <stdio.h>
#include <gbpCommon.h>
#include <gbpSID.h>
int count_lines_data(FILE *fp){
  int     n_lines=0;
  char   *line   =NULL;
  size_t  n      =0;
  int     r;
  while(!feof(fp)){
    r=getline(&line,&n,fp);
    if(r>0 && !check_comment(line)) 
      n_lines++;
  }
  SID_free(SID_FARG line);
  rewind(fp);
  return(n_lines);
}
