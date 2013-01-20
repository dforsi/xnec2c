/*
 *  xnec2c - GTK2-based version of nec2c, the C translation of NEC2
 *  Copyright (C) 2003-2010 N. Kyriazis neoklis.kyriazis(at)gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/******* Translated to the C language by N. Kyriazis  20 Aug 2003 ******

  Program NEC(input,tape5=input,output,tape11,tape12,tape13,tape14,
  tape15,tape16,tape20,tape21)

  Numerical Electromagnetics Code (NEC2)  developed at Lawrence
  Livermore lab., Livermore, CA.  (contact G. Burke at 415-422-8414
  for problems with the NEC code. For problems with the vax implem-
  entation, contact J. Breakall at 415-422-8196 or E. Domning at 415
  422-5936)
  file created 4/11/80.

 ***********Notice**********
 This computer code material was prepared as an account of work
 sponsored by the United States government.  Neither the United
 States nor the United States Department Of Energy, nor any of
 their employees, nor any of their contractors, subcontractors,
 or their employees, makes any warranty, express or implied, or
 assumes any legal liability or responsibility for the accuracy,
 completeness or usefulness of any information, apparatus, product
 or process disclosed, or represents that its use would not infringe
 privately-owned rights.

 **********************************************************************/

#include "fields.h"

/* common  /crnt/ */
extern crnt_t crnt;

/* common  /data/ */
extern data_t data;

/* common  /dataj/ */
dataj_t dataj;

/* common  /fpat/ */
extern fpat_t fpat;

/* common  /gnd/ */
extern gnd_t gnd;

/* common  /gwav/ */
extern gwav_t gwav;

/* common  /incom/ */
incom_t incom;

/* common  /tmi/ */
static tmi_t tmi;

/*common  /tmh/ */
static tmh_t tmh;

/* pointers to input/output files */
extern FILE *input_fp;

/* Near E/H field data */
extern near_field_t near_field;

/*-------------------------------------------------------------------*/

/* compute near e fields of a segment with sine, cosine, and */
/* constant currents.  ground effect included. */
void efld( long double xi, long double yi,
	long double zi, long double ai, int ij )
{
  #define	txk	egnd[0]
  #define	tyk	egnd[1]
  #define	tzk	egnd[2]
  #define	txs	egnd[3]
  #define	tys	egnd[4]
  #define	tzs	egnd[5]
  #define	txc	egnd[6]
  #define	tyc	egnd[7]
  #define	tzc	egnd[8]

  int ip;
  long double xij, yij, ijx, rfl, salpr, zij, zp, rhox;
  long double rhoy, rhoz, rh, r, rmag, cth, px, py;
  long double xymag, xspec, yspec, rhospc, dmin;
  complex long double epx, epy, refs, refps, zrsin, zratx, zscrn;
  complex long double tezs, ters, tezc, terc, tezk, terk;
  static complex long double *egnd = NULL;

  size_t mreq = 9 * sizeof(complex long double);
  mem_alloc( (void *)&egnd, mreq, "in fields.c");

  xij= xi- dataj.xj;
  yij= yi- dataj.yj;
  ijx= ij;
  rfl=-1.0l;

  for( ip = 0; ip < gnd.ksymp; ip++ )
  {
	if( ip == 1)
	  ijx=1;
	rfl= -rfl;
	salpr= dataj.salpj* rfl;
	zij= zi- rfl* dataj.zj;
	zp= xij* dataj.cabj+ yij* dataj.sabj+ zij* salpr;
	rhox= xij- dataj.cabj* zp;
	rhoy= yij- dataj.sabj* zp;
	rhoz= zij- salpr* zp;

	rh= sqrtl( rhox* rhox+ rhoy* rhoy+ rhoz* rhoz+ ai* ai);
	if( rh <= 1.0e-10l)
	{
	  rhox=0.0l;
	  rhoy=0.0l;
	  rhoz=0.0l;
	}
	else
	{
	  rhox= rhox/ rh;
	  rhoy= rhoy/ rh;
	  rhoz= rhoz/ rh;
	}

	/* lumped current element approx. for large separations */
	r= sqrtl( zp* zp+ rh* rh);
	if( r >= dataj.rkh)
	{
	  rmag= TP* r;
	  cth= zp/ r;
	  px= rh/ r;
	  txk= cmplx( cosl( rmag),- sinl( rmag));
	  py= TP* r* r;
	  tyk= ETA* cth* txk* cmplx(1.0l,-1.0l/ rmag)/ py;
	  tzk= ETA* px* txk* cmplx(1.0l, rmag-1.0l/ rmag)/(2.0l* py);
	  tezk= tyk* cth- tzk* px;
	  terk= tyk* px+ tzk* cth;
	  rmag= sinl( PI* dataj.s)/ PI;
	  tezc= tezk* rmag;
	  terc= terk* rmag;
	  tezk= tezk* dataj.s;
	  terk= terk* dataj.s;
	  txs=CPLX_00;
	  tys=CPLX_00;
	  tzs=CPLX_00;

	} /* if( r >= dataj.rkh) */

	if( r < dataj.rkh)
	{
	  /* eksc for thin wire approx. or ekscx for extended t.w. approx. */
	  if( dataj.iexk != 1)
		eksc( dataj.s, zp, rh, TP, ijx, &tezs, &ters,
			&tezc, &terc, &tezk, &terk );
	  else
		ekscx( dataj.b, dataj.s, zp, rh, TP, ijx, dataj.ind1,
			dataj.ind2,	&tezs, &ters, &tezc, &terc, &tezk, &terk);

	  txs= tezs* dataj.cabj+ ters* rhox;
	  tys= tezs* dataj.sabj+ ters* rhoy;
	  tzs= tezs* salpr+ ters* rhoz;

	} /* if( r < dataj.rkh) */

	txk= tezk* dataj.cabj+ terk* rhox;
	tyk= tezk* dataj.sabj+ terk* rhoy;
	tzk= tezk* salpr+ terk* rhoz;
	txc= tezc* dataj.cabj+ terc* rhox;
	tyc= tezc* dataj.sabj+ terc* rhoy;
	tzc= tezc* salpr+ terc* rhoz;

	if( ip == 1)
	{
	  if( gnd.iperf <= 0)
	  {
		zratx= gnd.zrati;
		rmag= r;
		xymag= sqrtl( xij* xij+ yij* yij);

		/* set parameters for radial wire ground screen. */
		if( gnd.nradl != 0)
		{
		  xspec=( xi* dataj.zj+ zi* dataj.xj)/( zi+ dataj.zj);
		  yspec=( yi* dataj.zj+ zi* dataj.yj)/( zi+ dataj.zj);
		  rhospc= sqrtl( xspec* xspec+ yspec* yspec+ gnd.t2* gnd.t2);

		  if( rhospc <= gnd.scrwl)
		  {
			zscrn= gnd.t1* rhospc* logl( rhospc/ gnd.t2);
			zratx=( zscrn* gnd.zrati)/( ETA* gnd.zrati+ zscrn);
		  }
		} /* if( gnd.nradl != 0) */

		/* calculation of reflection coefficients
		 * when ground is specified. */
		if( xymag <= 1.0e-6l)
		{
		  px=0.0l;
		  py=0.0l;
		  cth=1.0l;
		  zrsin=CPLX_10;
		}
		else
		{
		  px= -yij/ xymag;
		  py= xij/ xymag;
		  cth= zij/ rmag;
		  zrsin= csqrtl(1.0l - zratx*zratx*(1.0l - cth*cth) );

		} /* if( xymag <= 1.0e-6) */

		refs=( cth- zratx* zrsin)/( cth+ zratx* zrsin);
		refps=-( zratx* cth- zrsin)/( zratx* cth+ zrsin);
		refps= refps- refs;
		epy= px* txk+ py* tyk;
		epx= px* epy;
		epy= py* epy;
		txk= refs* txk+ refps* epx;
		tyk= refs* tyk+ refps* epy;
		tzk= refs* tzk;
		epy= px* txs+ py* tys;
		epx= px* epy;
		epy= py* epy;
		txs= refs* txs+ refps* epx;
		tys= refs* tys+ refps* epy;
		tzs= refs* tzs;
		epy= px* txc+ py* tyc;
		epx= px* epy;
		epy= py* epy;
		txc= refs* txc+ refps* epx;
		tyc= refs* tyc+ refps* epy;
		tzc= refs* tzc;

	  } /* if( gnd.iperf <= 0) */

	  dataj.exk= dataj.exk- txk* gnd.frati;
	  dataj.eyk= dataj.eyk- tyk* gnd.frati;
	  dataj.ezk= dataj.ezk- tzk* gnd.frati;
	  dataj.exs= dataj.exs- txs* gnd.frati;
	  dataj.eys= dataj.eys- tys* gnd.frati;
	  dataj.ezs= dataj.ezs- tzs* gnd.frati;
	  dataj.exc= dataj.exc- txc* gnd.frati;
	  dataj.eyc= dataj.eyc- tyc* gnd.frati;
	  dataj.ezc= dataj.ezc- tzc* gnd.frati;
	  continue;

	} /* if( ip == 1) */

	dataj.exk= txk;
	dataj.eyk= tyk;
	dataj.ezk= tzk;
	dataj.exs= txs;
	dataj.eys= tys;
	dataj.ezs= tzs;
	dataj.exc= txc;
	dataj.eyc= tyc;
	dataj.ezc= tzc;

  } /* for( ip = 0; ip < gnd.ksymp; ip++ ) */

  if( gnd.iperf != 2)
	return;

  /* field due to ground using sommerfeld/norton */
  incom.sn= sqrtl( dataj.cabj* dataj.cabj+ dataj.sabj* dataj.sabj);
  if( incom.sn >= 1.0e-5l)
  {
	incom.xsn= dataj.cabj/ incom.sn;
	incom.ysn= dataj.sabj/ incom.sn;
  }
  else
  {
	incom.sn=0.0l;
	incom.xsn=1.0l;
	incom.ysn=0.0l;
  }

  /* displace observation point for thin wire approximation */
  zij= zi+ dataj.zj;
  salpr= -dataj.salpj;
  rhox= dataj.sabj* zij- salpr* yij;
  rhoy= salpr* xij- dataj.cabj* zij;
  rhoz= dataj.cabj* yij- dataj.sabj* xij;
  rh= rhox* rhox+ rhoy* rhoy+ rhoz* rhoz;

  if( rh <= 1.0e-10l)
  {
	incom.xo= xi- ai* incom.ysn;
	incom.yo= yi+ ai* incom.xsn;
	incom.zo= zi;
  }
  else
  {
	rh= ai/ sqrtl( rh);
	if( rhoz < 0.0l)
	  rh= -rh;
	incom.xo= xi+ rh* rhox;
	incom.yo= yi+ rh* rhoy;
	incom.zo= zi+ rh* rhoz;

  } /* if( rh <= 1.0e-10l) */

  r= xij* xij+ yij* yij+ zij* zij;
  if( r <= .95l)
  {
	long double shaf;

	/* field from interpolation is integrated over segment */
	incom.isnor=1;
	dmin=
	  dataj.exk* conjl( dataj.exk) +
	  dataj.eyk* conjl( dataj.eyk) +
	  dataj.ezk* conjl( dataj.ezk);
	dmin=.01l* sqrtl( dmin);
	shaf=.5l* dataj.s;
	rom2(- shaf, shaf, egnd, dmin);
  }
  else
  {
	/* norton field equations and lumped
	 * current element approximation */
	incom.isnor=2;
	sflds(0.0l, egnd);
  } /* if( r <= .95l) */

  if( r > .95l)
  {
	zp= xij* dataj.cabj+ yij* dataj.sabj+ zij* salpr;
	rh= r- zp* zp;
	if( rh <= 1.0e-10l)
	  dmin=0.0l;
	else
	  dmin= sqrtl( rh/( rh+ ai* ai));

	if( dmin <= .95l)
	{
	  px=1.0l- dmin;
	  terk=( txk* dataj.cabj+ tyk* dataj.sabj+ tzk* salpr)* px;
	  txk= dmin* txk+ terk* dataj.cabj;
	  tyk= dmin* tyk+ terk* dataj.sabj;
	  tzk= dmin* tzk+ terk* salpr;
	  ters=( txs* dataj.cabj+ tys* dataj.sabj+ tzs* salpr)* px;
	  txs= dmin* txs+ ters* dataj.cabj;
	  tys= dmin* tys+ ters* dataj.sabj;
	  tzs= dmin* tzs+ ters* salpr;
	  terc=( txc* dataj.cabj+ tyc* dataj.sabj+ tzc* salpr)* px;
	  txc= dmin* txc+ terc* dataj.cabj;
	  tyc= dmin* tyc+ terc* dataj.sabj;
	  tzc= dmin* tzc+ terc* salpr;

	} /* if( dmin <= .95l) */

  } /* if( r > .95l) */

  dataj.exk= dataj.exk+ txk;
  dataj.eyk= dataj.eyk+ tyk;
  dataj.ezk= dataj.ezk+ tzk;
  dataj.exs= dataj.exs+ txs;
  dataj.eys= dataj.eys+ tys;
  dataj.ezs= dataj.ezs+ tzs;
  dataj.exc= dataj.exc+ txc;
  dataj.eyc= dataj.eyc+ tyc;
  dataj.ezc= dataj.ezc+ tzc;

  return;
}

