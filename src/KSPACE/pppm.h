/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under 
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef KSPACE_CLASS

KSpaceStyle(pppm,PPPM)

#else

#ifndef LMP_PPPM_H
#define LMP_PPPM_H

#include "lmptype.h"
#include "mpi.h"

#ifdef FFT_SINGLE
typedef float FFT_SCALAR;
#define MPI_FFT_SCALAR MPI_FLOAT
#else
typedef double FFT_SCALAR;
#define MPI_FFT_SCALAR MPI_DOUBLE
#endif

#include "kspace.h"

namespace LAMMPS_NS {

class PPPM : public KSpace {
 public:
  PPPM(class LAMMPS *, int, char **);
  virtual ~PPPM();
  virtual void init();
  virtual void setup();
  virtual void compute(int, int);
  virtual void timing(int, double &, double &);
  virtual double memory_usage();

 protected:
  int me,nprocs;
  double precision;
  int nfactors;
  int *factors;
  double qsum,qsqsum;
  double cutoff;
  double volume;
  double delxinv,delyinv,delzinv,delvolinv;
  double shift,shiftone;

  int nxlo_in,nylo_in,nzlo_in,nxhi_in,nyhi_in,nzhi_in;
  int nxlo_out,nylo_out,nzlo_out,nxhi_out,nyhi_out,nzhi_out;
  int nxlo_ghost,nxhi_ghost,nylo_ghost,nyhi_ghost,nzlo_ghost,nzhi_ghost;
  int nxlo_fft,nylo_fft,nzlo_fft,nxhi_fft,nyhi_fft,nzhi_fft;
  int nlower,nupper;
  int ngrid,nfft,nbuf,nfft_both;

  FFT_SCALAR ***density_brick;
  FFT_SCALAR ***vdx_brick,***vdy_brick,***vdz_brick;
  double *greensfn;
  double **vg;
  double *fkx,*fky,*fkz;
  FFT_SCALAR *density_fft;
  FFT_SCALAR *work1,*work2;
  FFT_SCALAR *buf1,*buf2;

  double *gf_b;
  FFT_SCALAR **rho1d,**rho_coeff;

  class FFT3d *fft1,*fft2;
  class Remap *remap;

  int **part2grid;             // storage for particle -> grid mapping
  int nmax;

  int triclinic;               // domain settings, orthog or triclinic
  double *boxlo;
                               // TIP4P settings
  int typeH,typeO;             // atom types of TIP4P water H and O atoms
  double qdist;                // distance from O site to negative charge
  double alpha;                // geometric factor

  void set_grid();
  virtual void allocate();
  virtual void deallocate();
  int factorable(int);
  double rms(double, double, bigint, double, double **);
  double diffpr(double, double, double, double, double **);
  void compute_gf_denom();
  double gf_denom(double, double, double);
  virtual void particle_map();
  virtual void make_rho();
  virtual void brick2fft();
  virtual void fillbrick();
  virtual void poisson(int, int);
  virtual void fieldforce();
  void procs2grid2d(int,int,int,int *, int*);
  void compute_rho1d(const FFT_SCALAR &, const FFT_SCALAR &, 
		     const FFT_SCALAR &);
  void compute_rho_coeff();
  void slabcorr(int);
};

}

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

E: Cannot (yet) use PPPM with triclinic box

This feature is not yet supported.

E: Cannot use PPPM with 2d simulation

The kspace style pppm cannot be used in 2d simulations.  You can use
2d PPPM in a 3d simulation; see the kspace_modify command.

E: Kspace style requires atom attribute q

The atom style defined does not have these attributes.

E: Cannot use nonperiodic boundaries with PPPM

For kspace style pppm, all 3 dimensions must have periodic boundaries
unless you use the kspace_modify command to define a 2d slab with a
non-periodic z dimension.

E: Incorrect boundaries with slab PPPM

Must have periodic x,y dimensions and non-periodic z dimension to use
2d slab option with PPPM.

E: PPPM order cannot be greater than %d

Self-explanatory.

E: KSpace style is incompatible with Pair style

Setting a kspace style requires that a pair style with a long-range
Coulombic component be selected.

E: Bond and angle potentials must be defined for TIP4P

Cannot use TIP4P pair potential unless bond and angle potentials
are defined.

E: Bad TIP4P angle type for PPPM/TIP4P

Specified angle type is not valid.

E: Bad TIP4P bond type for PPPM/TIP4P

Specified bond type is not valid.

E: Cannot use kspace solver on system with no charge

No atoms in system have a non-zero charge.

W: System is not charge neutral, net charge = %g

The total charge on all atoms on the system is not 0.0, which
is not valid for Ewald or PPPM.

W: Reducing PPPM order b/c stencil extends beyond neighbor processor

LAMMPS is attempting this in order to allow the simulation
to run.  It should not effect the PPPM accuracy.

E: PPPM grid is too large

The global PPPM grid is larger than OFFSET in one or more dimensions.
OFFSET is currently set to 4096.  You likely need to decrease the
requested precision.

E: PPPM order has been reduced to 0

LAMMPS has attempted to reduce the PPPM order to enable the simulation
to run, but can reduce the order no further.  Try increasing the
accuracy of PPPM by reducing the tolerance size, thus inducing a 
larger PPPM grid.

E: Cannot compute PPPM G

LAMMPS failed to compute a valid approximation for the PPPM g_ewald
factor that partitions the computation between real space and k-space.

E: Out of range atoms - cannot compute PPPM

One or more atoms are attempting to map their charge to a PPPM grid
point that is not owned by a processor.  This is likely for one of two
reasons, both of them bad.  First, it may mean that an atom near the
boundary of a processor's sub-domain has moved more than 1/2 the
"neighbor skin distance"_neighbor.html without neighbor lists being
rebuilt and atoms being migrated to new processors.  This also means
you may be missing pairwise interactions that need to be computed.
The solution is to change the re-neighboring criteria via the
"neigh_modify"_neigh_modify command.  The safest settings are "delay 0
every 1 check yes".  Second, it may mean that an atom has moved far
outside a processor's sub-domain or even the entire simulation box.
This indicates bad physics, e.g. due to highly overlapping atoms, too
large a timestep, etc.

*/
