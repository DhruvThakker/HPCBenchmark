#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<omp.h>
#include<math.h>


#define CLK CLOCK_MONOTONIC
/* #define CLK CLOCK_REALTIME */
/* #define CLK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLK CLOCK_THREAD_CPUTIME_ID */
/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

#define max(a,b)            (((a) > (b)) ? (a) : (b))

#define min(a,b)            (((a) < (b)) ? (a) : (b))
typedef struct {
  unsigned char red,green,blue;
} PPMPixel;

typedef struct {
  int x, y;
  PPMPixel *data;
} PPMImage;

typedef struct {
  unsigned char gs;
} PPMPixelGS;


typedef struct {
  int x, y;
  PPMPixelGS *data;
} PPMImageGS;



#define RGB_COMPONENT_COLOR 255


void writePPMGS(const char *filename, PPMImageGS *img)
{
  FILE *fp;
  //open file for output
  fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  //write the header file
  //image format
  fprintf(fp, "P5\n");

    

  //image size
  fprintf(fp, "%d %d\n",img->x,img->y);

  // rgb component depth
  fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

  // pixel data
  fwrite(img->data, img->x, img->y, fp);
  fclose(fp);
}





static PPMImage *readPPM(const char *filename)
{
  char buff[16];
  PPMImage *img;
  FILE *fp;
  int c, rgb_comp_color;
  //open PPM file for reading
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  //read image format
  if (!fgets(buff, sizeof(buff), fp)) {
    perror(filename);
    exit(1);
  }

  //check the image format
  if (buff[0] != 'P' || buff[1] != '6') {
    fprintf(stderr, "Invalid image format (must be 'P6')\n");
    exit(1);
  }

  //alloc memory form image
  img = (PPMImage *)malloc(sizeof(PPMImage));
  if (!img) {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(1);
  }

  //check for comments
  c = getc(fp);
  while (c == '#') {
    while (getc(fp) != '\n') ;
    c = getc(fp);
  }

  ungetc(c, fp);
  //read image size information
  if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
    fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
    exit(1);
  }

  //read rgb component
  if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
    fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
    exit(1);
  }

  //check rgb component depth
  if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
    fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
    exit(1);
  }

  while (fgetc(fp) != '\n') ;
  //memory allocation for pixel data
  img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

  if (!img) {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(1);
  }

  //read pixel data from file
  if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
    fprintf(stderr, "Error loading image '%s'\n", filename);
    exit(1);
  }

  fclose(fp);
  return img;
}

void writePPM(const char *filename, PPMImage *img)
{
  FILE *fp;
  //open file for output
  fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  //write the header file
  //image format
  fprintf(fp, "P6\n");

  //comments


  //image size
  fprintf(fp, "%d %d\n",img->x,img->y);

  // rgb component depth
  fprintf(fp, "%d\n",255);

  // pixel data
  fwrite(img->data, 3 * img->x, img->y, fp);
  fclose(fp);
}






void quicksort ( unsigned char x[], int first,int last) 
{
   int pivot,j,temp,i;

     if(first<last){
         pivot=first;
         i=first;
         j=last;

         while(i<j){
             while(x[i]<=x[pivot]&&i<last)
                 i++;
             while(x[j]>x[pivot])
                 j--;
             if(i<j){
                 temp=x[i];
                  x[i]=x[j];
                  x[j]=temp;
             }
         }

         temp=x[pivot];
         x[pivot]=x[j];
         x[j]=temp;
         quicksort(x,first,j-1);
         quicksort(x,j+1,last);

    }

}

unsigned char median(unsigned char array[], int first,int last  )
    {
    int wsize=last;
    
     quicksort(array, first, last);
    
    return array[wsize/2]; // as the array contains odd number of elements
    }

PPMImage * medianImage(PPMImage * inputImage,int width,int height,int n) //here n depicts the halfwidth
{
  
    
    PPMImage *outputImage = (PPMImage *) malloc(sizeof(PPMImage));
    outputImage->x=height;
    outputImage->y=width; 
    outputImage->data =(PPMPixel *) malloc(height*width*sizeof(PPMPixel));
    int i,j,left,right,bottom,top,u,v,count;
    int size=2*n+1; //here n=halfwidth
    unsigned char r_val[size*size];
    unsigned char g_val[size*size];
    unsigned char b_val[size*size];
    
    
    #pragma omp parallel for\
     shared(inputImage,outputImage,width,height)\
     private(i,j,v,u,top,bottom,left,right,r_val,g_val,b_val,count) firstprivate(n) schedule(static)
    for(i=0;i<height;i++)
    {
        
       for(j=0;j<width;j++) 
         {
      top = (i-n)>0 ? (i-n) : 0;
                bottom = (i+n)<(height-1)  ? (i+n) : (height-1);
            
            left=(j-n)>0 ? (j-n) : 0;
               right=(j+n)<(width-1) ? (j+n) : (width-1);

            
                 count=0;           
            
            for(v=top;v<=bottom;v++)
            {
                             
                for(u=left;u<=right;u++)
                    {
                    r_val[count]=(inputImage->data)[v*width+u].red;
                    g_val[count]=(inputImage->data)[v*width+u].green;
                    b_val[count]=(inputImage->data)[v*width+u].blue;
                    
                    
                    count++;
                }
            }
         
            //find median
           (outputImage->data)[i*width+j].red=median(r_val,0, count-1);
           (outputImage->data)[i*width+j].green=median(g_val,0, count-1);
           (outputImage->data)[i*width+j].blue=median(b_val,0, count-1);
     
         }
     
    }
return outputImage;
}

int main(int argc, char* argv[]){

   struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
  /* Should start before anything else */
  clock_gettime(CLK, &start_e2e);

   if(argc < 3){
    /* Compare to 4 in parallel code if file input is taken. */
    printf( "Usage: %s n p\np=0 for serial code.", argv[0] );
    return -1;
  }
  
  char filename[100];
  
  strcpy(filename,"../../testFiles/");
  strcat(filename,argv[1]);
  strcat(filename,".ppm");

  char finalname[50];
  
  strcpy(finalname,argv[1]);
  strcat(finalname,"mf3p.ppm");


  int n = atoi(argv[1]);
  
  //n = n*n;
  int p=atoi(argv[2]);  /* Use p=0 for serial!*/
  char *problem_name = "median_filtering"; /* Change */
  char *approach_name = "qsort"; /* Change */


  PPMImage *image;
  clock_t start, end;

  image = readPPM(filename);
  int side = 3;
	int rows=image->x;
  int cols=image->y;


  /* Core algorithm follows */
  clock_gettime(CLK, &start_alg);   /* Start the algo timer */

  omp_set_num_threads(p);
  
  PPMImage * im2 = medianImage(image,cols,rows,side);
  
  

  clock_gettime(CLK, &end_alg); /* End the algo timer */

  //t2 = omp_get_wtime()-t1;
  //printf("Time: %f\n", t2);
  clock_gettime(CLK, &end_e2e);
  e2e = diff(start_e2e, end_e2e);
  alg = diff(start_alg, end_alg);
    
  printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);
  writePPM(finalname,im2);

return 0;
}


struct timespec diff(struct timespec start, struct timespec end){
  struct timespec temp;
  if((end.tv_nsec-start.tv_nsec)<0){
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  }
  else{
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}