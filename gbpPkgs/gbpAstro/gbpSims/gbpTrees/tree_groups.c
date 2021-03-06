#define _MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gbpLib.h>
#include <gbpMath.h>
#include <gbpHalos.h>
#include <gbpTrees.h>

int main(int argc, char *argv[]){
  char        filename_tree_in[256];
  int         n_trees;
  int         n_halos_total;
  int        *n_halos;
  int         i_tree;
  FILE       *fp;
  halo_properties_SAGE_info  *halos;
  halo_properties_SAGE_info   halo;
  int        *snap_num;
  size_t     *snap_num_index;
  char       *flag_done;
  int         i_snap,i_halo,j_halo,k_halo;
  int         n_halos_snap;
  int        *group_halo_first;
  int         group_halo_last;
  size_t     *group_halo_first_index;
  int        *snap_index;
  int         group_size;
  int descendant_min,descendant_max;
  int progenitor_first_min,progenitor_first_max;
  int progenitor_next_min,progenitor_next_max;
  int group_halo_first_min,group_halo_first_max;
  int group_halo_next_min,group_halo_next_max;
  int snap_num_min,snap_num_max;
  int halo_index_min,halo_index_max;
  int n_gal=0;
  int max_snap=0;
  int n_halos_max;
  int n_subtrees;
  double statistic;
  int    tree_select;
  int    current;

  SID_init(&argc,&argv,NULL,NULL);

  // Fetch user inputs
  strcpy(filename_tree_in,argv[1]);
  tree_select=atoi(argv[2]);

  SID_log("Displaying group ordering for tree #%d in {%s}...",SID_LOG_OPEN|SID_LOG_TIMER,tree_select,filename_tree_in);
  fp=fopen(filename_tree_in,"r");
  fread(&n_trees,      sizeof(int),1,fp);
  fread(&n_halos_total,sizeof(int),1,fp);
  SID_log("(%d trees and %d halos)...size_of(halo_properties_SAGE_info)=%uld...",SID_LOG_CONTINUE,n_trees,n_halos_total,sizeof(halo_properties_SAGE_info));
  n_halos=(int *)SID_malloc(sizeof(int)*n_trees);
  fread(n_halos,sizeof(int),n_trees,fp);
  calc_max(n_halos,&n_halos_max,n_trees,SID_INT,CALC_MODE_DEFAULT);
  SID_log("max_size=%d...",SID_LOG_CONTINUE,n_halos_max);
  halos               =(halo_properties_SAGE_info *)SID_malloc(sizeof(halo_properties_SAGE_info)*n_halos_max);
  snap_num            =(int       *)SID_malloc(sizeof(int)*n_halos_max);
  snap_index          =(int       *)SID_malloc(sizeof(int)*n_halos_max);
  group_halo_first    =(int       *)SID_malloc(sizeof(int)*n_halos_max);
  flag_done           =(char      *)SID_malloc(sizeof(char)*n_halos_max);
  for(i_tree=0;i_tree<MIN(n_trees,tree_select);i_tree++){
    fread(halos,sizeof(halo_properties_SAGE_info),n_halos[i_tree],fp);
    //if(i_tree==tree_select){
      for(i_halo=0;i_halo<n_halos[i_tree];i_halo++) flag_done[i_halo]=FALSE;
      for(i_halo=0;i_halo<n_halos[i_tree];i_halo++){
        if(!flag_done[i_halo]){
          current=halos[i_halo].group_halo_first;
          group_size=0;
          while(current>=0){
            group_size++;
            current=halos[current].group_halo_next;
          }
          current=halos[i_halo].group_halo_first;
          while(current>=0){
            if(group_size>1)
              printf("%d ",halos[current].n_particles);
            flag_done[current]=TRUE;
            current=halos[current].group_halo_next;
          }
          if(group_size>1)
            printf("\n");
        }
      }
    //}
  }

  // Clean-up
  fclose(fp);
  SID_free((void **)&snap_num);
  SID_free((void **)&snap_index);
  SID_free((void **)&group_halo_first);
  SID_free((void **)&n_halos);
  SID_free((void **)&halos);
  SID_free((void **)&flag_done);
  SID_log("Done.",SID_LOG_CLOSE);

  SID_exit(0);
}
