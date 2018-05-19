#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int64_t getnumlines(const char *fname,const char comment)
{
    FILE *fp= NULL;
    const int MAXLINESIZE = 10000;
    int64_t nlines=0;
    char str_line[MAXLINESIZE];

    fp = fopen(fname,"rt");
    if(fp == NULL) {
        fprintf(stderr,"Error: Could not open file `%s'\n", fname);
        perror(NULL);
        return -1;
    }

    while(1){
        if(fgets(str_line, MAXLINESIZE,fp)!=NULL) {
            //WARNING: this does not remove white-space. You might
            //want to implement that (was never an issue for me)
            if(str_line[0] !=comment)
                nlines++;
        } else
            break;
    }
    fclose(fp);
    return nlines;
}


int64_t read_ascii_file(const char *filename, double **xpos, double **ypos, double **zpos)
{
    int64_t numlines = getnumlines(filename, '#');
    if(numlines <= 0) return numlines;

    double *x = calloc(numlines, sizeof(*x));
    double *y = calloc(numlines, sizeof(*y));
    double *z = calloc(numlines, sizeof(*z));
    FILE *fp = fopen(filename, "rt");
    if(fp == NULL) {
        fprintf(stderr,"Error:Could not open file `%s' in function %s\n", filename, __FUNCTION__);
        fprintf(stderr,"This is strange because the function `getnumlines' successfully counted the number of lines in that file\n");
        fprintf(stderr,"Did that file (`%s') just get deleted?\n", filename);
        perror(NULL);
        return -1;
    }

    int64_t index=0;
    const int nitems = 3;
    const int MAXLINESIZE = 10000;
    char buf[MAXLINESIZE];
    while(1) {
        if(fgets(buf,MAXLINESIZE,fp)!=NULL) {
            int nread=sscanf(buf,"%lf %lf %lf",&x[index], &y[index], &z[index]);
            if(nread==nitems) {
                index++;
            }
        } else {
            break;
        }
    }
    fclose(fp);
    if(index != numlines) {
        fprintf(stderr,"Error: There are supposed to be `%'"PRId64" lines of data in the file\n", numlines);
        fprintf(stderr,"But could only parse `%'"PRId64" lines containing (x y z) data\n", index);
        fprintf(stderr,"exiting...\n");
        return -1;
    }
    
    *xpos = x;
    *ypos = y;
    *zpos = z;
    return numlines;
}

int setup_bins_double(const char *fname,double *rmin,double *rmax,int *nbin,double **rupp)
{
    //set up the bins according to the binned data file
    //the form of the data file should be <rlow  rhigh ....>
    const int MAXBUFSIZE=1000;
    char buf[MAXBUFSIZE];
    FILE *fp=NULL;
    double low,hi;
    const char comment='#';
    const int nitems=2;
    int nread=0;
    *nbin = ((int) getnumlines(fname,comment))+1;
    *rupp = calloc(*nbin+1, sizeof(double));

    fp = fopen(fname,"rt");
    if(fp == NULL) {
        free(*rupp);
        fprintf(stderr,"Error: Could not open file `%s'..exiting\n",fname);
        perror(NULL);
        return EXIT_FAILURE;
    }
    int index=1;
    while(1) {
        if(fgets(buf,MAXBUFSIZE,fp)!=NULL) {
            nread=sscanf(buf,"%lf %lf",&low,&hi);
            if(nread==nitems) {

                if(index==1) {
                    *rmin=low;
                    (*rupp)[0]=low;
                }

                (*rupp)[index] = hi;
                index++;
            }
        } else {
            break;
        }
    }
    *rmax = (*rupp)[index-1];
    fclose(fp);

    (*rupp)[*nbin]=*rmax ;
    (*rupp)[*nbin-1]=*rmax ;

    return EXIT_SUCCESS;
}


int main(int argc, char **argv)
{
    if(argc < 3) {
        fprintf(stderr,"Usage: ./%s `filename' `filename-with-bins'\n", argv[0]);
        fprintf(stderr,"filename: an ascii file containing particle data (white-space-separated, 3 columns of x y z)\n");
        fprintf(stderr,"filename-with-bins: an ascii file containing <rlow rmax> specifying logarithmic bins (number of lines equal the number of bins)\n");
        return EXIT_FAILURE;
    }
    double *xpos, *ypos, *zpos;
    int64_t Npart = read_ascii_file(argv[1], &xpos, &ypos, &zpos);
    if(Npart <= 0) {
        return Npart;
    }
    double rmin, rmax;
    double *rupp;
    int nbins;
    
    int status = setup_bins_double(argv[2], &rmin, &rmax, &nbins, &rupp);
    if(status < 0) {
        return status;
    }
    double *rpavg = calloc(nbins, sizeof(*rpavg));
    int64_t *npairs = calloc(nbins, sizeof(*npairs));
    const double sqr_rmin = rmin*rmin;
    const double sqr_rmax = rmax*rmax;
    for(int64_t i=0;i<Npart;i++) {
        for(int64_t j=0;j<Npart;j++) {
            const double dx = xpos[i] - xpos[j];
            const double dy = ypos[i] - ypos[j];
            const double dz = zpos[i] - zpos[j];
            
            const double r2 = dx*dx + dy*dy + dz*dz;
            if(r2 < sqr_rmin || r2 >= sqr_rmax) continue;
            const double r = sqrt(r2);
            for(int kbin=nbins-1;kbin>=1;kbin--) {
                if(r >= rupp[kbin-1])  {
                    npairs[kbin]++;
                    rpavg[kbin] += r;
                    break;
                }
            }
        }
    }

    double rlow = rupp[0];
    for(int i=1;i<nbins;i++) {
        if(npairs[i] > 0) rpavg[i] /= npairs[i];
        fprintf(stdout,"%e\t%e\t%e\t%12"PRIu64"\t%e\n",rlow, rupp[i], rpavg[i],npairs[i],0.0);
        rlow=rupp[i];
    }
    

    free(xpos);free(ypos);free(zpos);
    free(rupp);free(npairs);free(rpavg);
    return EXIT_SUCCESS;
}