/*-----------------------------------------------------------------------*/

/* compute e field of sine, cosine, and constant */
/* current filaments by thin wire approximation. */
void eksc( long double s, long double z, long double rh,
	long double xk, int ij,	complex long double *ezs,
	complex long double *ers, complex long double *ezc,
	complex long double *erc, complex long double *ezk,
	complex long double *erk )
{
  long double rhk, sh, shk, ss, cs, z1a, z2a, cint, sint;
  complex long double gz1, gz2, gp1, gp2, gzp1, gzp2;

  tmi.ij= ij;
  tmi.zpk= xk* z;
  rhk= xk* rh;
  tmi.rkb2= rhk* rhk;
  sh=.5l* s;
  shk= xk* sh;
  ss= sinl( shk);
  cs= cosl( shk);
  z2a= sh- z;
  z1a=-( sh+ z);
  gx( z1a, rh, xk, &gz1, &gp1);
  gx( z2a, rh, xk, &gz2, &gp2);
  gzp1= gp1* z1a;
  gzp2= gp2* z2a;
  *ezs=  CONST1*(( gz2- gz1)* cs* xk-( gzp2+ gzp1)* ss);
  *ezc= -CONST1*(( gz2+ gz1)* ss* xk+( gzp2- gzp1)* cs);
  *erk= CONST1*( gp2- gp1)* rh;
  intx(- shk, shk, rhk, ij, &cint, &sint);
  *ezk= -CONST1*( gzp2- gzp1+ xk* xk* cmplx( cint,- sint));
  gzp1= gzp1* z1a;
  gzp2= gzp2* z2a;

  if( rh >= 1.0e-10l)
  {
	*ers= -CONST1*(( gzp2+ gzp1+ gz2+ gz1)*
		ss-( z2a* gz2- z1a* gz1)* cs*xk)/ rh;
	*erc= -CONST1*(( gzp2- gzp1+ gz2- gz1)*
		cs+( z2a* gz2+ z1a* gz1)* ss*xk)/ rh;
	return;
  }

  *ers = CPLX_00;
  *erc = CPLX_00;

  return;
}

/*-----------------------------------------------------------------------*/

/* compute e field of sine, cosine, and constant current */
/* filaments by extended thin wire approximation. */
void ekscx( long double bx, long double s, long double z,
	long double rhx, long double xk, int ij, int inx1,
	int inx2, complex long double *ezs, complex long double *ers,
	complex long double *ezc, complex long double *erc,
	complex long double *ezk, complex long double *erk )
{
  int ira;
  long double b, rh, sh, rhk, shk, ss, cs, z1a;
  long double z2a, a2, bk, bk2, cint, sint;
  complex long double gz1, gz2, gzp1, gzp2, gr1, gr2;
  complex long double grp1, grp2, grk1, grk2, gzz1, gzz2;

  if( rhx >= bx)
  {
	rh= rhx;
	b= bx;
	ira=0;
  }
  else
  {
	rh= bx;
	b= rhx;
	ira=1;
  }

  sh=.5l* s;
  tmi.ij= ij;
  tmi.zpk= xk* z;
  rhk= xk* rh;
  tmi.rkb2= rhk* rhk;
  shk= xk* sh;
  ss= sinl( shk);
  cs= cosl( shk);
  z2a= sh- z;
  z1a=-( sh+ z);
  a2= b* b;

  if( inx1 != 2)
	gxx( z1a, rh, b, a2, xk, ira, &gz1,
		&gzp1, &gr1, &grp1, &grk1, &gzz1);
  else
  {
	gx( z1a, rhx, xk, &gz1, &grk1);
	gzp1= grk1* z1a;
	gr1= gz1/ rhx;
	grp1= gzp1/ rhx;
	grk1= grk1* rhx;
	gzz1= CPLX_00;
  }

  if( inx2 != 2)
	gxx( z2a, rh, b, a2, xk, ira, &gz2,
		&gzp2, &gr2, &grp2, &grk2, &gzz2);
  else
  {
	gx( z2a, rhx, xk, &gz2, &grk2);
	gzp2= grk2* z2a;
	gr2= gz2/ rhx;
	grp2= gzp2/ rhx;
	grk2= grk2* rhx;
	gzz2= CPLX_00;
  }

  *ezs= CONST1*(( gz2- gz1)* cs* xk-( gzp2+ gzp1)* ss);
  *ezc= -CONST1*(( gz2+ gz1)* ss* xk+( gzp2- gzp1)* cs);
  *ers= -CONST1*(( z2a* grp2+ z1a* grp1+ gr2+ gr1)*ss
	  -( z2a* gr2- z1a* gr1)* cs* xk);
  *erc= -CONST1*(( z2a* grp2- z1a* grp1+ gr2- gr1)*cs
	  +( z2a* gr2+ z1a* gr1)* ss* xk);
  *erk= CONST1*( grk2- grk1);
  intx(- shk, shk, rhk, ij, &cint, &sint);
  bk= b* xk;
  bk2= bk* bk*.25l;
  *ezk= -CONST1*( gzp2- gzp1+ xk* xk*(1.0l- bk2)*
	  cmplx( cint,- sint)-bk2*( gzz2- gzz1));

  return;
}

