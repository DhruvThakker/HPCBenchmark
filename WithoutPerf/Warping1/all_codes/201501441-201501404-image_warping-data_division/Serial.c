#include<time.h>
#include<stdlib.h>
#include<stdio.h>
#include<omp.h>
#include<math.h>
 
/* Some of the available clocks. 
   Use the MONOTONIC clock for your codes 
*/
 
#define CLK CLOCK_MONOTONIC
/* #define CLK CLOCK_REALTIME */
/* #define CLK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLK CLOCK_THREAD_CPUTIME_ID */
/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);
 
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


unsigned char bi(unsigned char r1,unsigned char r2,unsigned char r3,unsigned char r4) 
{
   return (r1+r2+r3+r4)/4;
    
}

PPMImage* twistImage(PPMImage *im)
{
  int rows = im->x;
  int cols = im->y;
  int i,j;
  int tx = rows / 2, ty = cols / 2;
  float PI = 3.141527f;
  float DRAD = PI / 180.0f;
  PPMImage *im2 = (PPMImage *) malloc(sizeof(PPMImage));
  im2->x = rows;
  im2->y = cols;
  im2->data = (PPMPixel*) malloc(rows*cols*sizeof(PPMPixel));
  int r,g,b,idx;

//#pragma omp parallel for shared(im, im2, rows, cols, tx, ty) private(i, j)
  for(i=0;i<rows;i++)
  {
      for(j=0; j<cols; j++)
    {
      int index = j + i * cols;
      float radius = sqrtf((i - tx) * (i - tx) + (j - ty) * (j - ty));
      float theta = (radius / 2) * DRAD ;
      float x = cos(theta) * (i - tx) - sin(theta) * (j - ty) + tx;
      float y = sin(theta) * (i - tx) + cos(theta) * (j - ty) + ty;
      
      if((x < 0 || y < 0 || x >= rows || y >= cols  ))
      {   x = 0;
          y = 0;   
      }
                    
        int x1= ceil(x);
        int x2= floor(x);
        int y1= ceil(y);
        int y2= floor(y);
        int index1 = (int)( x1 * cols + y1); int index2 = (int)( x1 * cols + y2);
        int index3 = (int)( x2 * cols + y1); int index4 = (int)( x2 * cols + y2);
        PPMPixel *t1 = im->data + index1; PPMPixel *t2 = im->data + index2;
        PPMPixel *t3 = im->data + index3; PPMPixel *t4 = im->data + index4;
        PPMPixel *tmp2 = im2->data + index;
        
        tmp2->red = bi(t1->red,t2->red,t3->red,t4->red);
        tmp2->green = bi(t1->green,t2->green,t3->green,t4->green);
        tmp2->blue = bi(t1->blue,t2->blue,t3->blue,t4->blue);
    } 
  }
  return im2;
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

int main(int argc, char* argv[]){
  struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
  /* Should start before anything else */
  clock_gettime(CLK, &start_e2e);
 
  /* Check if enough command-line arguments are taken in. */
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
  strcat(finalname,"tws.ppm");

  int n = atoi(argv[1]);
  
  int p=atoi(argv[2]);  /* Use p=0 for serial!*/
  char *problem_name = "image_warping"; /* Change */
  char *approach_name = "data_division"; /* Change */

   
  /* Here you would have your pre-processing elements. */
  
  char outputFileName[50];

  
  PPMImage *image;

  image = readPPM(filename);

  clock_gettime(CLK, &start_alg);   /* Start the algo timer */

  //omp_set_num_threads(p);
  
  PPMImage * x = twistImage(image);
  
  /* Core algorithm finished */
  
  clock_gettime(CLK, &end_alg); /* End the algo timer */
  
  /* Ensure that only the algorithm is present between these two
     timers. Further, the whole algorithm should be present. */
   
 
  /* Various methods in which you can print times. 
     Don't use these. Only for illustration. */
 
 
  /* Should end before anything else (printing comes later) */
  
  clock_gettime(CLK, &end_e2e);
  e2e = diff(start_e2e, end_e2e);
  alg = diff(start_alg, end_alg);
 
  /* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
     Change problem_name to whatever problem you've been assigned
     Change approach_name to whatever approach has been assigned
     p should be 0 for serial codes!! 
  */ 
  
  printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);
  
  writePPM(finalname,x);
  
  return 0;
}
 
 
 
/* 
Function to computes the difference between two time instances
 
Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/ 
 
Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
*/
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