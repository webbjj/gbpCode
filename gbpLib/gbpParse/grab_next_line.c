#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gbpCommon.h>
#include <gbpSID.h>
#include <gbpParse.h>
int grab_next_line(FILE *fp,char **line, size_t *n){
  return(getline(line,n,fp));
}