/*-----------------------------------------------------------------------*/

/* gf computes the integrand exp(jkr)/(kr) for numerical integration. */
void gf( long double zk, long double *co, long double *si )
{
  long double zdk, rk, rks;

  zdk= zk- tmi.zpk;
  rk= sqrtl( tmi.rkb2+ zdk* zdk);
  *si= sinl( rk)/ rk;

  if( tmi.ij != 0 )
  {
	*co= cosl( rk)/ rk;
	return;
  }

  if( rk >= .2l)
  {
	*co=( cosl( rk)-1.0l)/ rk;
	return;
  }

  rks= rk* rk;
  *co=((-1.38888889e-3l* rks+4.16666667e-2l)* rks-.5l)* rk;

  return;
}

/*-----------------------------------------------------------------------*/

/* integrand for h field of a wire */
void gh( long double zk, long double *hr, long double *hi)
{
  long double rs, r, ckr, skr, rr2, rr3;

  rs= zk- tmh.zpka;
  rs= tmh.rhks+ rs* rs;
  r= sqrtl( rs);
  ckr= cosl( r);
  skr= sinl( r);
  rr2=1.0l/ rs;
  rr3= rr2/ r;
  *hr= skr* rr2+ ckr* rr3;
  *hi= ckr* rr2- skr* rr3;

  return;
}

/*-----------------------------------------------------------------------*/

/* gwave computes the electric field, including ground wave, of a */
/* current element over a ground plane using formulas of k.a. norton */
/* (proc. ire, sept., 1937, pp.1203,1236) */

void gwave( complex long double *erv, complex long double *ezv,
	complex long double *erh, complex long double *ezh,
	complex long double *eph )
{
  long double sppp, sppp2, cppp2, cppp, spp, spp2, cpp2, cpp;
  complex long double rk1, rk2, t1, t2, t3, t4, p1, rv;
  complex long double omr, w, f, q1, rh, v, g, xr1, xr2;
  complex long double x1, x2, x3, x4, x5, x6, x7;

  sppp= gwav.zmh/ gwav.r1;
  sppp2= sppp* sppp;
  cppp2=1.0l- sppp2;

  if( cppp2 < 1.0e-20l)
	cppp2=1.0e-20l;

  cppp= sqrtl( cppp2);
  spp= gwav.zph/ gwav.r2;
  spp2= spp* spp;
  cpp2=1.0l- spp2;

  if( cpp2 < 1.0e-20l)
	cpp2=1.0e-20l;

  cpp= sqrtl( cpp2);
  rk1= -TPJ* gwav.r1;
  rk2= -TPJ* gwav.r2;
  t1=1.0l -gwav.u2* cpp2;
  t2= csqrtl( t1);
  t3=(1.0l -1.0l/ rk1)/ rk1;
  t4=(1.0l -1.0l/ rk2)/ rk2;
  p1= rk2* gwav.u2* t1/(2.0l* cpp2);
  rv=( spp- gwav.u* t2)/( spp+ gwav.u* t2);
  omr=1.0l- rv;
  w=1.0l/ omr;
  w=(4.0l + 0.0lj)* p1* w* w;
  fbar( w, &f );
  q1= rk2* t1/(2.0l* gwav.u2* cpp2);
  rh=( t2- gwav.u* spp)/( t2+ gwav.u* spp);
  v=1.0l/(1.0l+ rh);
  v=(4.0l + 0.0lj)* q1* v* v;
  fbar( v, &g );
  xr1= gwav.xx1/ gwav.r1;
  xr2= gwav.xx2/ gwav.r2;
  x1= cppp2* xr1;
  x2= rv* cpp2* xr2;
  x3= omr* cpp2* f* xr2;
  x4= gwav.u* t2* spp*2.0l* xr2/ rk2;
  x5= xr1* t3*(1.0l-3.0l* sppp2);
  x6= xr2* t4*(1.0l-3.0l* spp2);
  *ezv=( x1+ x2+ x3- x4- x5- x6)* (-CONST4);
  x1= sppp* cppp* xr1;
  x2= rv* spp* cpp* xr2;
  x3= cpp* omr* gwav.u* t2* f* xr2;
  x4= spp* cpp* omr* xr2/ rk2;
  x5=3.0l* sppp* cppp* t3* xr1;
  x6= cpp* gwav.u* t2* omr* xr2/ rk2*.50l;
  x7=3.0l* spp* cpp* t4* xr2;
  *erv=-( x1+ x2- x3+ x4- x5+ x6- x7)* (-CONST4);
  *ezh=-( x1- x2+ x3- x4- x5- x6+ x7)* (-CONST4);
  x1= sppp2* xr1;
  x2= rv* spp2* xr2;
  x4= gwav.u2* t1* omr* f* xr2;
  x5= t3*(1.0l-3.0l* cppp2)* xr1;
  x6= t4*(1.0l-3.0l* cpp2)*(1.0l- gwav.u2*(1.0l+ rv )-
	  gwav.u2* omr* f)* xr2;
  x7= gwav.u2* cpp2* omr*(1.0l-1.0l/ rk2)*( f*( gwav.u2* t1 -
		spp2-1.0l/ rk2)+1.0l/rk2)* xr2;
  *erh=( x1- x2- x4- x5+ x6+ x7)* (-CONST4);
  x1= xr1;
  x2= rh* xr2;
  x3=( rh+1.0l)* g* xr2;
  x4= t3* xr1;
  x5= t4*(1.0l- gwav.u2*(1.0l+ rv)- gwav.u2* omr* f)* xr2;
  x6=.5l* gwav.u2* omr*( f*( gwav.u2* t1- spp2-1.0l/ rk2) +
	  1.0l/ rk2)* xr2/ rk2;
  *eph=-( x1- x2+ x3- x4+ x5+ x6)* (-CONST4);

  return;
}

/*-----------------------------------------------------------------------*/

/* segment end contributions for thin wire approx. */
void gx( long double zz, long double rh, long double xk,
	complex long double *gz, complex long double *gzp)
{
  long double r, r2, rkz;

  r2= zz* zz+ rh* rh;
  r= sqrtl( r2);
  rkz= xk* r;
  *gz= cmplx( cosl( rkz),- sinl( rkz))/ r;
  *gzp= -cmplx(1.0l, rkz)* *gz/ r2;

  return;
}

/*-----------------------------------------------------------------------*/

/* segment end contributions for ext. thin wire approx. */
void gxx( long double zz, long double rh, long double a,
	long double a2, long double xk, int ira, complex long double *g1,
	complex long double *g1p, complex long double *g2,
	complex long double *g2p, complex long double *g3,
	complex long double *gzp )
{
  long double r, r2, r4, rk, rk2, rh2, t1, t2;
  complex long double  gz, c1, c2, c3;

  r2= zz* zz+ rh* rh;
  r= sqrtl( r2);
  r4= r2* r2;
  rk= xk* r;
  rk2= rk* rk;
  rh2= rh* rh;
  t1=.25l* a2* rh2/ r4;
  t2=.5l* a2/ r2;
  c1= cmplx(1.0l, rk);
  c2=3.0l* c1- rk2;
  c3= cmplx(6.0l, rk)* rk2-15.0l* c1;
  gz= cmplx( cosl( rk),- sinl( rk))/ r;
  *g2= gz*(1.0l+ t1* c2);
  *g1= *g2- t2* c1* gz;
  gz= gz/ r2;
  *g2p= gz*( t1* c3- c1);
  *gzp= t2* c2* gz;
  *g3= *g2p+ *gzp;
  *g1p= *g3* zz;

  if( ira != 1)
  {
	*g3=( *g3+ *gzp)* rh;
	*gzp= -zz* c1* gz;

	if( rh <= 1.0e-10l)
	{
	  *g2=0.0l;
	  *g2p=0.0l;
	  return;
	}

	*g2= *g2/ rh;
	*g2p= *g2p* zz/ rh;
	return;

  } /* if( ira != 1) */

  t2=.5l* a;
  *g2= -t2* c1* gz;
  *g2p= t2* gz* c2/ r2;
  *g3= rh2* *g2p- a* gz* c1;
  *g2p= *g2p* zz;
  *gzp= -zz* c1* gz;

  return;
}

