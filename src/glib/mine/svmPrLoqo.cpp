/*
* File:        pr_loqo.c
* Purpose:     solves quadratic programming problem for pattern recognition
*              for support vectors
*
* Author:      Alex J. Smola
* Created:     10/14/97
* Updated:     11/08/97
* Updated:     13/08/98 (removed exit(1) as it crashes svm lite when the margin
*                        in a not sufficiently conservative manner)
*
* 
* Copyright (c) 1997  GMD Berlin - All rights reserved
* THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of GMD Berlin
* The copyright notice above does not evidence any
* actual or intended publication of this work.
*
* Unauthorized commercial use of this software is not allowed
*/

#define PREDICTOR 1
#define CORRECTOR 2

 
#ifdef GLib_WIN
#pragma optimize( "", off )
#endif
 
/*****************************************************************
replace this by any other function that will exit gracefully
in a larger system
***************************************************************/

void TPrLoqo::nrerror(const char error_text[]) {
    //printf("ERROR: terminating optimizer - %s\n", error_text);
    /* exit(1); */
}

/*****************************************************************
taken from numerical recipes and modified to accept pointers
moreover numerical recipes code seems to be buggy (at least the
ones on the web)

cholesky solver and backsubstitution
leaves upper right triangle intact (rows first order)
***************************************************************/

void TPrLoqo::choldc(double a[], int n, double p[]) {
    //void nrerror(char error_text[]);
    int i, j, k;
    double sum;

    for (i = 0; i < n; i++){
        for (j = i; j < n; j++) {
            sum=a[n*i + j];
            for (k=i-1; k>=0; k--) sum -= a[n*i + k]*a[n*j + k];
            if (i == j) {
                if (sum <= 0.0) {
                    nrerror("choldc failed, matrix not positive definite");
                    sum = 0.0;
                }
                p[i]=sqrt(sum);
            } else a[n*j + i] = sum/p[i];
        }
    }
}

void TPrLoqo::cholsb(double a[], int n, double p[], double b[], double x[]) {
    int i, k;
    double sum;

    for (i=0; i<n; i++) {
        sum=b[i];
        for (k=i-1; k>=0; k--) sum -= a[n*i + k]*x[k];
        x[i]=sum/p[i];
    }

    for (i=n-1; i>=0; i--) {
        sum=x[i];
        for (k=i+1; k<n; k++) sum -= a[n*k + i]*x[k];
        x[i]=sum/p[i];
    }
}

/*****************************************************************
sometimes we only need the forward or backward pass of the
backsubstitution, hence we provide these two routines separately 
***************************************************************/

void TPrLoqo::chol_forward(double a[], int n, double p[], double b[], double x[])
{
    int i, k;
    double sum;

    for (i=0; i<n; i++) {
        sum=b[i];
        for (k=i-1; k>=0; k--) sum -= a[n*i + k]*x[k];
        x[i]=sum/p[i];
    }
}

void TPrLoqo::chol_backward(double a[], int n, double p[], double b[], double x[])
{
    int i, k;
    double sum;

    for (i=n-1; i>=0; i--) {
        sum=b[i];
        for (k=i+1; k<n; k++) sum -= a[n*k + i]*x[k];
        x[i]=sum/p[i];
    }
}

/*****************************************************************
solves the system | -H_x A' | |x_x| = |c_x|
|  A   H_y| |x_y|   |c_y|

with H_x (and H_y) positive (semidefinite) matrices
and n, m the respective sizes of H_x and H_y

for variables see pg. 48 of notebook or do the calculations on a
sheet of paper again

predictor solves the whole thing, corrector assues that H_x didn't
change and relies on the results of the predictor. therefore do
_not_ modify workspace

if you want to speed tune anything in the code here's the right
place to do so: about 95% of the time is being spent in
here. something like an iterative refinement would be nice,
especially when switching from double to single precision. if you
have a fast parallel cholesky use it instead of the numrec
implementations.

side effects: changes H_y (but this is just the unit matrix or zero anyway
in our case)
***************************************************************/

