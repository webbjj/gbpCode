#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <gbpLib.h>
#include <gbpRNG.h>
#include <gbpMCMC.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_interp.h>

void read_MCMC_state(MCMC_info *MCMC,char *filename_root){
  char      filename_output_dir[MAX_FILENAME_LENGTH];
  char      filename_chain_dir[MAX_FILENAME_LENGTH];
  char      filename_results_dir[MAX_FILENAME_LENGTH];
  char      filename_plots_dir[MAX_FILENAME_LENGTH];
  char      filename_run[MAX_FILENAME_LENGTH];
  char      filename_chain[MAX_FILENAME_LENGTH];
  char      filename_chain_iterations[MAX_FILENAME_LENGTH];
  char      filename_stats[MAX_FILENAME_LENGTH];
  char      filename_coverage[MAX_FILENAME_LENGTH];
  char      filename_chain_covariance[MAX_FILENAME_LENGTH];
  char      filename_covariance[MAX_FILENAME_LENGTH];
  char      filename_histograms[MAX_FILENAME_LENGTH];
  char      filename_results[MAX_FILENAME_LENGTH];
  char      filename_stop[MAX_FILENAME_LENGTH];
  int       my_chain;
  int       i_P,i_DS,i_M,i_array;
  int      *n_M_DS;
  FILE     *fp_run;
  FILE     *fp_chain;
  FILE     *fp_chain_iterations;
  FILE     *fp_stats;
  FILE     *fp_coverage;
  FILE     *fp_chain_covariance;
  FILE     *fp_covariance;
  FILE     *fp_histograms;
  FILE     *fp_results;
  FILE     *fp_stop;
  MCMC_DS_info *current_DS;

  my_chain=0;

  // Perform integration
    SID_log("Reading MCMC state from {%s}...",SID_LOG_OPEN,filename_root);
    strcpy(MCMC->filename_output_dir,filename_root);
    // Set directories
    sprintf(filename_output_dir, "%s/",        MCMC->filename_output_dir);
    sprintf(filename_chain_dir,  "%s/chains/", MCMC->filename_output_dir);
    sprintf(filename_results_dir,"%s/results/",MCMC->filename_output_dir);
    sprintf(filename_plots_dir,  "%s/plots/",  MCMC->filename_output_dir);

    // Set filenames
    sprintf(filename_run,             "%s/run.dat",                  MCMC->filename_output_dir);
    sprintf(filename_chain,           "%s/chain_trace_%06d.dat",     filename_chain_dir,my_chain);
    sprintf(filename_chain_iterations,"%s/chain_iterations_%06d.dat",filename_chain_dir,my_chain);
    sprintf(filename_chain_covariance,"%s/chain_covariance_%06d.dat",filename_chain_dir,my_chain);
    sprintf(filename_stats,           "%s/chain_stats_%06d.dat",     filename_chain_dir,my_chain);
    sprintf(filename_coverage,        "%s/coverage.dat",             filename_results_dir);
    sprintf(filename_histograms,      "%s/histograms.dat",           filename_results_dir);
    sprintf(filename_covariance,      "%s/covariance.dat",           filename_results_dir);

    MCMC->map_P_to_M                =NULL;
    MCMC->compute_MCMC_ln_likelihood=compute_MCMC_ln_likelihood_default;
    MCMC->params                    =NULL;
    MCMC->temperature               =1.0;
    MCMC->n_P                       =0;
    MCMC->n_thin                    =1;
    MCMC->n_DS                      =0;
    MCMC->n_M_total                 =0;
    MCMC->n_arrays                  =0;
    MCMC->array                     =NULL;
    MCMC->V                         =NULL;
    MCMC->m                         =NULL;
    MCMC->flag_integrate_on         =TRUE;
    MCMC->flag_analysis_on          =TRUE;
    MCMC->first_map_call            =TRUE;
    MCMC->mode                      =MCMC_MODE_DEFAULT;
    MCMC->DS                        =NULL;
    MCMC->last                      =NULL;
    MCMC->problem_name              =(char *)SID_malloc(sizeof(char)*MCMC_NAME_SIZE);
    strcpy(MCMC->filename_output_dir,filename_root);
    // Read/Write Header file
    if((fp_run=fopen(filename_run,"rb"))!=NULL){
      fp_run=fopen(filename_run,"rb");
      fread(MCMC->problem_name,       sizeof(char),MCMC_NAME_SIZE,fp_run);
      fread(&(MCMC->n_avg),           sizeof(int),              1,fp_run);
      fread(&(MCMC->n_avg_covariance),sizeof(int),              1,fp_run);
      fread(&(MCMC->flag_autocor_on), sizeof(int),              1,fp_run);
      fread(&(MCMC->n_P),             sizeof(int),              1,fp_run);
      MCMC->P_names    =(char  **)SID_malloc(sizeof(char *)*MCMC->n_P);
      MCMC->P_init     =(double *)SID_malloc(sizeof(double)*MCMC->n_P);
      MCMC->flag_limit =(int    *)SID_malloc(sizeof(int)*MCMC->n_P);
      MCMC->P_limit_min=(double *)SID_malloc(sizeof(double)*MCMC->n_P);
      MCMC->P_limit_max=(double *)SID_malloc(sizeof(double)*MCMC->n_P);
      for(i_P=0;i_P<MCMC->n_P;i_P++){
        MCMC->flag_limit[i_P]=TRUE;
        if(MCMC->flag_limit[i_P]){
          MCMC->P_limit_min[i_P]=DBL_MIN;
          MCMC->P_limit_max[i_P]=DBL_MAX;
        }  
      }
      for(i_P=0;i_P<MCMC->n_P;i_P++){
        MCMC->P_names[i_P]=(char *)SID_malloc(sizeof(char)*MCMC_NAME_SIZE);
        fread(MCMC->P_names[i_P],  sizeof(char),  MCMC_NAME_SIZE,fp_run);
        fread(&(MCMC->P_init[i_P]),sizeof(double),             1,fp_run);
      }
      fread(&(MCMC->n_arrays),sizeof(int),1,fp_run);
      for(i_array=0;i_array<MCMC->n_arrays;i_array++)
        fread(MCMC->array_name[i_array],sizeof(char),MCMC_NAME_SIZE,fp_run);
      fread(&(MCMC->n_DS),sizeof(int),1,fp_run);
      n_M_DS=(int *)SID_malloc(sizeof(int)*MCMC->n_DS);
      fread(n_M_DS,sizeof(int),MCMC->n_DS,fp_run);
      SID_log("Reading datasets...",SID_LOG_OPEN);
      for(i_DS=0;i_DS<MCMC->n_DS;i_DS++){
        SID_log("Dataset #%03d:",SID_LOG_OPEN,i_DS);
        current_DS           =(MCMC_DS_info *)SID_malloc(sizeof(MCMC_DS_info));
        current_DS->n_M      =n_M_DS[i_DS];
        current_DS->M_target =(double *)SID_malloc(sizeof(double)*current_DS->n_M);
        current_DS->dM_target=(double *)SID_malloc(sizeof(double)*current_DS->n_M);
        current_DS->params=NULL;
        fread(current_DS->name,       sizeof(char),  MCMC_NAME_SIZE, fp_run);
        SID_log("name    ={%s}",SID_LOG_COMMENT,current_DS->name);
        SID_log("n_M     =%d",  SID_LOG_COMMENT,current_DS->n_M);
        fread(current_DS->M_target,   sizeof(double),current_DS->n_M,fp_run);
        fread(current_DS->dM_target,  sizeof(double),current_DS->n_M,fp_run);
        fread(&(current_DS->n_arrays),sizeof(int),                 1,fp_run);
        SID_log("n_arrays=%d",  SID_LOG_COMMENT,current_DS->n_arrays);
        current_DS->n_arrays=0;
        for(i_array=0;i_array<current_DS->n_arrays;i_array++){
          fread(current_DS->array_name[i_array],sizeof(char),MCMC_NAME_SIZE,fp_run);
        }
        current_DS->next=NULL;
        if(MCMC->DS==NULL)
          MCMC->DS=current_DS;
        else
          MCMC->last->next=current_DS;
        MCMC->last=current_DS;
        SID_log(NULL,SID_LOG_CLOSE|SID_LOG_NOPRINT);
      }
      SID_log("Done.",SID_LOG_CLOSE);
      fclose(fp_run);
    }

    // ... fetch the number of intervals that have already been computed ...
    fp_chain_iterations=fopen(filename_chain_iterations,"rb");
    fread(&(MCMC->n_iterations),     sizeof(int),1,fp_chain_iterations);
    fread(&(MCMC->n_iterations_burn),sizeof(int),1,fp_chain_iterations);
    SID_log("# burn  iterations = %d",SID_LOG_COMMENT,MCMC->n_iterations_burn);
    SID_log("# total iterations = %d",SID_LOG_COMMENT,MCMC->n_iterations);
    fclose(fp_chain_iterations);
    SID_free(SID_FARG n_M_DS);
    SID_log("Done.",SID_LOG_CLOSE);
}