/*-----------------------------------------------------------------------*/

/* hfk computes the h field of a uniform current */
/* filament by numerical integration */
void hfk( long double el1, long double el2, long double rhk,
	long double zpkx, long double *sgr, long double *sgi )
{
  int nx = 1, nma = 65536, nts = 4;
  int ns, nt;
  int flag = TRUE;
  long double rx = 1.0e-4l;
  long double z, ze, s, ep, zend, dz=0.0l, zp, dzot=0.0l;
  long double t00r, g1r, g5r=0.0l, t00i, g1i, g5i=0.0l, t01r;
  long double g3r=0.0l, t01i, g3i=0.0l, t10r, t10i, te1i;
  long double te1r, g2r, g4r, t02i, g2i, g4i, t11r, t11i;
  long double t20r, t20i, te2i, te2r, t02r;

  tmh.zpka= zpkx;
  tmh.rhks= rhk* rhk;
  z= el1;
  ze= el2;
  s= ze- z;
  ep= s/(10.0l* nma);
  zend= ze- ep;
  *sgr=0.0l;
  *sgi=0.0l;
  ns= nx;
  nt=0;
  gh( z, &g1r, &g1i);

  while( TRUE )
  {
	if( flag )
	{
	  dz= s/ ns;
	  zp= z+ dz;

	  if( zp > ze )
	  {
		dz= ze- z;
		if( fabsl(dz) <= ep )
		{
		  *sgr= *sgr* rhk*.5l;
		  *sgi= *sgi* rhk*.5l;
		  return;
		}
	  }

	  dzot= dz*.5l;
	  zp= z+ dzot;
	  gh( zp, &g3r, &g3i);
	  zp= z+ dz;
	  gh( zp, &g5r, &g5i);

	} /* if( flag ) */

	t00r=( g1r+ g5r)* dzot;
	t00i=( g1i+ g5i)* dzot;
	t01r=( t00r+ dz* g3r)*0.5l;
	t01i=( t00i+ dz* g3i)*0.5l;
	t10r=(4.0l* t01r- t00r)/3.0l;
	t10i=(4.0l* t01i- t00i)/3.0l;

	test( t01r, t10r, &te1r, t01i, t10i, &te1i, 0.0l);
	if( (te1i <= rx) && (te1r <= rx) )
	{
	  *sgr= *sgr+ t10r;
	  *sgi= *sgi+ t10i;
	  nt += 2;

	  z += dz;
	  if( z >= zend)
	  {
		*sgr= *sgr* rhk*.5l;
		*sgi= *sgi* rhk*.5l;
		return;
	  }

	  g1r= g5r;
	  g1i= g5i;
	  if( nt >= nts)
		if( ns > nx)
		{
		  ns= ns/2;
		  nt=1;
		}
	  flag = TRUE;
	  continue;

	} /* if( (te1i <= rx) && (te1r <= rx) ) */

	zp= z+ dz*0.25l;
	gh( zp, &g2r, &g2i);
	zp= z+ dz*0.75l;
	gh( zp, &g4r, &g4i);
	t02r=( t01r+ dzot*( g2r+ g4r))*0.5l;
	t02i=( t01i+ dzot*( g2i+ g4i))*0.5l;
	t11r=(4.0l* t02r- t01r)/3.0l;
	t11i=(4.0l* t02i- t01i)/3.0l;
	t20r=(16.0l* t11r- t10r)/15.0l;
	t20i=(16.0l* t11i- t10i)/15.0l;

	test( t11r, t20r, &te2r, t11i, t20i, &te2i, 0.0l);
	if( (te2i > rx) || (te2r > rx) )
	{
	  nt=0;
	  if( ns >= nma)
	  {
		fprintf( stderr,
			"\nxnec2c: step size limited at z= %10.5LF", z );
	  }
	  else
	  {
		ns= ns*2;
		dz= s/ ns;
		dzot= dz*0.5l;
		g5r= g3r;
		g5i= g3i;
		g3r= g2r;
		g3i= g2i;

		flag = FALSE;
		continue;
	  }

	} /* if( (te2i > rx) || (te2r > rx) ) */

	*sgr= *sgr+ t20r;
	*sgi= *sgi+ t20i;
	nt++;

	z += dz;
	if( z >= zend)
	{
	  *sgr= *sgr* rhk*.5l;
	  *sgi= *sgi* rhk*.5l;
	  return;
	}

	g1r= g5r;
	g1i= g5i;
	if( nt >= nts)
	  if( ns > nx)
	  {
		ns= ns/2;
		nt=1;
	  }
	flag = TRUE;

  } /* while( TRUE ) */

}

/*-----------------------------------------------------------------------*/

/* hintg computes the h field of a patch current */
void hintg( long double xi, long double yi, long double zi )
{
  int ip;
  long double rx, ry, rfl, xymag, pxx, pyy, cth;
  long double rz, rsq, r, rk, cr, sr, t1zr, t2zr;
  complex long double  gam, f1x, f1y, f1z, f2x, f2y, f2z, rrv, rrh;

  rx= xi- dataj.xj;
  ry= yi- dataj.yj;
  rfl=-1.0l;
  dataj.exk=CPLX_00;
  dataj.eyk=CPLX_00;
  dataj.ezk=CPLX_00;
  dataj.exs=CPLX_00;
  dataj.eys=CPLX_00;
  dataj.ezs=CPLX_00;

  for( ip = 1; ip <= gnd.ksymp; ip++ )
  {
	rfl= -rfl;
	rz= zi- dataj.zj* rfl;
	rsq= rx* rx+ ry* ry+ rz* rz;

	if( rsq < 1.0e-20l)
	  continue;

	r = sqrtl( rsq );
	rk= TP* r;
	cr= cosl( rk);
	sr= sinl( rk);
	gam=-( cmplx(cr,-sr)+rk*cmplx(sr,cr) )/( FPI*rsq*r )* dataj.s;
	dataj.exc= gam* rx;
	dataj.eyc= gam* ry;
	dataj.ezc= gam* rz;
	t1zr= dataj.t1zj* rfl;
	t2zr= dataj.t2zj* rfl;
	f1x= dataj.eyc* t1zr- dataj.ezc* dataj.t1yj;
	f1y= dataj.ezc* dataj.t1xj- dataj.exc* t1zr;
	f1z= dataj.exc* dataj.t1yj- dataj.eyc* dataj.t1xj;
	f2x= dataj.eyc* t2zr- dataj.ezc* dataj.t2yj;
	f2y= dataj.ezc* dataj.t2xj- dataj.exc* t2zr;
	f2z= dataj.exc* dataj.t2yj- dataj.eyc* dataj.t2xj;

	if( ip != 1)
	{
	  if( gnd.iperf == 1)
	  {
		f1x= -f1x;
		f1y= -f1y;
		f1z= -f1z;
		f2x= -f2x;
		f2y= -f2y;
		f2z= -f2z;
	  }
	  else
	  {
		xymag= sqrtl( rx* rx+ ry* ry);
		if( xymag <= 1.0e-6l)
		{
		  pxx=0.0l;
		  pyy=0.0l;
		  cth=1.0l;
		  rrv=CPLX_10;
		}
		else
		{
		  pxx= -ry/ xymag;
		  pyy= rx/ xymag;
		  cth= rz/ r;
		  rrv= csqrtl(1.0l- gnd.zrati* gnd.zrati*(1.0l- cth* cth));

		} /* if( xymag <= 1.0e-6l) */

		rrh= gnd.zrati* cth;
		rrh=( rrh- rrv)/( rrh+ rrv);
		rrv= gnd.zrati* rrv;
		rrv=-( cth- rrv)/( cth+ rrv);
		gam=( f1x* pxx+ f1y* pyy)*( rrv- rrh);
		f1x= f1x* rrh+ gam* pxx;
		f1y= f1y* rrh+ gam* pyy;
		f1z= f1z* rrh;
		gam=( f2x* pxx+ f2y* pyy)*( rrv- rrh);
		f2x= f2x* rrh+ gam* pxx;
		f2y= f2y* rrh+ gam* pyy;
		f2z= f2z* rrh;

	  } /* if( gnd.iperf == 1) */

	} /* if( ip != 1) */

	dataj.exk += f1x;
	dataj.eyk += f1y;
	dataj.ezk += f1z;
	dataj.exs += f2x;
	dataj.eys += f2y;
	dataj.ezs += f2z;

  } /* for( ip = 1; ip <= gnd.ksymp; ip++ ) */

  return;
}

