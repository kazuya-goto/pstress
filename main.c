#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static char *ProgName;

void usage()
{
  fprintf(stderr,
	  "Usage: %s [OPTIONS] [FILE]\n"
	  "  Computes principal stresses.\n"
	  "  If FILE is specified, input is read from FILE.\n"
	  "  If FILE is not specified, input is read from stdin.\n"
	  "  It is assumed that each line of the input contains 6 stress components\n"
	  "  in the order s11 s22 s33 s12 s23 s13.\n"
	  "  Each line can contain other columns, but the total number of columns\n"
	  "  must be <= 12.\n"
	  "  Computed principal stress components are added to each input line and\n"
	  "  output to stdout."
	  "\n"
	  "Options:\n"
	  " -h num   Specify number of columns before stress components\n"
          , ProgName);
}

extern void
dsyev_(char*, char*, int*, double*, int*, double*, double*, int*, int*);

void calc_eigen_33(double s[6], double ps[3])
{
  char jobz = 'N';
  char uplo = 'L';
  int n = 3;
  double a[9];
  int lda = 3;
  double w[3];
  double work[16];
  int lwork = 16;
  int info;

  a[0] = s[0];
  a[1] = s[3];  a[4] = s[1];
  a[2] = s[5];  a[5] = s[4];  a[8] = s[2];

  dsyev_(&jobz, &uplo, &n, a, &lda, w, work, &lwork, &info);
  if (info < 0) {
    fprintf(stderr, "INTERNAL ERROR: dsyev: %d'th argument is illegal\n", -info);
    abort();
  }
  if (info > 0) {
    fprintf(stderr, "ERROR: dsyev: not converged\n");
    exit(EXIT_FAILURE);
  }

  ps[0] = w[2];
  ps[1] = w[1];
  ps[2] = w[0];
}

#define BUFSIZE 1024

int main(int argc, char **argv)
{
  int head_cols = 0;
  int ch;
  FILE *fp = stdin;
  char buf[BUFSIZE];

  ProgName = argv[0];

  /*
   * get command-line options
   */
  while ((ch = getopt(argc, argv, "h:")) != -1) {
    switch (ch) {
    case 'h':
      head_cols = atoi(optarg);
      break;
    case '?':
    default:
      usage();
      exit(EXIT_SUCCESS);
    }
  }
  argc -= optind;
  argv += optind;

  /*
   * check the options
   */
  if (head_cols < 0 || head_cols > 6) {
    fprintf(stderr, "ERROR: invalid number of head_cols: %d\n", head_cols);
    usage();
    exit(EXIT_FAILURE);
  }

  /*
   * get arguments
   */
  if (argc > 1) {
    fprintf(stderr, "ERROR: too many arguments\n");
    usage();
    exit(EXIT_FAILURE);
  } else if (argc == 1) {
    fp = fopen(argv[0], "r");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: cannot open file %s\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /*
   * process each line
   */
  while (fgets(buf, BUFSIZE, fp)) {
    int ret;
    double x[12];
    double s[6], ps[3];
    int i;

    ret = sscanf(buf, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		 x, x+1, x+2, x+3, x+4, x+5, x+6, x+7, x+8, x+9, x+10, x+11);
    if (ret < head_cols + 6) {
      fprintf(stderr, "ERROR: too few columns\n");
      exit(EXIT_FAILURE);
    }

    for (i = 0; i < 6; i++)
      s[i] = x[head_cols + i];

    calc_eigen_33(s, ps);

    //fprintf(stdout, "%e %e %e\n", ps[0], ps[1], ps[2]);
    char *p;
    if (p = strchr(buf, '\n')) *p = '\0';
    fprintf(stdout, "%s %12.4e %12.4e %12.4e\n", buf, ps[0], ps[1], ps[2]);
  }

  if (fp != stdin) fclose(fp);
  exit(EXIT_SUCCESS);
}