void TPrLoqo::solve_reduced(int n, int m, double h_x[], double h_y[], 
                            double a[], double x_x[], double x_y[],
                            double c_x[], double c_y[],
                            double workspace[], int step)
{
    int i,j,k;

    double *p_x;
    double *p_y;
    double *t_a;
    double *t_c;
    double *t_y;

    p_x = workspace;		/* together n + m + n*m + n + m = n*(m+2)+2*m */
    p_y = p_x + n;
    t_a = p_y + m;
    t_c = t_a + n*m;
    t_y = t_c + n;

    if (step == PREDICTOR) {
        choldc(h_x, n, p_x);	/* do cholesky decomposition */

        for (i=0; i<m; i++)         /* forward pass for A' */
            chol_forward(h_x, n, p_x, a+i*n, t_a+i*n);

        for (i=0; i<m; i++)         /* compute (h_y + a h_x^-1A') */
            for (j=i; j<m; j++)
                for (k=0; k<n; k++) 
                    h_y[m*i + j] += t_a[n*j + k] * t_a[n*i + k];

        choldc(h_y, m, p_y);	/* and cholesky decomposition */
    }

    chol_forward(h_x, n, p_x, c_x, t_c);
    /* forward pass for c */

    for (i=0; i<m; i++) {		/* and solve for x_y */
        t_y[i] = c_y[i];
        for (j=0; j<n; j++)
            t_y[i] += t_a[i*n + j] * t_c[j];
    }

    cholsb(h_y, m, p_y, t_y, x_y);

    for (i=0; i<n; i++) {		/* finally solve for x_x */
        t_c[i] = -t_c[i];
        for (j=0; j<m; j++)
            t_c[i] += t_a[j*n + i] * x_y[j];
    }

    chol_backward(h_x, n, p_x, t_c, x_x);
}

/*****************************************************************
matrix vector multiplication (symmetric matrix but only one triangle
given). computes m*x = y
no need to tune it as it's only of O(n^2) but cholesky is of
O(n^3). so don't waste your time _here_ although it isn't very
elegant. 
***************************************************************/

void TPrLoqo::matrix_vector(int n, double m[], double x[], double y[])
{
    int i, j;

    for (i=0; i<n; i++) {
        y[i] = m[(n+1) * i] * x[i];

        for (j=0; j<i; j++)
            y[i] += m[i + n*j] * x[j];

        for (j=i+1; j<n; j++) 
            y[i] += m[n*i + j] * x[j]; 
    }
}

/*****************************************************************
call only this routine; this is the only one you're interested in
for doing quadratical optimization

the restart feature exists but it may not be of much use due to the
fact that an initial setting, although close but not very close the
the actual solution will result in very good starting diagnostics
(primal and dual feasibility and small infeasibility gap) but incur
later stalling of the optimizer afterwards as we have to enforce
positivity of the slacks.
***************************************************************/