/*-----------------------------------------------------------------------*/

/* hsfld computes the h field for constant, sine, and */
/* cosine current on a segment including ground effects. */
void hsfld( long double xi, long double yi,
	long double zi, long double ai )
{
  int ip;
  long double xij, yij, rfl, salpr, zij, zp, rhox;
  long double rhoy, rhoz, rh, phx, phy, phz, rmag;
  long double xymag, xspec, yspec, rhospc, px, py, cth;
  complex long double hpk, hps, hpc, qx, qy, qz, rrv, rrh, zratx;

  xij= xi- dataj.xj;
  yij= yi- dataj.yj;
  rfl=-1.0l;

  for( ip = 0; ip < gnd.ksymp; ip++ )
  {
	rfl= -rfl;
	salpr= dataj.salpj* rfl;
	zij= zi- rfl* dataj.zj;
	zp= xij* dataj.cabj+ yij* dataj.sabj+ zij* salpr;
	rhox= xij- dataj.cabj* zp;
	rhoy= yij- dataj.sabj* zp;
	rhoz= zij- salpr* zp;
	rh= sqrtl( rhox* rhox+ rhoy* rhoy+ rhoz* rhoz+ ai* ai);

	if( rh <= 1.0e-10l)
	{
	  dataj.exk=0.0l;
	  dataj.eyk=0.0l;
	  dataj.ezk=0.0l;
	  dataj.exs=0.0l;
	  dataj.eys=0.0l;
	  dataj.ezs=0.0l;
	  dataj.exc=0.0l;
	  dataj.eyc=0.0l;
	  dataj.ezc=0.0l;
	  continue;
	}

	rhox= rhox/ rh;
	rhoy= rhoy/ rh;
	rhoz= rhoz/ rh;
	phx= dataj.sabj* rhoz- salpr* rhoy;
	phy= salpr* rhox- dataj.cabj* rhoz;
	phz= dataj.cabj* rhoy- dataj.sabj* rhox;

	hsflx( dataj.s, rh, zp, &hpk, &hps, &hpc);

	if( ip == 1 )
	{
	  if( gnd.iperf != 1 )
	  {
		zratx= gnd.zrati;
		rmag= sqrtl( zp* zp+ rh* rh);
		xymag= sqrtl( xij* xij+ yij* yij);

		/* set parameters for radial wire ground screen. */
		if( gnd.nradl != 0)
		{
		  xspec=( xi* dataj.zj+ zi* dataj.xj)/( zi+ dataj.zj);
		  yspec=( yi* dataj.zj+ zi* dataj.yj)/( zi+ dataj.zj);
		  rhospc= sqrtl( xspec* xspec+ yspec* yspec+ gnd.t2* gnd.t2);

		  if( rhospc <= gnd.scrwl)
		  {
			rrv= gnd.t1* rhospc* logl( rhospc/ gnd.t2);
			zratx=( rrv* gnd.zrati)/( ETA* gnd.zrati+ rrv);
		  }
		}

		/* calculation of reflection coefficients
		 * when ground is specified. */
		if( xymag <= 1.0e-6l)
		{
		  px=0.0l;
		  py=0.0l;
		  cth=1.0l;
		  rrv=CPLX_10;
		}
		else
		{
		  px= -yij/ xymag;
		  py= xij/ xymag;
		  cth= zij/ rmag;
		  rrv= csqrtl(1.0l- zratx* zratx*(1.0l- cth* cth));
		}

		rrh= zratx* cth;
		rrh=-( rrh- rrv)/( rrh+ rrv);
		rrv= zratx* rrv;
		rrv=( cth- rrv)/( cth+ rrv);
		qy=( phx* px+ phy* py)*( rrv- rrh);
		qx= qy* px+ phx* rrh;
		qy= qy* py+ phy* rrh;
		qz= phz* rrh;
		dataj.exk= dataj.exk- hpk* qx;
		dataj.eyk= dataj.eyk- hpk* qy;
		dataj.ezk= dataj.ezk- hpk* qz;
		dataj.exs= dataj.exs- hps* qx;
		dataj.eys= dataj.eys- hps* qy;
		dataj.ezs= dataj.ezs- hps* qz;
		dataj.exc= dataj.exc- hpc* qx;
		dataj.eyc= dataj.eyc- hpc* qy;
		dataj.ezc= dataj.ezc- hpc* qz;
		continue;

	  } /* if( gnd.iperf != 1 ) */

	  dataj.exk= dataj.exk- hpk* phx;
	  dataj.eyk= dataj.eyk- hpk* phy;
	  dataj.ezk= dataj.ezk- hpk* phz;
	  dataj.exs= dataj.exs- hps* phx;
	  dataj.eys= dataj.eys- hps* phy;
	  dataj.ezs= dataj.ezs- hps* phz;
	  dataj.exc= dataj.exc- hpc* phx;
	  dataj.eyc= dataj.eyc- hpc* phy;
	  dataj.ezc= dataj.ezc- hpc* phz;
	  continue;

	} /* if( ip == 1 ) */

	dataj.exk= hpk* phx;
	dataj.eyk= hpk* phy;
	dataj.ezk= hpk* phz;
	dataj.exs= hps* phx;
	dataj.eys= hps* phy;
	dataj.ezs= hps* phz;
	dataj.exc= hpc* phx;
	dataj.eyc= hpc* phy;
	dataj.ezc= hpc* phz;

  } /* for( ip = 0; ip < gnd.ksymp; ip++ ) */

  return;
}

/*-----------------------------------------------------------------------*/

/* calculates h field of sine cosine, and constant current of segment */
void hsflx( long double s, long double rh, long double zpx,
	complex long double *hpk, complex long double *hps,
	complex long double *hpc )
{
  complex long double fjk, ekr1, ekr2, t1, t2, cons;

  fjk = -TPJ;
  if( rh >= 1.0e-10l)
  {
	long double zp, z2a, hss, dh, z1;
	long double rhz, dk, cdk, sdk, hkr, hki;

	if( zpx >= 0.0l)
	{
	  zp= zpx;
	  hss=1.0l;
	}
	else
	{
	  zp= -zpx;
	  hss=-1.0l;
	}

	dh=.5l* s;
	z1= zp+ dh;
	z2a= zp- dh;
	if( z2a >= 1.0e-7l)
	  rhz= rh/ z2a;
	else
	  rhz=1.0l;

	dk= TP* dh;
	cdk= cosl( dk);
	sdk= sinl( dk);
	hfk(- dk, dk, rh* TP, zp* TP, &hkr, &hki);
	*hpk= cmplx( hkr, hki);

	if( rhz >= 1.0e-3l)
	{
	  long double rh2, r1, r2;

	  rh2= rh* rh;
	  r1= sqrtl( rh2+ z1* z1);
	  r2= sqrtl( rh2+ z2a* z2a);
	  ekr1= cexp( fjk* r1);
	  ekr2= cexp( fjk* r2);
	  t1= z1* ekr1/ r1;
	  t2= z2a* ekr2/ r2;
	  *hps=( cdk*( ekr2- ekr1)- CPLX_01* sdk*( t2+ t1))* hss;
	  *hpc= -sdk*( ekr2+ ekr1)- CPLX_01* cdk*( t2- t1);
	  cons= -CPLX_01/(2.0l* TP* rh);
	  *hps= cons* *hps;
	  *hpc= cons* *hpc;
	  return;

	} /* if( rhz >= 1.0e-3l) */

	ekr1= cmplx( cdk, sdk)/( z2a* z2a);
	ekr2= cmplx( cdk,- sdk)/( z1* z1);
	t1= TP*(1.0l/ z1-1.0l/ z2a);
	t2= cexp( fjk* zp)* rh/PI8;
	*hps= t2*( t1+( ekr1+ ekr2)* sdk)* hss;
	*hpc= t2*(- CPLX_01* t1+( ekr1- ekr2)* cdk);
	return;

  } /* if( rh >= 1.0e-10l) */

  *hps=CPLX_00;
  *hpc=CPLX_00;
  *hpk=CPLX_00;

  return;
}

/*-----------------------------------------------------------------------*/

