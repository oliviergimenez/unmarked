#include <R.h>
#include <R_ext/BLAS.h>
#include <Rdefines.h>
#define K 3

void forward(int *M, int *nY, double *psi_all, double *A_all, 
	     double *pT_all, double *nLL) {
    
    double *psi = (double *)R_alloc(K + 1,sizeof(double));
    double *A = (double *)R_alloc(pow(K+1,2), sizeof(double));
    const char *trans = "T";    // transpose A
//    const char *transb = "N";
    const int m = K + 1, incx = 1, incy = 1, matsize = pow(K+1,2);
    const double one = 1.0, zero = 0.0;
    double *lik = (double *)R_alloc(*M, sizeof(double));
    double *pT = (double *)R_alloc(K + 1, sizeof(double));
    double *ones = (double *)R_alloc(*M, sizeof(double));
    double *y = (double *)R_alloc(K + 1, sizeof(double));

    for(int i = 0; i < *M; i++){
	ones[i] = 1;
    }

    for(int t = 0; t < *nY - 1; t++){
	for(int i = 0; i < *M; i++){

	    // extract psit vector for a particular site
	    for(int k = 0; k <= K; k++){
		psi[k] = psi_all[(K+1)*i + k];
	    }

	    // extract the matrix for a particular site/year
	    for(int k = 0; k < pow(K+1,2); k++){
		A[k] = A_all[(t*(*M) + i)*matsize + k];
//		printf("%f ",A[k]);
	    }

	    printf("A^t:\n");
	    for(int i = 0; i <= K; i++){
		for(int j = 0; j <= K; j++){
		    printf("%f ", A[i*(K+1) + j]);
		}
		printf("\n");
	    }

	    printf("\n");
	    for(int i = 0; i < ((*M)*(K+1)) ; i++){
		printf("%f ",psi_all[i]);
	    }
	    printf("\n");

	    printf("before mult: %f %f %f %f\n", psi[0], psi[1], psi[2], psi[3]);


/*	    F77_CALL(dgemm)(transa, transb, &m,
			    &m, &one, &one,
			    A, &m,
			    psi, &m,
			    &zero, */

/* C := alpha*op( A )*op( B ) + beta*C */
/*F77_NAME(dgemm)(const char *transa, const char *transb, const int *m,
		const int *n, const int *k, const double *alpha,
		const double *a, const int *lda,
		const double *b, const int *ldb,
		const double *beta, double *c, const int *ldc);*/

	    F77_CALL(dgemv)(trans, &m, &m, 
			    &one, A, &m, 
			    psi, &incx, &zero, 
			    y, &incy);

/* y := alpha*A*x + beta*y,
F77_NAME(dgemv)(const char *trans, const int *m, const int *n,
		const double *alpha, const double *a, const int *lda,
		const double *x, const int *incx, const double *beta,
		double *y, const int *incy); */
	    
	    printf("after mult: %f %f %f %f\n", y[0], y[1], y[2], y[3]);

	    // put psit back in psi vector
	    for(int k = 0; k <= K; k++){
		psi_all[(K+1)*i + k] = y[k];
	    }

	}
    }

    for(int i = 0; i < *M; i++){
	
	// extract p_y,T
	for(int k = 0; k <= K; k++){
	    pT[k] = pT_all[K*i + k];
	}

	// extract psit vector for a particular site
	for(int k = 0; k <= K; k++){
	    psi[k] = psi_all[K*i + k];
	}

	lik[i] = -log(F77_CALL(ddot)(&m, pT, &incx, psi, &incy));
/* DDOT - inner product of x and y
F77_NAME(ddot)(const int *n, const double *dx, const int *incx,
const double *dy, const int *incy); */

    }

    *nLL = F77_CALL(ddot)(M, lik, &incx, ones, &incy);

}