int TPrLoqo::pr_loqo(int n, int m, double c[], double h_x[], double a[], double b[],
                     double l[], double u[], double primal[], double dual[], 
                     int verb, double sigfig_max, int counter_max, 
                     double margin, double bound, int restart)
{
    /* the knobs to be tuned ... */
    /* double margin = -0.95;	   we will go up to 95% of the
    distance between old variables and zero */
    /* double bound = 10;		   preset value for the start. small
    values give good initial
    feasibility but may result in slow
    convergence afterwards: we're too
    close to zero */
    /* to be allocated */
    double *workspace;
    double *diag_h_x;
    double *h_y;
    double *c_x;
    double *c_y;
    double *h_dot_x;
    double *rho;
    double *nu;
    double *tau;
    double *sigma;
    double *gamma_z;
    double *gamma_s;  

    double *hat_nu;
    double *hat_tau;

    double *delta_x;
    double *delta_y;
    double *delta_s;
    double *delta_z;
    double *delta_g;
    double *delta_t;

    double *d;

    /* from the header - pointers into primal and dual */
    double *x;
    double *y;
    double *g;
    double *z;
    double *s;
    double *t;  

    /* auxiliary variables */
    double b_plus_1;
    double c_plus_1;

    double x_h_x;
    double primal_inf;
    double dual_inf;

    double sigfig;
    double primal_obj, dual_obj;
    double mu;
    double alfa;//, step;
    int counter = 0;

    int status = STILL_RUNNING;

    int i,j;//,k;

    /* memory allocation */
    workspace = (double *)malloc((n*(m+2)+2*m)*sizeof(double));
    diag_h_x  = (double *)malloc(n*sizeof(double));
    h_y       = (double *)malloc(m*m*sizeof(double));
    c_x       = (double *)malloc(n*sizeof(double));
    c_y       = (double *)malloc(m*sizeof(double));
    h_dot_x   = (double *)malloc(n*sizeof(double));

    rho       = (double *)malloc(m*sizeof(double));
    nu        = (double *)malloc(n*sizeof(double));
    tau       = (double *)malloc(n*sizeof(double));
    sigma     = (double *)malloc(n*sizeof(double));

    gamma_z   = (double *)malloc(n*sizeof(double));
    gamma_s   = (double *)malloc(n*sizeof(double));

    hat_nu    = (double *)malloc(n*sizeof(double));
    hat_tau   = (double *)malloc(n*sizeof(double));

    delta_x   = (double *)malloc(n*sizeof(double));
    delta_y   = (double *)malloc(m*sizeof(double));
    delta_s   = (double *)malloc(n*sizeof(double));
    delta_z   = (double *)malloc(n*sizeof(double));
    delta_g   = (double *)malloc(n*sizeof(double));
    delta_t   = (double *)malloc(n*sizeof(double));

    d         = (double *)malloc(n*sizeof(double));

    /* pointers into the external variables */
    x = primal;			/* n */
    g = x + n;			/* n */
    t = g + n;			/* n */

    y = dual;			/* m */
    z = y + m;			/* n */
    s = z + n;			/* n */

    /* initial settings */
    b_plus_1 = 1;
    c_plus_1 = 0;
    for (i=0; i<n; i++) c_plus_1 += c[i];

    /* get diagonal terms */
    for (i=0; i<n; i++) diag_h_x[i] = h_x[(n+1)*i]; 

    /* starting point */
    if (restart == 1) {
        /* x, y already preset */
        for (i=0; i<n; i++) {	/* compute g, t for primal feasibility */
            g[i] = mx(fabs(x[i] - l[i]), bound);
            t[i] = mx(fabs(u[i] - x[i]), bound); 
        }

        matrix_vector(n, h_x, x, h_dot_x); /* h_dot_x = h_x * x */

        for (i=0; i<n; i++) {	/* sigma is a dummy variable to calculate z, s */
            sigma[i] = c[i] + h_dot_x[i];
            for (j=0; j<m; j++)
                sigma[i] -= a[n*j + i] * y[j];

            if (sigma[i] > 0) {
                s[i] = bound;
                z[i] = sigma[i] + bound;
            }
            else {
                s[i] = bound - sigma[i];
                z[i] = bound;
            }
        }
    }
    else {			/* use default start settings */
        for (i=0; i<m; i++)
            for (j=i; j<m; j++)
                h_y[i*m + j] = (i==j) ? 1 : 0;

        for (i=0; i<n; i++) {
            c_x[i] = c[i];
            h_x[(n+1)*i] += 1;
        }

        for (i=0; i<m; i++)
            c_y[i] = b[i];

        /* and solve the system [-H_x A'; A H_y] [x, y] = [c_x; c_y] */
        solve_reduced(n, m, h_x, h_y, a, x, y, c_x, c_y, workspace,
            PREDICTOR);

        /* initialize the other variables */
        for (i=0; i<n; i++) {
            g[i] = mx(fabs(x[i] - l[i]), bound);
            z[i] = mx(fabs(x[i]), bound);
            t[i] = mx(fabs(u[i] - x[i]), bound); 
            s[i] = mx(fabs(x[i]), bound); 
        }
    }

    for (i=0, mu=0; i<n; i++)
        mu += z[i] * g[i] + s[i] * t[i];
    mu = mu / (2*n);

    /* the main loop */
    if (verb >= STATUS) {
        printf("counter | pri_inf  | dual_inf  | pri_obj   | dual_obj  | ");
        printf("sigfig | alpha  | nu \n");
        printf("-------------------------------------------------------");
        printf("---------------------------\n");
    }

    while (status == STILL_RUNNING) {
        /* predictor */

        /* put back original diagonal values */
        for (i=0; i<n; i++) 
            h_x[(n+1) * i] = diag_h_x[i];

        matrix_vector(n, h_x, x, h_dot_x); /* compute h_dot_x = h_x * x */

        for (i=0; i<m; i++) {
            rho[i] = b[i];
            for (j=0; j<n; j++)
                rho[i] -= a[n*i + j] * x[j];
        }

        for (i=0; i<n; i++) {
            nu[i] = l[i] - x[i] + g[i];
            tau[i] = u[i] - x[i] - t[i];

            sigma[i] = c[i] - z[i] + s[i] + h_dot_x[i];
            for (j=0; j<m; j++)
                sigma[i] -= a[n*j + i] * y[j];

            gamma_z[i] = - z[i];
            gamma_s[i] = - s[i];
        }

        /* instrumentation */
        x_h_x = 0;
        primal_inf = 0;
        dual_inf = 0;

        for (i=0; i<n; i++) {
            x_h_x += h_dot_x[i] * x[i];
            primal_inf += sqr(tau[i]);
            primal_inf += sqr(nu[i]);
            dual_inf += sqr(sigma[i]);
        }
        for (i=0; i<m; i++) 
            primal_inf += sqr(rho[i]);
        primal_inf = sqrt(primal_inf)/b_plus_1;
        dual_inf = sqrt(dual_inf)/c_plus_1;

        primal_obj = 0.5 * x_h_x;
        dual_obj = -0.5 * x_h_x;
        for (i=0; i<n; i++) {
            primal_obj += c[i] * x[i];
            dual_obj += l[i] * z[i] - u[i] * s[i];
        }
        for (i=0; i<m; i++)
            dual_obj += b[i] * y[i];

        sigfig = log10(fabs(primal_obj) + 1) -
            log10(fabs(primal_obj - dual_obj));
        sigfig = mx(sigfig, 0);

        /* the diagnostics - after we computed our results we will
        analyze them */

        if (counter > counter_max) status = ITERATION_LIMIT;
        if (sigfig  > sigfig_max)  status = OPTIMAL_SOLUTION;
        if (primal_inf > 10e100)   status = PRIMAL_INFEASIBLE;
        if (dual_inf > 10e100)     status = DUAL_INFEASIBLE;
        if ((primal_inf > 10e100) & (dual_inf > 10e100)) status = PRIMAL_AND_DUAL_INFEASIBLE;
        if (fabs(primal_obj) > 10e100) status = PRIMAL_UNBOUNDED;
        if (fabs(dual_obj) > 10e100) status = DUAL_UNBOUNDED;

        /* write some nice routine to enforce the time limit if you
        _really_ want, however it's quite useless as you can compute
        the time from the maximum number of iterations as every
        iteration costs one cholesky decomposition plus a couple of 
        backsubstitutions */

        /* generate report */
        if ((verb >= FLOOD) | ((verb == STATUS) & (status != 0)))
            printf("%7i | %.2e | %.2e | % .2e | % .2e | %6.3f | %.4f | %.2e\n",
            counter, primal_inf, dual_inf, primal_obj, dual_obj,
            sigfig, alfa, mu);

        counter++;

        if (status == 0) {		/* we may keep on going, otherwise
                                it'll cost one loop extra plus a
                                messed up main diagonal of h_x */
            /* intermediate variables (the ones with hat) */
            for (i=0; i<n; i++) {
                hat_nu[i] = nu[i] + g[i] * gamma_z[i] / z[i];
                hat_tau[i] = tau[i] - t[i] * gamma_s[i] / s[i];
                /* diagonal terms */
                d[i] = z[i] / g[i] + s[i] / t[i];
            }

            /* initialization before the cholesky solver */
            for (i=0; i<n; i++) {
                h_x[(n+1)*i] = diag_h_x[i] + d[i];
                c_x[i] = sigma[i] - z[i] * hat_nu[i] / g[i] - 
                    s[i] * hat_tau[i] / t[i];
            }
            for (i=0; i<m; i++) {
                c_y[i] = rho[i];
                for (j=i; j<m; j++) 
                    h_y[m*i + j] = 0;
            }

            /* and do it */
            solve_reduced(n, m, h_x, h_y, a, delta_x, delta_y, c_x, c_y, workspace,
                PREDICTOR);

            for (i=0; i<n; i++) {
                /* backsubstitution */
                delta_s[i] = s[i] * (delta_x[i] - hat_tau[i]) / t[i];
                delta_z[i] = z[i] * (hat_nu[i] - delta_x[i]) / g[i];

                delta_g[i] = g[i] * (gamma_z[i] - delta_z[i]) / z[i];
                delta_t[i] = t[i] * (gamma_s[i] - delta_s[i]) / s[i];

                /* central path (corrector) */
                gamma_z[i] = mu / g[i] - z[i] - delta_z[i] * delta_g[i] / g[i];
                gamma_s[i] = mu / t[i] - s[i] - delta_s[i] * delta_t[i] / t[i];

                /* (some more intermediate variables) the hat variables */
                hat_nu[i] = nu[i] + g[i] * gamma_z[i] / z[i];
                hat_tau[i] = tau[i] - t[i] * gamma_s[i] / s[i];

                /* initialization before the cholesky */
                c_x[i] = sigma[i] - z[i] * hat_nu[i] / g[i] - s[i] * hat_tau[i] / t[i];
            }

            for (i=0; i<m; i++) {	/* comput c_y and rho */
                c_y[i] = rho[i];
                for (j=i; j<m; j++)
                    h_y[m*i + j] = 0;
            }

            /* and do it */
            solve_reduced(n, m, h_x, h_y, a, delta_x, delta_y, c_x, c_y, workspace,
                CORRECTOR);

            for (i=0; i<n; i++) {
                /* backsubstitution */
                delta_s[i] = s[i] * (delta_x[i] - hat_tau[i]) / t[i];
                delta_z[i] = z[i] * (hat_nu[i] - delta_x[i]) / g[i];

                delta_g[i] = g[i] * (gamma_z[i] - delta_z[i]) / z[i];
                delta_t[i] = t[i] * (gamma_s[i] - delta_s[i]) / s[i];
            }

            alfa = -1;
            for (i=0; i<n; i++) {
                alfa = mn(alfa, delta_g[i]/g[i]);
                alfa = mn(alfa, delta_t[i]/t[i]);
                alfa = mn(alfa, delta_s[i]/s[i]);
                alfa = mn(alfa, delta_z[i]/z[i]);
            }
            alfa = (margin - 1) / alfa;

            /* compute mu */
            for (i=0, mu=0; i<n; i++)
                mu += z[i] * g[i] + s[i] * t[i];
            mu = mu / (2*n);
            mu = mu * sqr((alfa - 1) / (alfa + 10));

            for (i=0; i<n; i++) {
                x[i] += alfa * delta_x[i];
                g[i] += alfa * delta_g[i];
                t[i] += alfa * delta_t[i];
                z[i] += alfa * delta_z[i];
                s[i] += alfa * delta_s[i];
            }

            for (i=0; i<m; i++) 
                y[i] += alfa * delta_y[i];
        }
    }
    if ((status == 1) && (verb >= STATUS)) {
        printf("----------------------------------------------------------------------------------\n");
        printf("optimization converged\n");
    }

    /* free memory */
    free(workspace);
    free(diag_h_x);
    free(h_y);
    free(c_x);
    free(c_y);
    free(h_dot_x);

    free(rho);
    free(nu);
    free(tau);
    free(sigma);
    free(gamma_z);
    free(gamma_s);

    free(hat_nu);
    free(hat_tau);

    free(delta_x);
    free(delta_y);
    free(delta_s);
    free(delta_z);
    free(delta_g);
    free(delta_t);

    free(d);

    /* and return to sender */
    return status;
}

#ifdef GLib_WIN
#pragma optimize( "", on )
#endif