/* nefld computes the near field at specified points in space after */
/* the structure currents have been computed. */
void nefld( long double xob, long double yob,
	long double zob, complex long double *ex,
	complex long double *ey, complex long double *ez )
{
  int i, ix, ipr, iprx, jc, ipa;
  long double zp, xi, ax;
  complex long double acx, bcx, ccx;

  *ex=CPLX_00;
  *ey=CPLX_00;
  *ez=CPLX_00;
  ax=0.0l;

  if( data.n != 0)
  {
	for( i = 0; i < data.n; i++ )
	{
	  dataj.xj= xob- data.x[i];
	  dataj.yj= yob- data.y[i];
	  dataj.zj= zob- data.z[i];
	  zp= data.cab[i]* dataj.xj+ data.sab[i] *
		dataj.yj+ data.salp[i]* dataj.zj;

	  if( fabsl( zp) > 0.5001l* data.si[i])
		continue;

	  zp= dataj.xj* dataj.xj+ dataj.yj* dataj.yj +
		dataj.zj* dataj.zj- zp* zp;
	  dataj.xj= data.bi[i];

	  if( zp > 0.9l* dataj.xj* dataj.xj)
		continue;

	  ax= dataj.xj;
	  break;

	} /* for( i = 0; i < n; i++ ) */

	for( i = 0; i < data.n; i++ )
	{
	  ix = i+1;
	  dataj.s= data.si[i];
	  dataj.b= data.bi[i];
	  dataj.xj= data.x[i];
	  dataj.yj= data.y[i];
	  dataj.zj= data.z[i];
	  dataj.cabj= data.cab[i];
	  dataj.sabj= data.sab[i];
	  dataj.salpj= data.salp[i];

	  if( dataj.iexk != 0)
	  {
		ipr= data.icon1[i];

		if (ipr > PCHCON) dataj.ind1 = 2;
		else if( ipr < 0 )
		{
		  ipr = -ipr;
		  iprx = ipr-1;

		  if( -data.icon1[iprx] != ix )
			dataj.ind1=2;
		  else
		  {
			xi= fabsl( dataj.cabj* data.cab[iprx]+ dataj.sabj*
				data.sab[iprx]+ dataj.salpj* data.salp[iprx]);
			if( (xi < 0.999999l) ||
				(fabsl(data.bi[iprx]/dataj.b-1.0l) > 1.0e-6l) )
			  dataj.ind1=2;
			else
			  dataj.ind1=0;
		  }
		} /* if( ipr < 0 ) */
		else
		  if( ipr == 0 )
			dataj.ind1=1;
		  else
		  {
			iprx = ipr-1;

			if( ipr != ix )
			{
			  if( data.icon2[iprx] != ix )
				dataj.ind1=2;
			  else
			  {
				xi= fabsl( dataj.cabj* data.cab[iprx]+ dataj.sabj*
					data.sab[iprx]+ dataj.salpj* data.salp[iprx]);
				if( (xi < 0.999999l) ||
					(fabsl(data.bi[iprx]/dataj.b-1.0l) > 1.0e-6l) )
				  dataj.ind1=2;
				else
				  dataj.ind1=0;
			  }
			} /* if( ipr != ix ) */
			else
			{
			  if( dataj.cabj* dataj.cabj +
				  dataj.sabj* dataj.sabj > 1.0e-8l)
				dataj.ind1=2;
			  else
				dataj.ind1=0;
			}
		  } /* else */

		ipr= data.icon2[i];

		if (ipr > PCHCON) dataj.ind2 = 2;
		else if( ipr < 0 )
		{
		  ipr = -ipr;
		  iprx = ipr-1;

		  if( -data.icon2[iprx] != ix )
			dataj.ind1=2;
		  else
		  {
			xi= fabsl( dataj.cabj* data.cab[iprx]+ dataj.sabj*
				data.sab[iprx]+ dataj.salpj* data.salp[iprx]);
			if( (xi < 0.999999l) ||
				(fabsl(data.bi[iprx]/dataj.b-1.0l) > 1.0e-6l) )
			  dataj.ind1=2;
			else
			  dataj.ind1=0;
		  }
		} /* if( ipr < 0 ) */
		else
		  if( ipr == 0 )
			dataj.ind2=1;
		  else
		  {
			iprx = ipr-1;

			if( ipr != ix )
			{
			  if( data.icon1[iprx] != ix )
				dataj.ind2=2;
			  else
			  {
				xi= fabsl( dataj.cabj* data.cab[iprx]+ dataj.sabj*
					data.sab[iprx]+ dataj.salpj* data.salp[iprx]);
				if( (xi < 0.999999l) ||
					(fabsl(data.bi[iprx]/dataj.b-1.0l) > 1.0e-6l) )
				  dataj.ind2=2;
				else
				  dataj.ind2=0;
			  }
			} /* if( ipr != (i+1) ) */
			else
			{
			  if( dataj.cabj* dataj.cabj +
				  dataj.sabj* dataj.sabj > 1.0e-8l)
				dataj.ind1=2;
			  else
				dataj.ind1=0;
			}

		  } /* else */

	  } /* if( dataj.iexk != 0) */

	  efld( xob, yob, zob, ax,1);
	  acx= cmplx( crnt.air[i], crnt.aii[i]);
	  bcx= cmplx( crnt.bir[i], crnt.bii[i]);
	  ccx= cmplx( crnt.cir[i], crnt.cii[i]);
	  *ex += dataj.exk* acx+ dataj.exs* bcx+ dataj.exc* ccx;
	  *ey += dataj.eyk* acx+ dataj.eys* bcx+ dataj.eyc* ccx;
	  *ez += dataj.ezk* acx+ dataj.ezs* bcx+ dataj.ezc* ccx;

	} /* for( i = 0; i < n; i++ ) */

	if( data.m == 0)
	  return;

  } /* if( n != 0) */

  jc= data.n-1;
  for( i = 0; i < data.m; i++ )
  {
	dataj.s= data.pbi[i];
	dataj.xj= data.px[i];
	dataj.yj= data.py[i];
	dataj.zj= data.pz[i];
	dataj.t1xj= data.t1x[i];
	dataj.t1yj= data.t1y[i];
	dataj.t1zj= data.t1z[i];
	dataj.t2xj= data.t2x[i];
	dataj.t2yj= data.t2y[i];
	dataj.t2zj= data.t2z[i];
	jc += 3;
	acx= dataj.t1xj* crnt.cur[jc-2]+ dataj.t1yj *
	  crnt.cur[jc-1]+ dataj.t1zj* crnt.cur[jc];
	bcx= dataj.t2xj* crnt.cur[jc-2]+ dataj.t2yj *
	  crnt.cur[jc-1]+ dataj.t2zj* crnt.cur[jc];

	for( ipa = 0; ipa < gnd.ksymp; ipa++ )
	{
	  dataj.ipgnd= ipa+1;
	  unere( xob, yob, zob);
	  *ex= *ex+ acx* dataj.exk+ bcx* dataj.exs;
	  *ey= *ey+ acx* dataj.eyk+ bcx* dataj.eys;
	  *ez= *ez+ acx* dataj.ezk+ bcx* dataj.ezs;
	}

  } /* for( i = 0; i < m; i++ ) */

  return;
}

/*-----------------------------------------------------------------------*/

/* compute near e or h fields over a range of points */
void nfpat( int nfeh )
{
  int i, j, kk, idx;
  long double znrt, cth=0.0l, sth=0.0l, ynrt, cph=0.0l, sph=0.0l, yob;
  long double xnrt, xob,zob, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
  complex long double ex, ey, ez;
  double r; /* Distance of field point from xyz origin */

  Alloc_Nearfield_Buffers( fpat.nrx, fpat.nry, fpat.nrz );

  /* Initialize according to E/H flag */
  if( nfeh == 1 )
	near_field.max_hr = 0.0l;
  else
	near_field.max_er = 0.0l;
  near_field.r_max = 0.0l;

  idx = 0;
  znrt= fpat.znr- fpat.dznr;
  for( i = 0; i < fpat.nrz; i++ )
  {
	znrt += fpat.dznr;
	if( fpat.near != 0)
	{
	  cth= cosl( TA* znrt);
	  sth= sinl( TA* znrt);
	}

	ynrt= fpat.ynr- fpat.dynr;
	for( j = 0; j < fpat.nry; j++ )
	{
	  ynrt += fpat.dynr;
	  if( fpat.near != 0)
	  {
		cph= cosl( TA* ynrt);
		sph= sinl( TA* ynrt);
	  }

	  xnrt= fpat.xnr- fpat.dxnr;
	  for( kk = 0; kk < fpat.nrx; kk++ )
	  {
		xnrt += fpat.dxnr;
		if( fpat.near != 0)
		{
		  xob= xnrt* sth* cph;
		  yob= xnrt* sth* sph;
		  zob= xnrt* cth;
		}
		else
		{
		  xob= xnrt;
		  yob= ynrt;
		  zob= znrt;
		}

		tmp1= xob/ data.wlam;
		tmp2= yob/ data.wlam;
		tmp3= zob/ data.wlam;

		if( nfeh == 1 ) /* Magnetic field */
		  nhfld( tmp1, tmp2, tmp3, &ex, &ey, &ez);
		else /* Electric field */
		  nefld( tmp1, tmp2, tmp3, &ex, &ey, &ez);

		/* Calculate total field vector */
		Near_Field_Total( ex, ey, ez, nfeh, idx );

		/* Save field point co-ordinates */
		near_field.px[idx] = (double)xob;
		near_field.py[idx] = (double)yob;
		near_field.pz[idx] = (double)zob;

		/* Find max distance from xyz origin */
		r = sqrt(
			near_field.px[idx] * near_field.px[idx] +
			near_field.py[idx] * near_field.py[idx] +
			near_field.pz[idx] * near_field.pz[idx] );
		if( near_field.r_max < r )
		  near_field.r_max = r;

		tmp1= cabsl(ex);
		tmp2= cang (ex);
		tmp3= cabsl(ey);
		tmp4= cang (ey);
		tmp5= cabsl(ez);
		tmp6= cang (ez);

		if( nfeh == 1 ) /* Magnetic field */
		{
		  near_field.hx[idx]  = (double)tmp1;
		  near_field.hy[idx]  = (double)tmp3;
		  near_field.hz[idx]  = (double)tmp5;
		  near_field.fhx[idx] = (double)(tmp2 * TA);
		  near_field.fhy[idx] = (double)(tmp4 * TA);
		  near_field.fhz[idx] = (double)(tmp6 * TA);
		}
		else /* Electric field */
		{
		  near_field.ex[idx]  = (double)tmp1;
		  near_field.ey[idx]  = (double)tmp3;
		  near_field.ez[idx]  = (double)tmp5;
		  near_field.fex[idx] = (double)(tmp2 * TA);
		  near_field.fey[idx] = (double)(tmp4 * TA);
		  near_field.fez[idx] = (double)(tmp6 * TA);
		}

		idx++;

	  } /* for( kk = 0; kk < fpat.nrx; kk++ ) */

	} /* for( j = 0; j < fpat.nry; j++ ) */

  } /* for( i = 0; i < fpat.nrz; i++ ) */

  /* Signal new valid near field data */
  near_field.newer = near_field.valid = 1;

  /* Signal new E/H pattern data */
  SetFlag( DRAW_NEW_EHFIELD );

  return;
}

