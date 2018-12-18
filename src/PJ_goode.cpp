#define PJ_LIB__

#include <errno.h>
#include <math.h>

#include "proj.h"
#include "projects.h"

PROJ_HEAD(goode, "Goode Homolosine") "\n\tPCyl, Sph";

#define Y_COR   0.05280
#define PHI_LIM 0.71093078197902358062

C_NAMESPACE PJ *pj_sinu(PJ *), *pj_moll(PJ *);

namespace { // anonymous namespace
struct pj_opaque {
    PJ *sinu;
    PJ *moll;
};
} // anonymous namespace


static XY s_forward (LP lp, PJ *P) {           /* Spheroidal, forward */
    XY xy;
    struct pj_opaque *Q = static_cast<struct pj_opaque*>(P->opaque);

    if (fabs(lp.phi) <= PHI_LIM)
        xy = Q->sinu->fwd(lp, Q->sinu);
    else {
        xy = Q->moll->fwd(lp, Q->moll);
        xy.y -= lp.phi >= 0.0 ? Y_COR : -Y_COR;
    }
    return xy;
}


static LP s_inverse (XY xy, PJ *P) {           /* Spheroidal, inverse */
    LP lp;
    struct pj_opaque *Q = static_cast<struct pj_opaque*>(P->opaque);

    if (fabs(xy.y) <= PHI_LIM)
        lp = Q->sinu->inv(xy, Q->sinu);
    else {
        xy.y += xy.y >= 0.0 ? Y_COR : -Y_COR;
        lp = Q->moll->inv(xy, Q->moll);
    }
    return lp;
}


static PJ *destructor (PJ *P, int errlev) {              /* Destructor */
    if (nullptr==P)
        return nullptr;
    if (nullptr==P->opaque)
        return pj_default_destructor (P, errlev);
    pj_free (static_cast<struct pj_opaque*>(P->opaque)->sinu);
    pj_free (static_cast<struct pj_opaque*>(P->opaque)->moll);
    return pj_default_destructor (P, errlev);
}



PJ *PROJECTION(goode) {
    struct pj_opaque *Q = static_cast<struct pj_opaque*>(pj_calloc (1, sizeof (struct pj_opaque)));
    if (nullptr==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;
    P->destructor = destructor;

    P->es = 0.;
    if (!(Q->sinu = pj_sinu(nullptr)) || !(Q->moll = pj_moll(nullptr)))
        return destructor (P, ENOMEM);
    Q->sinu->es = 0.;
    Q->sinu->ctx = P->ctx;
    Q->moll->ctx = P->ctx;
    if (!(Q->sinu = pj_sinu(Q->sinu)) || !(Q->moll = pj_moll(Q->moll)))
        return destructor (P, ENOMEM);

    P->fwd = s_forward;
    P->inv = s_inverse;

    return P;
}