/*-----------------------------------------------------------------------*/

/* nhfld computes the near field at specified points in space after */
/* the structure currents have been computed. */

void nhfld( long double xob, long double yob,
	long double zob, complex long double *hx,
	complex long double *hy, complex long double *hz )
{
  int i, jc;
  long double ax, zp;
  complex long double acx, bcx, ccx;

  *hx=CPLX_00;
  *hy=CPLX_00;
  *hz=CPLX_00;
  ax=0.0l;

  if( data.n != 0)
  {
	for( i = 0; i < data.n; i++ )
	{
	  dataj.xj= xob- data.x[i];
	  dataj.yj= yob- data.y[i];
	  dataj.zj= zob- data.z[i];
	  zp= data.cab[i]* dataj.xj+ data.sab[i] *
		dataj.yj+ data.salp[i]* dataj.zj;

	  if( fabsl( zp) > 0.5001l* data.si[i])
		continue;

	  zp= dataj.xj* dataj.xj+ dataj.yj* dataj.yj +
		dataj.zj* dataj.zj- zp* zp;
	  dataj.xj= data.bi[i];

	  if( zp > 0.9l* dataj.xj* dataj.xj)
		continue;

	  ax= dataj.xj;
	  break;
	}

	for( i = 0; i < data.n; i++ )
	{
	  dataj.s= data.si[i];
	  dataj.b= data.bi[i];
	  dataj.xj= data.x[i];
	  dataj.yj= data.y[i];
	  dataj.zj= data.z[i];
	  dataj.cabj= data.cab[i];
	  dataj.sabj= data.sab[i];
	  dataj.salpj= data.salp[i];
	  hsfld( xob, yob, zob, ax);
	  acx= cmplx( crnt.air[i], crnt.aii[i]);
	  bcx= cmplx( crnt.bir[i], crnt.bii[i]);
	  ccx= cmplx( crnt.cir[i], crnt.cii[i]);
	  *hx += dataj.exk* acx+ dataj.exs* bcx+ dataj.exc* ccx;
	  *hy += dataj.eyk* acx+ dataj.eys* bcx+ dataj.eyc* ccx;
	  *hz += dataj.ezk* acx+ dataj.ezs* bcx+ dataj.ezc* ccx;
	}

	if( data.m == 0)
	  return;

  } /* if( data.n != 0) */

  jc= data.n-1;
  for( i = 0; i < data.m; i++ )
  {
	dataj.s= data.pbi[i];
	dataj.xj= data.px[i];
	dataj.yj= data.py[i];
	dataj.zj= data.pz[i];
	dataj.t1xj= data.t1x[i];
	dataj.t1yj= data.t1y[i];
	dataj.t1zj= data.t1z[i];
	dataj.t2xj= data.t2x[i];
	dataj.t2yj= data.t2y[i];
	dataj.t2zj= data.t2z[i];
	hintg( xob, yob, zob);
	jc += 3;
	acx= dataj.t1xj* crnt.cur[jc-2]+ dataj.t1yj *
	  crnt.cur[jc-1]+ dataj.t1zj* crnt.cur[jc];
	bcx= dataj.t2xj* crnt.cur[jc-2]+ dataj.t2yj *
	  crnt.cur[jc-1]+ dataj.t2zj* crnt.cur[jc];
	*hx= *hx+ acx* dataj.exk+ bcx* dataj.exs;
	*hy= *hy+ acx* dataj.eyk+ bcx* dataj.eys;
	*hz= *hz+ acx* dataj.ezk+ bcx* dataj.ezs;
  }

  return;
}

/*-----------------------------------------------------------------------*/

/* integrate over patches at wire connection point */
void pcint( long double xi, long double yi, long double zi,
	long double cabi, long double sabi, long double salpi,
	complex long double *e )
{
  int nint, i1, i2;
  long double d, ds, da, gcon, fcon, xxj, xyj, xzj, xs, s1;
  long double xss, yss, zss, s2x, s2, g1, g2, g3, g4, f2, f1;
  complex long double e1, e2, e3, e4, e5, e6, e7, e8, e9;

  nint = 10;
  d= sqrtl( dataj.s)*.5l;
  ds=4.0l* d/ (long double) nint;
  da= ds* ds;
  gcon=1.0l/ dataj.s;
  fcon=1.0l/(2.0l* TP* d);
  xxj= dataj.xj;
  xyj= dataj.yj;
  xzj= dataj.zj;
  xs= dataj.s;
  dataj.s= da;
  s1= d+ ds*.5l;
  xss= dataj.xj+ s1*( dataj.t1xj+ dataj.t2xj);
  yss= dataj.yj+ s1*( dataj.t1yj+ dataj.t2yj);
  zss= dataj.zj+ s1*( dataj.t1zj+ dataj.t2zj);
  s1= s1+ d;
  s2x= s1;
  e1=CPLX_00;
  e2=CPLX_00;
  e3=CPLX_00;
  e4=CPLX_00;
  e5=CPLX_00;
  e6=CPLX_00;
  e7=CPLX_00;
  e8=CPLX_00;
  e9=CPLX_00;

  for( i1 = 0; i1 < nint; i1++ )
  {
	s1= s1- ds;
	s2= s2x;
	xss= xss- ds* dataj.t1xj;
	yss= yss- ds* dataj.t1yj;
	zss= zss- ds* dataj.t1zj;
	dataj.xj= xss;
	dataj.yj= yss;
	dataj.zj= zss;

	for( i2 = 0; i2 < nint; i2++ )
	{
	  s2= s2- ds;
	  dataj.xj= dataj.xj- ds* dataj.t2xj;
	  dataj.yj= dataj.yj- ds* dataj.t2yj;
	  dataj.zj= dataj.zj- ds* dataj.t2zj;
	  unere( xi, yi, zi);
	  dataj.exk= dataj.exk* cabi+ dataj.eyk *
		sabi+ dataj.ezk* salpi;
	  dataj.exs= dataj.exs* cabi+ dataj.eys *
		sabi+ dataj.ezs* salpi;
	  g1=( d+ s1)*( d+ s2)* gcon;
	  g2=( d- s1)*( d+ s2)* gcon;
	  g3=( d- s1)*( d- s2)* gcon;
	  g4=( d+ s1)*( d- s2)* gcon;
	  f2=( s1* s1+ s2* s2)* TP;
	  f1= s1/ f2-( g1- g2- g3+ g4)* fcon;
	  f2= s2/ f2-( g1+ g2- g3- g4)* fcon;
	  e1= e1+ dataj.exk* g1;
	  e2= e2+ dataj.exk* g2;
	  e3= e3+ dataj.exk* g3;
	  e4= e4+ dataj.exk* g4;
	  e5= e5+ dataj.exs* g1;
	  e6= e6+ dataj.exs* g2;
	  e7= e7+ dataj.exs* g3;
	  e8= e8+ dataj.exs* g4;
	  e9= e9+ dataj.exk* f1+ dataj.exs* f2;

	} /* for( i2 = 0; i2 < nint; i2++ ) */

  } /* for( i1 = 0; i1 < nint; i1++ ) */

  e[0]= e1;
  e[1]= e2;
  e[2]= e3;
  e[3]= e4;
  e[4]= e5;
  e[5]= e6;
  e[6]= e7;
  e[7]= e8;
  e[8]= e9;
  dataj.xj= xxj;
  dataj.yj= xyj;
  dataj.zj= xzj;
  dataj.s= xs;

  return;
}

/*-----------------------------------------------------------------------*/

/* calculates the electric field due to unit current */
/* in the t1 and t2 directions on a patch */
void unere( long double xob, long double yob, long double zob )
{
  long double zr, t1zr, t2zr, rx, ry, rz, r, tt1;
  long double tt2, rt, xymag, px, py, cth, r2;
  complex long double er, q1, q2, rrv, rrh, edp;

  zr= dataj.zj;
  t1zr= dataj.t1zj;
  t2zr= dataj.t2zj;

  if( dataj.ipgnd == 2)
  {
	zr= -zr;
	t1zr= -t1zr;
	t2zr= -t2zr;
  }

  rx= xob- dataj.xj;
  ry= yob- dataj.yj;
  rz= zob- zr;
  r2= rx* rx+ ry* ry+ rz* rz;

  if( r2 <= 1.0e-20l)
  {
	dataj.exk=CPLX_00;
	dataj.eyk=CPLX_00;
	dataj.ezk=CPLX_00;
	dataj.exs=CPLX_00;
	dataj.eys=CPLX_00;
	dataj.ezs=CPLX_00;
	return;
  }

  r= sqrtl( r2);
  tt1= -TP* r;
  tt2= tt1* tt1;
  rt= r2* r;
  er= cmplx( sinl( tt1),- cosl( tt1))*( CONST2* dataj.s);
  q1= cmplx( tt2-1.0l, tt1)* er/ rt;
  q2= cmplx(3.0l- tt2,-3.0l* tt1)* er/( rt* r2);
  er = q2*( dataj.t1xj* rx+ dataj.t1yj* ry+ t1zr* rz);
  dataj.exk= q1* dataj.t1xj+ er* rx;
  dataj.eyk= q1* dataj.t1yj+ er* ry;
  dataj.ezk= q1* t1zr+ er* rz;
  er= q2*( dataj.t2xj* rx+ dataj.t2yj* ry+ t2zr* rz);
  dataj.exs= q1* dataj.t2xj+ er* rx;
  dataj.eys= q1* dataj.t2yj+ er* ry;
  dataj.ezs= q1* t2zr+ er* rz;

  if( dataj.ipgnd == 1)
	return;

  if( gnd.iperf == 1)
  {
	dataj.exk= -dataj.exk;
	dataj.eyk= -dataj.eyk;
	dataj.ezk= -dataj.ezk;
	dataj.exs= -dataj.exs;
	dataj.eys= -dataj.eys;
	dataj.ezs= -dataj.ezs;
	return;
  }

  xymag= sqrtl( rx* rx+ ry* ry);
  if( xymag <= 1.0e-6l)
  {
	px=0.0l;
	py=0.0l;
	cth=1.0l;
	rrv=CPLX_10;
  }
  else
  {
	px= -ry/ xymag;
	py= rx/ xymag;
	cth= rz/ sqrtl( xymag* xymag+ rz* rz);
	rrv= csqrtl(1.0l- gnd.zrati* gnd.zrati*(1.0l- cth* cth));
  }

  rrh= gnd.zrati* cth;
  rrh=( rrh- rrv)/( rrh+ rrv);
  rrv= gnd.zrati* rrv;
  rrv=-( cth- rrv)/( cth+ rrv);
  edp=( dataj.exk* px+ dataj.eyk* py)*( rrh- rrv);
  dataj.exk= dataj.exk* rrv+ edp* px;
  dataj.eyk= dataj.eyk* rrv+ edp* py;
  dataj.ezk= dataj.ezk* rrv;
  edp=( dataj.exs* px+ dataj.eys* py)*( rrh- rrv);
  dataj.exs= dataj.exs* rrv+ edp* px;
  dataj.eys= dataj.eys* rrv+ edp* py;
  dataj.ezs= dataj.ezs* rrv;

  return;
}

/*-----------------------------------------------------------------------*/

/* Near_Field_Total()
 *
 * Calculates the value of Total Near Field vector
 */
void
Near_Field_Total(
	complex long double ex,
	complex long double ey,
	complex long double ez,
	int nfeh, int idx )
{
  /* Display a time-frozen "snapshot" of near field */
  if( isFlagSet(NEAREH_SNAPSHOT) )
  {
	if( nfeh == 1 ) /* Magnetic field */
	{
	  /* Near magnetic field components */
	  near_field.hrx[idx] = (double)creall(ex);
	  near_field.hry[idx] = (double)creall(ey);
	  near_field.hrz[idx] = (double)creall(ez);

	  /* Near total magnetic field vector*/
	  near_field.hr[idx]  = sqrt(
		  near_field.hrx[idx] * near_field.hrx[idx] +
		  near_field.hry[idx] * near_field.hry[idx] +
		  near_field.hrz[idx] * near_field.hrz[idx] );
	  if( near_field.max_hr < near_field.hr[idx] )
		near_field.max_hr = near_field.hr[idx];
	}
	else /* Electric field */
	{
	  /* Near electric field components */
	  /* Near electric field components */
	  near_field.erx[idx] = (double)creall(ex);
	  near_field.ery[idx] = (double)creall(ey);
	  near_field.erz[idx] = (double)creall(ez);

	  /* Near total electric field vector */
	  near_field.er[idx]  = sqrt(
		  near_field.erx[idx] * near_field.erx[idx] +
		  near_field.ery[idx] * near_field.ery[idx] +
		  near_field.erz[idx] * near_field.erz[idx] );
	  if( near_field.max_er < near_field.er[idx] )
		near_field.max_er = near_field.er[idx];
	} /* if( nfeh == 1 ) */

  } /* if( isFlagSet(NEAREH_SNAPSHOT) ) */
  else /* Display Total near field vector peak */
  {
	double
	  exm, eym, ezm,	/* Near field magnitude in x, y, z    */
	  exm2, eym2, ezm2,	/* Near field magnitude^2 in x, y, z  */
	  fx, fy, fz,		/* Time phase of near field vectors   */
	  fx2, fy2, fz2,	/* Time phase of near field vectors*2 */
	  cp, sp, tp, wt;	/* Some values needed in calculations */

	exm = (double)cabsl(ex);
	eym = (double)cabsl(ey);
	ezm = (double)cabsl(ez);
	/* Near total electric field vector */
	fx  = (double)cang(ex)/TD;
	fy  = (double)cang(ey)/TD;
	fz  = (double)cang(ez)/TD;

	fx2 = fx * 2.0;
	fy2 = fy * 2.0;
	fz2 = fz * 2.0;

	exm2 = exm*exm;
	eym2 = eym*eym;
	ezm2 = ezm*ezm;

	cp = exm2*cos(fx2) + eym2*cos(fy2) + ezm2*cos(fz2);
	sp = exm2*sin(fx2) + eym2*sin(fy2) + ezm2*sin(fz2);
	tp = sqrt(cp*cp + sp*sp);
	wt = atan2(-sp, cp)/2.0;

	if( nfeh == 1 ) /* Magnetic field */
	{
	  /* Near magnetic field components */
	  near_field.hrx[idx] = exm * cos(wt + fx);
	  near_field.hry[idx] = eym * cos(wt + fy);
	  near_field.hrz[idx] = ezm * cos(wt + fz);

	  /* Near total magnetic field vector, peak value */
	  near_field.hr[idx]  = sqrt( (exm2 + eym2 + ezm2 + tp)/2.0 );
	  if( near_field.max_hr < near_field.hr[idx] )
		near_field.max_hr = near_field.hr[idx];
	}
	else /* Electric field */
	{
	  /* Near electric field components */
	  near_field.erx[idx] = exm * cos(wt + fx);
	  near_field.ery[idx] = eym * cos(wt + fy);
	  near_field.erz[idx] = ezm * cos(wt + fz);

	  /* Near total electric field vector, peak value */
	  near_field.er[idx]  = sqrt( (exm2 + eym2 + ezm2 + tp)/2.0 );
	  if( near_field.max_er < near_field.er[idx] )
		near_field.max_er = near_field.er[idx];
	}
  }

} /* Near_Field_Total() */

/*-----------------------------------------------------------------------*/

